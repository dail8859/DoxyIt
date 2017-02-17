// This file is part of DoxyIt.
// 
// Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
// 
// DoxyIt is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "Utils.h"
#include "Parsers.h"
#include "Version.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "JumpLocations.h"

// --- Local variables ---
static bool do_active_commenting;	// active commenting - create or extend a document block
//static bool do_active_wrapping;	// active wrapping - wrap text inside of document blocks...todo

static NppData nppData;
static SettingsDialog sd;			// The settings dialog
static HANDLE _hModule;				// For dialog initialization
static HHOOK hook = NULL;
static bool hasFocus = true;
// --- Menu callbacks ---
static void doxyItFunction();
static void doxyItFile();
static void activeCommenting();
//static void activeWrapping();
static void showSettings();
static void showAbout();

ScintillaGateway editor;

// --- Global variables ---
ShortcutKey sk = {true, true, true, 'D'};
FuncItem funcItem[nbFunc] = {
	{TEXT("DoxyIt - Function"), doxyItFunction,   0, false, &sk},
	{TEXT("DoxyIt - File"),     doxyItFile,       0, false, NULL},
	{TEXT(""),                  NULL,             0, false, NULL}, // separator
	{TEXT("Active commenting"), activeCommenting, 0, false, NULL},
	{TEXT(""),                  NULL,             0, false, NULL}, // separator
	{TEXT("Settings..."),       showSettings,     0, false, NULL},
	{TEXT("About..."),          showAbout,        0, false, NULL}
};

inline LRESULT SendNpp(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(nppData._nppHandle, Msg, wParam, lParam);
}

static HWND getCurrentScintilla()
{
	int which = 0;
	SendNpp(NPPM_GETCURRENTSCINTILLA, SCI_UNUSED, (LPARAM)&which);
	return (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

// --- Configuration ---

void getIniFilePath(wchar_t *iniPath, int size)
{
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, size, (LPARAM) iniPath);
	wcscat_s(iniPath, size, TEXT("\\"));
	wcscat_s(iniPath, size, NPP_PLUGIN_NAME);
	wcscat_s(iniPath, size, TEXT(".ini"));
}

void configSave()
{
	wchar_t iniPath[MAX_PATH];
	std::wstring ws;

	getIniFilePath(iniPath, MAX_PATH);

	// Completely delete the file
	DeleteFile(iniPath);

	// [DoxyIt]
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("active_commenting"), BOOLTOSTR(do_active_commenting), iniPath);
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("version"), VERSION_LINEAR_TEXT, iniPath);
	WritePrivateProfileString(NPP_PLUGIN_NAME, TEXT("version_stage"), VERSION_STAGE, iniPath);

	for(auto const &p : parsers)
	{
		const ParserSettings *ps = &(p.ps);

		// Wrap everything in quotes to preserve whitespace
		ws = TEXT("\"") + toWideString(ps->doc_start) + TEXT("\"");
		WritePrivateProfileString(p.lang.c_str(), TEXT("doc_start"), ws.c_str(), iniPath);

		ws = TEXT("\"") + toWideString(ps->doc_line) + TEXT("\"");
		WritePrivateProfileString(p.lang.c_str(), TEXT("doc_line_"), ws.c_str(), iniPath);

		ws = TEXT("\"") + toWideString(ps->doc_end) + TEXT("\"");
		WritePrivateProfileString(p.lang.c_str(), TEXT("doc_end__"), ws.c_str(), iniPath);

		ws = TEXT("\"") + toWideString(ps->command_prefix) + TEXT("\"");
		WritePrivateProfileString(p.lang.c_str(), TEXT("command_prefix"), ws.c_str(), iniPath);

		// Encode \r\n as literal "\r\n" in the ini file
		ws = TEXT("\"") + toWideString(stringReplace(std::string(ps->file_format), "\r\n", "\\r\\n")) + TEXT("\"");
		WritePrivateProfileString(p.lang.c_str(), TEXT("file_format"), ws.c_str(), iniPath);

		// Encode \r\n as literal "\r\n" in the ini file
		ws = TEXT("\"") + toWideString(stringReplace(std::string(ps->function_format), "\r\n", "\\r\\n")) + TEXT("\"");
		WritePrivateProfileString(p.lang.c_str(), TEXT("function_format"), ws.c_str(), iniPath);

		// Write out internal parser attributes
		if(!p.external)
		{
			WritePrivateProfileString(p.lang.c_str(), TEXT("align"), BOOLTOSTR(ps->align), iniPath);
		}
		else // add it to the list of external settings
		{
			WritePrivateProfileString(TEXT("External"), p.language_name.c_str(), TEXT(""), iniPath);
		}
	}
}

