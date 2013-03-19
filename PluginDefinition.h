//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

#include <sstream>
#include <Shlwapi.h>
#include <tchar.h>
#include "PluginInterface.h"

#define SCI_UNUSED 0

// FingerText Messages
#define FINGERTEXT_ISENABLED 1
#define FINGERTEXT_GETVERSION 2
#define FINGERTEXT_ACTIVATE 3

const TCHAR NPP_PLUGIN_NAME[] = TEXT("DoxyIt");
const int nbFunc = 5;

// --- Helper function ---
LRESULT SendScintilla(UINT Msg, WPARAM wParam=SCI_UNUSED, LPARAM lParam=SCI_UNUSED);

// Calls from DoxyIt.cpp
void pluginInit(HANDLE hModule);						// Called from DllMain, DLL_PROCESS_ATTACH
void pluginCleanUp();									// Called from DllMain, DLL_PROCESS_DETACH
void setNppInfo(NppData notepadPlusData);				// Called from setInfo()
void handleNotification(SCNotification *notifyCode);	// Called from beNotified()

#endif //PLUGINDEFINITION_H
