//This file is part of DoxyIt.
//
//Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
//
//DoxyIt is free software; you can redistribute it and/or
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
#include "Utils.h"
#include "Parsers.h"
#include "Version.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"

// --- Global variables ---
FuncItem funcItem[nbFunc];

// --- Local variables ---
static bool do_active_commenting;	// active commenting - create or extend a document block
//static bool do_active_wrapping;	// active wrapping - wrap text inside of document blocks...todo
static bool use_fingertext;			// use fingertext if it is available
static bool fingertext_found;		// if we found the fingertext plugin installed
bool fingertext_enabled;			// if fingertext is enabled (global)

static NppData nppData;
static SciFnDirect pSciMsg;			// For direct scintilla call
static sptr_t pSciWndData;			// For direct scintilla call
static SettingsDialog sd;			// The settings dialog
static HANDLE _hModule;				// For dialog initialization

// --- Menu callbacks ---
static void doxyItFunction();
static void doxyItFile();
static void activeCommenting();
//static void useFingerText();
//static void activeWrapping();
static void showSettings();
static void showAbout();


LRESULT SendScintilla(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return pSciMsg(pSciWndData, Msg, wParam, lParam);
}

LRESULT SendNpp(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(nppData._nppHandle, Msg, wParam, lParam);
}

bool updateScintilla()
{
	HWND curScintilla;

	// Get the current scintilla
	int which = -1;
	SendNpp(NPPM_GETCURRENTSCINTILLA, SCI_UNUSED, (LPARAM)&which);
	if(which == -1) return false;
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Get the function and pointer to it for more efficient calls
	pSciMsg = (SciFnDirect) SendMessage(curScintilla, SCI_GETDIRECTFUNCTION, 0, 0);
	pSciWndData = (sptr_t) SendMessage(curScintilla, SCI_GETDIRECTPOINTER, 0, 0);

	return true;
}

// --- Configuration ---

void getIniFilePath(TCHAR *iniPath, int size)
{
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, size, (LPARAM) iniPath);
	_tcscat_s(iniPath, size, TEXT("\\"));
	_tcscat_s(iniPath, size, NPP_PLUGIN_NAME);
	_tcscat_s(iniPath, size, TEXT(".ini"));
}

void configSave()
{
	TCHAR iniPath[MAX_PATH];
	int len = sizeof(parsers) / sizeof(parsers[0]);

	getIniFilePath(iniPath, MAX_PATH);

	// [DoxyIt]
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("active_commenting"), BOOLTOSTR(do_active_commenting), iniPath);
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("use_fingertext"), BOOLTOSTR(use_fingertext), iniPath);
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("version"), VERSION_LINEAR_TEXT, iniPath);
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("version_stage"), VERSION_STAGE, iniPath);

	for(int i = 0; i < len; ++i)
	{
		const Parser *p = &parsers[i];
		const ParserDefinition *pd = &p->pd;
		std::wstring ws;

		// Wrap everything in quotes to preserve whitespace
		ws = TEXT("\"") + toWideString(pd->doc_start) + TEXT("\"");
		WritePrivateProfileString(p->lang.c_str(), TEXT("doc_start"), ws.c_str(), iniPath);

		ws = TEXT("\"") + toWideString(pd->doc_line) + TEXT("\"");
		WritePrivateProfileString(p->lang.c_str(), TEXT("doc_line_"), ws.c_str(), iniPath);

		ws = TEXT("\"") + toWideString(pd->doc_end) + TEXT("\"");
		WritePrivateProfileString(p->lang.c_str(), TEXT("doc_end__"), ws.c_str(), iniPath);

		ws = TEXT("\"") + toWideString(pd->command_prefix) + TEXT("\"");
		WritePrivateProfileString(p->lang.c_str(), TEXT("command_prefix"), ws.c_str(), iniPath);

		// Encode \r\n as literal "\r\n" in the ini file
		ws = TEXT("\"") + toWideString(stringReplace(pd->format, "\r\n", "\\r\\n")) + TEXT("\"");
		WritePrivateProfileString(p->lang.c_str(), TEXT("format"), ws.c_str(), iniPath);

		WritePrivateProfileString(p->lang.c_str(), TEXT("align"), BOOLTOSTR(pd->align_desc), iniPath);
	}
}

