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

#include "PluginDefinition.h"
#include "Utils.h"

void clearLine(int line)
{
	int lineStart = SendScintilla(SCI_POSITIONFROMLINE, line);
	int lineEnd = SendScintilla(SCI_GETLINEENDPOSITION, line);
	SendScintilla(SCI_SETSEL, lineStart, lineEnd);
	SendScintilla(SCI_REPLACESEL, SCI_UNUSED, (LPARAM) "");
}

char *getLineIndentStr(int line)
{
	int indentStart = SendScintilla(SCI_POSITIONFROMLINE, line);
	int indentEnd = SendScintilla(SCI_GETLINEINDENTPOSITION, line);
	
	if(indentStart != indentEnd) return getRange(indentStart, indentEnd);
	else return NULL;
}

void insertBeforeLines(char *str, int start, int end, bool force)
{
	for(int i = start; i < end; ++i)
	{
		int start = SendScintilla(SCI_POSITIONFROMLINE, i);

		// force=true will always insert the text in front of the line
		// force=false will only insert it if the line doesnt start with str
		if(force)
		{
			SendScintilla(SCI_INSERTTEXT, start, (LPARAM) str);
		}
		else
		{
			char *buffer = getRange(start, start + strlen(str));
			if(strncmp(buffer, str, strlen(str)) != 0) SendScintilla(SCI_INSERTTEXT, start, (LPARAM) str);
			delete[] buffer;
		}
	}
}

int findNext(char* text, bool regExp)
{
	int curPos = SendScintilla(SCI_GETCURRENTPOS);
	int flags = (regExp ? SCI_SETSEARCHFLAGS : 0);

	TextToFind ttf;
	ttf.chrg.cpMin = curPos;
	ttf.chrg.cpMax = curPos + 200;
	ttf.lpstrText = text;
	
	return SendScintilla(SCI_FINDTEXT, flags, (LPARAM) &ttf);
	//return ttf.chrgText.cpMin;
}

char *getRange(int start, int end)
{
	if (end > start)
	{
		TextRange tr;
		tr.chrg.cpMin = start;
		tr.chrg.cpMax = end;
		tr.lpstrText  = new char[end - start + 1];

		SendScintilla(SCI_GETTEXTRANGE, SCI_UNUSED, (LPARAM) &tr);
		return tr.lpstrText;
	}
	return NULL;
}

char *getLine(int lineNum)
{
	char *buffer;
	int lineLen = (int) SendScintilla(SCI_LINELENGTH, lineNum);

	buffer = new char[lineLen + 1];
	SendScintilla(SCI_GETLINE, lineNum, (LPARAM) buffer);
	buffer[lineLen] = '\0';

	return buffer;
}

char *getEolStr()
{
	int eolmode = SendScintilla(SCI_GETEOLMODE);
	static char *eol[] = {"\r\n","\r","\n"};
	return eol[eolmode];
}
