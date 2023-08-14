#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>

#include <tchar.h>
#include <vector>

#ifdef _DEBUG
#define LOGMESSAGE( str ) OutputDebugString( str );
#else
#define LOGMESSAGE( str )
#endif

class TaskbarManager {
public:
    TaskbarManager(TCHAR*, TCHAR*);

    void UpdateTaskbar();

private:
    bool ShouldHideTaskbar();
    void ShowTaskbar();
    void HideTaskbar();
    bool IsTaskbarVisible();

private:
    HWND trayWindow = NULL;
    HMONITOR monitor = NULL;
    RECT tbRect;

    std::vector<TCHAR*> blacklist;
    std::vector<TCHAR*> whitelist;
};