void configLoad()
{
	TCHAR iniPath[MAX_PATH];
	TCHAR tbuffer[MAX_PATH];
	char buffer[MAX_PATH];
	int len = sizeof(parsers) / sizeof(parsers[0]);
	//int version;

	getIniFilePath(iniPath, MAX_PATH);

	// [DoxyIt]
	GetPrivateProfileString(NPP_PLUGIN_NAME, TEXT("active_commenting"), TEXT("true"), tbuffer, MAX_PATH, iniPath);
	wcstombs(buffer, tbuffer, MAX_PATH);
	do_active_commenting = strcmp(buffer, "true") == 0;

	GetPrivateProfileString(NPP_PLUGIN_NAME, TEXT("use_fingertext"), TEXT("true"), tbuffer, MAX_PATH, iniPath);
	wcstombs(buffer, tbuffer, MAX_PATH);
	use_fingertext = strcmp(buffer, "true") == 0;
	use_fingertext = false; // Disable fingertext

	// Don't need these for now
	//version = GetPrivateProfileInt(NPP_PLUGIN_NAME, TEXT("version"), 0, iniPath);
	//version_stage = GetPrivateProfileString(NPP_PLUGIN_NAME, TEXT("version_stage"), TEXT(""), tbuffer, MAX_PATH, iniPath);

	for(int i = 0; i < len; ++i)
	{
		Parser *p = &parsers[i];

		// NOTE: We cant use the default value because GetPrivateProfileString strips the whitespace,
		// also, wrapping it in quotes doesn't seem to work either. So...use "!!!" as the default text
		// and if we find that the value wasn't found and we have "!!!" then use the default value in the
		// parser, else, use what we pulled from the file.
		GetPrivateProfileString(p->lang.c_str(), TEXT("doc_start"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->pd.doc_start = p->pd_default.doc_start;
		else p->pd.doc_start = buffer;

		GetPrivateProfileString(p->lang.c_str(), TEXT("doc_line_"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->pd.doc_line = p->pd_default.doc_line;
		else p->pd.doc_line = buffer;

		GetPrivateProfileString(p->lang.c_str(), TEXT("doc_end__"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->pd.doc_end = p->pd_default.doc_end;
		else p->pd.doc_end = buffer;

		GetPrivateProfileString(p->lang.c_str(), TEXT("command_prefix"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->pd.command_prefix = p->pd_default.command_prefix;
		else p->pd.command_prefix = buffer;

		GetPrivateProfileString(p->lang.c_str(), TEXT("format"), TEXT("!!!"), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		if(strncmp(buffer, "!!!", 3) == 0) p->pd.format = p->pd_default.format;
		else p->pd.format = stringReplace(buffer, "\\r\\n", "\r\n"); // Un-encode "\r\n" as \r\n

		GetPrivateProfileString(p->lang.c_str(), TEXT("align"), BOOLTOSTR(p->pd_default.align_desc), tbuffer, MAX_PATH, iniPath);
		wcstombs(buffer, tbuffer, MAX_PATH);
		p->pd.align_desc = strcmp(buffer, "true") == 0;
	}

	// Write out the file if it doesn't exist yet
	//if(!PathFileExists(iniPath)) configSave();
}



bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool checkOnInit = false)
{
	if (index >= nbFunc || !pFunc) return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = checkOnInit;
	funcItem[index]._pShKey = sk;

	return true;
}

void commandMenuInit()
{
	ShortcutKey *sk = new ShortcutKey();
	sk->_isAlt = sk->_isCtrl = sk->_isShift = TRUE;
	sk->_key = 'D';

	setCommand(0, TEXT("DoxyIt - Function"), doxyItFunction, sk);
	setCommand(1, TEXT("DoxyIt - File"), doxyItFile);
	// ---
	setCommand(3, TEXT("Active commenting"), activeCommenting, NULL, do_active_commenting);
	// ---
	setCommand(5, TEXT("Settings..."), showSettings);
	setCommand(6, TEXT("About..."), showAbout);

	//setCommand(X, TEXT("Use FingerText (if available)"), useFingerText, NULL, use_fingertext);
	//setCommand(X, TEXT("Active word wrapping"), activeWrapping, NULL, do_active_wrapping);
}

void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
}

void pluginInit(HANDLE hModule)
{
	InitializeParsers();

	_hModule = hModule;
}

void pluginCleanUp()
{
	commandMenuCleanUp();
	CleanUpParsers();
}

void setNppInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
	configLoad();
	commandMenuInit();

	// Dialog Init
	sd.init((HINSTANCE) _hModule, nppData);
}



// NOTE: when using this you should do 'fingertext_enabled = checkFingerText();'
// This function could set it explicitly, but that makes the code harder to follow
bool checkFingerText()
{
	if(fingertext_found && use_fingertext)
	{
		CommunicationInfo ci;
		ci.internalMsg = FINGERTEXT_ISENABLED;
		ci.srcModuleName = NPP_PLUGIN_NAME;
		ci.info = NULL;
		SendNpp(NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);
		return ci.info != NULL;
	}
	else
		return false;
}

void activateFingerText()
{
	if(fingertext_enabled)
	{
		CommunicationInfo ci;
		ci.internalMsg = FINGERTEXT_ACTIVATE;
		ci.srcModuleName = NPP_PLUGIN_NAME;
		ci.info = NULL;
		SendNpp(NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);
	}
}


// --- Menu call backs ---

void doxyItFunction()
{
	std::string doc_block;
	int startLine, endLine;
	char *indent = NULL;

	if(!updateScintilla()) return;

	// Check if it is enabled
	fingertext_enabled = checkFingerText();

	doc_block = Parse();

	// Don't issue any warning messages, let Parse() handle that for us since it knows
	// about the error. Just return if it is a zero length string
	if(doc_block.length() == 0) 
		return;

	// Keep track of where we started
	startLine = SendScintilla(SCI_LINEFROMPOSITION, SendScintilla(SCI_GETCURRENTPOS));

	// Get the whitespace of the next line so we can insert it in front of 
	// all the lines of the document block that is going to be inserted
	indent = getLineIndentStr(startLine + 1);
	
	SendScintilla(SCI_BEGINUNDOACTION);
	SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) doc_block.c_str());
	endLine = SendScintilla(SCI_LINEFROMPOSITION, SendScintilla(SCI_GETCURRENTPOS)); // get the end of the document block
	if(indent) insertBeforeLines(indent, startLine, endLine + 1);
	SendScintilla(SCI_ENDUNDOACTION);
	
	activateFingerText();

	if(indent) delete[] indent;
}

void doxyItFile()
{
	std::ostringstream doc_block;
	const ParserDefinition *pd;
	TCHAR fileName[MAX_PATH];
	const char *eol;

	if(!updateScintilla()) return;

	pd = getCurrentParserDefinition();
	if(!pd)
	{
		::MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return;
	}

	// Get the file name
	SendNpp(NPPM_GETFILENAME, MAX_PATH, (LPARAM) fileName);

	eol = getEolStr();

	// Check if it is enabled
	fingertext_enabled = checkFingerText();

	doc_block << pd->doc_start << eol;
	doc_block << pd->doc_line << pd->command_prefix << "file " << toString(fileName) << eol;
	doc_block << pd->doc_line << pd->command_prefix << "brief " << FT("Brief") << eol;
	//doc_block << pd->doc_line << eol;
	//doc_block << pd->doc_line << pd->command_prefix << "author " << eol;
	//doc_block << pd->doc_line << pd->command_prefix << "version 1.0" << eol;
	doc_block << pd->doc_end;

	SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) doc_block.str().c_str());

	activateFingerText();
}

void activeCommenting()
{
	do_active_commenting = !do_active_commenting;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItem[3]._cmdID, (LPARAM) do_active_commenting);
}

void useFingerText()
{
	use_fingertext = !use_fingertext;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItem[4]._cmdID, (LPARAM) use_fingertext);
}

