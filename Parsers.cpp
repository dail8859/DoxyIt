#include <ctime>
#include "PluginDefinition.h"
#include "trex.h"

char *getLine(HWND curScintilla, int lineNum);

typedef struct Parser
{
	int lang_type;
	bool (*initializer)(Parser *p);
	void (*callback)(Parser *p);
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

void Callback_C(Parser *p)
{
	char *buffer;
	int which = -1;
	HWND curScintilla;
	const TRexChar *begin,*end;
	// Get the current scintilla
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if(which == -1) return;
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	int curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
	int curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);

	buffer = getLine(curScintilla, curLine + 1);
	
	if(trex_search(p->tr_function, buffer, &begin, &end))
	{
		std::ostringstream doc_block;
		char date[32];
		TRexMatch return_match;
		TRexMatch func_match;
		TRexMatch params_match;
		const TRexChar *cur_params;
		const TRexChar *p_begin, *p_end;
		time_t t;

		trex_getsubexp(p->tr_function, 1, &return_match);
		trex_getsubexp(p->tr_function, 2, &func_match);
		trex_getsubexp(p->tr_function, 3, &params_match);

		t = time(NULL);
		strftime(date, 32, "%m/%d/%Y", localtime(&t));

		doc_block << doc_start << "\r\n";
		doc_block << doc_line << "\\brief [description]\r\n";
		doc_block << doc_line << "\r\n";
		
		// For each param
		cur_params = params_match.begin;
		while(trex_searchrange(p->tr_parameters, cur_params, end, &p_begin, &p_end))
		{
			TRexMatch param_match;
			trex_getsubexp(p->tr_parameters, 1, &param_match);

			doc_block << doc_line << "\\param [in] ";
			doc_block.write(param_match.begin, param_match.len);
			doc_block << " [description]\r\n";
			cur_params = p_end;
		}

		// Return value
		doc_block << doc_line << "\\return \\em ";
		doc_block.write(return_match.begin, return_match.len); doc_block << "\r\n";
		doc_block << doc_line << "\r\n";

		doc_block << doc_line << "\\revision 1 " << date << "\r\n";
		doc_block << doc_line << "\\history <b>Rev. 1 " << date << "</b> [description]\r\n";
		doc_block << doc_line << "\r\n";
		doc_block << doc_line << "\\details [description]\r\n";
		doc_block << doc_end;
		
		::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_block.str().c_str());
	}
	else
	{
		::MessageBox(NULL, TEXT("Cannot parse function definition"), TEXT("Error"), MB_OK|MB_ICONERROR);
	}

	delete[] buffer;
}


#define REGISTER_PARSER(lang) {L_##lang, Initialize_##lang, Callback_##lang, NULL, NULL}
static Parser parsers[] = 
{
	REGISTER_PARSER(C)
	//REGISTER_TYPE(CPP),
	//REGISTER_TYPE(PYTHON),
};

void Parse(int lang_type)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
	{
		if(parsers[i].lang_type == lang_type)
		{
			(*parsers[i].callback)(&parsers[i]);
			break;
		}
	}
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
