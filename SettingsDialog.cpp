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

void SettingsDialog::doDialog()
{
	if (!isCreated())
	{
		create(IDD_SETTINGSDLG);
	}
	goToCenter();

	initParserDefinitions();
	loadParserDefinition();
}

void SettingsDialog::initParserDefinitions()
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	
	parserDefinitions.clear();
	for(int i = 0; i < len; ++i)
	{
		ParserDefinition pd;
		Parser *p = &parsers[i];

		pd.doc_start.assign(p->doc_start.begin(), p->doc_start.end());
		pd.doc_line.assign(p->doc_line.begin(), p->doc_line.end());
		pd.doc_end.assign(p->doc_end.begin(), p->doc_end.end());
		pd.command_prefix.assign(p->command_prefix.begin(), p->command_prefix.end());

		parserDefinitions[p->lang] = pd;
	}
}


void SettingsDialog::savePreviousParserDefinition()
{
	HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
	wchar_t prev_name[32];
	wchar_t text[256];

	// Save the text from the edit controls for the previous selection
	ComboBox_GetLBText(cmb, last_selection, prev_name);
	ParserDefinition *prev_pd = &parserDefinitions[prev_name];
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_START), text, 256);
	prev_pd->doc_start.assign(text);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), text, 256);
	prev_pd->doc_line.assign(text);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_END), text, 256);
	prev_pd->doc_end.assign(text);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), text, 256);
	prev_pd->command_prefix.assign(text);
}

void SettingsDialog::loadParserDefinition()
{
	wchar_t name[32];
	HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);

	// Load the edit controls with the new parsers settings
	ComboBox_GetText(cmb, name, 32);
	ParserDefinition pd = parserDefinitions[name];
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_START), pd.doc_start.c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), pd.doc_line.c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_END), pd.doc_end.c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), pd.command_prefix.c_str());

	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), TEXT("TODO"));
	
}

void SettingsDialog::saveSettings()
{
	last_selection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
	savePreviousParserDefinition();

	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
	{
		Parser *p = &parsers[i];
		ParserDefinition pd = parserDefinitions[p->lang];

		p->doc_start.assign(pd.doc_start.begin(), pd.doc_start.end());
		p->doc_line.assign(pd.doc_line.begin(), pd.doc_line.end());
		p->doc_end.assign(pd.doc_end.begin(), pd.doc_end.end());
		p->command_prefix.assign(pd.command_prefix.begin(), pd.command_prefix.end());
	}
}

// http://msdn.microsoft.com/en-us/library/ff485897%28v=vs.85%29.aspx
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

			mono = CreateFont(0,10,0,0,0,0,0,0,0,0,0,0,0,TEXT("Courier New"));
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
				{
					savePreviousParserDefinition();
					loadParserDefinition();
					last_selection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
					return true;
				}
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
