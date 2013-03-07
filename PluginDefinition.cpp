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
#include "menuCmdID.h"
#include "trex.h"
#include "Utils.h"
#include "Parsers.h"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

bool do_active_commenting;
bool do_active_wrapping;
bool fingertext_found;
bool fingertext_enabled;

SciFnDirect pSciMsg;  // For direct scintilla call
sptr_t pSciWndData;   // For direct scintilla call

LRESULT SendScintilla(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return pSciMsg(pSciWndData, Msg, wParam, lParam);
}

bool updateScintilla()
{
	HWND curScintilla;

	// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if(which == -1) return false;
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Get the function and pointer to it for more effecient calls
	pSciMsg = (SciFnDirect) SendMessage(curScintilla,SCI_GETDIRECTFUNCTION, 0, 0);
	pSciWndData = (sptr_t) SendMessage(curScintilla,SCI_GETDIRECTPOINTER, 0, 0);

	return true;
}

//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit(HANDLE hModule)
{
	InitializeParsers();

	do_active_commenting = true;
	do_active_wrapping = true;
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	CleanUpParsers();
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
	//setCommand(1, TEXT("DoxyIt - File"), doxyItFile, NULL, false);
	setCommand(1, TEXT("Active commenting"), activeCommenting, NULL, do_active_commenting);
	//setCommand(3, TEXT("Active word wrapping"), activeWrapping, NULL, do_active_wrapping);
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



bool checkFingerText()
{
	if(fingertext_found)
	{
		CommunicationInfo ci;
		ci.internalMsg = FINGERTEXT_ISENABLED;
		ci.srcModuleName = NPP_PLUGIN_NAME;
		ci.info = NULL;
		::SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);
		return ci.info != NULL;
	}
	else
		return false;
}


// --- Menu call backs ---

void doxyItFunction()
{
	std::string doc_block;
	int lang_type;
	int startPos, endPos;
	int startLine, endLine;
	char *indent = NULL;

	if(!updateScintilla()) return;

	// Check if it is enabled
	fingertext_enabled = checkFingerText();

	// Get the current language type and parse it
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &lang_type);
	doc_block = Parse(lang_type);
	if(doc_block.length() == 0) return;
	
	// Keep track of where we started
	startPos = SendScintilla(SCI_GETCURRENTPOS);
	startLine = SendScintilla(SCI_LINEFROMPOSITION, startPos);

	// Get the whitespace of the next line so we can insert it infront of 
	// all the lines of the document block that is going to be inserted
	indent = getLineIndentStr(startLine + 1);

	SendScintilla(SCI_BEGINUNDOACTION);

	SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) doc_block.c_str());
	
	// get the end of the document block
	endPos = SendScintilla(SCI_GETCURRENTPOS);
	endLine = SendScintilla(SCI_LINEFROMPOSITION, endPos);

	if(indent) insertBeforeLines(indent, startLine, endLine + 1);
	
	SendScintilla(SCI_ENDUNDOACTION);


	// Activate FingerText
	if(fingertext_enabled)
	{
		CommunicationInfo ci;
		ci.internalMsg = FINGERTEXT_ACTIVATE;
		ci.srcModuleName = NPP_PLUGIN_NAME;
		ci.info = NULL;

		// Reset to where we started
		SendScintilla(SCI_SETCURRENTPOS, startPos);

		::SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);
	}

	if(indent) delete[] indent;
	// return (return_val, function_name, (parameters))
}

void doxyItFile()
{
	/*
	TCHAR fileName[MAX_PATH];
	char fname[MAX_PATH];
	std::ostringstream doc_block;
	char *eol;
	
	::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM) fileName);
	wcstombs(fname, fileName, sizeof(fname));

	if(!updateScintilla()) return;

	eol = getEolStr();

	// Check if it is enabled
	fingertext_enabled = checkFingerText();

	doc_block << doc_start << eol;
	doc_block << doc_line << command_prefix << "file " << fname << eol;
	doc_block << doc_line << command_prefix << "brief " << eol;
	doc_block << doc_line << eol;
	doc_block << doc_line << command_prefix << "author " << eol;
	doc_block << doc_line << command_prefix << "version 1.0" << eol;
	doc_block << doc_end;

	SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) doc_block.str().c_str());
	*/
}

void activeCommenting()
{
	do_active_commenting = !do_active_commenting;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[1]._cmdID, (LPARAM) do_active_commenting);
}

/*
void activeWrapping()
{
	do_active_wrapping = !do_active_wrapping;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[3]._cmdID, (LPARAM) do_active_wrapping);
}
*/

// --- Notification callbacks ---

void doxyItNewLine()
{
	const Parser *p;
	std::ostringstream doc_block;
	std::string indentation;
	std::string short_doc_start;
	char *previousLine, *eol, *found = NULL;
	int curPos, curLine;

	if(!updateScintilla()) return;

	if(!(p = getCurrentParser())) return;
	short_doc_start = p->doc_start.substr(0, 3);
	eol = getEolStr();

	curPos = (int) SendScintilla(SCI_GETCURRENTPOS);
	curLine = (int) SendScintilla(SCI_LINEFROMPOSITION, curPos);

	previousLine = getLine(curLine - 1);

	// NOTE: we cannot use getLineIndentStr() because doc_start or doc_line may start with whitespace
	// which we don't want counted towards the indentation string.
	
	// search for doc_start or doc_line in the previous line to see if we should complete a document
	// block or if we should add a single document line
	
	// short_doc_start is the first 3 characters of the doc_start. If doc_start is relatively long
	// we do not want the user typing the entire line, just the first 3 should suffice.
	if((found = strstr(previousLine, short_doc_start.c_str()))
		&& strstr(previousLine, p->doc_end.c_str()) == 0)
	{
		indentation.append(previousLine, found - previousLine);

		// Count the characters in common so we can add the rest
		unsigned int i = 0;
		while(i < p->doc_start.length() && found[i] == p->doc_start.at(i)) ++i;
		
		doc_block << &p->doc_start.c_str()[i] << eol;
		doc_block << indentation.c_str() << p->doc_line.c_str() << eol;
		doc_block << indentation.c_str() << p->doc_end.c_str();
		
		SendScintilla(SCI_BEGINUNDOACTION);
		clearLine(curLine); // Clear any automatic indentation
		SendScintilla(SCI_DELETEBACK);
		SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) doc_block.str().c_str());
		SendScintilla(SCI_ENDUNDOACTION);

		// Go up and to the end of the previous line
		SendScintilla(SCI_LINEUP);
		SendScintilla(SCI_LINEEND);
	}
	else if(found = strstr(previousLine, p->doc_line.c_str()))
	{
		indentation.append(previousLine, found - previousLine);

		doc_block << indentation.c_str() <<  p->doc_line.c_str();

		SendScintilla(SCI_BEGINUNDOACTION);
		clearLine(curLine); // Clear any automatic indentation
		SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) doc_block.str().c_str());
		SendScintilla(SCI_ENDUNDOACTION);
	}

	delete[] previousLine;
}
