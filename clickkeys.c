#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include "resource.h"

#ifdef LOAD_CRT
#	define ENTRY_POINT int main()
#else
#	define ENTRY_POINT DWORD CALLBACK RawEntryPoint()
#	pragma comment(linker, "/entry:RawEntryPoint /subsystem:windows /nodefaultlib:libcmt.lib /nodefaultlib:msvcrt.lib")
#	pragma comment(lib, "kernel32.lib")
#	pragma comment(lib, "user32.lib")
#	pragma comment(lib, "shell32.lib")
#endif

#define ABOUT_TEXT (\
	L"ClickKeys\n" \
	L"Copyright © 2014 IvyBits Software, All Rights Reserved.\n" \
	L"\n" \
	L"Usage:\n" \
	L"   • Pause/Break key is translated to a click\n" \
	L"     (can't be held down due to technical limitaions)\n" \
	L"   • Context menu key is translated mouse click (hold to drag)\n" \
	L"   • Alt + Shift + M to toggle context menu key -> mouse\n" \
	L"   • Win + Alt + Shift + M to toggle pause -> click\n" \
	L"   • Ctrl + Alt + Shift + M to exit"\
)

#define CLICK_KEY VK_APPS
#define HOTKEY_TOGGLE 41
#define HOTKEY_ACTIVATE 42
#define HOTKEY_EXIT 43
#define HOTKEY_TOGGLE_HOLD 44
#define NWM_NOTIFY (WM_USER + 42)
#define NWM_TOGGLE (WM_USER + 43)
#define NWM_TOGGLE_HOLD (WM_USER + 44)
#define AWM_ENABLE_HOOK (WM_APP + 42)
#define AWM_DISABLE_HOOK (WM_APP + 43)
#define AWM_EXIT_HOOK (WM_APP + 44)

HINSTANCE hInstance;
NOTIFYICONDATA nidIcon;
HICON hIcon;
HWND hwnd;
HMENU hMenu;
DWORD dwThread;

BOOL toggle = FALSE, toggleHold = FALSE;
BOOL wasDown;
DWORD down, up;

