#pragma once

#include <Windows.h>
#include <tchar.h>
#include <vector>

class TaskbarManager {
public:
    TaskbarManager(TCHAR*, TCHAR*);

    void UpdateTaskbar();

private:
    bool ShouldHideTaskbar();
    void ShowTaskbar(HWND trayWindow, HMONITOR monitor);
    void HideTaskbar(HWND trayWindow);
    bool IsTaskbarVisible(HWND trayWindow);

private:
    HWND trayWindow = NULL;
    HMONITOR monitor = NULL;
    RECT tbRect;

    std::vector<TCHAR*> blacklist;
    std::vector<TCHAR*> whitelist;
};

