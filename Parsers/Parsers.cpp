// This file is part of DoxyIt.
// 
// Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
// 
// DoxyIt is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Parsers.h"
#include <vector>

const char *default_function_format = 
	"\r\n"
	"$@brief Brief\r\n"
	"\r\n"
	"$@param [in] $PARAM $|Parameter_Description\r\n"
	"$@return Return_Description\r\n"
	"\r\n"
	"$@details Details\r\n"
	;

const char *default_internal_function_format = 
	"\r\n"
	"$@brief Brief\r\n"
	"\r\n"
	"$@param [in] theParam Parameter_Description\r\n"
	"$@return Return_Description\r\n"
	"\r\n"
	"$@details Details\r\n"
	;

const char *default_file_format = 
	"\r\n"
	"$@file $FILENAME\r\n"
	"$@brief Brief\r\n"
	;



const Parser *getParserByName(std::wstring name)
{
	for(auto const &p : parsers)
		if(p.language_name == name)
			return &p;

	return NULL;
}

// 'update' causes the current parser pointer to be updated
// else just return the currently cached parser
const Parser *getCurrentParser(bool update)
{
	static const Parser *current = NULL;

	if(update)
	{
		int lang_type;
		SendNpp(NPPM_GETCURRENTLANGTYPE, SCI_UNUSED, (LPARAM) &lang_type);

		if(lang_type == L_USER /* || lang_type == L_EXTERNAL */)
		{
			wchar_t *name = NULL;
			int len = 0;

			len = SendNpp(NPPM_GETLANGUAGENAME, lang_type, NULL) + 1;
			name = (wchar_t *) malloc(sizeof(wchar_t) * len);
			SendNpp(NPPM_GETLANGUAGENAME, lang_type, (LPARAM) name);

			for(auto const &p : parsers)
			{
				// Use [6] because the name returned is "udl - mylangname"
				if(p.language_name == &name[6])
				{
					current = &p;
					free(name);
					return current;
				}
			}

			free(name);
		}
		else
		{
			for(auto const &p : parsers)
			{
				if(p.lang_type == lang_type)
				{
					current = &p;
					return current;
				}
			}
		}

		// Parser wasn't found for current language
		current = nullptr;
	}

	return current;
}

const ParserSettings *getCurrentParserSettings(void)
{
	const Parser *p = getCurrentParser();
	return (p ? &p->ps : nullptr);
}

void addNewParser(std::string name, ParserSettings *ps)
{
	parsers.emplace_back();

	Parser *p = &parsers[parsers.size() - 1];

	p->lang_type = L_USER;
	p->lang = toWideString(name);
	p->language_name = toWideString(name);
	p->external = true;

	if(ps == nullptr)
	{
		// Fill in default values
		p->ps.doc_start = "/**";
		p->ps.doc_line  = " *  ";
		p->ps.doc_end   = " */";
		p->ps.command_prefix = "\\";
		p->ps.file_format = default_file_format;
		p->ps.function_format = default_internal_function_format;
		p->ps.align = false; // not used
	}
	else
	{
		p->ps = *ps;
	}
}

// Very ugly macro
#define REGISTER_PARSER(strategy, lang, language_name, doc_start, doc_line, doc_end, command_prefix, example) \
	strategy, L_##lang, TEXT(#lang), TEXT(language_name), example, doc_start, doc_line, doc_end, command_prefix

std::vector<Parser> parsers;
void InitializeParsers(void)
{
	parsers.reserve(8);
	parsers.emplace_back(REGISTER_PARSER(parse_c, C, "C", "/**", " *  ", " */", "\\", "int function(const char *ptr, int index)"));
	parsers.emplace_back(REGISTER_PARSER(parse_c, CPP, "C++", "/**", " *  ", " */", "\\", "std::string function(const char *ptr, int &index)"));
	parsers.emplace_back(REGISTER_PARSER(parse_c, JAVA, "Java", "/**", " *  ", " */", "@", "public boolean action(Event event, Object arg)"));
	parsers.emplace_back(REGISTER_PARSER(parse_python, PYTHON, "Python", "## ", "#  ", "#  ", "@", "def foo(bar, string=None)"));
	parsers.emplace_back(REGISTER_PARSER(parse_c, PHP, "PHP", "/**", " *  ", " */", "@", "function myFunction($abc, $defg)"));
	parsers.emplace_back(REGISTER_PARSER(parse_c, JS, "JavaScript", "/**", " *  ", " */", "@", "function myFunction(abc, defg)"));
	parsers.emplace_back(REGISTER_PARSER(parse_c, JAVASCRIPT, "JavaScript", "/**", " *  ", " */", "@", "function myFunction(abc, defg)"));
	parsers.emplace_back(REGISTER_PARSER(parse_c, CS, "C#", "/// ", "/// ", "/// ", "\\", "public int Method(ref int abc, int defg)"));
}

void CleanUpParsers(void)
{
}

