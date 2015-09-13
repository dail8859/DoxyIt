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


class SettingsDialog final : public StaticDialog
{
private:
	std::vector<ParserSettings> _parserSettings;
	HFONT _monoFont;
	NppData _nppData;
	HWND _HSource;
	int _previousSelection;
	bool _updating;
	ParserSettings *_current;

public:
	SettingsDialog() : StaticDialog(), _previousSelection(0), _updating(false), _current(NULL), _monoFont(NULL) {}
	~SettingsDialog() { destroy(); }

	void init(HINSTANCE hInst, NppData nppData);
	void doDialog();
	void destroy() { if(_monoFont) DeleteObject(_monoFont); }

private:
	void initParserSettings();
	void storeParserSettings(int index);
	void loadParserSettings();
	void removeParserSettings();
	void addParserSettings();
	bool validateText(std::string text, int idc);
	bool validateSettings();
	void saveSettings();
	void updatePreview();
	void swapFormat();

protected:
	BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

#endif