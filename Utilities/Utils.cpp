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

#include "PluginDefinition.h"
#include "Utils.h"

// Clears the line
void clearLine(int line)
{
	auto lineStart = editor.PositionFromLine(line);
	auto lineEnd = editor.GetLineEndPosition(line);

	editor.SetSel(lineStart, lineEnd);
	editor.ReplaceSel("");
}

// Get the whitespace of the line, the returned value must be free'd
char *getLineIndentStr(int line)
{
	int indentStart = editor.PositionFromLine(line);
	int indentEnd = editor.GetLineIndentPosition(line);

	if(indentStart != indentEnd) return getRange(indentStart, indentEnd);
	else return NULL;
}

// Insert str in front of each line between start and end
// If the line already starts with the string it is skipped unless force = true
void insertBeforeLines(char *str, int start, int end, bool force)
{
	for(int i = start; i < end; ++i)
	{
		int line = editor.PositionFromLine(i);

		// force=true will always insert the text in front of the line
		// force=false will only insert it if the line doesnt start with str
		if(force)
		{
			editor.InsertText(line, str);
		}
		else
		{
			char *buffer = getRange(line, line + static_cast<int>(strlen(str)));
			if(strncmp(buffer, str, strlen(str)) != 0) editor.InsertText(line, str);
			delete[] buffer;
		}
	}
}

// Find the next instance of text
int findNext(char* text, int len, bool regExp)
{
	int curPos = editor.GetCurrentPos();
	int flags = (regExp ? SCFIND_REGEXP : 0);

	TextToFind ttf;
	ttf.chrg.cpMin = curPos;
	ttf.chrg.cpMax = curPos + len;
	ttf.lpstrText = text;

	return editor.FindText(flags, &ttf);
}

std::pair<int, int> findInRange(const char *text, int start, int stop, bool regExp)
{
	TextToFind ttf;
	ttf.chrg.cpMin = start;
	ttf.chrg.cpMax = stop;
	ttf.lpstrText = text;

	int flags = (regExp ? SCFIND_REGEXP : 0);

	if (editor.FindText(flags, &ttf) != INVALID_POSITION)
		return std::make_pair(ttf.chrgText.cpMin, ttf.chrgText.cpMax);

	return std::make_pair(INVALID_POSITION, INVALID_POSITION);
}

// Get a range of text from start to end, returned string must be free'd
char *getRange(int start, int end)
{
	if (end > start)
	{
		TextRange tr;
		tr.chrg.cpMin = start;
		tr.chrg.cpMax = end;
		tr.lpstrText  = new char[end - start + 1];

		editor.GetTextRange(&tr);
		return tr.lpstrText;
	}
	return NULL;
}

// Get a line
char *getLine(int lineNum)
{
	int lineLen = editor.LineLength(lineNum);
	char *buffer = new char[lineLen + 1];

	editor.GetLine(lineNum, buffer);
	buffer[lineLen] = '\0';

	return buffer;
}

// Get the end of line string for the current eol mode
const char *getEolStr()
{
	static char *eol[] = {"\r\n", "\r", "\n"};
	auto eolmode = editor.GetEOLMode();
	return eol[eolmode];
}

std::wstring toWideString(std::string s)
{
	return std::wstring(s.begin(), s.end());
}

std::string toString(const wchar_t *w)
{
	std::wstring wide(w);
	return std::string(wide.begin(), wide.end());
}

bool isWhiteSpace(const std::string& str)
{
	for(unsigned int i = 0; i < str.length(); ++i)
		if(!isspace(str[i]))
			return false;
	return true;
}

// Changes str in place and also returns it
std::string& stringReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
	size_t pos = 0;
	while((pos = str.find(oldStr, pos)) != std::string::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
	return str;
}

std::vector<std::string> splitLines(const std::string &str, const std::string &split)
{
	std::vector<std::string> lines;
	size_t prev_pos = 0, pos = 0;

	while((pos = str.find(split, pos)) != std::string::npos)
	{
		lines.push_back(str.substr(prev_pos, pos - prev_pos));
		prev_pos = pos = pos + split.length();
	}
	// Get the final line
	lines.push_back(str.substr(prev_pos, std::string::npos));

	return lines;
}
