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

#ifndef PARSERS_H
#define PARSERS_H

#include "PluginDefinition.h"
#include "Utils.h"
#include "trex.h"
#include <map>
#include <vector>

extern const char *default_function_format;
extern const char *default_internal_function_format;
extern const char *default_file_format;

typedef std::map<std::string, std::vector<std::string>> Keywords;

// These are the settable fields in the parser
// This makes it easier when changing settings in the dialog box
typedef struct ParserDefinition
{
	std::string doc_start;
	std::string doc_line;
	std::string doc_end;
	std::string command_prefix;
	std::string function_format;
	std::string file_format;
	bool align;
} ParserDefinition;

typedef struct Parser
{
	int lang_type;
	std::wstring lang;					// Short language name. Postfix of enum from lang_type
	std::wstring language_name;			// User readable name
	std::string example;				// Example function/method to parse for Settings Dialog
	bool external;						// whether this is an internal or external(UDL||external lexer)

	ParserDefinition pd;

	// Registered functions
	bool (*initializer)(void);
	void (*cleanup)(void);
	Keywords (*parse)(const ParserDefinition *pd, const char *text);

	// Constructor...with 11 parameters
	// Don't judge me
	Parser(int lt, std::wstring l, std::wstring ln, std::string e, std::string ds, std::string dl, std::string de,
		std::string cp, bool (*i)(void), void (*c)(void), Keywords (*p)(const ParserDefinition *, const char *))
	{
		lang_type = lt;
		lang = l;
		language_name = ln;
		example = e;
		external = false;
		pd.doc_start = ds;
		pd.doc_line = dl;
		pd.doc_end = de;
		pd.command_prefix = cp;
		pd.function_format = default_function_format;
		pd.file_format = default_file_format;
		pd.align = false;
		initializer = i;
		cleanup = c;
		parse = p;
	}

	Parser() {}
} Parser;

extern std::vector<Parser *> parsers;


// Macro to help define the functions of each parser
#define DEFINE_PARSER(lang) \
	bool Initialize_##lang(void); \
	void CleanUp_##lang(void); \
	Keywords Parse_##lang(const ParserDefinition *pd, const char *text);

DEFINE_PARSER(C);
DEFINE_PARSER(Python);
DEFINE_PARSER(Null);



const Parser *getParserByName(std::wstring name);
const Parser *getCurrentParser(bool update=false);
const ParserDefinition *getCurrentParserDefinition();
void addNewParser(std::string name, ParserDefinition *pd=NULL);

void InitializeParsers();
void CleanUpParsers();

std::string FormatFunctionBlock(const Parser *p,const ParserDefinition *pd, const char *text);
std::string FormatFileBlock(const ParserDefinition *pd);
std::string Parse();

#endif
