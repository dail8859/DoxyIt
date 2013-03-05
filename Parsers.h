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

#ifndef PARSERS_H
#define PARSERS_H

#include "PluginDefinition.h"
#include "Utils.h"
#include "trex.h"


// C Parser
bool Initialize_C(void);
void CleanUp_C(void);
std::string Callback_C(void);

// CPP Parser. This is just a wrapper for the C implementation
bool Initialize_CPP(void);
void CleanUp_CPP(void);
std::string Callback_CPP(void);


typedef struct Parser
{
	int lang_type;
	bool (*initializer)(void);
	void (*cleanup)(void);
	std::string (*callback)(void);
} Parser;

#define REGISTER_PARSER(lang) {L_##lang, Initialize_##lang, CleanUp_##lang, Callback_##lang}
static Parser parsers[] = 
{
	REGISTER_PARSER(C),
	REGISTER_PARSER(CPP)
	//REGISTER_PARSER(CS),
	//REGISTER_PARSER(JAVA),
	//REGISTER_PARSER(PHP),
	//REGISTER_PARSER(PYTHON)
};

#endif