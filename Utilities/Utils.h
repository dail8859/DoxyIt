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

#ifndef UTILS_H
#define UTILS_H

#include <vector>

#define BOOLTOSTR(b) ((b) ? TEXT("true") : TEXT("false"))

// Scintilla Utilities
void clearLine(int line);
char *getLineIndentStr(int line);
void insertBeforeLines(char *str, int start, int end, bool force=false);
int findNext(char* text, int len=200, bool regExp=false);
char *getRange(int start, int end);
char *getLine(int lineNum);
const char *getEolStr();

// Generic string utilites
std::wstring toWideString(std::string s);
std::string toString(const wchar_t *w);
bool isWhiteSpace(const std::string& str);
std::string& stringReplace(std::string& str, const std::string& oldStr, const std::string& newStr);
std::vector<std::string> splitLines(const std::string &str, const std::string &split);

#endif
