//Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
//
//This program is free software; you can redistribute it and/or
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

#include "PluginDefinition.h"
#include "trex.h"
#include "Utils.h"


typedef struct Parser
{
	int lang_type;
	bool (*initializer)(Parser *p);
	std::string (*callback)(Parser *p);
	TRex *tr_function;
	TRex *tr_parameters;
} Parser;

bool Initialize_C(Parser *p)
{
	const TRexChar *error = NULL;
	p->tr_function = trex_compile("(\\w+)[*]*\\s+[*]*(\\w+)\\s*(\\(.*\\))", &error);
	p->tr_parameters = trex_compile("(\\w+)\\s*[,)]", &error);
	if(!p->tr_function || !p->tr_parameters)
		return false;
	return true;
}

std::string Callback_C(Parser *p)
{
	char *buffer;
	const TRexChar *begin,*end;
	char *eol;
	std::ostringstream doc_block;

	int curPos = (int) SendScintilla(SCI_GETCURRENTPOS, 0, 0);
	int curLine = (int) SendScintilla(SCI_LINEFROMPOSITION, curPos, 0);
	int found = findNext(")", false);
	if(found == -1) return "";

	buffer = getRange(curPos, found + 1);
	eol = getEolStr();

	if(trex_search(p->tr_function, buffer, &begin, &end))
	{
		TRexMatch return_match;
		TRexMatch func_match;
		TRexMatch params_match;
		const TRexChar *cur_params;
		const TRexChar *p_begin, *p_end;

		trex_getsubexp(p->tr_function, 1, &return_match);
		trex_getsubexp(p->tr_function, 2, &func_match);
		trex_getsubexp(p->tr_function, 3, &params_match);

		doc_block << doc_start << eol;
		doc_block << doc_line << command_prefix << "brief $[![[Brief]]!]" << eol;
		doc_block << doc_line << eol;
		
		// For each param
		cur_params = params_match.begin;
		while(trex_searchrange(p->tr_parameters, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(p->tr_parameters, 1, &param_match);

			doc_block << doc_line << command_prefix << "param [in] ";
			doc_block.write(param_match.begin, param_match.len);
			doc_block << " $[![[Param ";
			doc_block.write(param_match.begin, param_match.len);
			doc_block << " Description]]!]" << eol;
			cur_params = p_end;
		}

		// Return value
		doc_block << doc_line << command_prefix << "return " << command_prefix << "em ";
		doc_block.write(return_match.begin, return_match.len); 
		doc_block << " $[![[Return Description]]!]" << eol;
		doc_block << doc_line << eol;

		//doc_block << doc_line << command_prefix << "revision 1 $[![(key)DATE:MM/dd/yyyy]!]" << eol;
		//doc_block << doc_line << command_prefix << "history <b>Rev. 1 $[![(key)DATE:MM/dd/yyyy]!]</b> $[![description]!]" << eol;
		//doc_block << doc_line << eol;
		doc_block << doc_line << command_prefix << "details $[![[Details]]!]" << eol;
		doc_block << doc_end;
	}
	else
	{
		::MessageBox(NULL, TEXT("Cannot parse function definition"), TEXT("Error"), MB_OK|MB_ICONERROR);
	}

	delete[] buffer;

	return doc_block.str();
}


#define REGISTER_PARSER(lang) {L_##lang, Initialize_##lang, Callback_##lang, NULL, NULL}
static Parser parsers[] = 
{
	REGISTER_PARSER(C)
	//REGISTER_TYPE(CPP),
	//REGISTER_TYPE(PYTHON),
};

std::string Parse(int lang_type)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
	{
		if(parsers[i].lang_type == lang_type)
		{
			return (*parsers[i].callback)(&parsers[i]);
			break;
		}
	}

	return "";
}

void InitializeParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		if((*parsers[i].initializer)(&parsers[i]) == false)
			::MessageBox(NULL, TEXT("Doxyit initialization failed"), TEXT("Doxyit"), MB_OK|MB_ICONERROR);
}

void CleanUpParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
	{
		trex_free(parsers[i].tr_function);
		trex_free(parsers[i].tr_parameters);
	}

}