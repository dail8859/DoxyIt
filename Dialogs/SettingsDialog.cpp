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

const wchar_t *msg = TEXT("An option is blank (or all whitespace). If this is desired, it is recommended that you disable Active Commenting! Continue anyways?");
const wchar_t *help = TEXT("\
Format Keywords:\r\n\
$FILENAME - The current file name.\r\n\
$FUNCTION - The name of the function/method.\r\n\
$PARAM - Expands to a single function/method parameter. Any line containing this will get repeated for each parameter.\r\n\
$COMPUTER - Current computer name.\r\n\
$USER - User name of current user.\r\n\
$DATE - Current date and time.\r\n\
$YEAR - Part of date: year\r\n\
$@ - Expands to the prefix character for Doxygen commands.\r\n\
$| - Marks the alignment position. This is only valid for lines containing $PARAM.\r\n");


INT_PTR CALLBACK inputDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_LANG));
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_LANG), 30);
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDOK:
			{
				wchar_t *text;
				int len = Edit_GetTextLength(GetDlgItem(hwndDlg, IDC_EDIT_LANG));

				text = new wchar_t[len + 1];
				Edit_GetText(GetDlgItem(hwndDlg, IDC_EDIT_LANG), text, len + 1);
				EndDialog(hwndDlg, (INT_PTR) text);
				return true;
			}
			default:
				return false;
		}
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwndDlg, NULL);
		return true;
	}

	return false;
}

void SettingsDialog::init(HINSTANCE hInst, NppData nppData)
{
	_nppData = nppData;
	Window::init(hInst, nppData._nppHandle);
}

void SettingsDialog::doDialog()
{
	if (!isCreated()) create(IDD_SETTINGSDLG);
	goToCenter();

	initParserSettings();
	loadParserSettings();
	updatePreview();
}

void SettingsDialog::initParserSettings()
{
	_parserSettings.clear();
	for(unsigned int i = 0; i < parsers.size(); ++i)
		_parserSettings.push_back(parsers[i].ps);
}

void SettingsDialog::storeParserSettings(int index)
{
	wchar_t *dtext;
	wchar_t text[256]; // Edit_LimitText is used to limit to 255 chars
	int len;

	ParserSettings *prev_ps = &_parserSettings[index];

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_START), text, 256);
	prev_ps->doc_start = toString(text);

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), text, 256);
	prev_ps->doc_line = toString(text);

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_END), text, 256);
	prev_ps->doc_end = toString(text);

	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), text, 256);
	prev_ps->command_prefix = toString(text);

	len = Edit_GetTextLength(GetDlgItem(_hSelf, IDC_EDIT_FORMAT)) + 1;
	dtext = (wchar_t *) malloc(sizeof(wchar_t) * len);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), dtext, len);

	if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED)
		prev_ps->function_format = toString(dtext);
	else
		prev_ps->file_format = toString(dtext);

	free(dtext);

	// Go ahead and get the align checkbox even though it doesnt apply to external parsers
	prev_ps->align = (Button_GetCheck(GetDlgItem(_hSelf, IDC_CHB_ALIGN)) == BST_CHECKED);
}

