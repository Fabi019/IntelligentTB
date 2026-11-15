#pragma once

#ifdef _DEBUG
#define LOGMESSAGE( str ) OutputDebugString( str );
#else
#define LOGMESSAGE( str )
#endif

const int MAX_TASKBARS = 16;

struct TaskbarInfo {
    HWND hwnd = NULL;
    HMONITOR monitor = NULL;
    RECT rect = {};
};

class TaskbarManager {
public:
    TaskbarManager(TCHAR*, TCHAR*);

    void UpdateTaskbar();
    bool RefreshTaskbars();

private:
    bool ShouldHideTaskbar(const TaskbarInfo&);
    void ShowTaskbar(TaskbarInfo&);
    void HideTaskbar(TaskbarInfo&);
    bool IsTaskbarVisible(const TaskbarInfo&);
    bool ShouldRetryTaskbarDiscovery();

private:
    TaskbarInfo taskbars[MAX_TASKBARS];
    int taskbarCount = 0;
    ULONGLONG lastRefreshTick = 0;

    int bl_count;
    int wl_count;
    TCHAR** blacklist;
    TCHAR** whitelist;
};
