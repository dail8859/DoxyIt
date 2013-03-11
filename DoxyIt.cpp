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

#include <Shlwapi.h>
#include <tchar.h>
#include "PluginDefinition.h"
#include "Parsers.h"

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

void configSave()
{
	TCHAR iniPath[MAX_PATH];
	int len = sizeof(parsers) / sizeof(parsers[0]);
	
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM) iniPath);
	::_tcscat_s(iniPath, TEXT("\\"));
	::_tcscat_s(iniPath, NPP_PLUGIN_NAME);
	::_tcscat_s(iniPath, TEXT(".ini"));

	// [DoxyIt]
	::WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("active_commenting"), do_active_commenting ? TEXT("true") : TEXT("false"), iniPath);

	for(int i = 0; i < len; ++i)
	{
		Parser *p = &parsers[i];
		std::wstring ws;

		// Wrap everything in quotes to perserve whitespace
		ws.assign(p->doc_start.begin(), p->doc_start.end());
		ws = TEXT("\"") + ws + TEXT("\"");
		::WritePrivateProfileString(p->lang.c_str(), TEXT("doc_start"), ws.c_str(), iniPath);

		ws.assign(p->doc_line.begin(), p->doc_line.end());
		ws = TEXT("\"") + ws + TEXT("\"");
		::WritePrivateProfileString(p->lang.c_str(), TEXT("doc_line_"), ws.c_str(), iniPath);

		ws.assign(p->doc_end.begin(), p->doc_end.end());
		ws = TEXT("\"") + ws + TEXT("\"");
		::WritePrivateProfileString(p->lang.c_str(), TEXT("doc_end__"), ws.c_str(), iniPath);

		ws.assign(p->command_prefix.begin(), p->command_prefix.end());
		ws = TEXT("\"") + ws + TEXT("\"");
		::WritePrivateProfileString(p->lang.c_str(), TEXT("command_prefix"), ws.c_str(), iniPath);
	}
}

void configLoad()
{
	TCHAR iniPath[MAX_PATH];
	int len = sizeof(parsers) / sizeof(parsers[0]);
	TCHAR tbuffer[MAX_PATH];
	char buffer[MAX_PATH];

	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM) iniPath);
	::_tcscat_s(iniPath, TEXT("\\"));
	::_tcscat_s(iniPath, NPP_PLUGIN_NAME);
	::_tcscat_s(iniPath, TEXT(".ini"));

	// [DoxyIt]
	GetPrivateProfileString(NPP_PLUGIN_NAME, TEXT("active_commenting"), TEXT("true"), tbuffer, MAX_PATH, iniPath);
	wcstombs(buffer, tbuffer, MAX_PATH);
	do_active_commenting = strcmp(buffer, "true") == 0;

	for(int i = 0; i < len; ++i)
	{
		Parser *p = &parsers[i];

		// NOTE: We cant use the default value because GetPrivateProfileString strips the whitespace,
		// also, wrapping it in quotes doesn't seem to work either. So...use "!!!" as the default text
		// and if we find that the value wasnt found and we have "!!!" then use the default value in the
		// parser, else, use what we pulled from the file.
		GetPrivateProfileString(p->lang.c_str(), TEXT("doc_start"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->doc_start.assign(p->default_doc_start.begin(), p->default_doc_start.end());
		else p->doc_start.assign(buffer);

		GetPrivateProfileString(p->lang.c_str(), TEXT("doc_line_"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->doc_line.assign(p->default_doc_line.begin(), p->default_doc_line.end());
		else p->doc_line.assign(buffer);

		GetPrivateProfileString(p->lang.c_str(), TEXT("doc_end__"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->doc_end.assign(p->default_doc_end.begin(), p->default_doc_end.end());
		else p->doc_end.assign(buffer);

		GetPrivateProfileString(p->lang.c_str(), TEXT("command_prefix"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->command_prefix.assign(p->default_command_prefix.begin(), p->default_command_prefix.end());
		else p->command_prefix.assign(buffer);
	}

	// Write out the file if it doesnt exist yet
	if(!PathFileExists(iniPath)) configSave();
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
	configLoad();
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
	static bool do_newline = false;
	NotifyHeader nh = notifyCode->nmhdr;
	int ch = notifyCode->ch;
	
	switch(nh.code)
	{
		case SCN_UPDATEUI:
			// Now is when we can check to see if we do the commenting
			if(do_newline)
			{
				do_newline = false;
				if(!updateScintilla()) return;
				doxyItNewLine();
			}
			break;
		case SCN_CHARADDED:
			// Set a flag so that all line endings can trigger the commenting
			if((ch == '\r' || ch == '\n') && do_active_commenting)
			{
				do_newline = true;
			}
			break;
		case NPPN_READY:
			CommunicationInfo ci;
			
			// Check if FingerText is installed
			ci.internalMsg = FINGERTEXT_GETVERSION;
			ci.srcModuleName = NPP_PLUGIN_NAME;
			ci.info = NULL;
		
			// NPPM_MSGTOPLUGIN returns true if the dll is found
			if(::SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci))
			{
				if((int) ci.info >= 561) fingertext_found = true;
				else fingertext_found = false;
			}
			else
			{
				::MessageBox(NULL, TEXT("FingerText not found."), NPP_PLUGIN_NAME, MB_OK);
				fingertext_found = false;
				return;
			}
			break;
		case NPPN_SHUTDOWN:
			configSave();
			break;
	}
	/*
	else if(do_active_wrapping) // && line starts with doc_line
	{
		int lineMax = 40;
		// Get the line length without counting line endings
		int lineStart = ::SendMessage(curScintilla, SCI_POSITIONFROMLINE, curLine, 0);
		int lineEnd = ::SendMessage(curScintilla, SCI_GETLINEENDPOSITION, curLine, 0);
		int lineLen = lineEnd - lineStart;

		if(lineLen > lineMax)
		{
			int char_width = ::SendMessage(curScintilla, SCI_TEXTWIDTH, STYLE_DEFAULT, (LPARAM) " ");
			::SendMessage(curScintilla, SCI_SETTARGETSTART, lineStart, 0);
			::SendMessage(curScintilla, SCI_SETTARGETEND, lineStart, 0);
			::SendMessage(curScintilla, SCI_LINESSPLIT, lineMax * char_width, 0);

			// Check the next few lines to insert the doc_line in front of them
			for(int i = 1; i < 5; ++i)
			{
				// Get the length and allocate a buffer
				int lineLen = ::SendMessage(curScintilla, SCI_LINELENGTH, curLine + i, 0);
				char *text = new char[lineLen + 1];

				// Get the text
				::SendMessage(curScintilla, SCI_GETLINE, curLine + i, (LPARAM) text);
				text[lineLen] = '\0';

				// if it doesn't start with doc_line or doc_start, insert the doc_line
				// else we are done
				if(strncmp(text, doc_line.c_str(), doc_line.length()) != 0 && strncmp(text, doc_end.c_str(), doc_end.length()) != 0)
				{
					int lineStart = ::SendMessage(curScintilla, SCI_POSITIONFROMLINE, curLine + i, 0);
					::SendMessage(curScintilla, SCI_INSERTTEXT, lineStart, (LPARAM) doc_line.c_str());
				}
				else
				{
					delete[] text;
					break;
				}

				delete[] text;
			}
		}
	}
	*/
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
