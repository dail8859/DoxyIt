#include "PluginDefinition.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;


BOOL APIENTRY DllMain( HANDLE hModule,
					   DWORD  reasonForCall,
					   LPVOID lpReserved )
{
	switch (reasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		pluginInit(hModule);
		break;

	case DLL_PROCESS_DETACH:
		commandMenuCleanUp();
		pluginCleanUp();
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	NotifyHeader nh = notifyCode->nmhdr;
	int ch = notifyCode->ch;

	if(nh.code == SCN_CHARADDED)
	{
		int which = -1;

		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
		if (which == -1) return;

		HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

		int curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
		int curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);

		if(ch == '\n' && do_active_commenting)
		{
			char *buffer;
			int lineLen = (int) ::SendMessage(curScintilla, SCI_LINELENGTH, curLine - 1, 0);
		
			buffer = new char[lineLen + 1];
			::SendMessage(curScintilla, SCI_GETLINE, curLine - 1, (LPARAM) buffer);
			buffer[lineLen] = '\0';
		
			// Creates a new comment block
			if(strncmp(buffer, doc_start.c_str(), doc_start.length()) == 0)
			{
				std::string temp = "\r\n" + doc_end;
				::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_line.c_str());
				::SendMessage(curScintilla, SCI_INSERTTEXT, -1, (LPARAM) temp.c_str());
			}

			// Adds a new line of the comment block
			if(strncmp(buffer, doc_line.c_str(), doc_line.length()) == 0)
			{
				::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_line.c_str());
			}

			delete[] buffer;
		}
		else if(do_active_wrapping) // && line starts with doc_line
		{
			int lineMax = 40;
			// Get the line length without counting line endings
			int lineStart = ::SendMessage(curScintilla, SCI_POSITIONFROMLINE, curLine, 0);
			int lineEnd = ::SendMessage(curScintilla, SCI_GETLINEENDPOSITION, curLine, 0);
			int lineLen = lineEnd - lineStart;

			if(lineLen > lineMax)
			{
				TCHAR ttext[128];
				char *text = new char[128];
				TextRange tr;
				tr.chrg.cpMin = lineStart;
				tr.chrg.cpMax = lineStart + 127;
				tr.lpstrText = text;

				::SendMessage(curScintilla, SCI_GETTEXTRANGE, 0, (LPARAM) &tr);

				int i = lineMax;
				while(i > 0 && text[i] == ' ') --i;
				while(i > 0 && text[i] != ' ') --i;

				
				int j = 0;
				//while(isalpha(text[lineEnd + j])) ++j;
				
				
				//if(strncmp(&text[j], doc_line.c_str(), doc_line.length()) == 0) j += doc_line.length();
				//else return;
				//while(j < 128 && text[j] != ' ') ++j;

				::SendMessage(curScintilla, SCI_SETSEL, lineEnd, lineEnd + 6);
				::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) "\0");
				::SendMessage(curScintilla, SCI_INSERTTEXT, lineStart + i, (LPARAM) "\r\n *  ");
				::SendMessage(curScintilla, SCI_GOTOPOS, curPos, 0);


				//mbstowcs(ttext, &text[i], 256);

				//::MessageBox(NULL, ttext, TEXT("hi"), MB_OK);

				//::SendMessage(curScintilla, SCI_INSERTTEXT, lineStart + i, (LPARAM) "\r\n");

				delete[] text;
			}
		}
	}
}


// Here you can process the Npp Messages
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}
#endif //UNICODE
