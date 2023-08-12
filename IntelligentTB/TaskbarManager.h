#pragma once

#include <Windows.h>
#include <tchar.h>

class TaskbarManager {
public:
    TaskbarManager();

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
};