// Note: Setting the text of edit boxes causes notifications to be generated, which update the 
// preview multiple times when calling loadParserSettings(). The 'updating' flag is used to 
// temporarily ignore notifications.
void SettingsDialog::loadParserSettings()
{
	int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	_current = &_parserSettings[index];

	// Load the edit controls with the new parsers settings
	_updating = true;

	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_START), toWideString(_current->doc_start).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_LINE), toWideString(_current->doc_line).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_END), toWideString(_current->doc_end).c_str());
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), toWideString(_current->command_prefix).c_str());

	Button_SetCheck(GetDlgItem(_hSelf, IDC_CHB_ALIGN), _current->align); // Cannot be last!  Doesn't update preview

	if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED)
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(_current->function_format).c_str());
	else
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(_current->file_format).c_str());

	if(!parsers[index].external)
	{
		_updating = false; // NOTE: if the main proc ignores the following messages, it does not redraw them correctly
		EnableWindow(GetDlgItem(_hSelf, IDC_CHB_ALIGN), TRUE);
		EnableWindow(GetDlgItem(_hSelf, IDC_BTN_REMOVE), FALSE);
		_updating = true; // just to be safe incase we add anything later
	}
	else
	{
		Button_SetCheck(GetDlgItem(_hSelf, IDC_CHB_ALIGN), BST_UNCHECKED);

		_updating = false; // NOTE: if the main proc ignores the following messages, it does not redraw them correctly
		EnableWindow(GetDlgItem(_hSelf, IDC_CHB_ALIGN), FALSE);
		EnableWindow(GetDlgItem(_hSelf, IDC_BTN_REMOVE), TRUE);
		_updating = true; // just to be safe incase we add anything later
	}

	_updating = false;
}

void SettingsDialog::removeParserSettings()
{
	int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	_parserSettings.erase(_parserSettings.begin() + index);
	parsers.erase(parsers.begin() + index);

	ComboBox_DeleteString(GetDlgItem(_hSelf, IDC_CMB_LANG), index);
	ComboBox_SetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG), index - 1);
	_previousSelection = index - 1;

	loadParserSettings();
	updatePreview();
}

void SettingsDialog::addParserSettings()
{
	wchar_t *text;
	HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
				
	text = (wchar_t *) DialogBox((HINSTANCE) _hInst, MAKEINTRESOURCE(IDD_NEWLANG), _hSelf, inputDlgProc);
	if(text == NULL) return; // user canceled the dialog

	std::wstring name(text);
	delete[] text;

	// make sure len > 0 && no white space
	if(name.length() == 0 || name.find(TEXT(' ')) != std::string::npos)
	{
		MessageBox(NULL, TEXT("Error: New language name cannot be blank or contain whitespace."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
		return;
	}

	// make sure not already in list (case insensitive!)
	for(auto const &p : parsers)
	{
		if(_wcsicmp(p.lang.c_str(), name.c_str()) == 0)
		{
			MessageBox(NULL, TEXT("Error: Naming conflict with another language."), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
			return;
		}
	}

	addNewParser(toString(name.c_str()));
	_parserSettings.push_back(parsers.back().ps);

	name += TEXT('*');
	ComboBox_SetCurSel(cmb, ComboBox_AddString(cmb, name.c_str()));

	// Update the window
	storeParserSettings(_previousSelection);
	loadParserSettings();
	updatePreview();
	_previousSelection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	MessageBox(NULL, TEXT("Please note: Custom configurations do not parse function/method defintions, but can still be used to insert comment blocks."), NPP_PLUGIN_NAME, MB_OK|MB_ICONINFORMATION);
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
		const ParserSettings *ps = &_parserSettings[i];

		if(!validateText(ps->doc_start, IDC_EDIT_START)) ret = false;
		if(!validateText(ps->doc_line, IDC_EDIT_LINE)) ret = false;
		if(!validateText(ps->doc_end, IDC_EDIT_END)) ret = false;
		if(!validateText(ps->command_prefix, IDC_EDIT_PREFIX)) ret = false;

		if(!ret)
		{
			ComboBox_SetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG), i);
			loadParserSettings();
			updatePreview();
			return false;
		}
	}

	return true;
}

void SettingsDialog::saveSettings()
{
	for(unsigned int i = 0; i < parsers.size(); ++i)
		parsers[i].ps = _parserSettings[i];
}

