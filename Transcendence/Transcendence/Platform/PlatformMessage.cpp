//	PlatformMessage.cpp
//	macOS message queue and Win32 message-payload helpers.
//
//	This translation unit owns the queue that carries Win32-style messages from
//	the SDL event pump (and the SDL timer thread) to the game UI, plus the
//	payload packing helpers described in PlatformMessage.h.
//
//	It is kept free of the SDL window layer so that the queue can be exercised
//	headlessly; the only platform state it reads is the cursor position, which
//	PlatformInput.cpp tracks.

#include "PlatformMessage.h"

#include <deque>
#include <mutex>
#include <atomic>
#include <time.h>

//	The shell drains the queue once per frame. The producers are the SDL event
//	pump, the SDL timer thread and the game's task-completion callbacks, so
//	every access goes through the mutex.

static std::mutex g_MessageQueueCS;
static std::deque<SPlatformMessage> g_MessageQueue;

static std::atomic<void*> g_pMessageWindow(nullptr);
static PlatformMessageDispatch g_pMessageDispatch = nullptr;
static PlatformCloseRequest g_pCloseRequest = nullptr;

//	Milliseconds since an unspecified fixed point, in the same spirit as
//	GetTickCount(). The queue only needs a monotonically non-decreasing stamp,
//	so it reads the monotonic clock directly instead of going through SDL.

static DWORD PlatformGetMessageTimeMs (void)
	{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		return 0;

	return (DWORD)((unsigned long long)ts.tv_sec * 1000ULL
			+ (unsigned long long)(ts.tv_nsec / 1000000));
	}

//	Fills every field of a message, so that a dequeued message never exposes
//	uninitialized payload (PDR-017). The cursor position is reported in screen
//	coordinates, which is the space Win32 MSG.pt uses.

static void FillMessage (SPlatformMessage *pMessage, int msg, WPARAM wParam, LPARAM lParam)
	{
	POINT pt;
	pt.x = 0;
	pt.y = 0;
	PlatformGetCursorPos(&pt);

	pMessage->hwnd = g_pMessageWindow.load();
	pMessage->message = (unsigned int)msg;
	pMessage->wParam = wParam;
	pMessage->lParam = lParam;
	pMessage->time = PlatformGetMessageTimeMs();
	pMessage->pt = pt;
	}

void PlatformSetMessageWindow (void* pWindow)
	{
	g_pMessageWindow.store(pWindow);
	}

void* PlatformGetMessageWindow (void)
	{
	return g_pMessageWindow.load();
	}

void PlatformSetMessageDispatch (PlatformMessageDispatch pDispatch)
	{
	g_pMessageDispatch = pDispatch;
	}

void PlatformSetCloseRequest (PlatformCloseRequest pCloseRequest)
	{
	g_pCloseRequest = pCloseRequest;
	}

bool PlatformPostMessage (int msg, WPARAM wParam, LPARAM lParam)
	{
	SPlatformMessage message;
	FillMessage(&message, msg, wParam, lParam);

	std::lock_guard<std::mutex> lock(g_MessageQueueCS);
	g_MessageQueue.push_back(message);
	return true;
	}

int PlatformPeekMessage (SPlatformMessage* pMessage)
	{
	std::lock_guard<std::mutex> lock(g_MessageQueueCS);

	if (g_MessageQueue.empty())
		return 0;

	if (pMessage)
		*pMessage = g_MessageQueue.front();

	g_MessageQueue.pop_front();
	return 1;
	}

void PlatformClearMessageQueue (void)
	{
	std::lock_guard<std::mutex> lock(g_MessageQueueCS);
	g_MessageQueue.clear();
	}

LRESULT PlatformSendMessage (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
	(void)hWnd;

	//	PDR-018: Win32 SendMessage runs the window procedure before returning.
	//	The shell registers the dispatcher that owns the game-UI message
	//	handling, so a synchronous send observes the handler's effects as soon
	//	as the call returns. A headless build has no dispatcher and falls back
	//	to the queue, which is what the message loop would have drained anyway.

	if (Msg == WM_CLOSE || Msg == WM_DESTROY)
		{
		PlatformCloseRequest pCloseRequest = g_pCloseRequest;
		if (pCloseRequest != nullptr)
			{
			pCloseRequest();
			return 0;
			}
		}

	PlatformMessageDispatch pDispatch = g_pMessageDispatch;
	if (pDispatch != nullptr)
		{
		SPlatformMessage message;
		FillMessage(&message, (int)Msg, wParam, lParam);
		return pDispatch(message);
		}

	PlatformPostMessage((int)Msg, wParam, lParam);
	return 0;
	}

//	PDR-019: Win32 packs two coordinates into one 32-bit payload as two signed
//	16-bit values and unpacks them with GET_X_LPARAM/GET_Y_LPARAM, which
//	sign-extend. The macOS shell used to unpack with an unsigned cast, so a
//	click on a monitor to the left of the origin came back as a coordinate far
//	to the right, and it packed with a plain truncation, so an out-of-range
//	coordinate wrapped to the opposite side of the desktop.

static int ClampToSigned16 (int iValue)
	{
	if (iValue > 32767)
		return 32767;

	if (iValue < -32768)
		return -32768;

	return iValue;
	}

DWORD PlatformPackPoint (int x, int y)
	{
	const DWORD dwX = (DWORD)(unsigned short)(short)ClampToSigned16(x);
	const DWORD dwY = (DWORD)(unsigned short)(short)ClampToSigned16(y);

	return (dwX | (dwY << 16));
	}

void PlatformUnpackPoint (DWORD dwPoint, int *px, int *py)
	{
	if (px)
		*px = (int)(short)(unsigned short)(dwPoint & 0xFFFF);

	if (py)
		*py = (int)(short)(unsigned short)((dwPoint >> 16) & 0xFFFF);
	}

DWORD PlatformPackMouseWheel (WORD wKeyFlags, int iDelta)
	{
	const DWORD dwDelta = (DWORD)(unsigned short)(short)ClampToSigned16(iDelta);

	return ((DWORD)wKeyFlags | (dwDelta << 16));
	}

int PlatformUnpackMouseWheelDelta (DWORD dwWParam)
	{
	return (int)(short)(unsigned short)((dwWParam >> 16) & 0xFFFF);
	}

WORD PlatformUnpackMouseWheelFlags (DWORD dwWParam)
	{
	return (WORD)(dwWParam & 0xFFFF);
	}
