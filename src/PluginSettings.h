#pragma once

#include "PluginInterface.h"
#include "Docking.h"
#include <lua.hpp>

#define sPluginName L"Lua utils"

struct ExecData {
	HINSTANCE hNPP;
	HWND hConsole;
	RECT rcConsole;
	//HWND hConsoleScintilla;
	HMENU hMenu;
	BOOL ConsoleOpen;
};

// Extern Variables
extern NppData nppData;
extern ExecData execData;
extern FuncItem funcItems[];
extern tTbData dockingData;

void InitFuncItem(int            nItem,
	const TCHAR*   szName, 
	PFUNCPLUGINCMD pFunc ,
	ShortcutKey*   pShortcut);

// Plugin Functions
LRESULT SendSci(UINT iMessage, WPARAM wParam = 0, LPARAM lParam = 0);
HWND GetCurrentScintilla();
void GlobalInitialize();
void GlobalDeinitialize();

void AddStr(TCHAR* msg, bool bNL = false);

HWND GetConsole();
void OnClearConsole();
void OpenCloseConsole();
void SetCharFormat(COLORREF color, DWORD dwMask , DWORD dwEffects , DWORD dwOptions);
BOOL CALLBACK ConsoleProcDlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

class SysUniConv
{
protected:

	static int SysUniConv::a2w(wchar_t* ws, int wml, const char* as, int al, UINT acp);
	static int SysUniConv::w2a(char* as, int aml, const wchar_t* ws, int wl, UINT acp);
	static int SysUniConv::u2a(char* as, int aml, const char* us, int ul, UINT acp);

public:
	static int SysUniConv::str_unsafe_len( const char* str );
	static int SysUniConv::strw_unsafe_len( const wchar_t* strw );
	static int SysUniConv::UTF8ToMultiByte(char* aStr, int aMaxLen, const char* uStr, int uLen = -1, UINT aCodePage = CP_ACP);
	static int SysUniConv::MultiByteToUnicode(wchar_t* wStr, int wMaxLen, const char* aStr, int aLen = -1, UINT aCodePage = CP_ACP);
	static int SysUniConv::UnicodeToMultiByte(char* aStr, int aMaxLen, const wchar_t* wStr, int wLen = -1, UINT aCodePage = CP_ACP);
};