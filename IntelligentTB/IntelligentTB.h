#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
VOID				LoadSettings();