void configLoad()
{
	wchar_t iniPath[MAX_PATH];
	wchar_t tbuffer[2048]; // "relatively" large
	wchar_t tbuffer2[2048];
	wchar_t *current;
	ParserSettings ps;

	getIniFilePath(iniPath, MAX_PATH);

	// [DoxyIt]
	GetPrivateProfileString(NPP_PLUGIN_NAME, TEXT("active_commenting"), TEXT("true"), tbuffer, MAX_PATH, iniPath);
	do_active_commenting = (lstrcmp(tbuffer, TEXT("true")) == 0);

	// NPPM_SETMENUITEMCHECK does not seem to work unless the 
	// menu item is actually clicked, so lets do it manually
	if(do_active_commenting) CheckMenuItem(GetMenu(nppData._nppHandle), funcItem[3]._cmdID, MF_CHECKED);

	// Don't need these for now
	//version = GetPrivateProfileInt(NPP_PLUGIN_NAME, TEXT("version"), 0, iniPath);
	//version_stage = GetPrivateProfileString(NPP_PLUGIN_NAME, TEXT("version_stage"), TEXT(""), tbuffer, MAX_PATH, iniPath);

	for(auto &p : parsers)
	{
		// NOTE: We cant use the default value because GetPrivateProfileString strips the whitespace,
		// also, wrapping it in quotes doesn't seem to work either. So...use "!!!" as the default text
		// and if we find that the value wasn't found and we have "!!!" then use the default value in the
		// parser, else, use what we pulled from the file.
		GetPrivateProfileString(p.lang.c_str(), TEXT("doc_start"), TEXT("!!!"), tbuffer, 2048, iniPath);
		if(lstrcmp(tbuffer, TEXT("!!!")) != 0) p.ps.doc_start = toString(tbuffer);

		GetPrivateProfileString(p.lang.c_str(), TEXT("doc_line_"), TEXT("!!!"), tbuffer, 2048, iniPath);
		if(lstrcmp(tbuffer, TEXT("!!!")) != 0) p.ps.doc_line = toString(tbuffer);

		GetPrivateProfileString(p.lang.c_str(), TEXT("doc_end__"), TEXT("!!!"), tbuffer, 2048, iniPath);
		if(lstrcmp(tbuffer, TEXT("!!!")) != 0) p.ps.doc_end = toString(tbuffer);

		GetPrivateProfileString(p.lang.c_str(), TEXT("command_prefix"), TEXT("!!!"), tbuffer, 2048, iniPath);
		if(lstrcmp(tbuffer, TEXT("!!!")) != 0) p.ps.command_prefix = toString(tbuffer);

		GetPrivateProfileString(p.lang.c_str(), TEXT("function_format"), TEXT("!!!"), tbuffer, 2048, iniPath);
		if(lstrcmp(tbuffer, TEXT("!!!")) != 0) p.ps.function_format = stringReplace(toString(tbuffer), "\\r\\n", "\r\n");

		GetPrivateProfileString(p.lang.c_str(), TEXT("file_format"), TEXT("!!!"), tbuffer, 2048, iniPath);
		if(lstrcmp(tbuffer, TEXT("!!!")) != 0) p.ps.file_format = stringReplace(toString(tbuffer), "\\r\\n", "\r\n");

		GetPrivateProfileString(p.lang.c_str(), TEXT("align"), BOOLTOSTR(p.ps.align), tbuffer, 2048, iniPath);
		p.ps.align = (lstrcmp(tbuffer, TEXT("true")) == 0);
	}


	GetPrivateProfileSection(TEXT("External"), tbuffer, 2048, iniPath);
	current = tbuffer;
	while(current[0] != NULL)
	{
		wchar_t *equals = wcschr(current, TEXT('='));

		// Temporarily remove the '=' that was found
		*equals = NULL;

		GetPrivateProfileString(current, TEXT("doc_start"), TEXT("/**"), tbuffer2, 2048, iniPath);
		ps.doc_start = toString(tbuffer2);

		GetPrivateProfileString(current, TEXT("doc_line_"), TEXT(" *  "), tbuffer2, 2048, iniPath);
		ps.doc_line = toString(tbuffer2);

		GetPrivateProfileString(current, TEXT("doc_end__"), TEXT(" */"), tbuffer2, 2048, iniPath);
		ps.doc_end = toString(tbuffer2);

		GetPrivateProfileString(current, TEXT("command_prefix"), TEXT("\\"), tbuffer2, 2048, iniPath);
		ps.command_prefix = toString(tbuffer2);

		GetPrivateProfileString(current, TEXT("function_format"), TEXT("!!!"), tbuffer2, 2048, iniPath);
		if(lstrcmp(tbuffer2, TEXT("!!!")) != 0) ps.function_format = stringReplace(toString(tbuffer2), "\\r\\n", "\r\n");
		else ps.function_format = default_internal_function_format;

		GetPrivateProfileString(current, TEXT("file_format"), TEXT("!!!"), tbuffer2, 2048, iniPath);
		if(lstrcmp(tbuffer2, TEXT("!!!")) != 0) ps.file_format = stringReplace(toString(tbuffer2), "\\r\\n", "\r\n");
		else ps.file_format = default_file_format;

		addNewParser(toString(current), &ps);

		// add back in the equals so we can correctly calculate the length
		*equals = TEXT('=');

		current = &current[lstrlen(current) + 1];
	}
}

