#include "resource.h"
#include "PluginSettings.h"
#include "PluginOptions.h"
#include "helpers.h"
#include "Docking.h"
#include <string>
#include <richedit.h>
#include "lua_main.h"
#include "ctime"

///////////////////
// Plugin Variables
///////////////////
NppData nppData;
ExecData execData;

CPluginOptions g_opt;
bool bNeedClear = false;
tTbData dockingData;
//SciFnDirect pSciMsg;
//LRESULT pSciWndData;
SCNotification *scNotification;
HMODULE m_hRichEditDll;

WNDPROC OriginalRichEditProc = NULL;
LRESULT CALLBACK RichEditWndProc(
	HWND   hEd, 
	UINT   uMessage, 
	WPARAM wParam, 
	LPARAM lParam)
{
	if ( uMessage == WM_LBUTTONDBLCLK ) OpenCloseConsole();
	return OriginalRichEditProc(hEd, uMessage, wParam, lParam);
};

FuncItem funcItems[nbFunc];

void InitFuncItem(int nItem, const TCHAR* szName, PFUNCPLUGINCMD pFunc = NULL, ShortcutKey* pShortcut = NULL)
{
	lstrcpy(funcItems[nItem]._itemName, szName);
	funcItems[nItem]._pFunc = pFunc;
	funcItems[nItem]._init2Check = false; //bCheck;
	funcItems[nItem]._pShKey = pShortcut;
}

HWND GetCurrentScintilla()
{
	int currentView = 0;
	SendNpp(NPPM_GETCURRENTSCINTILLA, 0, &currentView);
	return (currentView == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

LRESULT SendSci(UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    return SendMessage(GetCurrentScintilla(), iMessage, wParam, lParam);
}

void GlobalInitialize()
{
	// Fetch the menu
	execData.hMenu = GetMenu(nppData._nppHandle);

	// Create the docking dialog
	execData.hConsole = CreateDialog( execData.hNPP,
		MAKEINTRESOURCE(IDD_CONSOLE), nppData._nppHandle,
		(DLGPROC) ConsoleProcDlg);

	dockingData.hClient = execData.hConsole;
	dockingData.pszName = L" Console ";
	dockingData.dlgID = 0;
	dockingData.uMask = DWS_DF_CONT_BOTTOM;
	dockingData.pszModuleName = L"";

	// Register the docking dialog
	SendNpp(NPPM_DMMREGASDCKDLG, 0, &dockingData);
	SendNpp(NPPM_MODELESSDIALOG, execData.hConsole, MODELESSDIALOGADD);
	
	SendSci(SCI_SETMOUSEDWELLTIME, 500);

	g_opt.ReadOptions();

	if (g_opt.m_bConsoleOpenOnInit)
	{
		SendNpp(NPPM_SETMENUITEMCHECK, funcItems[ShowHideConsole]._cmdID, TRUE);
		SendNpp(NPPM_DMMSHOW, 0, execData.hConsole);
	}
	else
	{
		SendNpp(NPPM_DMMHIDE, 0, execData.hConsole);
	}
	
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[AutoClear]._cmdID, g_opt.m_bConsoleAutoclear);
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[PrintTime]._cmdID, g_opt.m_bShowRunTime);

	OriginalRichEditProc = (WNDPROC) SetWindowLongPtr(GetConsole(), GWLP_WNDPROC, (LONG_PTR) RichEditWndProc);

}

void GlobalDeinitialize()
{
	SendNpp(NPPM_MODELESSDIALOG, execData.hConsole,	MODELESSDIALOGREMOVE);
	g_opt.SaveOptions();
	HWND hDlgItem = GetConsole();
	if (hDlgItem)
		SetWindowLongPtr(hDlgItem, GWLP_WNDPROC, (LONG_PTR) OriginalRichEditProc);
}

void OnSwitchLang()
{
	g_opt.OnSwitchLang();
}

