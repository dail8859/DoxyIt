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

Keywords parse_c(const ParserSettings *ps, const char *text)
{
	static TrRegex tr_function("(?:([\\w:]+)[*&]*\\s+(?:[*&=]*\\s+)?[*&]*)?([\\w:]+)\\s*(\\([^)]*\\))");
	static TrRegex tr_parameters("(\\$?\\w+|\\.\\.\\.)(\\s*=\\s*[\\\"\\w\\.']+)?\\s*[,)]");
	Keywords keywords;
	
	const TRexChar *begin, *end;
	int chevron = 0;

	// HACK: Duplicate the string so it can be modified and any commas within <...> needs removed to support templates e.g. std::pair<int, double>
	std::string dup(text);

	for (char &c : dup)
	{
		if (c == '<') chevron++;
		else if (c == '>') chevron--;
		else if (c == ',' && chevron > 0) c = ' ';
	}

	if (trex_search(tr_function.regex, dup.c_str(), &begin, &end))
	{
		std::vector<std::string> params;
		std::vector<std::string> function;
		TRexMatch params_match, func_match;
		const TRexChar *cur_params;
		const TRexChar *p_begin, *p_end;

		//trex_getsubexp(tr_function, 1, &return_match);	// not used for now
		trex_getsubexp(tr_function.regex, 2, &func_match);
		trex_getsubexp(tr_function.regex, 3, &params_match);

		keywords.function = std::string(func_match.begin, func_match.len);

		// For each param
		cur_params = params_match.begin;
		while (trex_searchrange(tr_parameters.regex, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(tr_parameters.regex, 1, &param_match);

			// handle "func(void)" by skipping it
			if (strncmp(param_match.begin, "void", 4) != 0)
				keywords.parameters.push_back(std::string(param_match.begin, param_match.len));

			cur_params = p_end;
		}
	}

	return keywords;
}
