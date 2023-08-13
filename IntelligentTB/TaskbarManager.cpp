#include "TaskbarManager.h"

TaskbarManager::TaskbarManager(TCHAR* bl, TCHAR* wl) {
	// Initialize tray handle and current monitor
	trayWindow = FindWindow(_T("Shell_TrayWnd"), nullptr);
	monitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

	TCHAR* token;
	TCHAR* context;

	// Parse the blacklist
	token = _tcstok_s(bl, _T(","), &context);
	while (token != NULL) {
		OutputDebugString(token);
		OutputDebugString(_T("\n"));

		blacklist.push_back(token);
		token = _tcstok_s(NULL, _T(","), &context);
	}

	// Parse the whitelist
	token = _tcstok_s(wl, _T(","), &context);
	while (token != NULL) {
		OutputDebugString(token);
		OutputDebugString(_T("\n"));

		whitelist.push_back(token);
		token = _tcstok_s(NULL, _T(","), &context);
	}

	// Initially show taskbar
	ShowTaskbar(trayWindow, monitor);

	// Small delay to wait for the taskbar to fully show
	Sleep(1);

	if (!GetWindowRect(trayWindow, &tbRect)) {
		OutputDebugString(_T("Unable to get task bar window rect!\n"));
		return;
	}
}

bool TaskbarManager::ShouldHideTaskbar() {
	HWND foregroundWnd = GetForegroundWindow();

	if (foregroundWnd == nullptr) {
		OutputDebugString(_T("No foreground window!\n"));
		return false;
	}

	if (!IsWindowVisible(foregroundWnd)) {
		OutputDebugString(_T("Foreground window not visible!\n"));
		return false;
	}

	RECT fgRect;
	if (!GetWindowRect(foregroundWnd, &fgRect)) {
		OutputDebugString(_T("Unable to get foreground window rect!\n"));
		return false;
	}

	TCHAR title[256];
	int len = GetClassName(foregroundWnd, title, 256);

	OutputDebugString(_T("Foreground: "));
	OutputDebugString(title);
	OutputDebugString(_T("\n"));

	for (auto bl : blacklist) {
		if (_tcscmp(title, bl) == 0) {
			OutputDebugString(_T("Foreground is desktop!\n"));
			return false;
		}
	}

	POINT curPos;
	if (!GetCursorPos(&curPos)) {
		OutputDebugString(_T("Unable to get cursor position!\n"));
		return false;
	}

	HWND cursorWnd = WindowFromPoint(curPos);

	if (cursorWnd == nullptr) {
		OutputDebugString(_T("No window under cursor window!\n"));
		return false;
	}

	len = GetClassName(cursorWnd, title, 256);

	OutputDebugString(_T("Hovering: "));
	OutputDebugString(title);
	OutputDebugString(_T("\n"));

	for (auto wl : whitelist) {
		if (_tcscmp(title, wl) == 0) {
			OutputDebugString(_T("Mouse hoverig taskbar!\n"));
			return false;
		}
	}

	bool intersect = fgRect.left < tbRect.right
		&& fgRect.right > tbRect.left
		&& fgRect.top < tbRect.bottom
		&& fgRect.bottom > tbRect.top;

	return intersect;
}

void TaskbarManager::ShowTaskbar(HWND trayWindow, HMONITOR monitor) {
	PostMessage(trayWindow, 0x05D1, (WPARAM)1, (LPARAM)monitor);
}

void TaskbarManager::HideTaskbar(HWND trayWindow) {
	PostMessage(trayWindow, 0x05D1, (WPARAM)0, (LPARAM)0);
}

void TaskbarManager::UpdateTaskbar() {
	bool visible = IsTaskbarVisible(trayWindow);
	bool shouldHide = ShouldHideTaskbar();

	if (visible && shouldHide) {
		OutputDebugString(_T("Hiding taskbar\n"));
		HideTaskbar(trayWindow);
	}
	else if (!visible && !shouldHide) {
		OutputDebugString(_T("Showing taskbar\n"));
		ShowTaskbar(trayWindow, monitor);
	}
}

bool TaskbarManager::IsTaskbarVisible(HWND trayWindow) {
	RECT rect;
	if (!GetWindowRect(trayWindow, &rect)) {
		OutputDebugString(_T("Unable to get task bar window rect!\n"));
		return false;
	}

	int bottomDelta = rect.bottom - GetSystemMetrics(SM_CYSCREEN);
	// taskbar on the top or bottom
	if (bottomDelta == 0)
		return true;

	if (bottomDelta > 0)
		return false;

	int topDelta = rect.top - 0;
	return topDelta == 0;
}
