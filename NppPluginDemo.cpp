//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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

	if(do_active_commenting)
	{
		if(nh.code == SCN_CHARADDED && ch == '\n')
		{
			//wchar_t wbuffer[256];
			char buffer[256];
			int which = -1;

			::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
			if (which == -1) return;

			HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

			int curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
			int curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);
			int lineLen = (int) ::SendMessage(curScintilla, SCI_GETLINE, curLine - 1, (LPARAM) buffer);
			buffer[lineLen] = '\0';

			//::SendMessage(curScintilla, SCI_GETCURLINE, 256, (LPARAM) buffer);
			//mbstowcs(wbuffer, buffer, 256);
			//::MessageBox(NULL, wbuffer, TEXT("hi"), MB_OK);

			if(buffer[0] == '/' && buffer[1] == '*' && buffer[2] == '*')
			{
				::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) " * ");
				::SendMessage(curScintilla, SCI_INSERTTEXT, -1, (LPARAM) "\r\n */");
			}
			if(buffer[0] == ' ' && buffer[1] == '*' && buffer[2] == ' ')
			{
				::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) " * ");
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
