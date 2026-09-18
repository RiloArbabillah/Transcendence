// PlatformToolStubs.cpp
// Headless platform hooks required by command-line tools on macOS.
//
// The command-line tools link the engine libraries but never create a window,
// so the window-relative platform hooks are answered here instead of by the
// SDL application layer. Keeping them in one place means the tools do not
// depend on which engine objects the linker happens to pull in.

#include "Kernel.h"

int PlatformPeekMessage(int *pMsg, int *pWParam, void **ppLParam)
	{
	if (pMsg) *pMsg = 0;
	if (pWParam) *pWParam = 0;
	if (ppLParam) *ppLParam = nullptr;
	return 0;
	}

//	No window means nowhere to move the cursor.

void PlatformWarpMouseInWindow(int xClient, int yClient)
	{
	(void)xClient;
	(void)yClient;
	}

//	There is no video device to drive, so the cursor stays visible and the
//	mouse is never captured. Win32 reports success for both.

int PlatformShowCursor(BOOL bShow)
	{
	(void)bShow;
	return 1;
	}

BOOL PlatformSetCapture(HWND hWnd)
	{
	(void)hWnd;
	return TRUE;
	}

BOOL PlatformReleaseCapture(void)
	{
	return TRUE;
	}
