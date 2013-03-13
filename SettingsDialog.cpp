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

#include "SettingsDialog.h"

void SettingsDialog::doDialog()
{
	if (!isCreated())
	{
		create(IDD_PLUGINGOLINE_DEMO, true);
	}
	goToCenter();
}

BOOL CALLBACK SettingsDialog::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG:
			return true;
		case WM_COMMAND: 
			switch (wParam)
			{
				case IDOK:
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
