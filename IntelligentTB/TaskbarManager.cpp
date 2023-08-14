#include "TaskbarManager.h"

TaskbarManager::TaskbarManager(TCHAR* bl, TCHAR* wl) {
	// Initialize tray handle and current monitor
	trayWindow = FindWindow(_T("Shell_TrayWnd"), nullptr);
	monitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

	if (!trayWindow) {
		MessageBox(NULL,
			_T("Handle to taskbar window not found!"),
			_T("Error"),
			MB_ICONERROR);
	}

	// Check if auto-hide is enabled
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = trayWindow;
	UINT taskbarState = (UINT)SHAppBarMessage(ABM_GETSTATE, &abd);
	if (!(taskbarState & ABS_AUTOHIDE)) {
		int result = MessageBox(NULL,
			_T("The auto-hide taskbar is not enabled.\nThis app will most likely not function without it.\nDo you want to open the taskbar settings?"),
			_T("Warning"),
			MB_YESNO | MB_ICONWARNING);

		if (result == IDYES) {
			// Open taskbar settings
			ShellExecute(NULL, _T("open"), _T("ms-settings:taskbar"), NULL, NULL, SW_SHOWNORMAL);
		}
	}

	TCHAR* token;
	TCHAR* context;

	// Parse the blacklist
	token = _tcstok_s(bl, _T(","), &context);
	while (token != NULL) {
		LOGMESSAGE(token);
		LOGMESSAGE(_T("\n"));

		blacklist.push_back(token);
		token = _tcstok_s(NULL, _T(","), &context);
	}

	// Parse the whitelist
	token = _tcstok_s(wl, _T(","), &context);
	while (token != NULL) {
		LOGMESSAGE(token);
		LOGMESSAGE(_T("\n"));

		whitelist.push_back(token);
		token = _tcstok_s(NULL, _T(","), &context);
	}

	// Initially show taskbar
	ShowTaskbar();

	// Small delay to wait for the taskbar to fully show
	Sleep(1);

	if (!GetWindowRect(trayWindow, &tbRect)) {
		LOGMESSAGE(_T("Unable to get task bar window rect!\n"));
		return;
	}
}

bool TaskbarManager::ShouldHideTaskbar() {
	HWND foregroundWnd = GetForegroundWindow();

	if (!foregroundWnd) {
		LOGMESSAGE(_T("No foreground window!\n"));
		return false;
	}

	if (!IsWindowVisible(foregroundWnd)) {
		LOGMESSAGE(_T("Foreground window not visible!\n"));
		return false;
	}

	RECT fgRect;
	if (!GetWindowRect(foregroundWnd, &fgRect)) {
		LOGMESSAGE(_T("Unable to get foreground window rect!\n"));
		return false;
	}

	TCHAR title[256];
	GetClassName(foregroundWnd, title, 256);

	LOGMESSAGE(_T("Foreground: "));
	LOGMESSAGE(title);
	LOGMESSAGE(_T("\n"));

	for (auto bl : blacklist) {
		if (_tcscmp(title, bl) == 0) {
			LOGMESSAGE(_T("Foreground is desktop!\n"));
			return false;
		}
	}

	POINT curPos;
	if (!GetCursorPos(&curPos)) {
		LOGMESSAGE(_T("Unable to get cursor position!\n"));
		return false;
	}

	HWND cursorWnd = WindowFromPoint(curPos);

	if (cursorWnd == nullptr) {
		LOGMESSAGE(_T("No window under cursor window!\n"));
		return false;
	}

	GetClassName(cursorWnd, title, 256);

	LOGMESSAGE(_T("Hovering: "));
	LOGMESSAGE(title);
	LOGMESSAGE(_T("\n"));

	for (auto wl : whitelist) {
		if (_tcscmp(title, wl) == 0) {
			LOGMESSAGE(_T("Mouse hoverig taskbar!\n"));
			return false;
		}
	}

	bool intersect = fgRect.left < tbRect.right
		&& fgRect.right > tbRect.left
		&& fgRect.top < tbRect.bottom
		&& fgRect.bottom > tbRect.top;

	return intersect;
}

void TaskbarManager::ShowTaskbar() {
	PostMessage(trayWindow, 0x05D1, (WPARAM)1, (LPARAM)monitor);
}

void TaskbarManager::HideTaskbar() {
	PostMessage(trayWindow, 0x05D1, (WPARAM)0, (LPARAM)0);
}

void TaskbarManager::UpdateTaskbar() {
	bool visible = IsTaskbarVisible();
	bool shouldHide = ShouldHideTaskbar();

	if (visible && shouldHide) {
		LOGMESSAGE(_T("Hiding taskbar\n"));
		HideTaskbar();
	}
	else if (!visible && !shouldHide) {
		LOGMESSAGE(_T("Showing taskbar\n"));
		ShowTaskbar();
	}
}

bool TaskbarManager::IsTaskbarVisible() {
	RECT rect;
	if (!GetWindowRect(trayWindow, &rect)) {
		LOGMESSAGE(_T("Unable to get task bar window rect!\n"));
		return false;
	}

	int bottomDelta = rect.bottom - GetSystemMetrics(SM_CYSCREEN);
	return bottomDelta == 0;
}
