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

#ifndef UTILS_H
#define UTILS_H

std::string FT(const char *p);
void clearLine(int line);
char *getLineIndentStr(int line);
void insertBeforeLines(char *str, int start, int end, bool force=false);
int findNext(char* text, bool regExp);
char *getRange(int start, int end);
char *getLine(int lineNum);
char *getEolStr();

#endif