void pluginInit(HANDLE hModule)
{
	_hModule = hModule;
}

void pluginCleanUp()
{
}

void setNppInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;

	// Set this as early as possible so its in a valid state
	editor.SetScintillaInstance(nppData._scintillaMainHandle);

	sd.init((HINSTANCE) _hModule, nppData);
}


// --- Menu call backs ---

void doxyItFunction()
{
	const Parser *p;
	if (!(p = getCurrentParser()))
	{
		MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		return;
	}

	std::string text = GetFunctionToParse();

	// External parsers have no text to parse
	if (!p->external && text.empty())
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition. Make sure the cursor is on the line directly above the function or method definition."), NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		return;
	}

	std::string doc_block = FormatFunctionBlock(p, &p->ps, text.c_str());

	if (doc_block.length() == 0)
	{
		MessageBox(NULL, TEXT("Error: Cannot parse function definition. Make sure the cursor is on the line directly above the function or method definition."), NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		return;
	}

	editor.BeginUndoAction();

	auto indentation = GetLineIndentString(editor.LineFromPosition(editor.GetSelectionEnd()) + 1);
	auto range = InsertDocumentationBlock(doc_block, indentation);
	ProcessTextRangeForNewJumpLocations(range.first, range.second);

	editor.EndUndoAction();
}

