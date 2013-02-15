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

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>

extern std::string doc_start;
extern std::string doc_line;
extern std::string doc_end;

//
// All definitions of plugin interface
//
#include "PluginInterface.h"

//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT("DoxyIt");

//
// Here define the number of your plugin commands
//
const int nbFunc = 3;


//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);


// FingerText Messages
#define FINGERTEXT_ISENABLED 1
#define FINGERTEXT_GETVERSION 2
#define FINGERTEXT_ACTIVATE 3

// --- Menu call backs ---
void doxyItFunction();
void doxyItFile();
void activeCommenting();
//void activeWrapping();

// --- Notification callbacks ---
void doxyItNewLine();

extern bool do_active_commenting;
extern bool do_active_wrapping;
extern bool fingertext_found;
extern bool fingertext_enabled;

extern NppData nppData;


typedef struct FunctionDefinition
{
	std::string return_val;
	std::string function_name;
	std::vector<std::string> parameters;	
} FunctionDefinition;

void InitializeParsers();
void CleanUpParsers();
void Parse(int lang_type);

#endif //PLUGINDEFINITION_H
