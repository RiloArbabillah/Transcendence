//	GameUIBridge.cpp
//	Game UI integration

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"
#include "Platform/PlatformMessage.h"
#include <cstdio>
#include <cstdarg>
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <sys/stat.h>

static CTranscendenceController* g_pController = nullptr;

static void log_msg(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

static void log_va(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log_msg(buf);
}


//	PDR-018: SendMessage() runs the registered dispatcher before it returns, so
//	the engine sees the effect of a synchronous send immediately instead of on
//	the next frame. The dispatcher handles a single message; UpdateGameUI()
//	below drains the queue with the same routine.

static LRESULT DispatchGameUIMessage(const SPlatformMessage& message);

//	The platform layer expects a void callback; the shell's own close request
//	reports whether the session actually accepted the close.

static void OnPlatformCloseRequest(void)
	{
	RequestGameClose();
	}

bool InitGameUI(SAppState& state, const char *pszCommandLine)
{
	if (!CHumanInterface::Create())
		{
		log_msg("InitGameUI error: unable to create human interface");
		return false;
		}

	//	The message queue stamps every message with the SDL window pointer (the
	//	macOS port has no real HWND) and asks the shell to close the session when
	//	the engine sends WM_CLOSE/WM_DESTROY.

	PlatformSetMessageWindow(state.pWindow);
	PlatformSetCloseRequest(OnPlatformCloseRequest);
	PlatformSetMessageDispatch(DispatchGameUIMessage);

    g_pController = new CTranscendenceController();
    g_pHI->SetController(g_pController);

    SHIOptions Options;
    Options.m_bWindowedMode = true;
    Options.m_bNoGPUAcceleration = false;
    CString sError;
    const char *pCmdLine = (pszCommandLine ? pszCommandLine : "");
    ALERROR error = g_pController->OnBoot(const_cast<char *>(pCmdLine), &Options, &sError);

    if (error == NOERROR)
    {
        if (!g_pHI->InitFromSDL((HWND)state.pWindow, Options, &sError))
            error = ERR_FAIL;
    }

    if (error == NOERROR)
        error = g_pController->OnInit(&sError);

    if (error != NOERROR)
		{
        log_va("InitGameUI error: %d (%s)", error, sError.GetASCIIZPointer());
		CHumanInterface::Destroy();
		g_pController = nullptr;
		return false;
		}

	return true;
}

//	Handles one queued message. The mouse coordinates are unpacked through the
//	platform helpers so that they sign-extend exactly the way Win32's
//	GET_X_LPARAM/GET_Y_LPARAM do (PDR-019); the old code cast through
//	(unsigned short), which turned a click on a monitor left of the origin into
//	a coordinate far to the right.

static LRESULT DispatchGameUIMessage(const SPlatformMessage& message)
	{
	if (!g_pHI)
		return 0;

	const UINT msg = message.message;
	const WPARAM wParam = message.wParam;
	const LPARAM lParam = message.lParam;

	int x = 0;
	int y = 0;

	if (msg == WM_HI_COMMAND)
		g_pHI->OnPostCommand(lParam);
	else if (msg == WM_HI_TASK_COMPLETE)
		g_pHI->OnTaskComplete((DWORD)wParam, lParam);
	else if (msg == WM_TIMER)
		g_pHI->OnTimer((DWORD)wParam);
	else if (msg == WM_KEYDOWN)
		g_pHI->WMKeyDown((int)wParam, (DWORD)lParam);
	else if (msg == WM_KEYUP)
		g_pHI->WMKeyUp((int)wParam, (DWORD)lParam);
	else if (msg == WM_MOUSEMOVE)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMMouseMove(x, y, (DWORD)wParam);
		}
	else if (msg == WM_LBUTTONDOWN)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMLButtonDown(x, y, (DWORD)wParam);
		}
	else if (msg == WM_RBUTTONDOWN)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMRButtonDown(x, y, (DWORD)wParam);
		}
	else if (msg == WM_MBUTTONDOWN)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMMButtonDown(x, y, (DWORD)wParam);
		}
	else if (msg == WM_LBUTTONUP)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMLButtonUp(x, y, (DWORD)wParam);
		}
	else if (msg == WM_RBUTTONUP)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMRButtonUp(x, y, (DWORD)wParam);
		}
	else if (msg == WM_MBUTTONUP)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMMButtonUp(x, y, (DWORD)wParam);
		}
	else if (msg == WM_MOUSEWHEEL)
		{
		PlatformUnpackPoint((DWORD)lParam, &x, &y);
		g_pHI->WMMouseWheel(PlatformUnpackMouseWheelDelta((DWORD)wParam), x, y, (DWORD)PlatformUnpackMouseWheelFlags((DWORD)wParam));
		}
	else if (msg == WM_SIZE)
		{
		g_pHI->WMSize((int)LOWORD((DWORD)lParam), (int)HIWORD((DWORD)lParam), (int)wParam);
		}
	else if (msg == WM_MOVE)
		{
		g_pHI->WMMove((int)LOWORD((DWORD)lParam), (int)HIWORD((DWORD)lParam));
		}
	else if (msg == WM_CHAR)
		g_pHI->WMChar((char)wParam, 0);

	return 0;
	}

void UpdateGameUI(SAppState& state)
{
    (void)state;
    static int tick = 0;
    tick++;

    SPlatformMessage message;
    while (PlatformPeekMessage(&message))
    {
        if (!g_pHI)
            break;

        DispatchGameUIMessage(message);
    }

    if (g_pHI) {
        g_pHI->OnAnimate();
    }
}

bool RequestGameClose(void)
{
	if (!g_pHI)
		return true;

	g_pHI->Exit();
	return !App_IsRunning();
}

void CleanUpGameUI(void)
{
	PlatformSetMessageDispatch(nullptr);
	PlatformSetCloseRequest(nullptr);
	PlatformSetMessageWindow(nullptr);

	CHumanInterface::Destroy();
	g_pController = nullptr;
}
