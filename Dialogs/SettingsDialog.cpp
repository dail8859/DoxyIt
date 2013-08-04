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

#include <WindowsX.h>
#include "SettingsDialog.h"
#include "PluginDefinition.h"
#include "Hyperlinks.h"

const TCHAR *msg = TEXT("An option is blank (or all whitespace). If this is desired, it is recommended that you disable Active Commenting! Continue anyways?");
const TCHAR *help = TEXT("\
Format Keywords:\r\n\
$FILENAME - The current file name.\r\n\
$FUNCTION - The name of the function/method.\r\n\
$PARAM - Expands to a single function/method parameter. Any line containing this will get repeated for each parameter.\r\n\
$@ - Expands to the prefix character for Doxygen commands.\r\n\
$| - Marks the alignment position. This is only valid for lines containing $PARAM.\r\n");


INT_PTR CALLBACK inputDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR *buff;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		buff = (TCHAR *) lParam;
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_LANG), 31);
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			Edit_GetText(GetDlgItem(hwndDlg, IDC_EDIT_LANG), buff, 31);
			EndDialog(hwndDlg, wParam);
			return true;
		default:
			return false;
		}
	case WM_DESTROY:
		EndDialog(hwndDlg, wParam);
		return true;
	}

	return false;
}

void SettingsDialog::init(HINSTANCE hInst, NppData nppData)
{
	m_nppData = nppData;
	Window::init(hInst, nppData._nppHandle);
}

void SettingsDialog::doDialog()
{
	if (!isCreated()) create(IDD_SETTINGSDLG);
	goToCenter();

	initParserDefinitions();
	loadParserDefinition();
	updatePreview();
}

void SettingsDialog::initParserDefinitions()
{
	parserDefinitions.clear();
	for(unsigned int i = 0; i < parsers.size(); ++i)
		parserDefinitions.push_back(parsers[i]->pd);
}

void SettingsDialog::saveParserDefinition(int index)
{
	TCHAR text[256]; // Edit_LimitText is used to limit to 255 chars

	// Save the text from the edit controls for the previous selection
	ParserDefinition *prev_pd = &parserDefinitions[index];

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_START), text, 256);
	prev_pd->doc_start = toString(text);

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), text, 256);
	prev_pd->doc_line = toString(text);

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_END), text, 256);
	prev_pd->doc_end = toString(text);

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), text, 256);
	prev_pd->command_prefix = toString(text);

	if(!parsers[index]->external)
	{
		TCHAR *dtext;
		int len;

		len = Edit_GetTextLength(GetDlgItem(_hSelf, IDC_EDIT_FORMAT)) + 1;
		dtext = (TCHAR *) malloc(sizeof(TCHAR) * len);
		Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), dtext, len);

		if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED)
			prev_pd->function_format = toString(dtext);
		else
			prev_pd->file_format = toString(dtext);

		free(dtext);

		prev_pd->align = (Button_GetCheck(GetDlgItem(_hSelf, IDC_CHB_ALIGN)) == BST_CHECKED ? true : false);
	}
}

// Note: Setting the text of edit boxes causes notifications to be generated, which update the 
// preview multiple times when calling loadParserDefinition(). The 'updating' flag is used to 
// temporarily ignore notifications.
void SettingsDialog::loadParserDefinition()
{
	int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	current = &parserDefinitions[index];

	// Load the edit controls with the new parsers settings
	m_updating = true;

	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_START), toWideString(current->doc_start).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), toWideString(current->doc_line).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_END), toWideString(current->doc_end).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), toWideString(current->command_prefix).c_str());

	if(!parsers[index]->external)
	{
		m_updating = false; // NOTE: if the main proc ignores the following messages, it does not redraw them correctly
		SendDlgItemMessage(_hSelf, IDC_EDIT_FORMAT, EM_SETREADONLY, FALSE, 0);
		EnableWindow(GetDlgItem(_hSelf, IDC_RAD_FUNCTION), TRUE);
		EnableWindow(GetDlgItem(_hSelf, IDC_RAD_FILE), TRUE);
		EnableWindow(GetDlgItem(_hSelf, IDC_CHB_ALIGN), TRUE);
		EnableWindow(GetDlgItem(_hSelf, IDC_BTN_REMOVE), FALSE);
		m_updating = true;

		Button_SetCheck(GetDlgItem(_hSelf, IDC_CHB_ALIGN), current->align); // Cannot be last!  Doesn't update preview

		if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED)
			Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(current->function_format).c_str());
		else
			Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(current->file_format).c_str());
	}
	else
	{
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), TEXT("User added languages are limited\r\nto active commenting only."));
		Button_SetCheck(GetDlgItem(_hSelf, IDC_CHB_ALIGN), BST_UNCHECKED);

		m_updating = false; // NOTE: if the main proc ignores the following messages, it does not redraw them correctly
		SendDlgItemMessage(_hSelf, IDC_EDIT_FORMAT, EM_SETREADONLY, TRUE, 0);
		EnableWindow(GetDlgItem(_hSelf, IDC_RAD_FUNCTION), FALSE);
		EnableWindow(GetDlgItem(_hSelf, IDC_RAD_FILE), FALSE);
		EnableWindow(GetDlgItem(_hSelf, IDC_CHB_ALIGN), FALSE);
		EnableWindow(GetDlgItem(_hSelf, IDC_BTN_REMOVE), TRUE);
		m_updating = true; // just to be safe incase we add anything later
	}

	m_updating = false;
}

