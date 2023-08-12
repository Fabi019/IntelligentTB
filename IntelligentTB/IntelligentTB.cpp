#include "IntelligentTB.h"

static TCHAR szWindowClass[] = _T("IntelligentTB");
static TCHAR szTitle[] = _T("IntelligentTB");

// Global variables
NOTIFYICONDATA g_nid;
HANDLE g_hMutex;

TCHAR settingsFile[MAX_PATH];

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
        return 1; // Exit the application
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL,
            _T("Another instance of the application is already running."),
            szTitle,
            MB_ICONINFORMATION);
        CloseHandle(g_hMutex);
        return 0; // Exit the second instance
    }

    GetModuleFileName(NULL, settingsFile, MAX_PATH);
    _tcscpy_s(_tcsrchr(settingsFile, _T('\\')) + 1, MAX_PATH - _tcslen(settingsFile), _T("settings.ini"));

    // Load settings
    LoadSettings();

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
    if (!RegisterClassEx(&wcex))
    {
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

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindowEx failed!"),
            szTitle,
            MB_ICONERROR);

        return 1;
    }

    // Initialize NOTIFYICONDATA structure
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_USER + 1;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
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
        case WM_RBUTTONUP: // Right-click context menu
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, _T("Settings"));
            AppendMenu(hMenu, MF_STRING, 2, _T("Exit"));
            SetForegroundWindow(hWnd);

            UINT cmd = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
            switch (cmd) {
            case 1: // Settings
                ShellExecute(NULL, _T("open"), settingsFile, NULL, NULL, SW_SHOWNORMAL);
                break;

            case 2: // Exit
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            }

            DestroyMenu(hMenu);
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

VOID LoadSettings() {
    TCHAR szValue[MAX_PATH];
    GetPrivateProfileString(_T("Settings"), _T("Option1"), _T(""), szValue, MAX_PATH, settingsFile);
    OutputDebugString(_T("Option1: "));
    OutputDebugString(szValue);
    OutputDebugString(_T("\n"));

    GetPrivateProfileString(_T("Settings"), _T("Option2"), _T(""), szValue, MAX_PATH, settingsFile);
    OutputDebugString(_T("Option2: "));
    OutputDebugString(szValue);
    OutputDebugString(_T("\n"));
}