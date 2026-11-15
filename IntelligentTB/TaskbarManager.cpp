#include "stdafx.h"
#include "TaskbarManager.h"

const TCHAR* delimiter = _T(",");

const int STARTUP_RETRY_COUNT = 30;
const int STARTUP_RETRY_DELAY_MS = 100;
const ULONGLONG RETRY_INTERVAL_MS = 2000;

TaskbarManager::TaskbarManager(TCHAR* bl, TCHAR* wl) {
	for (int i = 0; i < STARTUP_RETRY_COUNT && taskbarCount == 0; i++) {
		if (i > 0) {
			Sleep(STARTUP_RETRY_DELAY_MS);
		}
		RefreshTaskbars();
	}

	if (taskbarCount == 0) {
		LOGMESSAGE(_T("Taskbar window not found yet, still looking...\n"));
	}

	// Check if auto-hide is enabled
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = taskbarCount > 0 ? taskbars[0].hwnd : NULL;
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

	// Count the number of elements in blacklist
	bl_count = 1;
	for (TCHAR* p = bl; *p != _T('\0'); p++) {
		if (*p == *delimiter) {
			bl_count++;
		}
	}

	// Allocate memory for the blacklist
	blacklist = reinterpret_cast<TCHAR**>(malloc(sizeof(TCHAR*) * bl_count));

	TCHAR* token;
	TCHAR* context = nullptr;

	// Parse the blacklist
	token = _tcstok_s(bl, delimiter, &context);
	int i = 0;
	while (token != NULL) {
		LOGMESSAGE(token);
		LOGMESSAGE(_T("\n"));

		size_t tokenLength = _tcslen(token);

		// Allocate memory for the substring and copy the data
		blacklist[i] = new TCHAR[tokenLength + 1];
		_tcscpy_s(blacklist[i++], tokenLength + 1, token);

		token = _tcstok_s(NULL, delimiter, &context);
	}

	// Count the number of elements in whitelist
	wl_count = 1;
	for (TCHAR* p = wl; *p != _T('\0'); p++) {
		if (*p == *delimiter) {
			wl_count++;
		}
	}

	// Allocate memory for the whitelist
	whitelist = reinterpret_cast<TCHAR**>(malloc(sizeof(TCHAR*) * wl_count));

	context = nullptr; // Reset context

	// Parse the whitelist
	token = _tcstok_s(wl, delimiter, &context);
	i = 0;
	while (token != NULL) {
		LOGMESSAGE(token);
		LOGMESSAGE(_T("\n"));

		size_t tokenLength = _tcslen(token);

		// Allocate memory for the substring and copy the data
		whitelist[i] = new TCHAR[tokenLength + 1];
		_tcscpy_s(whitelist[i++], tokenLength + 1, token);

		token = _tcstok_s(NULL, delimiter, &context);
	}

}

struct EnumTaskbarContext {
	TaskbarInfo* taskbars;
	int count;
};

static void AddTaskbar(EnumTaskbarContext& context, HWND hwnd) {
	if (context.count >= MAX_TASKBARS) {
		return;
	}

	bool known = false;
	for (int i = 0; i < context.count; i++) {
		if (context.taskbars[i].hwnd == hwnd) {
			known = true;
			break;
		}
	}

	if (known) {
		return;
	}

	TaskbarInfo& taskbar = context.taskbars[context.count];

	taskbar.hwnd = hwnd;
	taskbar.monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	context.count++;
}

static BOOL CALLBACK EnumTaskbarProc(HWND hwnd, LPARAM lParam) {
	TCHAR className[64];
	if (!GetClassName(hwnd, className, 64)) {
		return TRUE;
	}

	if (_tcscmp(className, _T("Shell_TrayWnd")) != 0 &&
		_tcscmp(className, _T("Shell_SecondaryTrayWnd")) != 0) {
		return TRUE;
	}

	EnumTaskbarContext* context = reinterpret_cast<EnumTaskbarContext*>(lParam);
	if (context->count >= MAX_TASKBARS) {
		return FALSE;
	}

	AddTaskbar(*context, hwnd);

	return TRUE;
}

static int FindTaskbars(TaskbarInfo* taskbars) {
	EnumTaskbarContext context = { taskbars, 0 };

	const TCHAR* classes[] = { _T("Shell_TrayWnd"), _T("Shell_SecondaryTrayWnd") };
	const int classCount = _countof(classes);

	for (int c = 0; c < classCount && context.count < MAX_TASKBARS; c++) {
		HWND hwnd = NULL;
		while (context.count < MAX_TASKBARS
			&& (hwnd = FindWindowEx(NULL, hwnd, classes[c], NULL)) != NULL) {
			AddTaskbar(context, hwnd);
		}
	}

	if (context.count == 0) {
		EnumWindows(EnumTaskbarProc, reinterpret_cast<LPARAM>(&context));
	}

	return context.count;
}

