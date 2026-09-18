// PlatformToolStubs.cpp
// Headless platform hooks required by command-line tools on macOS.
//
// The command-line tools link the engine libraries but never create a window,
// so the window-relative platform hooks are answered here instead of by the
// SDL application layer. Keeping them in one place means the tools do not
// depend on which engine objects the linker happens to pull in.

#include "Kernel.h"

//	The message queue itself lives in Platform/PlatformMessage.cpp, which these
//	tools link: the queue never receives anything without a window, so the tools
//	see an empty queue rather than a second, divergent implementation.

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