void OnShowAboutDlg()
{		
	TCHAR txt[1024];
	TCHAR ws1[] = 
		L" Lua plugin v1.5 "
#ifdef UNICODE
		L"Unicode "
#else
		L" "
#endif
#ifdef _WIN64
		L"(64-bit)"
#else
		L"(32-bit)"
#endif  
		L"\n\n"\
		L" Author: Charsi <charsi2011@gmail.com>\n\n"\
		L" Syntax checking of Lua scripts.\n"\
		L" Run the script from current file.\n\n"\
		L" Components:\n\n"\
		L" - Lua ";
	wsprintf(txt, L"%s %d.%d.%d %s", ws1, LUA_VERSION_MAJOR[0] - '0', LUA_VERSION_MINOR[0] - '0', LUA_VERSION_RELEASE[0] - '0', L"(http://www.lua.org)\n"\
		L" - lfs 1.6.3 (http://www.keplerproject.org/luafilesystem)\n"\

#ifdef LUA_MARLIB
		L" - marshal 1.5 (Richard Hundt <richardhundt@gmail.com>)"	
#endif
		
		);
	MessageBox(NULL, txt, L"Lua plugin for Notepad++", MB_OK);
}

void OpenCloseConsole()
{
	g_opt.m_bConsoleOpenOnInit = !g_opt.m_bConsoleOpenOnInit;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[ShowHideConsole]._cmdID, g_opt.m_bConsoleOpenOnInit);
	SendNpp(g_opt.m_bConsoleOpenOnInit ? NPPM_DMMSHOW : NPPM_DMMHIDE, 0, execData.hConsole);
}

HWND GetConsole()
{
	return GetDlgItem(execData.hConsole, IDC_RICHEDIT21);
}

void OnClearConsole()
{
	HWND hRE = GetConsole();
	int ndx = GetWindowTextLength(hRE);
	SendMessage(hRE, EM_SETSEL, 0, ndx);
	SendMessage(hRE, EM_REPLACESEL, 0, (LPARAM)L"");
}

void OnSize()
{
	HWND hRE = GetConsole();
	if (hRE)
	{
		GetClientRect(execData.hConsole, &execData.rcConsole);
		SetWindowPos(hRE, NULL, 0, 0,
			execData.rcConsole.right, execData.rcConsole.bottom, SWP_NOZORDER);
	}
}

BOOL CALLBACK ConsoleProcDlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*) lParam;
			if (pnmh->hwndFrom == nppData._nppHandle)
			{
				switch( LOWORD(pnmh->code) )
				{
				case DMN_CLOSE: // closing dialog
					SendNpp(NPPM_SETMENUITEMCHECK, funcItems[ShowHideConsole]._cmdID, FALSE);
					g_opt.m_bConsoleOpenOnInit = FALSE;
					break;
				//case DMN_FLOAT: // floating dialog
					//break;
				//case DMN_DOCK:  // docking dialog
					//break;
				}
			}
		}
		break;		
		case WM_SIZE:
			OnSize();
		break;
    }
    return FALSE;
}

void AddStr(TCHAR* msg, bool bNL)
{
	if (!msg) return;

	HWND hRE = GetConsole();
	int ndx = GetWindowTextLength (hRE);
	SendMessage(hRE, EM_SETSEL, ndx, ndx);

	if (bNL) lstrcat(msg, L"\r\n");

	SendMessage(hRE, EM_REPLACESEL, 0, (LPARAM)msg);
}

void SetCharFormat(COLORREF color = RGB(0,0,0), DWORD dwMask = CFM_COLOR, DWORD dwEffects = 0, DWORD dwOptions = SCF_ALL)
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = dwMask;
	cf.dwEffects = dwEffects;
	cf.crTextColor = color; 
	SendMessage(GetConsole(), EM_SETCHARFORMAT, dwOptions, (LPARAM)&cf);
}

void OnPrintTime()
{
	g_opt.m_bShowRunTime = !g_opt.m_bShowRunTime;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[PrintTime]._cmdID, g_opt.m_bShowRunTime);
}