void alignLines(std::vector<std::string> &lines)
{
	const char *flag = "$|";
	unsigned int align_max = 0;

	// Find the max position the flag is found at
	for(unsigned int i = 0; i < lines.size(); ++i)
	{
		unsigned int pos = lines[i].find(flag);
		if(pos != std::string::npos) align_max = max(pos, align_max);
	}

	// Replace the flag with an appropriate number of spaces
	for(unsigned int i = 0; align_max != 0 && i < lines.size(); ++i)
	{
		unsigned int pos = lines[i].find(flag);
		lines[i].replace(pos, strlen(flag), align_max - pos, ' ');
	}
}

std::string FormatBlock(const ParserSettings *ps, Keywords& keywords, const std::string &format)
{
	std::stringstream ss;
	std::vector<std::string> lines;
	std::vector<std::string> params = keywords["$PARAM"];
	std::string format_copy(format);
	const char *eol = getEolStr();

	// Replace keywords
	stringReplace(format_copy, "$@", ps->command_prefix);
	stringReplace(format_copy, "$FILENAME", keywords["$FILENAME"][0]);
	
	// $FUNCTION may not exist
	if(keywords.find("$FUNCTION") != keywords.end())
		stringReplace(format_copy, "$FUNCTION", keywords["$FUNCTION"][0]);
	else
		stringReplace(format_copy, "$FUNCTION", "");

	lines = splitLines(format_copy, "\r\n");

	for(unsigned int i = 0; i < lines.size(); ++i)
	{
		if(lines[i].find("$PARAM") != std::string::npos)
		{
			// Duplicate the current line for each $PARAM
			std::vector<std::string> formatted_lines;
			for(unsigned int j = 0; j < params.size(); ++j)
			{
				// Make a copy of lines[i] before calling stringReplace
				std::string line = lines[i];
				stringReplace(line, "$PARAM", params[j]);
				formatted_lines.push_back(line);
			}

			// If the align flag is set, align the lines, else remove all "$|" flags
			if(ps->align)
				alignLines(formatted_lines);
			else
				for(unsigned int j = 0; j < formatted_lines.size(); ++j)
					stringReplace(formatted_lines[j], "$|", "");

			// Insert the lines
			for(unsigned int j = 0; j < formatted_lines.size(); ++j)
			{
				if(i == 0 && j == 0)
					ss << ps->doc_start << formatted_lines[j] << eol;
				else if(i == lines.size() - 1 && j == formatted_lines.size() - 1)
					ss << ps->doc_end << formatted_lines[j] << eol;
				else
					ss << ps->doc_line << formatted_lines[j] << eol;
			}
		}
		else
		{
			if(i == 0)
				ss << ps->doc_start << lines[i] << eol;
			else if(i == lines.size() -1)
				ss << ps->doc_end << lines[i];
			else
				ss << ps->doc_line << lines[i] << eol;
		}
	}

	return ss.str();
}

void FillExtraKeywords(Keywords &kw)
{
	std::vector<std::string> filename;
	wchar_t fileName[MAX_PATH];

	// Insert the current file name into the map
	SendNpp(NPPM_GETFILENAME, MAX_PATH, (LPARAM) fileName);
	filename.push_back(toString(fileName));
	kw["$FILENAME"] = filename;
}

std::string FormatFileBlock(const ParserSettings *ps)
{
	Keywords kw;

	FillExtraKeywords(kw);

	return FormatBlock(ps, kw, ps->file_format);
}

std::string FormatFunctionBlock(const Parser *p, const ParserSettings *ps, const char *text)
{
	Keywords kw;

	if(!p->external)
	{
		kw = p->strategy(ps, text);
		if(kw.size() == 0) return std::string("");
	}

	FillExtraKeywords(kw);

	return FormatBlock(ps, kw, ps->function_format);
}

// Get the current parser and text to parse
std::string Parse(void)
{
	std::string doc_block;
	const Parser *p;
	int found, curLine, foundLine;
	char *buffer;

	if(!(p = getCurrentParser()))
	{
		MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return std::string("");
	}

	// External parsers are simple enough since they don't need any text to parse
	if(p->external)
		return FormatFunctionBlock(p, &p->ps, nullptr); 

	// Get the text until a closing parenthesis. Find '(' first
	if((found = findNext("(")) == -1)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition. Make sure the cursor is on the line directly above the function or method definition."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return std::string("");
	}

	// Do some sanity checking. Make sure curline <= found <= curline+2
	curLine = SendNpp(NPPM_GETCURRENTLINE);
	foundLine = SendScintilla(SCI_LINEFROMPOSITION, found);
	if(foundLine < curLine || foundLine > curLine + 2)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition. Make sure the cursor is on the line directly above the function or method definition."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return std::string("");
	}

	// Find the matching closing brace
	if((found = SendScintilla(SCI_BRACEMATCH, found, 0)) == -1)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition. Make sure the cursor is on the line directly above the function or method definition."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return std::string("");
	}

	buffer = getRange(SendScintilla(SCI_GETCURRENTPOS), found + 1);
	doc_block = FormatFunctionBlock(p, &p->ps, buffer);
	delete[] buffer;

	// I don't think there is currently a case where callback() will return a zero length string,
	// but check it just in case we decide to for the future.
	if(doc_block.length() == 0)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition. Make sure the cursor is on the line directly above the function or method definition."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return std::string("");
	}

	return doc_block;
}
