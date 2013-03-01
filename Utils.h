#ifndef UTILS_H
#define UTILS_H

void clearLine(int line);
char *getLineIndentStr(int line);
void insertBeforeLines(char *str, int start, int end, bool force=false);
int findNext(char* text, bool regExp);
char *getRange(int start, int end);
char *getLine(int lineNum);
char *getEolStr();

#endif