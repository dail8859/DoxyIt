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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <map>
#include "resource.h"
#include "PluginInterface.h"
#include "StaticDialog.h"

typedef struct ParserDefintion
{
	std::wstring doc_start;
	std::wstring doc_line;
	std::wstring doc_end;
	std::wstring command_prefix;
} ParserDefinition;

class SettingsDialog : public StaticDialog
{
public:
	SettingsDialog() : StaticDialog() { last_selection = 0;};

	void init(HINSTANCE hInst, NppData nppData)
	{
		_nppData = nppData;
		Window::init(hInst, nppData._nppHandle);
	};

	void initParserDefinitions();
	void savePreviousParserDefinition();
	void loadParserDefinition();
	void saveSettings();

	void doDialog();
	virtual void destroy() {DeleteObject(mono);};

protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	std::map<std::wstring, ParserDefinition> parserDefinitions;
	HFONT mono;
	int last_selection;

	NppData _nppData;
	HWND _HSource;
};

#endif