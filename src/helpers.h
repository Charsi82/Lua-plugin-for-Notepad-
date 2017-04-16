#pragma once

#include "PluginSettings.h"

#define INFO_WIN(text) MessageBox(nppData._nppHandle, text, \
	sPluginName, MB_OK | MB_ICONINFORMATION)
#define WARN_WIN(text) MessageBox(nppData._nppHandle, text, \
	sPluginName, MB_OK | MB_ICONWARNING)
#define ERR_WIN(text) MessageBox(nppData._nppHandle, text, \
	sPluginName, MB_OK | MB_ICONERROR)

#define SendNpp(msg, wParam, lParam) SendMessage(nppData._nppHandle, msg, \
	(WPARAM) wParam, (LPARAM) lParam)
// #define SendMainSci(msg, wParam, lParam) SendMessage( \
// 	nppData._scintillaMainHandle, msg, (WPARAM) wParam, (LPARAM) lParam)
// #define SendSecondSci(msg, wParam, lParam) SendMessage( \
// 	nppData._scintillaSecondHandle, msg, (WPARAM) wParam, (LPARAM) lParam)