/*
void activeWrapping()
{
	do_active_wrapping = !do_active_wrapping;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItem[3]._cmdID, (LPARAM) do_active_wrapping);
}
*/

void showSettings()
{
	if(!updateScintilla()) return;
	sd.doDialog();
}

void showAbout()
{
	updateScintilla();
	CreateDialog((HINSTANCE) _hModule, MAKEINTRESOURCE(IDD_ABOUTDLG), nppData._nppHandle, abtDlgProc);
}


// --- Notification callbacks ---

void doxyItNewLine()
{
	std::string indentation;
	const ParserDefinition *pd;
	const char *eol;
	char *previousLine, *found = NULL;
	int curLine;

	if(!updateScintilla()) return;

	pd = getCurrentParserDefinition();
	if(!pd) return;

	eol = getEolStr();

	curLine = (int) SendScintilla(SCI_LINEFROMPOSITION, SendScintilla(SCI_GETCURRENTPOS));

	previousLine = getLine(curLine - 1);

	// NOTE: we cannot use getLineIndentStr() because doc_start or doc_line may start with whitespace
	// which we don't want counted towards the indentation string.

	if(found = strstr(previousLine, pd->doc_line.c_str()))
	{
		indentation.append(previousLine, found - previousLine);

		// doc_line should have only whitespace in front of it
		if(isWhiteSpace(indentation))
		{
			SendScintilla(SCI_BEGINUNDOACTION);
			SendScintilla(SCI_DELLINELEFT);	// Clear any automatic indentation
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) indentation.c_str());
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) pd->doc_line.c_str());
			SendScintilla(SCI_ENDUNDOACTION);
			SendScintilla(SCI_CHOOSECARETX);
		}

	}
	// If doc_start is relatively long we do not want the user typing the entire line, just the first 3 should suffice.
	// Also, if doc_end is found, this means a doc block was closed. This allows e.g. /** inline comments */
	else if((found = strstr(previousLine, pd->doc_start.substr(0, 3).c_str())) &&
		strstr(previousLine, pd->doc_end.c_str()) == 0)
	{
		indentation.append(previousLine, found - previousLine);

		if(isWhiteSpace(indentation))
		{
			int pos;
			unsigned int i = 0;

			// Count the characters in common so we can add the rest
			while(i < pd->doc_start.length() && found[i] == pd->doc_start.at(i)) ++i;

			SendScintilla(SCI_BEGINUNDOACTION);
			SendScintilla(SCI_DELLINELEFT);			// Clear any automatic indentation
			SendScintilla(SCI_DELETEBACK);			// Clear the newline
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) &pd->doc_start.c_str()[i]);	// Fill the rest of doc_start
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) eol);
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) indentation.c_str());
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) pd->doc_line.c_str());
			pos = SendScintilla(SCI_GETCURRENTPOS);	// Save this position so we can restore it
			SendScintilla(SCI_LINEEND);				// Skip any text the user carried to next line
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) eol);
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) indentation.c_str());
			SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) pd->doc_end.c_str());
			SendScintilla(SCI_ENDUNDOACTION);
			SendScintilla(SCI_CHOOSECARETX);

			// Restore the position
			SendScintilla(SCI_GOTOPOS, pos);
		}
	}

	delete[] previousLine;
}