void SettingsDialog::removeParserDefinition()
{
	int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	parserDefinitions.erase(parserDefinitions.begin() + index);
	parsers.erase(parsers.begin() + index);

	ComboBox_DeleteString(GetDlgItem(_hSelf, IDC_CMB_LANG), index);
	ComboBox_SetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG), index - 1);
	m_previousSelection = index - 1;

	loadParserDefinition();
	updatePreview();
}

bool SettingsDialog::validateText(std::string text, int idc)
{
	if(text.length() != 0 && !isWhiteSpace(text))
		return true;

	SetFocus(GetDlgItem(_hSelf, idc));
	return false;
}
bool SettingsDialog::validateSettings()
{
	for(unsigned int i = 0; i < parsers.size(); ++i)
	{
		bool ret = true;
		const ParserDefinition *pd = &parserDefinitions[i];

		if(!validateText(pd->doc_start, IDC_EDIT_START)) ret = false;
		if(!validateText(pd->doc_line, IDC_EDIT_LINE)) ret = false;
		if(!validateText(pd->doc_end, IDC_EDIT_END)) ret = false;
		if(!validateText(pd->command_prefix, IDC_EDIT_PREFIX)) ret = false;

		if(!ret)
		{
			ComboBox_SelectString(GetDlgItem(_hSelf, IDC_CMB_LANG), -1, parsers[i]->language_name.c_str());
			loadParserDefinition();
			return false;
		}
	}

	return true;
}

void SettingsDialog::saveSettings()
{
	for(unsigned int i = 0; i < parsers.size(); ++i)
		parsers[i]->pd = parserDefinitions[i];
}

void SettingsDialog::updatePreview()
{
	std::string block;
	int prev_eol_mode;
	int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	if(parsers[index]->external)
	{
		block = current->doc_start + "\r\n" + current->doc_line + current->command_prefix + "note Example text\r\n" + current->doc_end;
	}
	else
	{
		// Set eol mode to "\r\n" so it will display correctly in the dialogbox
		prev_eol_mode = SendScintilla(SCI_GETEOLMODE);
		SendScintilla(SCI_SETEOLMODE, SC_EOL_CRLF);

		if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED)
		{
			// Get the parser and have it parse the example
			const Parser *p = parsers[index];
			block = FormatFunctionBlock(p, current, p->example.c_str());
			block += "\r\n" + p->example;
		}
		else // IDC_RAD_FILE is set
		{
			block = FormatFileBlock(current);
		}

		// Restore the eol mode
		SendScintilla(SCI_SETEOLMODE, prev_eol_mode);
	}

	// Set the preview
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), toWideString(block).c_str());
}

void SettingsDialog::swapFormat()
{
	TCHAR *text;
	unsigned int len;
	static int last_rad = IDC_RAD_FUNCTION; // In case the selected radio button is clicked again

	// Get the text length, dynamically allocate it
	len = Edit_GetTextLength(GetDlgItem(_hSelf, IDC_EDIT_FORMAT)) + 1;
	text = new TCHAR[len];
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), text, len);

	if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED && last_rad == IDC_RAD_FILE)
	{
		current->file_format = toString(text);
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(current->function_format).c_str());
		last_rad = IDC_RAD_FUNCTION;
	}
	else if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FILE)) == BST_CHECKED && last_rad == IDC_RAD_FUNCTION)
	{
		current->function_format = toString(text);
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(current->file_format).c_str());
		last_rad = IDC_RAD_FILE;
	}

	delete[] text;
}