void OnAutoClear()
{
	g_opt.m_bConsoleAutoclear = !g_opt.m_bConsoleAutoclear;
	SendNpp(NPPM_SETMENUITEMCHECK, funcItems[AutoClear]._cmdID, g_opt.m_bConsoleAutoclear);
}

int inline get_line_number(const char* str){ // s:match(":(%d+):")
	bool f = false;
	char res[10];
	int j = 0;

	//str
	for(int i = 4; str[i]; i++){
		if (!f && str[i]==':')
			f = true;
		else
		{
			if (f){
				if(str[i]==':') break;
				res[j++] = str[i];
			};
		};		
	};
	res[j]='\0';
	return atoi(res);
	//int res = 0;
	//sscanf(str, ":%d:", res);
	//return res;
}

void OnCheckSyntax()
{
	// save file
	::SendMessage( nppData._nppHandle, NPPM_SAVECURRENTFILE, 0, 0 );

	TCHAR ws_full_path[MAX_PATH];
	::SendMessage( nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)ws_full_path );

	if (ws_full_path[1]!=':' && ws_full_path[2]!='\\') return;

	// get language type
	LangType docType = L_EXTERNAL;
	::SendMessage( nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType );

	if (docType != L_LUA) return; // не Lua скрипт!	

	//if (g_opt.m_bConsoleAutoclear)
	OnClearConsole();
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	lua_State* L = luaL_newstate();
	init_lua_libs(L);

	char full_path[MAX_PATH];
	SysUniConv::UnicodeToMultiByte(full_path, MAX_PATH, ws_full_path);
	int rslt = luaL_loadfile(L, full_path);
	if (rslt)
	{
		const char* szError = lua_tostring(L, -1);
		int line = get_line_number(szError) - 1;
		UINT start_sel = (UINT)SendSci(SCI_POSITIONFROMLINE, line);
		SendSci(SCI_MARKERDELETE, line, SC_MARK_ARROWS); // clear if added
		SendSci(SCI_MARKERADD, line, SC_MARK_ARROWS);
		SendSci(SCI_GOTOPOS, start_sel);

		SetCharFormat(RGB(128,0,0));
		TCHAR wszError[MAX_PATH];
		SysUniConv::MultiByteToUnicode(wszError, MAX_PATH, szError);
		OnClearConsole();
		AddStr(wszError);
	}
	else
	{
		SetCharFormat(RGB(0,128,0));
		//lstrcat(ws_full_path, L" - Syntax OK!");
		OnClearConsole();
		AddStr(ws_full_path); AddStr(L" - Syntax OK!");
	};
	lua_close(L);
	::SetFocus( GetCurrentScintilla() );

	bNeedClear = true;
}

void OnRunScript()
{
	// save file
	::SendMessage( nppData._nppHandle, NPPM_SAVECURRENTFILE, 0, 0 );

	TCHAR ws_full_path[MAX_PATH];
	::SendMessage( nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)ws_full_path );

	if (ws_full_path[1]!=':' && ws_full_path[2]!='\\') return;

	// get language type
	LangType docType = L_EXTERNAL;
	::SendMessage( nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType );

	if (docType != L_LUA) return; // не Lua скрипт!

	if (g_opt.m_bConsoleAutoclear || bNeedClear) OnClearConsole();
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	lua_State* L = luaL_newstate();
	init_lua_libs(L);

	char full_path[MAX_PATH];
	SysUniConv::UnicodeToMultiByte(full_path, MAX_PATH, ws_full_path);
	clock_t run_time = clock();
	int rslt = luaL_dofile(L, full_path);
	run_time = clock() - run_time;

	if (rslt)
	{
		const char* szError = lua_tostring(L, -1);
		int line = get_line_number(szError);
		UINT start_sel = (UINT)SendSci(SCI_POSITIONFROMLINE, line - 1);
		SendSci(SCI_SETEMPTYSELECTION, start_sel);
		TCHAR wszError[MAX_PATH];
		SysUniConv::MultiByteToUnicode(wszError, MAX_PATH, szError);
		AddStr(wszError, true);
		bNeedClear = true;
	}
	else
	{		
		TCHAR str[MAX_PATH];
		SYSTEMTIME sTime;
		GetLocalTime(&sTime);

		wsprintf(str, L"Success: %02d:%02d:%02d", sTime.wHour, sTime.wMinute, sTime.wSecond);
		AddStr(str, true );
		
		if (g_opt.m_bShowRunTime)
		{
			wsprintf(str, L"Runtime: %d ms", run_time);
			AddStr(str, true);
		}		

		bNeedClear = false;
	};

	lua_close(L);
	SetCharFormat();
}

