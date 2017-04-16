#include "lua_main.h"
#include "PluginSettings.h"
#include "PluginOptions.h"

extern CPluginOptions g_opt;

int Lua_print(lua_State* L)
{
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();
	int args = lua_gettop(L);
	bool need_ret = args>0;
	while (args > 0 )
	{
		const char* tmp = luaL_tolstring(L, -args, NULL);
		TCHAR* wtmp = new TCHAR[strlen(tmp)+1];
		ZeroMemory(wtmp, sizeof(*wtmp));
		SysUniConv::MultiByteToUnicode(wtmp, strlen(tmp), tmp);
		AddStr(wtmp);
		delete wtmp;
		lua_pop(L, 1);
		AddStr(L" ");
		args--;
	}
	if (need_ret) AddStr(L"\n");
	return 0;
}

int Lua_printf(lua_State* L)
{
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();

	int top = lua_gettop(L);
	lua_getglobal(L, "string");
	lua_getfield(L, -1, "format");
	int i = 1;
	while ( i <= top ) lua_pushvalue(L,i++);
	lua_call(L,top,1);
	const char* _str = lua_tostring(L, -1);
	lua_settop(L, 0);
	lua_pushstring(L, _str);
	Lua_print(L);
	return 0;
}

int Lua_clear_console(lua_State* L)
{
	OnClearConsole();
	return 0;
}

int Lua_show_console(lua_State* L)
{
	if (!g_opt.m_bConsoleOpenOnInit) OpenCloseConsole();
	::SetFocus(GetCurrentScintilla());
	return 0;
}

int Lua_MessageBox(lua_State* L)
{
	int args = lua_gettop(L);
	char sz_caption[MAX_PATH], sz_txt[MAX_PATH];
	auto wnd_type = MB_OK;
	switch (args)
	{
	case 0:
		return 0;
	case 1:
		strcpy(sz_txt, lua_tostring(L, -1));
		strcpy(sz_caption, "");
		break;
	case 2:
		strcpy(sz_txt, lua_tostring(L, -2));
		strcpy(sz_caption, lua_tostring(L, -1));
		break;
	case 3:
		strcpy(sz_caption, lua_tostring(L, -3));
		strcpy(sz_txt, lua_tostring(L, -2));

		switch (int(lua_tonumber(L, -1)))
		{
		case 0:
			//MB_OK;
			break;
		case 1:
			wnd_type = MB_OKCANCEL;
			break;
		case 2:
			wnd_type = MB_ABORTRETRYIGNORE;
			break;
		case 3:
			wnd_type = MB_YESNOCANCEL;
			break;
		case 4:
			wnd_type = MB_YESNO;
			break;
		case 5:
			wnd_type = MB_RETRYCANCEL;
			break;
		case 6:
			wnd_type = MB_CANCELTRYCONTINUE;
			break;
		}
	}
	int result = MessageBoxA(NULL, sz_txt, sz_caption, wnd_type);
	lua_pushnumber(L, result);
	return 1;
}

void init_lua_libs(lua_State* L)
{
	luaL_openlibs(L);
	lua_register(L, "print", &Lua_print);
	lua_register(L, "printf", &Lua_printf);
	lua_register(L, "clear_console", &Lua_clear_console);
	lua_register(L, "show_console", &Lua_show_console);
	lua_register(L, "messagebox", &Lua_MessageBox);
}
