// JournalRecord.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "JournalRecord.h"
#define MAX_RECORD	2000

HINSTANCE g_hInst;

namespace {
	static const LPCTSTR g_Title = TEXT("JournalRecord");
	static HHOOK g_hHook = NULL;
	static HWND g_hWnd = NULL;
	static BOOL g_bHook = FALSE;
	static BOOL g_bRecording = FALSE;
	static int n = 0;
	static int exit = 0;
	static EVENTMSG g_Events[MAX_RECORD] = {0};
	static DWORD g_dwStart = 0;
	static DWORD g_dwAdjust = 0;

	static HANDLE g_hFile = NULL;
};

BOOL SetMainHWND(HWND hMyWnd)
{
	if (hMyWnd == NULL) {
		MessageBox(g_hWnd, TEXT("HWNDが間違っています"), g_Title, 0);
		return FALSE;
	} else {
		MessageBox(g_hWnd, TEXT("HWNDを受け取りました"), g_Title, 0);
	}
	g_hWnd = hMyWnd;
	return TRUE;
}

BOOL StartRecord()
{
	ZeroMemory(g_Events, sizeof(g_Events));
	g_hHook = SetWindowsHookEx(WH_JOURNALRECORD, (HOOKPROC)MyHookProc, g_hInst, 0);
	if (g_hHook == NULL) {
		TCHAR szError[64];
		wsprintf(szError, L"フックに失敗しました :%d", GetLastError());
		MessageBox(g_hWnd, szError, g_Title, 0);
		return FALSE;
	}

	g_dwStart = GetTickCount();
	n = 0;
	g_bHook = TRUE;
	g_bRecording = TRUE;

	g_hFile = CreateFile(L"journalrecord.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (g_hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	SetFilePointer(g_hFile, 0, NULL, FILE_END);
	return TRUE;
}

BOOL StartPlay()
{
	g_hHook = SetWindowsHookEx(WH_JOURNALPLAYBACK, (HOOKPROC)MyPlayProc, g_hInst, 0);
	if (g_hHook == NULL) {
		MessageBox(g_hWnd, TEXT("フックに失敗しました"), g_Title, 0);
		return FALSE;
	}

	g_dwAdjust = GetTickCount() - g_dwStart;
	n = 0;
	g_bHook = TRUE;
	return TRUE;
}

BOOL EndHook()
{
	if (g_hFile != NULL) {
		CloseHandle(g_hFile);
		g_hFile = NULL;
	}
	if (!UnhookWindowsHookEx(g_hHook)) {
		MessageBox(g_hWnd, TEXT("フック解除に失敗しました"), g_Title, 0);
		return FALSE;
	} else {
		MessageBox(g_hWnd, TEXT("フック解除に成功しました"), g_Title, 0);
		g_bHook = FALSE;
	}
	return TRUE;
}

void MenuCheck()
{
	HMENU hMenu, hSubMenu;
	hMenu = GetMenu(g_hWnd);
	hSubMenu = GetSubMenu(hMenu, 1);

	if (g_bHook == TRUE) {
		EnableMenuItem(hSubMenu, 0, MF_BYPOSITION | MF_GRAYED);
		EnableMenuItem(hSubMenu, 1, MF_BYPOSITION | MF_GRAYED);
		return;
	}

	if (g_bRecording == TRUE && g_bHook == FALSE) {
		EnableMenuItem(hSubMenu, 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(hSubMenu, 1, MF_BYPOSITION | MF_ENABLED);
	}
	if (g_bRecording == FALSE && g_bHook == FALSE) {
		EnableMenuItem(hSubMenu, 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(hSubMenu, 1, MF_BYPOSITION | MF_GRAYED);
	}
}

BOOL IsEndOK()
{
	if (g_bHook) {
		return FALSE;
	} else {
		return TRUE;
	}
}

LRESULT CALLBACK MyHookProc(int nCode, WPARAM wp, LPARAM lp)
{
	EVENTMSG *lpEM;

	if (nCode == HC_ACTION) {
		lpEM = (EVENTMSG*)lp;
		if (lpEM->message == WM_KEYDOWN && LOBYTE(lpEM->paramL) == VK_F2) {
			exit = n;
			PostMessage(g_hWnd, WM_RECORD_END, 0, 0);
			PostMessage(g_hWnd, WM_END_HOOK, 0, 0);
			return TRUE;
		}
		LPCSTR msg = NULL;
		switch (lpEM->message) {
		case WM_ACTIVATE:	msg = "Open\r\n";			break;
		case WM_SYSKEYDOWN: msg = "SysKeyDown\r\n";		break;
		case WM_SYSKEYUP:	msg = "SysKeyUp\r\n";		break;
		case WM_KEYDOWN:	msg = "KeyDown\r\n";		break;
		case WM_KEYUP:		msg = "KeyUp\r\n";			break;
		//case WM_MOUSEMOVE:	msg = "MouseMove\r\n";		break;
		case WM_LBUTTONDOWN:msg = "LButtonDown\r\n";	break;
		case WM_LBUTTONUP:	msg = "LButtonUp\r\n";		break;
		case WM_RBUTTONDOWN:msg = "RButtonDown\r\n";	break;
		case WM_RBUTTONUP:	msg = "RButtonUp\r\n";		break;
		case WM_MBUTTONDOWN:msg = "MButtonDown\r\n";	break;
		case WM_MBUTTONUP:	msg = "MButtonUp\r\n";		break;
		}
		DWORD dwWritten = 0;
		if (msg != NULL && !WriteFile(g_hFile, msg, strlen(msg), &dwWritten, NULL)) {
			MessageBox(NULL, L"WriteFile!!", g_Title, 0);
		}
		FlushFileBuffers(g_hFile);

		g_Events[n].hwnd	= lpEM->hwnd;
		g_Events[n].message	= lpEM->message;
		g_Events[n].paramH = lpEM->paramH;
		g_Events[n].paramL = lpEM->paramL;
		g_Events[n].time = lpEM->time;
		n++;
		if (n >= MAX_RECORD - 1) {
			exit = n;
			PostMessage(g_hWnd, WM_RECORD_MAX, 0, 0);
			PostMessage(g_hWnd, WM_END_HOOK, 0, 0);
			return TRUE;
		}
		return TRUE;
	}

	return CallNextHookEx(g_hHook, nCode, wp, lp);
}

LRESULT CALLBACK MyPlayProc(int nCode, WPARAM wp, LPARAM lp)
{
	EVENTMSG* lpEM;
	DWORD dwReturnTime;

	if (nCode == HC_GETNEXT) {
		lpEM = (EVENTMSG*)lp;
		lpEM->hwnd = g_Events[n].hwnd;
		lpEM->message = g_Events[n].message;
		lpEM->paramH = g_Events[n].paramH;
		lpEM->paramL = g_Events[n].paramL;
		lpEM->time = g_Events[n].time + g_dwAdjust;
		dwReturnTime = lpEM->time - GetTickCount();

		if ((int)dwReturnTime < 0) {
			dwReturnTime = 0;
			lpEM->time = GetTickCount();
		}
		return dwReturnTime;
	}
	if (nCode == HC_SKIP) {
		n++;
		if (exit <= n || g_Events[n].message == 0) {
			PostMessage(g_hWnd, WM_PLAY_END, 0, 0);
			PostMessage(g_hWnd, WM_END_HOOK, 0, 0);
			return FALSE;
		}
	}

	return CallNextHookEx(g_hHook, nCode, wp, lp);
}