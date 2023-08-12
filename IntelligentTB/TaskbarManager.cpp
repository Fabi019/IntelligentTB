#include "TaskbarManager.h"

TaskbarManager::TaskbarManager() {
	trayWindow = FindWindow(L"Shell_TrayWnd", nullptr);
	monitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

	// Initially show taskbar
	ShowTaskbar(trayWindow, monitor);

	if (!GetWindowRect(trayWindow, &tbRect)) {
		OutputDebugString(L"Unable to get task bar window rect!\n");
		return;
	}
}

bool TaskbarManager::ShouldHideTaskbar() {
	HWND foregroundWnd = GetForegroundWindow();

	if (foregroundWnd == nullptr) {
		OutputDebugString(L"No foreground window!\n");
		return false;
	}

	if (!IsWindowVisible(foregroundWnd)) {
		OutputDebugString(L"Foreground window not visible!\n");
		return false;
	}

	RECT fgRect;
	if (!GetWindowRect(foregroundWnd, &fgRect)) {
		OutputDebugString(L"Unable to get foreground window rect!\n");
		return false;
	}

	TCHAR title[256];
	int len = GetClassName(foregroundWnd, title, 256);

	if (len == 0
		|| _tcscmp(title, _T("Progman")) == 0
		|| _tcscmp(title, _T("XamlExplorerHostIslandWindow")) == 0
		|| _tcscmp(title, _T("Shell_TrayWnd")) == 0
		|| _tcscmp(title, _T("TopLevelWindowForOverflowXamlIsland")) == 0
		|| _tcscmp(title, _T("Windows.UI.Core.CoreWindow")) == 0
		|| _tcscmp(title, _T("WindowsDashboard")) == 0) {
		_tprintf(L"Foreground is desktop!\n");
		return false;
	}

	POINT curPos;
	if (!GetCursorPos(&curPos)) {
		OutputDebugString(L"Unable to get cursor position!\n");
		return false;
	}

	HWND cursorWnd = WindowFromPoint(curPos);

	if (cursorWnd == nullptr) {
		OutputDebugString(L"No window under cursor window!\n");
		return false;
	}

	len = GetClassName(cursorWnd, title, 256);

	if (_tcscmp(title, _T("Shell_TrayWnd")) == 0
		|| _tcscmp(title, _T("MSTaskSwWClass")) == 0
		|| _tcscmp(title, _T("TrayNotifyWnd")) == 0) {
		OutputDebugString(L"Mouse hoverig taskbar!\n");
		return false;
	}

	OutputDebugString(title);
	OutputDebugString(L"\n");

	return fgRect.left < tbRect.right && fgRect.right > tbRect.left && fgRect.top < tbRect.bottom && fgRect.bottom > tbRect.top;
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
		OutputDebugString(L"Hiding taskbar\n");
		HideTaskbar(trayWindow);
	}
	else if (!visible && !shouldHide) {
		OutputDebugString(L"Showing taskbar\n");
		ShowTaskbar(trayWindow, monitor);
	}
}

bool TaskbarManager::IsTaskbarVisible(HWND trayWindow) {
	RECT rect;
	if (!GetWindowRect(trayWindow, &rect)) {
		OutputDebugString(L"Unable to get task bar window rect!\n");
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
