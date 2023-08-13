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

