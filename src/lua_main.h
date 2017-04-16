#include "lua.hpp"
#include "PluginSettings.h"

//int Lua_console_show(lua_State* L);
//int Lua_clear_console(lua_State* L);
int Lua_print(lua_State* L);
void init_lua_libs(lua_State* L);

enum enumNFuncItems
{
	CheckSyntax = 0,
	RunScript,
	CheckFiles,
	Separator1,
	ShowHideConsole,
	ClearConsole,
	AutoClear,
	PrintTime,
	Separator2,
	//TestItem,
	SwitchLang,
	About,

	nbFunc
};

