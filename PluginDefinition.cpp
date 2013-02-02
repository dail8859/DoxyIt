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

#include <stdio.h>

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "trex.h"
/**
 *  comment block
 */

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

bool do_active_commenting;
TRex *c_tr;
//TRex *cpp_tr;

//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit(HANDLE hModule)
{
	const TRexChar *error = NULL;
	c_tr = trex_compile(TEXT("^\\s*(\\w+)\\s+(\\w+)\\s*\\((.*)\\)"), &error);
	if(!c_tr)
	{
		::MessageBox(NULL, TEXT("Regular expression compilation failed"), TEXT("Error"), MB_OK);
	}
	do_active_commenting = true;
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	trex_free(c_tr);
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

	ShortcutKey *sk = new ShortcutKey();
	sk->_isAlt = TRUE;
	sk->_isCtrl = TRUE;
	sk->_isShift = TRUE;
	sk->_key = 'D';

	setCommand(0, TEXT("DoxyIt - Function"), doxyItFunction, sk, false);
	setCommand(1, TEXT("DoxyIt - File"), doxyItFile, NULL, false);
	setCommand(2, TEXT("Active commenting"), activeCommenting, NULL, do_active_commenting);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
	if (index >= nbFunc || !pFunc) return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void doxyItFunction()
{
	char buffer[256];
	wchar_t wbuffer[256];
	int which = -1;
	HWND curScintilla;
	const TRexChar *begin,*end;

	// Get the current scintilla
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1) return;
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
	
	int curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
	int curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);
	int lineLen = (int) ::SendMessage(curScintilla, SCI_GETLINE, curLine + 1, (LPARAM) buffer);
	buffer[lineLen] = '\0';

	mbstowcs(wbuffer, buffer, 256);

	if(trex_search(c_tr, wbuffer, &begin, &end))
	{
		TRexMatch return_match;
		TRexMatch func_match;
		TRexMatch params_match;
		wchar_t wdoc_buf[1024];
		char doc_buf[1024];
		int offset = 0;

		trex_getsubexp(c_tr, 1, &return_match);
		trex_getsubexp(c_tr, 2, &func_match);
		trex_getsubexp(c_tr, 3, &params_match);

		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT("/**\r\n"));
		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT(" * \\brief [description]\r\n"));
		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT(" * \r\n"));
		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT(" * \\return \\em %.*s\r\n"), return_match.len, return_match.begin);
		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT(" * \r\n"));
		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT(" * \\param [in] %.*s [description]\r\n"), params_match.len, params_match.begin);
		offset += _snwprintf(wdoc_buf + offset, 1024 - offset, TEXT(" */"));

		wcstombs(doc_buf, wdoc_buf, 1024);
		// ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);
		::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_buf);
		// ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) "/**\r\n");
		// ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) " * \\brief Description\r\n");
		// ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) " * \r\n");
		// ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) " * \\params \r\n");
		// ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) " */");
		// ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	}
	else
	{
		::MessageBox(NULL, TEXT("Cannot parse function definition"), TEXT("Error"), MB_OK);
	}
}

void doxyItFile()
{
}

void activeCommenting()
{
	do_active_commenting = !do_active_commenting;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[1]._cmdID, (LPARAM) do_active_commenting);
}
