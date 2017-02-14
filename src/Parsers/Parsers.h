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

// Default formats
extern const char *default_function_format;
extern const char *default_internal_function_format;
extern const char *default_file_format;

typedef struct {
	std::string function;
	std::vector<std::string> parameters;
	std::map<std::string, std::string> extras;
} Keywords;

// These are the settable fields in the parser
// This makes it easier when changing settings in the dialog box
typedef struct ParserSettings final
{
	std::string doc_start;
	std::string doc_line;
	std::string doc_end;
	std::string command_prefix;
	std::string function_format;
	std::string file_format;
	bool align;
} ParserSettings;

typedef Keywords(*ParserStrategy)(const ParserSettings *ps, const char *text);

class Parser final
{
public:
	int lang_type;
	std::wstring lang;           // Short language name. Postfix of enum from lang_type
	std::wstring language_name;  // User readable name
	std::string example;         // Example function/method to parse for Settings Dialog
	bool external;               // whether this is an internal or external(UDL||external lexer)

	ParserSettings ps;
	ParserStrategy strategy;

	// Don't judge me
	Parser(ParserStrategy st, int lt, std::wstring l, std::wstring ln, std::string e, std::string ds, std::string dl, std::string de, std::string cp)
	{
		strategy = st; 
		lang_type = lt;
		lang = l;
		language_name = ln;
		example = e;
		external = false;
		ps.doc_start = ds;
		ps.doc_line = dl;
		ps.doc_end = de;
		ps.command_prefix = cp;
		ps.function_format = default_function_format;
		ps.file_format = default_file_format;
		ps.align = false;
	}

	Parser() {}
};

extern std::vector<Parser> parsers;

const Parser *getParserByName(std::wstring name);
const Parser *getCurrentParser(bool update=false);
const ParserSettings *getCurrentParserSettings();
void addNewParser(std::string name, ParserSettings *ps = nullptr);

void InitializeParsers();
void CleanUpParsers();

std::string FormatFunctionBlock(const Parser *p, const ParserSettings *ps, const char *text);
std::string FormatFileBlock(const ParserSettings *ps);
std::string Parse();

// Simple class to make sure the regexs get cleaned up
class TrRegex final {
public:
	TRex *regex = nullptr;

	explicit TrRegex(const char *pattern) {
		const TRexChar *error = NULL;
		regex = trex_compile(pattern, &error);
	}

	~TrRegex() {
		trex_free(regex);
	}
};

Keywords parse_c(const ParserSettings *ps, const char *text);
Keywords parse_python(const ParserSettings *ps, const char *text);
Keywords parse_null(const ParserSettings *ps, const char *text);
#endif
