#pragma once

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
    void RefreshTaskbarHandle();

private:
    HWND trayWindow = NULL;
    HMONITOR monitor = NULL;
    RECT tbRect;

    int bl_count;
    int wl_count;
    TCHAR** blacklist;
    TCHAR** whitelist;
};

