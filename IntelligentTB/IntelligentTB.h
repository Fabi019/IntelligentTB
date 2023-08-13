#pragma once

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
VOID	CALLBACK	TimerCallback(HWND, UINT, UINT_PTR, DWORD);
VOID				LoadSettings();