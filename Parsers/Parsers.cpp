//This file is part of DoxyIt.
//
//Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
//
//DoxyIt is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Parsers.h"

// Very ugly macro
#define REGISTER_PARSER(lang, parser, language_name, doc_start, doc_line, doc_end, command_prefix, example) \
	{L_##lang, TEXT(#lang),TEXT(language_name), example, \
	{"", "", "", "", false}, \
	{doc_start, doc_line, doc_end, command_prefix, false}, \
	Initialize_##parser, CleanUp_##parser, Parse_##parser}

Parser parsers[] = 
{
	REGISTER_PARSER(C,      C,      "C",          "/**",  " *  ", " */",  "\\", "int function(const char *ptr, int index)"),
	REGISTER_PARSER(CPP,    C,      "C++",        "/**",  " *  ", " */",  "\\", "std::string function(const char *ptr, int &index)"),
	REGISTER_PARSER(JAVA,   C,      "Java",       "/**",  " *  ", " */",  "@",  "public boolean action(Event event, Object arg)"),
	REGISTER_PARSER(PYTHON, Python, "Python",     "## ",  "#  ",  "#  ",  "@",  "def foo(bar, string=None)"),
	REGISTER_PARSER(PHP,    C,      "PHP",        "/**",  " *  ", " */",  "@",  "function myFunction($abc, $defg)"),
	REGISTER_PARSER(JS,     C,      "JavaScript", "/**",  " *  ", " */",  "@",  "function myFunction(abc, defg)"),
	REGISTER_PARSER(CS,     C,      "C#",         "/// ", "/// ", "/// ", "\\", "public int Method(ref int abc, int defg)")
};

const Parser *getParserByName(std::wstring name)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		if(parsers[i].language_name == name)
			return &parsers[i];

	return NULL;
}

// "update" causes the current parser pointer to be updated
// else just return the currently cached parser
const Parser *getCurrentParser(bool update)
{
	static const Parser *current = NULL;

	if(update)
	{
		int lang_type;
		int len = sizeof(parsers) / sizeof(parsers[0]);
		SendNpp(NPPM_GETCURRENTLANGTYPE, SCI_UNUSED, (LPARAM) &lang_type);

		for(int i = 0; i < len; ++i)
		{
			if(parsers[i].lang_type == lang_type)
			{
				current = &parsers[i];
				return current;
			}
		}

		// Parser wasn't found for current language
		current = NULL;
	}

	return current;
}

const ParserDefinition *getCurrentParserDefinition(void)
{
	const Parser *p;

	p = getCurrentParser();
	if(p) return &p->pd;
	else return NULL;
}

std::string Parse(void)
{
	std::string doc_block;
	const Parser *p;
	int found, curLine, foundLine;
	char *buffer;

	if(!(p = getCurrentParser()))
	{
		::MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Get the text until a closing parenthesis. Find '('
	if((found = findNext("(")) == -1)
	{
		::MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Do some sanity checking. Make sure curline <= found <= curline+2
	curLine = SendScintilla(SCI_LINEFROMPOSITION, SendScintilla(SCI_GETCURRENTPOS));
	foundLine = SendScintilla(SCI_LINEFROMPOSITION, found);
	if(foundLine < curLine || foundLine > curLine + 2)
	{
		::MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Find the matching closing brace
	if((found = SendScintilla(SCI_BRACEMATCH, found, SCI_UNUSED)) == -1)
	{
		::MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	buffer = getRange(SendScintilla(SCI_GETCURRENTPOS), found + 1);
	doc_block = p->callback(&p->pd, buffer);
	delete[] buffer;

	// I don't think there is currently a case where callback() will return a zero length string,
	// but check it just in case we decide to for the future.
	if(doc_block.length() == 0)
	{
		::MessageBox(NULL, TEXT("Error: Cannot parse function definition"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	return doc_block;
}

void InitializeParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		if((*parsers[i].initializer)() == false)
			::MessageBox(NULL, TEXT("DoxyIt initialization failed"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
}

void CleanUpParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		(*parsers[i].cleanup)();
}
