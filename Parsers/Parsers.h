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

#ifndef PARSERS_H
#define PARSERS_H

#include "PluginDefinition.h"
#include "Utils.h"
#include "trex.h"

// These are the settable fields in the parser
// This makes it easier when changing settings in the dialog box
typedef struct ParserDefinition
{
	std::string doc_start;
	std::string doc_line;
	std::string doc_end;
	std::string command_prefix;
} ParserDefinition;

typedef struct Parser
{
	int lang_type;
	std::wstring lang;

	ParserDefinition pd;

	const std::string example;

	// Store default values. For convenience, these are wstring to load and save easier
	const std::wstring default_doc_start;
	const std::wstring default_doc_line;
	const std::wstring default_doc_end;
	const std::wstring default_command_prefix;

	// Registered functions
	bool (*initializer)(void);
	void (*cleanup)(void);
	std::string (*callback)(const ParserDefinition *pd, const char *text);
} Parser;

extern Parser parsers[6];


// Macro to help define the functions of each parser
#define DEFINE_PARSER(lang) \
	bool Initialize_##lang(void); \
	void CleanUp_##lang(void); \
	std::string Parse_##lang(const ParserDefinition *pd, const char *text);

DEFINE_PARSER(C);
DEFINE_PARSER(CPP);
DEFINE_PARSER(JAVA);
DEFINE_PARSER(PYTHON);
DEFINE_PARSER(Null);

const Parser *getParserByName(std::wstring name);
const Parser *getCurrentParser();
const ParserDefinition *getCurrentParserDefinition();

void InitializeParsers();
void CleanUpParsers();
std::string Parse();

#endif
