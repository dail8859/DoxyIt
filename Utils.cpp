#include "PluginDefinition.h"
#include "Utils.h"

void clearLine(int line)
{
	int lineStart = SendScintilla(SCI_POSITIONFROMLINE, line, 0);
	int lineEnd = SendScintilla(SCI_GETLINEENDPOSITION, line, 0);
	SendScintilla(SCI_SETSEL, lineStart, lineEnd);
	SendScintilla(SCI_REPLACESEL, 0, (LPARAM) "");
}

char *getLineIndentStr(int line)
{
	int indentStart = SendScintilla(SCI_POSITIONFROMLINE, line, 0);
	int indentEnd = SendScintilla(SCI_GETLINEINDENTPOSITION, line, 0);
	
	if(indentStart != indentEnd) return getRange(indentStart, indentEnd);
	else return NULL;
}

void insertBeforeLines(char *str, int start, int end, bool force)
{
	for(int i = start; i < end; ++i)
	{
		int start = SendScintilla(SCI_POSITIONFROMLINE, i, 0);

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
	int curPos = SendScintilla(SCI_GETCURRENTPOS, 0, 0);
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

		SendScintilla(SCI_GETTEXTRANGE, 0, (LPARAM) &tr);
		return tr.lpstrText;
	}
	return NULL;
}

char *getLine(int lineNum)
{
	char *buffer;
	int lineLen = (int) SendScintilla(SCI_LINELENGTH, lineNum, 0);

	buffer = new char[lineLen + 1];
	SendScintilla(SCI_GETLINE, lineNum, (LPARAM) buffer);
	buffer[lineLen] = '\0';

	return buffer;
}

char *getEolStr()
{
	int eolmode = SendScintilla(SCI_GETEOLMODE, 0, 0);
	static char *eol[] = {"\r\n","\r","\n"};
	return eol[eolmode];
}
