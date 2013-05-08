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

#include <vector>
#include "Parsers.h"

static TRex *tr_function;
static TRex *tr_parameters;

bool Initialize_C(void)
{
	const TRexChar *error = NULL;
	tr_function = trex_compile("(:?([\\w:]+)[*&]*\\s+(?:[*&]*\\s+)?[*&]*)?([\\w:]+)\\s*(\\([^)]*\\))", &error);
	tr_parameters = trex_compile("(\\$?\\w+|\\.\\.\\.)(\\s*=\\s*[\\\"\\w\\.]+)?\\s*[,)]", &error);

	if(!tr_function || !tr_parameters) return false;
	return true;
}

void CleanUp_C(void)
{
	trex_free(tr_function);
	trex_free(tr_parameters);
}

std::string Parse_C(const ParserDefinition *pd, const char *text)
{
	std::ostringstream doc_block;
	std::vector<std::string> params;
	const TRexChar *begin,*end;
	const char *eol;
	unsigned int max = 0;

	eol = getEolStr();

	if(trex_search(tr_function, text, &begin, &end))
	{
		TRexMatch params_match;
		const TRexChar *cur_params;
		const TRexChar *p_begin, *p_end;

		//trex_getsubexp(tr_function, 1, &return_match);	// not used for now
		//trex_getsubexp(tr_function, 2, &func_match);		// not used for now
		trex_getsubexp(tr_function, 3, &params_match);

		doc_block << pd->doc_start << eol;
		doc_block << pd->doc_line << pd->command_prefix << "brief " << FT("Brief") << eol;
		doc_block << pd->doc_line << eol;

		// For each param
		cur_params = params_match.begin;
		while(trex_searchrange(tr_parameters, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(tr_parameters, 1, &param_match);

			// handle "func(void)" by skipping it
			if(strncmp(param_match.begin, "void", 4) != 0)
			{
				std::string param;
				param.append(param_match.begin, param_match.len);
				params.push_back(param);
				if(param.length() > max) max = param.length();
			}
			cur_params = p_end;
		}

		for(unsigned int i = 0; i < params.size(); ++i)
		{
			std::string param = params[i];

			doc_block << pd->doc_line << pd->command_prefix << "param [in] " << param;
			if(pd->align_desc) doc_block << std::string(max - param.length(), ' ');
			doc_block << " " << FT("Parameter_Description") << eol;
		}

		// Return value
		doc_block << pd->doc_line << pd->command_prefix << "return ";
		doc_block << FT("Return_Description") << eol;
		doc_block << pd->doc_line << eol;

		doc_block << pd->doc_line << pd->command_prefix << "details " << FT("Details") << eol;
		doc_block << pd->doc_end;
	}

	return doc_block.str();
}
