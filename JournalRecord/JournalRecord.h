#include "stdafx.h"

#ifdef JOURNALRECORD_EXPORTS
#define DLL_EXPORT		__declspec(dllexport)
#else
#define DLL_EXPORT		__declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	DLL_EXPORT BOOL SetMainHWND(HWND);
	DLL_EXPORT BOOL StartRecord();
	DLL_EXPORT BOOL StartPlay();
	DLL_EXPORT BOOL EndHook();
	DLL_EXPORT void MenuCheck();
	DLL_EXPORT BOOL IsEndOK();
	DLL_EXPORT LRESULT CALLBACK MyHookProc(int, WPARAM, LPARAM);
	DLL_EXPORT LRESULT CALLBACK MyPlayProc(int, WPARAM, LPARAM);

#define WM_PLAY_END		WM_USER
#define WM_RECORD_MAX	(WM_USER+1)
#define WM_RECORD_END	(WM_USER+2)
#define WM_END_HOOK		(WM_USER+3)

#ifdef __cplusplus
}
#endif