bool check_file_syntax(lua_State* L, TCHAR* path)
{
	if (!path) return true;
	bool res = true;

	char mb_path[MAX_PATH];
	SysUniConv::UnicodeToMultiByte(mb_path, MAX_PATH, path);
	int rslt = luaL_loadfile(L, mb_path);
	if (rslt)
	{
		TCHAR szError[1024];
		SysUniConv::MultiByteToUnicode(szError, 1023, lua_tostring(L, -1));
		AddStr(L"<error> : ");
		AddStr(szError, true);
		res = false;
	}
	lua_settop(L, 0);	
	return res;
}
void OnCheckFiles()
{
	// save file
	::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILE, 0, 0);

	TCHAR ws_full_path[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM)ws_full_path);

	if (ws_full_path[1] != L':' && ws_full_path[2] != L'\\') return;

	// get language type
	LangType docType = L_EXTERNAL;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);

	if (docType != L_LUA) return; // не Lua скрипт!	

	//if (g_opt.m_bConsoleAutoclear)
	OnClearConsole();
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	TCHAR f_ext[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, MAX_PATH, (LPARAM)f_ext);

	TCHAR ws_full_pattern[MAX_PATH];
	lstrcpy(ws_full_pattern, ws_full_path);
	lstrcat(ws_full_pattern, L"\\*");
	lstrcat(ws_full_pattern, f_ext);
	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(ws_full_pattern, &f);
	bool stat = true;
	int _cnt = 0;
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	TCHAR ws_full_filepath[MAX_PATH];
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			lstrcpy(ws_full_filepath, ws_full_path);
			lstrcat(ws_full_filepath, L"\\");
			lstrcat(ws_full_filepath, f.cFileName);
			bool res = check_file_syntax(L, ws_full_filepath);
			stat = stat && res;
			if (!res) _cnt++;
		} while (FindNextFile(h, &f));

		wsprintf(ws_full_filepath, L"Done! Found %d file(s) with errors.", _cnt);
		AddStr(ws_full_filepath, true);
		SetCharFormat(RGB(stat ? 0 : 128, stat ? 128 : 0, 0));
	}
	else
	{
		AddStr(L"Error opening directory\n");
	}
	lua_close(L);
}
///////// encoding ///////////
int SysUniConv::str_unsafe_len( const char* str )
{
	if (!str) return 0;
	const char* str0 = str;
	while ( *str )  ++str;
	return ( (int) (str - str0) );
}

int SysUniConv::strw_unsafe_len( const wchar_t* strw )
{
	if (!strw) return 0;
	const wchar_t* strw0 = strw;
	while ( *strw )  ++strw;
	return ( (int) (strw - strw0) );
}

int SysUniConv::MultiByteToUnicode(wchar_t* wStr, int wMaxLen, const char* aStr, 
	int aLen , UINT aCodePage )
{
	if ( aStr && wStr && (wMaxLen > 0) )
	{
		if ( aLen < 0 )
			aLen = str_unsafe_len(aStr);
		if ( aLen > 0 )
		{
			return a2w(wStr, wMaxLen, aStr, aLen, aCodePage);
		}
	}
	if ( wStr && (wMaxLen > 0) )
		wStr[0] = 0;
	return 0;
}