BOOL CALLBACK SettingsDialog::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(m_updating) return true;

	switch(message) 
	{
	case WM_INITDIALOG:
		{
			HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
			long lfHeight = -MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);

			for(unsigned int i = 0; i < parsers.size(); ++i)
			{
				if(parsers[i]->external)
				{
					std::basic_string<TCHAR> name = parsers[i]->language_name + TEXT("*");
					ComboBox_AddString(cmb, name.c_str());
				}
				else
				{
					ComboBox_AddString(cmb, parsers[i]->language_name.c_str());
				}
			}
			ComboBox_SetCurSel(cmb, m_previousSelection);
			
			// I have no idea what these values do, but these work
			m_monoFont = CreateFont(lfHeight,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("Courier New"));
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_START), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_LINE), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_END), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_BTN_ADD), m_monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_BTN_REMOVE), m_monoFont, false);

			// Limit the input boxes
			Edit_LimitText(GetDlgItem(_hSelf, IDC_EDIT_START), 255);
			Edit_LimitText(GetDlgItem(_hSelf, IDC_EDIT_LINE), 255);
			Edit_LimitText(GetDlgItem(_hSelf, IDC_EDIT_END), 255);
			Edit_LimitText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), 1);

			Button_SetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION), BST_CHECKED);

			ConvertStaticToHyperlink(_hSelf, IDC_SETTINGS_HELP);

			return true;
		}
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
		case CBN_SELCHANGE:
			saveParserDefinition(m_previousSelection);
			loadParserDefinition();
			updatePreview();
			m_previousSelection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
			return true;
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDOK:
				m_previousSelection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
				saveParserDefinition(m_previousSelection);
				if(validateSettings())
				{
					saveSettings();
					display(false);
				}
				else if(MessageBox(_hSelf, msg, NPP_PLUGIN_NAME, MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
				{
					saveSettings();
					display(false);
				}
				return true;
			case IDCANCEL:
				display(false);
				return true;
			case IDC_BTN_ADD:
			{
				TCHAR buff[32];
				ParserDefinition pd;
				HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
				int index;

				DialogBoxParam((HINSTANCE) _hInst, MAKEINTRESOURCE(IDD_NEWLANG), _hSelf, inputDlgProc, (LPARAM) buff);
				MessageBox(NULL, buff, NPP_PLUGIN_NAME, MB_OK);

				// do data validation
				pd.doc_start = "/**";
				pd.doc_line  = " *  ";
				pd.doc_end   = " */";
				pd.command_prefix = "\\";

				addNewParser(toString(buff), &pd);
				parserDefinitions.push_back(parsers.back()->pd);

				std::basic_string<TCHAR> name = parsers.back()->language_name + TEXT("*");
				index = ComboBox_AddString(cmb, name.c_str());
				ComboBox_SetCurSel(cmb, index);

				saveParserDefinition(m_previousSelection);
				loadParserDefinition();
				updatePreview();
				m_previousSelection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

				return true;
			}
			case IDC_BTN_REMOVE:
			{
				int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
				std::basic_string<TCHAR> message = TEXT("Do you want to remove ") + parsers[index]->language_name + TEXT("?");
				if(MessageBox(_hSelf, message.c_str(), NPP_PLUGIN_NAME, MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
					removeParserDefinition();
				return true;
			}
			case IDC_CHB_ALIGN:
				saveParserDefinition(ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG)));
				updatePreview();
				return true;
			case IDC_RAD_FUNCTION:
			case IDC_RAD_FILE:
				swapFormat();
				updatePreview();
				return true;
			case IDC_SETTINGS_HELP:
				MessageBox(NULL, help, NPP_PLUGIN_NAME, MB_OK);
				return true;
			}
		case EN_CHANGE:
			saveParserDefinition(ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG)));
			updatePreview();
			return true;
		case EN_MAXTEXT:
			MessageBeep(MB_ICONERROR);
			return true;
		}
	default:
		return StaticDialog::dlgProc(m_HSource, message, wParam, lParam);
	}

	return false;
}