HKEY AutoRunKey() {
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, NULL, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hKey, NULL)
			== ERROR_SUCCESS)
		return hKey;
	else
		return NULL;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		LPKBDLLHOOKSTRUCT key = (LPKBDLLHOOKSTRUCT) lParam;
		if (key->vkCode == CLICK_KEY && !(key->flags & LLKHF_INJECTED)) {
			switch (wParam) {
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					if (!wasDown)
						mouse_event(down, 0, 0, 0, 0);
					wasDown = TRUE;
					break;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					mouse_event(up, 0, 0, 0, 0);
					wasDown = FALSE;
					break;
			}
			return 1;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD CALLBACK LowLevelKeyboardThread(LPVOID lpParam) {
	HHOOK hHook;
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		switch (msg.message) {
		case AWM_ENABLE_HOOK:
			wasDown = GetAsyncKeyState(CLICK_KEY) < 0;
			hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
			break;
		case AWM_DISABLE_HOOK:
			UnhookWindowsHookEx(hHook);
			break;
		case AWM_EXIT_HOOK:
			if (toggleHold)
				PostThreadMessage(dwThread, AWM_DISABLE_HOOK, 0, 0);
			PostQuitMessage(0);
			break;
		default:
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

BOOL CALLBACK AboutDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
		SetWindowText(GetDlgItem(hwndDlg, IDD_ABOUT_TEXT), ABOUT_TEXT);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK NotifyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		nidIcon.cbSize = sizeof nidIcon;
		nidIcon.hWnd = hwnd;
		nidIcon.uID = 42;
		nidIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		nidIcon.hIcon = hIcon;
		nidIcon.uVersion = NOTIFYICON_VERSION_4;
		nidIcon.uCallbackMessage = NWM_NOTIFY;
		lstrcpy(nidIcon.szTip, L"ClickKeys");
		Shell_NotifyIcon(NIM_ADD, &nidIcon);
		Shell_NotifyIcon(NIM_SETVERSION, &nidIcon);

		RegisterHotKey(hwnd, HOTKEY_ACTIVATE, MOD_NOREPEAT, VK_PAUSE);
		RegisterHotKey(hwnd, HOTKEY_TOGGLE, MOD_WIN | MOD_SHIFT | MOD_ALT | MOD_NOREPEAT, 'M');
		RegisterHotKey(hwnd, HOTKEY_TOGGLE_HOLD, MOD_SHIFT | MOD_ALT | MOD_NOREPEAT, 'M');
		RegisterHotKey(hwnd, HOTKEY_EXIT, MOD_SHIFT | MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'M');
		SendMessage(hwnd, NWM_TOGGLE, 0, 0);
		SendMessage(hwnd, NWM_TOGGLE_HOLD, 0, 0);
		return 0;
	case WM_DESTROY:
		if (toggle)
			UnregisterHotKey(hwnd, HOTKEY_ACTIVATE);
		UnregisterHotKey(hwnd, HOTKEY_TOGGLE);
		UnregisterHotKey(hwnd, HOTKEY_EXIT);

		Shell_NotifyIcon(NIM_DELETE , &nidIcon);
		PostQuitMessage(0);
		return 0;
	case NWM_NOTIFY:
		switch (LOWORD(lParam)) {
		case WM_CONTEXTMENU: {
			HKEY hkRun = AutoRunKey();
			BOOL bRun = RegQueryValueEx(hkRun, L"clickkeys", NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
			RegCloseKey(hkRun);
			SetForegroundWindow(hwnd);
			CheckMenuItem(hMenu, IDC_TOGGLE, MF_BYCOMMAND | (toggle ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDC_TOGGLE_HOLD, MF_BYCOMMAND | (toggleHold ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(hMenu, IDC_AUTORUN, MF_BYCOMMAND | (bRun ? MF_CHECKED : MF_UNCHECKED));
			TrackPopupMenu(hMenu, 0, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam), 0, hwnd, NULL);
			break;
		  }
		}
		return 0;
	case NWM_TOGGLE:
		toggle = !toggle;
		if (toggle)
			RegisterHotKey(hwnd, HOTKEY_ACTIVATE, MOD_NOREPEAT, VK_PAUSE);
		else
			UnregisterHotKey(hwnd, HOTKEY_ACTIVATE);
		return 0;
	case NWM_TOGGLE_HOLD:
		toggleHold = !toggleHold;
		PostThreadMessage(dwThread, toggleHold ? AWM_ENABLE_HOOK : AWM_DISABLE_HOOK, 0, 0);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXIT:
			DestroyWindow(hwnd);
			break;
		case IDC_ABOUT:
			DialogBox(hInstance, L"AboutDialog", hwnd, AboutDialogProc);
			break;
		case IDC_TOGGLE:
			SendMessage(hwnd, NWM_TOGGLE, 0, 0);
			break;
		case IDC_TOGGLE_HOLD:
			SendMessage(hwnd, NWM_TOGGLE_HOLD, 0, 0);
			break;
		case IDC_AUTORUN: {
			HKEY hkRun = AutoRunKey();
			TCHAR szPath[MAX_PATH];
			GetModuleFileName(NULL, szPath, MAX_PATH);
			if (RegQueryValueEx(hkRun, L"clickkeys", NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
				RegDeleteValue(hkRun, L"clickkeys");
			else
				RegSetValueEx(hkRun, L"clickkeys", 0, REG_SZ, (const BYTE*) szPath, (lstrlen(szPath) + 1) * sizeof(TCHAR));
			RegCloseKey(hkRun);
			break;
		  }
		}
		return 0;
	case WM_HOTKEY:
		switch (wParam) {
		case HOTKEY_ACTIVATE:
			mouse_event(down, 0, 0, 0, 0);
			mouse_event(up, 0, 0, 0, 0);
			break;
		case HOTKEY_TOGGLE:
			SendMessage(hwnd, NWM_TOGGLE, 0, 0);
			break;
		case HOTKEY_TOGGLE_HOLD:
			SendMessage(hwnd, NWM_TOGGLE_HOLD, 0, 0);
			break;
		case HOTKEY_EXIT:
			DestroyWindow(hwnd);
			break;
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ENTRY_POINT
{
	WNDCLASS wc = {0};
	MSG msg;
	BOOL swapped = GetSystemMetrics(SM_SWAPBUTTON);
	HANDLE hThread;

	down = swapped ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
	up   = swapped ? MOUSEEVENTF_RIGHTUP   : MOUSEEVENTF_LEFTUP;

	hInstance = (HINSTANCE) GetModuleHandle(NULL);
	hIcon = LoadImage(hInstance, L"MouseIcon", IMAGE_ICON, 16, 16, 0);
	hMenu = GetSubMenu(LoadMenu(hInstance, L"NotifyMenu"), 0);

	wc.lpfnWndProc   = NotifyWindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = L"ClickKeysNotify";
	if (!RegisterClass(&wc)) return 1;

	hThread = CreateThread(NULL, 0, LowLevelKeyboardThread, NULL, 0, &dwThread);
	hwnd = CreateWindow(L"ClickKeysNotify", L"ClickKeysNotify",
			0, 0, 0, 0, 0, NULL, NULL, hInstance, 0);
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	PostThreadMessage(dwThread, AWM_EXIT_HOOK, 0, 0);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return msg.wParam;
}
