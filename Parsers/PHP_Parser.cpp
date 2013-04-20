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

bool Initialize_PHP(void)
{
	return true;
}

void CleanUp_PHP(void)
{
}

std::string Parse_PHP(const ParserDefinition *pd, const char *text)
{
	std::ostringstream doc_block;
	const char *eol = getEolStr();

	doc_block << pd->doc_start << eol;
	doc_block << pd->doc_line << pd->command_prefix << "brief [Brief]" << eol;
	doc_block << pd->doc_line << eol;
	doc_block << pd->doc_line << pd->command_prefix << "return [Return Description]" << eol;
	doc_block << pd->doc_line << eol;
	doc_block << pd->doc_line << pd->command_prefix << "details [Details]" << eol;
	doc_block << pd->doc_end;

	return doc_block.str();
}