#include "stdafx.h"
#include "resource.h"
#include "IntelligentTB.h"
#include "TaskbarManager.h"

const TCHAR FILE_NAME[] = _T("settings.ini");
const TCHAR SETTINGS_CATEGORY[] = _T("Settings");
const TCHAR DEFAULT_AUTOSTART[] = _T("0");
const TCHAR DEFAULT_EFFMODE[] = _T("1");
const TCHAR DEFAULT_TIMERMS[] = _T("200");
const TCHAR DEFAULT_BLACKLIST[] = _T("Progman,XamlExplorerHostIslandWindow,Shell_TrayWnd,TopLevelWindowForOverflowXamlIsland,Windows.UI.Core.CoreWindow,WindowsDashboard,WorkerW");
const TCHAR DEFAULT_WHITELIST[] = _T("Shell_TrayWnd,MSTaskSwWClass,TrayNotifyWnd");

static TCHAR szWindowClass[] = _T("IntelligentTB");
static TCHAR szTitle[] = _T("IntelligentTB");
static TCHAR settingsFile[MAX_PATH];

// Settings
static INT timerMs;
static TCHAR blacklist[512];
static TCHAR whitelist[512];
static BOOL autoStart;
static BOOL efficiencyMode;

// Global variables
NOTIFYICONDATA g_nid;
HANDLE g_hMutex = NULL;
UINT_PTR g_timerID = 0;
TaskbarManager* g_tbm;
BOOL g_disabled = false;

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
) {
    // Check if another instance is already running
    g_hMutex = CreateMutex(NULL, TRUE, szWindowClass);

    if (g_hMutex == NULL) {
        MessageBox(NULL, 
            _T("Mutex creation failed!"), 
            szTitle,
            MB_ICONERROR);
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL,
            _T("Another instance of the application is already running."),
            szTitle,
            MB_ICONINFORMATION);
        CloseHandle(g_hMutex);
        return 0; // Exit the second instance
    }

    // Load settings
    LoadSettings();

    // Enable efficiency mode
    if (efficiencyMode) {
        EnableEfficiencyMode();
    }

    // Set startup
    SetStartup(autoStart);

    // Initialize variables
    MSG msg;
    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        WndProc, 0L, 0L,
        GetModuleHandle(NULL),
        NULL, NULL, NULL, NULL,
        szWindowClass, NULL
    };

    // Register window class
    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            szTitle,
            MB_ICONERROR);

        return 1;
    }

    // Create a hidden window
    HWND hWnd = CreateWindow(
        wcex.lpszClassName, szTitle,
        WS_OVERLAPPEDWINDOW,
        0, 0, 0, 0, NULL, NULL,
        wcex.hInstance, NULL
    );

    if (!hWnd) {
        MessageBox(NULL,
            _T("Call to CreateWindowEx failed!"),
            szTitle,
            MB_ICONERROR);

        return 1;
    }

    // Setup taskbar manager
    auto tbm = TaskbarManager(blacklist, whitelist);
    g_tbm = &tbm;

    // Set a timer
    g_timerID = SetTimer(hWnd, 1, timerMs, TimerCallback);

    // Initialize NOTIFYICONDATA structure
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_USER + 1;
    g_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    _tcscpy_s(g_nid.szTip, szTitle);

    // Add the tray icon
    Shell_NotifyIcon(NIM_ADD, &g_nid);

    // Main message loop
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up and remove the tray icon
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    UnregisterClass(wcex.lpszClassName, wcex.hInstance);

    // Release the mutex before exiting
    ReleaseMutex(g_hMutex);
    CloseHandle(g_hMutex);

    // Clean up tbm reference
    g_tbm = nullptr;

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(
    _In_ HWND   hWnd,
    _In_ UINT   message,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
) {
    switch (message) {
    case WM_USER + 1: // Tray icon message
        switch (LOWORD(lParam)) {
        case WM_RBUTTONUP: {// Right-click context menu
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 4, g_disabled ? _T("Enable") : _T("Disable"));
            AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
            AppendMenu(hMenu, MF_STRING, 2, _T("Settings"));
            AppendMenu(hMenu, MF_STRING, 3, _T("About..."));
            AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
            AppendMenu(hMenu, MF_STRING, 1, _T("Exit"));
            SetForegroundWindow(hWnd);

            UINT cmd = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
            switch (cmd) {
            case 1: // Exit
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                break;

            case 2: { // Settings
                DWORD fileAttributes = GetFileAttributes(settingsFile);
                if ((fileAttributes == INVALID_FILE_ATTRIBUTES)
                    || (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    int result = MessageBox(NULL,
                        _T("The settings file does not exist. Do you want to create it?"),
                        szTitle,
                        MB_YESNO | MB_ICONQUESTION);

                    if (result == IDYES) {
                        WritePrivateProfileString(SETTINGS_CATEGORY, _T("TimerMs"), DEFAULT_TIMERMS, settingsFile);
                        WritePrivateProfileString(SETTINGS_CATEGORY, _T("Blacklist"), DEFAULT_BLACKLIST, settingsFile);
                        WritePrivateProfileString(SETTINGS_CATEGORY, _T("Whitelist"), DEFAULT_WHITELIST, settingsFile);
                        WritePrivateProfileString(SETTINGS_CATEGORY, _T("EfficiencyMode"), DEFAULT_EFFMODE, settingsFile);
                        WritePrivateProfileString(SETTINGS_CATEGORY, _T("AutoStart"), DEFAULT_AUTOSTART, settingsFile);
                    }
                }

                ShellExecute(NULL, _T("open"), settingsFile, NULL, NULL, SW_SHOWNORMAL);
                break;
            }
            case 3: // About
                ShellExecute(NULL, _T("open"), _T("https://www.github.com/Fabi019/IntelligentTB"), NULL, NULL, SW_SHOWNORMAL);
                break;

            case 4: // Toggle
                g_disabled = !g_disabled;
                break;
            }

            DestroyMenu(hMenu);
            break;
        }
        case WM_LBUTTONUP: // Left-click
            g_disabled = !g_disabled;
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

VOID CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    if (g_disabled)
        return;
    g_tbm->UpdateTaskbar();
}

VOID LoadSettings() {
    // Construct settings path
    GetModuleFileName(NULL, settingsFile, MAX_PATH);
    _tcscpy_s(_tcsrchr(settingsFile, _T('\\')) + 1, MAX_PATH - _tcslen(settingsFile), FILE_NAME);

    TCHAR szValue[512];

    GetPrivateProfileString(SETTINGS_CATEGORY, _T("EfficiencyMode"), DEFAULT_EFFMODE, szValue, 2, settingsFile);
    efficiencyMode = !!_tstoi(szValue);

    GetPrivateProfileString(SETTINGS_CATEGORY, _T("AutoStart"), DEFAULT_AUTOSTART, szValue, 2, settingsFile);
    autoStart = !!_tstoi(szValue);

    GetPrivateProfileString(SETTINGS_CATEGORY, _T("TimerMs"), DEFAULT_TIMERMS, szValue, 8, settingsFile);
    timerMs = _tstoi(szValue);
    if (timerMs < 0) {
        timerMs = 0;
    }

    GetPrivateProfileString(SETTINGS_CATEGORY, _T("Blacklist"), DEFAULT_BLACKLIST, szValue, 512, settingsFile);
    _tcscpy_s(blacklist, szValue);

    GetPrivateProfileString(SETTINGS_CATEGORY, _T("Whitelist"), DEFAULT_WHITELIST, szValue, 512, settingsFile);
    _tcscpy_s(whitelist, szValue);
}

VOID EnableEfficiencyMode() {
    // Set process priority to idle
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

    // Throttle execution speed (EcoQos)
    PROCESS_POWER_THROTTLING_STATE pic;
    pic.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    pic.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    pic.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

    SetProcessInformation(GetCurrentProcess(),
        ProcessPowerThrottling,
        &pic,
        sizeof(PROCESS_POWER_THROTTLING_STATE));
}

VOID SetStartup(BOOL enable) {
    HKEY hKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_SET_VALUE, &hKey);

    if (enable) {
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, MAX_PATH);
        RegSetValueEx(hKey, szTitle, 0, REG_SZ, (BYTE*)szPath, (DWORD)(_tcslen(szPath) + 1) * sizeof(TCHAR));
    }
    else {
        RegDeleteValue(hKey, szTitle);
    }

    RegCloseKey(hKey);
}