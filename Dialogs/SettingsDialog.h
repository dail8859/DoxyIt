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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <map>
#include "Parsers.h"
#include "PluginInterface.h"
#include "StaticDialog.h"
#include "resource.h"


class SettingsDialog : public StaticDialog
{
private:
	std::vector<ParserDefinition> parserDefinitions;
	HFONT m_monoFont;
	NppData m_nppData;
	HWND m_HSource;
	int m_previousSelection;
	bool m_updating;
	ParserDefinition *current;

public:
	SettingsDialog() : StaticDialog(), m_previousSelection(0), m_updating(false), current(NULL), m_monoFont(NULL) {};
	~SettingsDialog() { destroy(); }

	void init(HINSTANCE hInst, NppData nppData);
	void doDialog();
	void destroy() { if(m_monoFont) DeleteObject(m_monoFont); }

private:
	void initParserDefinitions();
	void storeParserDefinition(int index);
	void loadParserDefinition();
	void removeParserDefinition();
	void addParserDefinition();
	bool validateText(std::string text, int idc);
	bool validateSettings();
	void saveSettings();
	void updatePreview();
	void swapFormat();

protected:
	BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

#endif