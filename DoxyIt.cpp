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

	if(nh.code == SCN_CHARADDED && ch == '\n' && do_active_commenting)
	{
		char *buffer;
		int which = -1;

		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
		if (which == -1) return;

		HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

		int curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
		int curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);
		int lineLen = (int) ::SendMessage(curScintilla, SCI_LINELENGTH, curLine - 1, 0);
		
		buffer = new char[lineLen + 1];
		::SendMessage(curScintilla, SCI_GETLINE, curLine - 1, (LPARAM) buffer);
		buffer[lineLen] = '\0';
		
		if(strncmp(buffer, doc_start.c_str(), doc_start.length()) == 0)
		{
			std::string temp = "\r\n" + doc_end;
			::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_line.c_str());
			::SendMessage(curScintilla, SCI_INSERTTEXT, -1, (LPARAM) temp.c_str());
		}
		if(strncmp(buffer, doc_line.c_str(), doc_line.length()) == 0)
		{
			::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_line.c_str());
		}

		delete[] buffer;
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
