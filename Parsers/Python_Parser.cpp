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

static TRex *tr_function = NULL;
static TRex *tr_parameters = NULL;

bool Initialize_Python(void)
{
	if(tr_function == NULL && tr_parameters == NULL)
	{
		const TRexChar *error = NULL;
		tr_function = trex_compile("def\\s+(\\w+)\\s*(\\([^)]*\\))", &error);
		tr_parameters = trex_compile("(\\w+)(\\s*=\\s*\\w+)?\\s*[,)]", &error);

		if(!tr_function || !tr_parameters) return false;
	}
	return true;
}

void CleanUp_Python(void)
{
	if(tr_function != NULL && tr_parameters != NULL)
	{
		trex_free(tr_function);
		trex_free(tr_parameters);

		tr_function = NULL;
		tr_parameters = NULL;
	}
}

Keywords Parse_Python(const ParserDefinition *pd, const char *text)
{
	Keywords keywords;
	std::vector<std::string> params;
	std::vector<std::string> function;
	const TRexChar *begin,*end;

	if(trex_search(tr_function, text, &begin, &end))
	{
		//TRexMatch func_match;
		TRexMatch params_match, func_match;
		const TRexChar *cur_params;
		const TRexChar *p_begin, *p_end;

		trex_getsubexp(tr_function, 1, &func_match);
		trex_getsubexp(tr_function, 2, &params_match);

		function.push_back(std::string(func_match.begin, func_match.len));

		// For each param
		cur_params = params_match.begin;
		while(trex_searchrange(tr_parameters, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(tr_parameters, 1, &param_match);
			
			params.push_back(std::string(param_match.begin, param_match.len));

			cur_params = p_end;
		}
		keywords["$PARAM"] = params;
		keywords["$FUNCTION"] = function;
	}

	return keywords;
}