void SettingsDialog::updatePreview()
{
	std::string block;
	int prev_eol_mode;
	int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));

	// Set eol mode to "\r\n" so it will display correctly in the dialogbox
	prev_eol_mode = SendScintilla(SCI_GETEOLMODE);
	SendScintilla(SCI_SETEOLMODE, SC_EOL_CRLF);

	if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED)
	{
		// Get the parser and have it parse the example
		const Parser *p = &parsers[index];

		if(!p->external)
		{
			block = FormatFunctionBlock(p, _current, p->example.c_str());
			block += "\r\n" + p->example;
		}
		else // External parsers dont have any example text to parse
		{
			block = FormatFunctionBlock(p, _current, NULL);
		}
	}
	else // IDC_RAD_FILE is set
	{
		block = FormatFileBlock(_current);
	}

	// Restore the eol mode
	SendScintilla(SCI_SETEOLMODE, prev_eol_mode);

	// Set the preview
	Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), toWideString(block).c_str());
}

void SettingsDialog::swapFormat()
{
	wchar_t *text;
	unsigned int len;
	static int last_rad = IDC_RAD_FUNCTION; // In case the selected radio button is clicked again

	// Get the text length, dynamically allocate it
	len = Edit_GetTextLength(GetDlgItem(_hSelf, IDC_EDIT_FORMAT)) + 1;
	text = (wchar_t *) malloc(sizeof(wchar_t) * len);
	Edit_GetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), text, len);

	if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FUNCTION)) == BST_CHECKED && last_rad == IDC_RAD_FILE)
	{
		_current->file_format = toString(text);
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(_current->function_format).c_str());
		last_rad = IDC_RAD_FUNCTION;
	}
	else if(Button_GetCheck(GetDlgItem(_hSelf, IDC_RAD_FILE)) == BST_CHECKED && last_rad == IDC_RAD_FUNCTION)
	{
		_current->function_format = toString(text);
		Edit_SetText(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), toWideString(_current->file_format).c_str());
		last_rad = IDC_RAD_FILE;
	}

	free(text);
}

BOOL CALLBACK SettingsDialog::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(_updating) return true;

	switch(message) 
	{
	case WM_INITDIALOG:
		{
			HWND cmb = GetDlgItem(_hSelf, IDC_CMB_LANG);
			long lfHeight = -MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);

			//for(unsigned int i = 0; i < parsers.size(); ++i)
			for(auto const &p : parsers)
			{
				if(p.external)
				{
					std::wstring name = p.language_name + TEXT("*");
					ComboBox_AddString(cmb, name.c_str());
				}
				else
				{
					ComboBox_AddString(cmb, p.language_name.c_str());
				}
			}
			ComboBox_SetCurSel(cmb, _previousSelection);
			
			// I have no idea what these values do, but these work
			_monoFont = CreateFont(lfHeight,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("Courier New"));
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_START), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_LINE), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_END), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_PREFIX), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_PREVIEW), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_EDIT_FORMAT), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_BTN_ADD), _monoFont, false);
			SetWindowFont(GetDlgItem(_hSelf, IDC_BTN_REMOVE), _monoFont, false);

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
			storeParserSettings(_previousSelection);
			loadParserSettings();
			updatePreview();
			_previousSelection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
			return true;
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDOK:
				_previousSelection = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
				storeParserSettings(_previousSelection);
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
				addParserSettings();
				return true;
			case IDC_BTN_REMOVE:
			{
				int index = ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG));
				std::wstring message = TEXT("Do you want to remove ") + parsers[index].language_name + TEXT("?");
				
				if(MessageBox(_hSelf, message.c_str(), NPP_PLUGIN_NAME, MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
					removeParserSettings();
				
				return true;
			}
			case IDC_CHB_ALIGN:
				storeParserSettings(ComboBox_GetCurSel(GetDlgItem(_hSelf, IDC_CMB_LANG)));
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
			storeParserSettings(_previousSelection);
			updatePreview();
			return true;
		case EN_MAXTEXT:
			MessageBeep(MB_ICONERROR);
			return true;
		}
	default:
		return StaticDialog::dlgProc(_HSource, message, wParam, lParam);
	}

	return false;
}
