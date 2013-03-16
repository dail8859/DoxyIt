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
#include <windowsx.h>
#include "SettingsDialog.h"
#include "Parsers.h"
#include "PluginDefinition.h"

extern bool fingertext_enabled;

void SettingsDialog::doDialog()
{
	if (!isCreated()) create(IDD_SETTINGSDLG);
	goToCenter();

	initParserDefinitions();
	loadParserDefinition();
}

void SettingsDialog::initParserDefinitions()
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	
	parserDefinitions.clear();
	for(int i = 0; i < len; ++i)
		parserDefinitions[parsers[i].lang] = parsers[i].pd;
}

void SettingsDialog::saveParserDefinition(int index)
{
	HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
	wchar_t prev_name[32];
	wchar_t text[256];

	// Save the text from the edit controls for the previous selection
	ComboBox_GetLBText(cmb, index, prev_name);
	ParserDefinition *prev_pd = &parserDefinitions[prev_name];
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_START), text, 256);
	prev_pd->doc_start = toString(text);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), text, 256);
	prev_pd->doc_line = toString(text);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_END), text, 256);
	prev_pd->doc_end = toString(text);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), text, 256);
	prev_pd->command_prefix = toString(text);
}

void SettingsDialog::loadParserDefinition()
{
	wchar_t name[32];
	HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);

	// Load the edit controls with the new parsers settings
	ComboBox_GetText(cmb, name, 32);
	ParserDefinition pd = parserDefinitions[name];
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_START), toWideString(pd.doc_start).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), toWideString(pd.doc_line).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_END), toWideString(pd.doc_end).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), toWideString(pd.command_prefix).c_str());
}

void SettingsDialog::saveSettings()
{
	last_selection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
	saveParserDefinition(last_selection);

	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		parsers[i].pd = parserDefinitions[parsers[i].lang];
}

// HACK: This probably isn't a good way of doing it, but this will work for now
void SettingsDialog::updatePreview()
{
	ParserDefinition *pd;
	wchar_t name[32];
	HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
	int prev_eol_mode;

	ComboBox_GetText(cmb, name, 32);
	pd = &parserDefinitions[name];
	
	// Disable fingertext for the preview
	fingertext_enabled = false;

	// Set eol mode to "\r\n" so it will display correctly in the dialogbox
	prev_eol_mode = SendScintilla(SCI_GETEOLMODE);
	SendScintilla(SCI_SETEOLMODE, SC_EOL_CRLF);

	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
	{
		if(parsers[i].lang == name)
		{
			// Pass in the ParserDefinition held by the Dialog box
			std::string block = parsers[i].callback(pd, parsers[i].example.c_str());
			block += "\r\n" + parsers[i].example;
			
			std::wstring wblock(block.begin(), block.end());
			
			Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), wblock.c_str());
			break;
		}
	}

	SendScintilla(SCI_SETEOLMODE, prev_eol_mode);
}


BOOL CALLBACK SettingsDialog::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
		case WM_INITDIALOG:
		{
			HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
			int len = sizeof(parsers) / sizeof(parsers[0]);

			for(int i = 0; i < len; ++i)
				ComboBox_AddString(cmb, parsers[i].lang.c_str());
			ComboBox_SetCurSel(cmb, last_selection);

			// I have no idea what these values do, but these work
			mono = CreateFont(16,8,0,0,0,0,0,0,0,0,0,0,0,TEXT("Courier New"));
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_START), mono, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_LINE), mono, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_END), mono, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), mono, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), mono, false);

			return true;
		}
		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case CBN_SELCHANGE:
					saveParserDefinition(last_selection);
					loadParserDefinition();
					last_selection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
					return true;
				case EN_CHANGE:
					saveParserDefinition(ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG)));
					updatePreview();
					return true;
			}
			switch(wParam)
			{
				case IDOK:
					saveSettings();
					display(false);
					return true;
				case IDCANCEL:
					display(false);
					return true;
				default:
					return false;
			}
		default:
			return StaticDialog::dlgProc(_HSource, message, wParam, lParam);
	}
}
