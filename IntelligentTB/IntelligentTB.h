#pragma once

#define _CRT_SECURE_NO_DEPRECATE

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