void doxyItFile()
{
	std::string doc_block;
	const ParserSettings *ps;

	ps = getCurrentParserSettings();
	if(!ps)
	{
		MessageBox(NULL, TEXT("Unrecognized language type."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return;
	}

	doc_block = FormatFileBlock(ps);

	editor.BeginUndoAction();

	auto range = InsertDocumentationBlock(doc_block, std::string(""));
	ProcessTextRangeForNewJumpLocations(range.first, range.second);

	editor.EndUndoAction();
}

void activeCommenting()
{
	do_active_commenting = !do_active_commenting;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItem[3]._cmdID, (LPARAM) do_active_commenting);
}

void showSettings()
{
	sd.doDialog();
}

static void showAbout() {
	HWND hSelf = CreateDialogParam((HINSTANCE)_hModule, MAKEINTRESOURCE(IDD_ABOUTDLG), nppData._nppHandle, (DLGPROC)abtDlgProc, (LPARAM)NULL);

	// Go to center
	RECT rc;
	::GetClientRect(nppData._nppHandle, &rc);
	POINT center;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	center.x = rc.left + w / 2;
	center.y = rc.top + h / 2;
	::ClientToScreen(nppData._nppHandle, &center);

	RECT dlgRect;
	::GetClientRect(hSelf, &dlgRect);
	int x = center.x - (dlgRect.right - dlgRect.left) / 2;
	int y = center.y - (dlgRect.bottom - dlgRect.top) / 2;

	::SetWindowPos(hSelf, HWND_TOP, x, y, (dlgRect.right - dlgRect.left), (dlgRect.bottom - dlgRect.top), SWP_SHOWWINDOW);
}


// --- Notification callbacks ---

void doxyItNewLine()
{
	std::string indentation;
	const ParserSettings *ps;
	const char *eol;
	char *previousLine, *found = NULL;
	int curLine;

	ps = getCurrentParserSettings();
	if(!ps) return;

	eol = getEolStr();

	curLine = static_cast<int>(SendNpp(NPPM_GETCURRENTLINE));

	previousLine = GetLine(curLine - 1);

	// NOTE: we cannot use getLineIndentStr() because doc_start or doc_line may start with whitespace
	// which we don't want counted towards the indentation string.

	if(found = strstr(previousLine, ps->doc_line.c_str()))
	{
		indentation.append(previousLine, found - previousLine);

		// doc_line should have only whitespace in front of it
		if(isWhiteSpace(indentation))
		{
			editor.BeginUndoAction();
			editor.DelLineLeft(); // Clear any automatic indentation
			editor.ReplaceSel(indentation.c_str());
			editor.ReplaceSel(ps->doc_line.c_str());
			editor.EndUndoAction();
			editor.ChooseCaretX();
		}

	}
	// If doc_start is relatively long we do not want the user typing the entire line, just the first 3 should suffice.
	// Also, if doc_end is found, this means a doc block was closed. This allows e.g. /** inline comments */
	else if((found = strstr(previousLine, ps->doc_start.substr(0, 3).c_str())) &&
		strstr(previousLine, ps->doc_end.c_str()) == 0)
	{
		indentation.append(previousLine, found - previousLine);

		if(isWhiteSpace(indentation))
		{
			bool wasTriggeredAboveFunction = !GetFunctionToParse().empty();

			if (wasTriggeredAboveFunction)
			{
				// The new block was triggered above a function, so do all that magic
				editor.DelLineLeft(); // Clear any automatic indentation
				editor.CharLeft(); // Back up over the new line
				editor.LineDelete(); // Clear the entire line
				doxyItFunction();
			}
			else
			{
				int pos;
				unsigned int i = 0;

				// Count the characters in common so we can add the rest
				while (i < ps->doc_start.length() && found[i] == ps->doc_start.at(i)) ++i;

				// Just open a blank block
				editor.BeginUndoAction();
				editor.DelLineLeft(); // Clear any automatic indentation
				editor.DeleteBack(); // Clear the newline
				editor.ReplaceSel(&ps->doc_start.c_str()[i]); // Fill the rest of doc_start
				editor.ReplaceSel(eol);
				editor.ReplaceSel(indentation.c_str());
				editor.ReplaceSel(ps->doc_line.c_str());
				pos = editor.GetCurrentPos(); // Save this position so we can restore it
				editor.LineEnd(); // Skip any text the user carried to next line
				editor.ReplaceSel(eol);
				editor.ReplaceSel(indentation.c_str());
				editor.ReplaceSel(ps->doc_end.c_str());
				editor.EndUndoAction();
				editor.ChooseCaretX();

				// Restore the position
				editor.GotoPos(pos);
			}
		}
	}

	delete[] previousLine;
}

LRESULT CALLBACK KeyboardProc(int ncode, WPARAM wparam, LPARAM lparam)
{
	if (ncode == HC_ACTION)
	{
		if ((HIWORD(lparam) & KF_UP) == 0)
		{
			if (hasFocus)
			{
				if (!(GetKeyState(VK_SHIFT) & KF_UP) && !(GetKeyState(VK_CONTROL) & KF_UP) && !(GetKeyState(VK_MENU) & KF_UP))
				{
					if (wparam == VK_TAB)
					{
						if (GoToNextJumpLocation(editor.GetCurrentPos()))
							return TRUE; // This key has been "handled" and won't propogate
					}
					else if (wparam == VK_ESCAPE)
					{
						ClearJumpLocations();
					}
				}
			}
		}
	}
	return CallNextHookEx(hook, ncode, wparam, lparam); //pass control to next hook in the hook chain.
}

void handleNotification(SCNotification *notifyCode)
{
	static bool do_newline = false;
	NotifyHeader nh = notifyCode->nmhdr;
	int ch = notifyCode->ch;

	switch(nh.code)
	{
	case SCN_UPDATEUI: // Now is when we can check to see if we do the commenting
		if (do_newline)
		{
			do_newline = false;
			doxyItNewLine();
		}
		break;
	case SCN_CHARADDED:
		// Set a flag so that all line endings can trigger the commenting
		if((ch == '\r' || ch == '\n') && do_active_commenting) do_newline = true;
		break;
	case SCN_FOCUSIN:
		hasFocus = true;
		break;
	case SCN_FOCUSOUT:
		hasFocus = false;
		break;
	case NPPN_READY:
		InitializeParsers();
		configLoad();
		getCurrentParser(true);
		hook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, (HINSTANCE)_hModule, ::GetCurrentThreadId());
		SendMessage(nppData._scintillaMainHandle, SCI_INDICSETSTYLE, JUMPLOCATION_INDICATOR, INDIC_DOTBOX);
		SendMessage(nppData._scintillaSecondHandle, SCI_INDICSETSTYLE, JUMPLOCATION_INDICATOR, INDIC_DOTBOX);
		SendMessage(nppData._scintillaMainHandle , SCI_INDICSETALPHA, JUMPLOCATION_INDICATOR, 30);
		SendMessage(nppData._scintillaSecondHandle, SCI_INDICSETALPHA, JUMPLOCATION_INDICATOR, 30);
		SendMessage(nppData._scintillaMainHandle  , SCI_INDICSETOUTLINEALPHA, JUMPLOCATION_INDICATOR, 180);
		SendMessage(nppData._scintillaSecondHandle, SCI_INDICSETOUTLINEALPHA, JUMPLOCATION_INDICATOR, 180);
		break;
	case NPPN_SHUTDOWN:
		configSave();
		CleanUpParsers();
		if (hook != NULL) UnhookWindowsHookEx(hook);
		break;
	case NPPN_BUFFERACTIVATED:
		editor.SetScintillaInstance(getCurrentScintilla());
		/* fall though */
	case NPPN_LANGCHANGED:
		// Don't actually need the parser here, but this forces it to updates the current reference
		getCurrentParser(true);
		break;
	}
	return;
}
