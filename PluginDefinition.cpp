#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "trex.h"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

bool do_active_commenting;
bool do_active_wrapping;
bool fingertext_found;
bool fingertext_enabled;

std::string doc_start;
std::string doc_line;
std::string doc_end;

//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit(HANDLE hModule)
{
	InitializeParsers();

	do_active_commenting = true;
	do_active_wrapping = true;

	doc_start = "/**";
	//doc_start = "/**************************************************************************************//**";
	doc_line  = " *  ";
	//doc_end   = " ******************************************************************************************/";
	doc_end   = " */";
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	CleanUpParsers();
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	ShortcutKey *sk = new ShortcutKey();
	sk->_isAlt = TRUE;
	sk->_isCtrl = TRUE;
	sk->_isShift = TRUE;
	sk->_key = 'D';

	setCommand(0, TEXT("DoxyIt - Function"), doxyItFunction, sk, false);
	setCommand(1, TEXT("DoxyIt - File"), doxyItFile, NULL, false);
	setCommand(2, TEXT("Active commenting"), activeCommenting, NULL, do_active_commenting);
	//setCommand(3, TEXT("Active word wrapping"), activeWrapping, NULL, do_active_wrapping);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
}

//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
	if (index >= nbFunc || !pFunc) return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}



bool checkFingerText()
{
	if(fingertext_found)
	{
		CommunicationInfo ci;
		ci.internalMsg = FINGERTEXT_ISENABLED;
		ci.srcModuleName = NPP_PLUGIN_NAME;
		ci.info = NULL;
		::SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);
		return ci.info;
	}
	else
		return false;
}


// --- Menu call backs ---

void doxyItFunction()
{
	int lang_type;

	// Check if it is enabled
	fingertext_enabled = checkFingerText();

	// Get the current language type
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &lang_type);
	
	Parse(lang_type);
	// return (return_val, function_name, (parameters))
}

void doxyItFile()
{
	TCHAR fileName[MAX_PATH];
	char fname[MAX_PATH];
	int which = -1;
	HWND curScintilla;
	std::ostringstream doc_block;
	
	::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM) fileName);
	wcstombs(fname, fileName, sizeof(fname));

	// Get the current scintilla
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &which);
	if (which == -1) return;
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Check if it is enabled
	fingertext_enabled = checkFingerText();

	doc_block << doc_start << "\r\n";
	doc_block << doc_line << "\\file " << fname << "\r\n";
	doc_block << doc_line << "\\brief \r\n";
	doc_block << doc_line << "\r\n";
	doc_block << doc_line << "\\author \r\n";
	doc_block << doc_line << "\\version 1.0\r\n";
	doc_block << doc_end;

	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_block.str().c_str());

	//CommunicationInfo ci;
	//ci.internalMsg = 1;
	//ci.srcModuleName = NPP_PLUGIN_NAME;
	//ci.info = NULL;

	// Get version
	//ci.internalMsg = FINGERTEXT_GETVERSION;
	//ci.srcModuleName = NPP_PLUGIN_NAME;
	//ci.info = NULL;
	//::SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);

	// Activate it
	//ci.internalMsg = FINGERTEXT_ACTIVATE;
	//ci.srcModuleName = NPP_PLUGIN_NAME;
	//ci.info = NULL;
	//::SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, (WPARAM) TEXT("FingerText.dll"), (LPARAM) &ci);
}

void activeCommenting()
{
	do_active_commenting = !do_active_commenting;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[2]._cmdID, (LPARAM) do_active_commenting);
}

/*
void activeWrapping()
{
	do_active_wrapping = !do_active_wrapping;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[3]._cmdID, (LPARAM) do_active_wrapping);
}
*/

// --- Notification callbacks ---

void doxyItNewLine()
{
	char *buffer;
	int which = -1;
	HWND curScintilla;
	int curPos, curLine, lineLen;

	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1) return;

	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	curPos = (int) ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
	curLine = (int) ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, curPos, 0);
	lineLen = (int) ::SendMessage(curScintilla, SCI_LINELENGTH, curLine - 1, 0);
		
	buffer = new char[lineLen + 1];
	::SendMessage(curScintilla, SCI_GETLINE, curLine - 1, (LPARAM) buffer);
	buffer[lineLen] = '\0';
		
	// Creates a new comment block
	if(strncmp(buffer, doc_start.c_str(), doc_start.length()) == 0)
	{
		std::string temp = "\r\n" + doc_end;
		::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_line.c_str());
		::SendMessage(curScintilla, SCI_INSERTTEXT, -1, (LPARAM) temp.c_str());
	}

	// Adds a new line of the comment block
	if(strncmp(buffer, doc_line.c_str(), doc_line.length()) == 0)
	{
		::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) doc_line.c_str());
	}

	delete[] buffer;
}
