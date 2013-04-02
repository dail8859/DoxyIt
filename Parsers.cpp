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

extern NppData nppData;

#define REGISTER_PARSER(lang, doc_start, doc_line, doc_end, command_prefix, example) {L_##lang, TEXT(#lang), \
	{"", "", "", ""}, \
	example, TEXT(##doc_start), TEXT(##doc_line), TEXT(##doc_end), TEXT(##command_prefix), \
	Initialize_##lang, CleanUp_##lang, Parse_##lang}

Parser parsers[] = 
{
	REGISTER_PARSER(C,      "/**", " *  ", " */", "\\", "int function(const char *p, int index)"),
	REGISTER_PARSER(CPP,    "/**", " *  ", " */", "\\", "std::string function(const char *p, int &index)"),
	REGISTER_PARSER(JAVA,   "/**", " *  ", " */", "@",  "public boolean action(Event event, Object arg)"),
	REGISTER_PARSER(PYTHON, "## ", "#  ",  "#  ", "@",  "def foo(bar, baz=none)")
	//REGISTER_PARSER(CS),
	//REGISTER_PARSER(PHP),
};

const Parser *getCurrentParser(void)
{
	int lang_type;
	int len = sizeof(parsers) / sizeof(parsers[0]);
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &lang_type);

	for(int i = 0; i < len; ++i)
		if(parsers[i].lang_type == lang_type)
			return &parsers[i];

	return NULL;
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
	int found;
	char *buffer;
	
	p = getCurrentParser();
	if(!p)
	{
		::MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return "";
	}

	// Get the text until a closing parenthesis, possibly make this settable for each parser
	found = findNext(")");
	if(found == -1) return "";

	buffer = getRange(SendScintilla(SCI_GETCURRENTPOS), found + 1);
	doc_block = p->callback(&p->pd, buffer);

	delete[] buffer;

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
