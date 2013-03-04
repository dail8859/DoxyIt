//Copyright (C)2013 Justin Dailey <dail8859@yahoo.com>
//
//This program is free software; you can redistribute it and/or
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

#include "Parsers.h"


std::string Parse(int lang_type)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
	{
		if(parsers[i].lang_type == lang_type)
		{
			return (*parsers[i].callback)();
			break;
		}
	}

	return "";
}

void InitializeParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		if((*parsers[i].initializer)() == false)
			::MessageBox(NULL, TEXT("Doxyit initialization failed"), NPP_PLUGIN_NAME, MB_OK|MB_ICONERROR);
}

void CleanUpParsers(void)
{
	int len = sizeof(parsers) / sizeof(parsers[0]);
	for(int i = 0; i < len; ++i)
		(*parsers[i].cleanup)();
}