#pragma once

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
VOID	CALLBACK	TimerCallback(HWND, UINT, UINT_PTR, DWORD);
VOID                Toggle();
VOID				LoadSettings();
VOID				EnableEfficiencyMode();
VOID				SetStartup(BOOL);

int __stdcall WinMainCRTStartup()
{
    return WinMain(GetModuleHandle(NULL), 0, 0, 0);
}