void handleNotification(SCNotification *notifyCode)
{
	static bool do_newline = false;
	NotifyHeader nh = notifyCode->nmhdr;
	int ch = notifyCode->ch;

	switch(nh.code)
	{
	case SCN_UPDATEUI: // Now is when we can check to see if we do the commenting
		if(do_newline)
		{
			do_newline = false;
			if(!updateScintilla()) return;
			doxyItNewLine();
		}
		break;
	case SCN_CHARADDED:
		// Set a flag so that all line endings can trigger the commenting
		if((ch == '\r' || ch == '\n') && do_active_commenting) do_newline = true;
		//else if(ch == '\\')
		//{
		//	if(!updateScintilla()) return;
		//	SendScintilla(SCI_AUTOCCANCEL);
		//	SendScintilla(SCI_AUTOCSETSEPARATOR, '|');
		//	SendScintilla(SCI_AUTOCSHOW, 1, (LPARAM) "\\a|\\br|\\brief|\\param|\\return");
		//}
		break;
	case NPPN_READY:
		CommunicationInfo ci;

		// Check if FingerText is installed
		ci.internalMsg = FINGERTEXT_GETVERSION;
		ci.srcModuleName = NPP_PLUGIN_NAME;
		ci.info = NULL;

		// NPPM_MSGTOPLUGIN returns true if the dll is found
		if(SendNpp(NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci))
		{
			if((int) ci.info >= 561) fingertext_found = true;
			else fingertext_found = false;
		}
		else
		{
			fingertext_found = false;
		}
		break;
	case NPPN_SHUTDOWN:
		configSave();
		break;
	case NPPN_BUFFERACTIVATED:
	case NPPN_LANGCHANGED:
		// Don't actually need the parser here, but this forces it to updates the current reference
		getCurrentParser(true);
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
	return;
}