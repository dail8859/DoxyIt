#include "PluginDefinition.h"
#include "trex.h"

char *getLine(HWND curScintilla, int lineNum);
char *getEolStr(HWND curScintilla);

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
	int which = -1;
	HWND curScintilla;
	const TRexChar *begin,*end;
	char *eol;
	std::ostringstream doc_block;

	// Get the current scintilla
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if(which == -1) return "";
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	int curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
	int curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);

	eol = getEolStr(curScintilla);
	buffer = getLine(curScintilla, curLine + 1);
	
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
		doc_block << doc_line << "\\brief $[![description]!]" << eol;
		doc_block << doc_line << eol;
		
		// For each param
		cur_params = params_match.begin;
		while(trex_searchrange(p->tr_parameters, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(p->tr_parameters, 1, &param_match);

			doc_block << doc_line << "\\param [in] ";
			doc_block.write(param_match.begin, param_match.len);
			doc_block << " $[![description]!]" << eol;
			cur_params = p_end;
		}

		// Return value
		doc_block << doc_line << "\\return \\em ";
		doc_block.write(return_match.begin, return_match.len); doc_block << eol;
		doc_block << doc_line << eol;

		doc_block << doc_line << "\\revision 1 $[![(key)DATE:MM/dd/yyyy]!]" << eol;
		doc_block << doc_line << "\\history <b>Rev. 1 $[![(key)DATE:MM/dd/yyyy]!]</b> $[![description]!]" << eol;
		doc_block << doc_line << eol;
		doc_block << doc_line << "\\details $[![description]!]" << eol;
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

// --- ---

char *getLine(HWND curScintilla, int lineNum)
{
	char *buffer;
	int lineLen = (int) ::SendMessage(curScintilla, SCI_LINELENGTH, lineNum, 0);

	buffer = new char[lineLen + 1];
	::SendMessage(curScintilla, SCI_GETLINE, lineNum, (LPARAM) buffer);
	buffer[lineLen] = '\0';

	return buffer;
}

char *getEolStr(HWND curScintilla)
{
	int eolmode = ::SendMessage(curScintilla, SCI_GETEOLMODE, 0, 0);
	static char *eol[] = {"\r\n","\r","\n"};

	return eol[eolmode];
}