int SysUniConv::UnicodeToMultiByte(char* aStr, int aMaxLen, const wchar_t* wStr, 
	int wLen, UINT aCodePage)
{
	if ( wStr && aStr && (aMaxLen > 0) )
	{
		if ( wLen < 0 )
			wLen = strw_unsafe_len(wStr);
		if ( wLen > 0 )
		{
			return w2a(aStr, aMaxLen, wStr, wLen, aCodePage);
		}
	}
	if ( aStr && (aMaxLen > 0) )
		aStr[0] = 0;
	return 0;
}

int SysUniConv::a2w(wchar_t* ws, int wml, const char* as, int al, UINT acp)
{
	int len = ::MultiByteToWideChar(acp, 0, as, al, ws, wml);
	ws[len] = 0;
	return len;
}

int SysUniConv::w2a(char* as, int aml, const wchar_t* ws, int wl, UINT acp)
{
	int len = ::WideCharToMultiByte(acp, 0, ws, wl, as, aml, NULL, NULL);
	as[len] = 0;
	return len;
}

int SysUniConv::u2a(char* as, int aml, const char* us, int ul, UINT acp)
{
	int      len = 0;
	wchar_t* ws = new wchar_t[ul + 1];
	if ( ws )
	{
		len = ::MultiByteToWideChar(CP_UTF8, 0, us, ul, ws, ul);
		ws[len] = 0;
		len = ::WideCharToMultiByte(acp, 0, ws, len, as, aml, NULL, NULL);
		delete [] ws;
	}
	as[len] = 0;
	return len;
}

int SysUniConv::UTF8ToMultiByte(char* aStr, int aMaxLen, const char* uStr, 
	int uLen , UINT aCodePage )
{
	if ( uStr && aStr && (aMaxLen > 0) )
	{
		if ( uLen < 0 )
			uLen = str_unsafe_len(uStr);
		if ( uLen > 0 )
		{
			return u2a(aStr, aMaxLen, uStr, uLen, aCodePage);
		}
	}
	if ( aStr && (aMaxLen > 0) )
		aStr[0] = 0;
	return 0;
}


///////////////////////////////////////////////////
// Main
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved)
{
	switch (reasonForCall)
	{
	case DLL_PROCESS_ATTACH:
		execData.hNPP = (HINSTANCE)hModule;

		// init menu
		InitFuncItem(CheckSyntax, L"Проверка синтаксиса", OnCheckSyntax);
		InitFuncItem(RunScript, L"Запуск скрипта", OnRunScript);
		InitFuncItem(CheckFiles, L"Проверить все файлы в папке", OnCheckFiles);
		InitFuncItem(Separator1, L"");
		InitFuncItem(ShowHideConsole, L"Показать\\cкрыть консоль", OpenCloseConsole);
		InitFuncItem(ClearConsole, L"Очистить консоль", OnClearConsole);
		InitFuncItem(AutoClear, L"Автоочистка консоли", OnAutoClear);
		InitFuncItem(PrintTime, L"Время выполнения", OnPrintTime);
		InitFuncItem(Separator2, L"");
		InitFuncItem(SwitchLang, L"Язык", OnSwitchLang);
		InitFuncItem(About, L"О плагине", OnShowAboutDlg);
		//InitFuncItem(TestItem, L"test", OnTestItem);

		m_hRichEditDll = LoadLibrary(L"Riched32.dll");
		break;

	case DLL_PROCESS_DETACH:
		if (m_hRichEditDll != NULL)
			FreeLibrary(m_hRichEditDll);
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return sPluginName;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItems;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	if (notifyCode->nmhdr.hwndFrom == nppData._nppHandle)
		switch (notifyCode->nmhdr.code)
	{
		case NPPN_READY:
			GlobalInitialize();
			break;

		case NPPN_SHUTDOWN:
			GlobalDeinitialize();
			break;

		default:
			break;
	}
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}