bool TaskbarManager::RefreshTaskbars() {
	lastRefreshTick = GetTickCount64();

	TaskbarInfo found[MAX_TASKBARS];
	taskbarCount = FindTaskbars(found);

	if (taskbarCount == 0) {
		LOGMESSAGE(_T("Handle to taskbar window not found!\n"));
		return false;
	}

	for (int i = 0; i < taskbarCount; i++) {
		taskbars[i] = found[i];
	}

	// Initially show all taskbars
	for (int i = 0; i < taskbarCount; i++) {
		ShowTaskbar(taskbars[i]);
	}

	// Small delay to wait for the taskbar to fully show
	Sleep(1);

	for (int i = 0; i < taskbarCount; i++) {
		RECT rect;
		if (!GetWindowRect(taskbars[i].hwnd, &rect)) {
			LOGMESSAGE(_T("Unable to get task bar window rect!\n"));
			continue;
		}

		MONITORINFO monitorInfo = {};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfo(taskbars[i].monitor, &monitorInfo)
			&& rect.bottom > monitorInfo.rcMonitor.bottom) {
			// The taskbar was still auto-hidden while the rect was read.
			// Normalize it to the position it occupies while shown.
			int delta = rect.bottom - monitorInfo.rcMonitor.bottom;
			rect.top -= delta;
			rect.bottom -= delta;
		}

		taskbars[i].rect = rect;
	}

	return true;
}

bool TaskbarManager::ShouldHideTaskbar(const TaskbarInfo& tb) {
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

	for (int i = 0; i < bl_count; i++) {
		if (_tcscmp(title, blacklist[i]) == 0) {
			LOGMESSAGE(_T("Foreground is desktop!\n"));
			return false;
		}
	}

	POINT curPos;
	if (!GetCursorPos(&curPos)) {
		LOGMESSAGE(_T("Unable to get cursor position!\n"));
		return false;
	}

	if (MonitorFromPoint(curPos, MONITOR_DEFAULTTONEAREST) == tb.monitor) {
		HWND cursorWnd = WindowFromPoint(curPos);

		if (cursorWnd == nullptr) {
			LOGMESSAGE(_T("No window under cursor window!\n"));
			return false;
		}

		GetClassName(cursorWnd, title, 256);

		LOGMESSAGE(_T("Hovering: "));
		LOGMESSAGE(title);
		LOGMESSAGE(_T("\n"));

		for (int i = 0; i < wl_count; i++) {
			if (_tcscmp(title, whitelist[i]) == 0) {
				LOGMESSAGE(_T("Mouse hoverig taskbar!\n"));
				return false;
			}
		}
	}

	bool intersect = fgRect.left < tb.rect.right
		&& fgRect.right > tb.rect.left
		&& fgRect.top < tb.rect.bottom
		&& fgRect.bottom > tb.rect.top;

	return intersect;
}

void TaskbarManager::ShowTaskbar(TaskbarInfo& tb) {
	PostMessage(tb.hwnd, 0x05D1, (WPARAM)1, (LPARAM)tb.monitor);
}

void TaskbarManager::HideTaskbar(TaskbarInfo& tb) {
	//PostMessage(tb.hwnd, 0x0579, (WPARAM)0, (LPARAM)0);
	//PostMessage(tb.hwnd, 0x04F3, (WPARAM)0, (LPARAM)0xD508A0);
	PostMessage(tb.hwnd, 0x05D1, (WPARAM)0, (LPARAM)0);
	PostMessage(tb.hwnd, 0x05BB, (WPARAM)0, (LPARAM)0);
	//PostMessage(tb.hwnd, 0x05C5, (WPARAM)0, (LPARAM)0);
}

void TaskbarManager::UpdateTaskbar() {
	if (taskbarCount == 0) {
		if (ShouldRetryTaskbarDiscovery()) {
			RefreshTaskbars();
		}
		return;
	}

	for (int i = 0; i < taskbarCount; i++) {
		TaskbarInfo& tb = taskbars[i];

		if (!IsWindow(tb.hwnd)) {
			taskbarCount = 0;
			if (ShouldRetryTaskbarDiscovery()) {
				RefreshTaskbars();
			}
			return;
		}

		bool visible = IsTaskbarVisible(tb);
		bool shouldHide = ShouldHideTaskbar(tb);

		if (visible && shouldHide) {
			LOGMESSAGE(_T("Hiding taskbar\n"));
			HideTaskbar(tb);
		}
		else if (!visible && !shouldHide) {
			LOGMESSAGE(_T("Showing taskbar\n"));
			ShowTaskbar(tb);
		}
	}
}

bool TaskbarManager::ShouldRetryTaskbarDiscovery() {
	return GetTickCount64() - lastRefreshTick >= RETRY_INTERVAL_MS;
}

bool TaskbarManager::IsTaskbarVisible(const TaskbarInfo& tb) {
	RECT rect;
	if (!GetWindowRect(tb.hwnd, &rect)) {
		return false;
	}

	MONITORINFO monitorInfo = {};
	monitorInfo.cbSize = sizeof(monitorInfo);
	if (!GetMonitorInfo(tb.monitor, &monitorInfo)) {
		return false;
	}

	int bottomDelta = rect.bottom - monitorInfo.rcMonitor.bottom;
	return bottomDelta == 0;
}
