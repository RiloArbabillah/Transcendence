//	AppCore.cpp
//	macOS SDL2 application shell
//
//	Provides SDL2-based platform layer for the game engine

#include "AppCore.h"
#include <SDL2/SDL.h>
#include "Alchemy.h"
#include "Kernel.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <queue>
#include <map>
#include <mutex>
#include <string>
#include <signal.h>
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#ifndef MAKELONG
#define MAKELONG(a, b) ((unsigned int)(((unsigned short)(a)) | ((unsigned int)((unsigned short)(b))) << 16))
#endif

#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif
#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif
#ifndef WM_CHAR
#define WM_CHAR 0x0102
#endif
#ifndef WM_LBUTTONDOWN
#define WM_LBUTTONDOWN 0x0201
#endif
#ifndef WM_LBUTTONUP
#define WM_LBUTTONUP 0x0202
#endif
#ifndef WM_RBUTTONDOWN
#define WM_RBUTTONDOWN 0x0204
#endif
#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP 0x0205
#endif
#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WM_SIZE
#define WM_SIZE 0x0005
#endif
#ifndef WM_MOVE
#define WM_MOVE 0x0003
#endif
#ifndef WM_MBUTTONDOWN
#define WM_MBUTTONDOWN 0x0207
#endif
#ifndef WM_MBUTTONUP
#define WM_MBUTTONUP 0x0208
#endif

#ifndef MK_LBUTTON
#define MK_LBUTTON 0x0001
#endif
#ifndef MK_RBUTTON
#define MK_RBUTTON 0x0002
#endif
#ifndef MK_SHIFT
#define MK_SHIFT 0x0004
#endif
#ifndef MK_CONTROL
#define MK_CONTROL 0x0008
#endif
#ifndef MK_MBUTTON
#define MK_MBUTTON 0x0010
#endif

#ifndef VK_UP
#define VK_UP 0x26
#endif
#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif
#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif
#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif
#ifndef VK_SPACE
#define VK_SPACE 0x20
#endif
#ifndef VK_BACK
#define VK_BACK 0x08
#endif
#ifndef VK_TAB
#define VK_TAB 0x09
#endif
#ifndef VK_SHIFT
#define VK_SHIFT 0x10
#endif
#ifndef VK_CONTROL
#define VK_CONTROL 0x11
#endif
#ifndef VK_MENU
#define VK_MENU 0x12
#endif
#ifndef VK_F1
#define VK_F1 0x70
#endif
#ifndef VK_F2
#define VK_F2 0x71
#endif
#ifndef VK_F3
#define VK_F3 0x72
#endif
#ifndef VK_F4
#define VK_F4 0x73
#endif
#ifndef VK_F5
#define VK_F5 0x74
#endif
#ifndef VK_F6
#define VK_F6 0x75
#endif
#ifndef VK_F7
#define VK_F7 0x76
#endif
#ifndef VK_F8
#define VK_F8 0x77
#endif
#ifndef VK_F9
#define VK_F9 0x78
#endif
#ifndef VK_F10
#define VK_F10 0x79
#endif
#ifndef VK_F11
#define VK_F11 0x7A
#endif
#ifndef VK_F12
#define VK_F12 0x7B
#endif
#ifndef VK_PRIOR
#define VK_PRIOR 0x21
#endif
#ifndef VK_NEXT
#define VK_NEXT 0x22
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_INSERT
#define VK_INSERT 0x2D
#endif
#ifndef VK_DELETE
#define VK_DELETE 0x2E
#endif
#ifndef VK_LWIN
#define VK_LWIN 0x5B
#endif
#ifndef VK_RWIN
#define VK_RWIN 0x5C
#endif

static const char* GetAppLogPath()
{
    static char sPath[1024];
    static bool bInit = false;

    if (!bInit)
    {
        const char* pHome = getenv("HOME");
        if (pHome && *pHome)
        {
            char basePath[1024];
            snprintf(basePath, sizeof(basePath), "%s/Library/Application Support", pHome);
            mkdir(basePath, 0755);

            snprintf(basePath, sizeof(basePath), "%s/Library/Application Support/Kronosaur", pHome);
            mkdir(basePath, 0755);

            snprintf(basePath, sizeof(basePath), "%s/Library/Application Support/Kronosaur/Transcendence", pHome);
            mkdir(basePath, 0755);

            snprintf(sPath, sizeof(sPath), "%s/trans_app.log", basePath);
        }
        else
            snprintf(sPath, sizeof(sPath), "%s", "/tmp/trans_app.log");

        bInit = true;
    }

    return sPath;
}

constexpr int DEFAULT_WIDTH = 1024;
constexpr int DEFAULT_HEIGHT = 768;

static FILE* g_Log = nullptr;

static void log_msg(const char* pMsg) {
    if (!g_Log) {
        g_Log = fopen(GetAppLogPath(), "w");
    }
    if (g_Log) {
        fprintf(g_Log, "%s\n", pMsg);
        fflush(g_Log);
    }
    fprintf(stderr, "%s\n", pMsg);
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

SAppState g_AppState;

int g_PlatformMouseX = 0;
int g_PlatformMouseY = 0;
int g_PlatformWindowWidth = 1024;
int g_PlatformWindowHeight = 768;

static std::mutex g_MessageQueueCS;
static std::mutex g_TimerCS;
static std::map<unsigned int, SDL_TimerID> g_Timers;
static const int PLATFORM_WM_TIMER = 0x0113;

static Uint32 TimerThunk(Uint32 interval, void *param)
{
    const unsigned int dwTimerID = (unsigned int)(uintptr_t)param;
    PlatformPostMessage(PLATFORM_WM_TIMER, (int)dwTimerID, nullptr);
    return interval;
}

static DWORD SDLModToMKFlags(SDL_Keymod mod)
{
    DWORD dwFlags = 0;

    if (mod & KMOD_SHIFT)
        dwFlags |= MK_SHIFT;

    if (mod & KMOD_CTRL)
        dwFlags |= MK_CONTROL;

    return dwFlags;
}

static DWORD SDLMouseStateToMKFlags(Uint32 dwButtons)
{
    DWORD dwFlags = SDLModToMKFlags(SDL_GetModState());

    if (dwButtons & SDL_BUTTON(SDL_BUTTON_LEFT))
        dwFlags |= MK_LBUTTON;

    if (dwButtons & SDL_BUTTON(SDL_BUTTON_RIGHT))
        dwFlags |= MK_RBUTTON;

    if (dwButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE))
        dwFlags |= MK_MBUTTON;

    return dwFlags;
}

SHORT PlatformGetAsyncKeyState(int vk)
{
    SDL_Keymod mod = SDL_GetModState();
    switch (vk)
    {
        case VK_SHIFT:
            return (mod & KMOD_SHIFT) ? (SHORT)0x8000 : 0;
        case VK_CONTROL:
            return (mod & KMOD_CTRL) ? (SHORT)0x8000 : 0;
        case VK_MENU:
            return (mod & KMOD_ALT) ? (SHORT)0x8000 : 0;
        case VK_NUMLOCK:
            return (mod & KMOD_NUM) ? (SHORT)0x8000 : 0;
    }

    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    SDL_Scancode sc = SDL_SCANCODE_UNKNOWN;
    if (vk >= 'A' && vk <= 'Z') sc = (SDL_Scancode)(SDL_SCANCODE_A + (vk - 'A'));
    else if (vk == '0') sc = SDL_SCANCODE_0;
    else if (vk >= '1' && vk <= '9') sc = (SDL_Scancode)(SDL_SCANCODE_1 + (vk - '1'));
    else if (vk == VK_DOWN) sc = SDL_SCANCODE_DOWN;
    else if (vk == VK_UP) sc = SDL_SCANCODE_UP;
    else if (vk == VK_NEXT) sc = SDL_SCANCODE_PAGEDOWN;
    else if (vk == VK_PRIOR) sc = SDL_SCANCODE_PAGEUP;
    else if (vk == VK_END) sc = SDL_SCANCODE_END;
    if (sc != SDL_SCANCODE_UNKNOWN && keyState[sc])
        return (SHORT)0x8000;
    return 0;
}

SHORT PlatformGetKeyState(int vk)
{
    SDL_Keymod mod = SDL_GetModState();
    SHORT result = PlatformGetAsyncKeyState(vk);
    if (vk == VK_NUMLOCK && (mod & KMOD_NUM))
        result |= 0x0001;
    return result;
}

static DWORD SDLMouseButtonEventToMKFlags(const SDL_MouseButtonEvent &Event, bool bIncludeCurrentButton)
{
    Uint32 dwButtons = SDL_GetMouseState(nullptr, nullptr);

    if (bIncludeCurrentButton)
        dwButtons |= SDL_BUTTON(Event.button);
    else
        dwButtons &= ~SDL_BUTTON(Event.button);

    return SDLMouseStateToMKFlags(dwButtons);
}

static void RecreateFrameBuffer(int cxWidth, int cyHeight)
{
    if (cxWidth <= 0 || cyHeight <= 0)
        return;

    g_PlatformWindowWidth = cxWidth;
    g_PlatformWindowHeight = cyHeight;

    if (g_AppState.pTexture)
        {
        SDL_DestroyTexture(g_AppState.pTexture);
        g_AppState.pTexture = nullptr;
        }

    if (g_AppState.pFrameBuffer)
        {
        delete[] g_AppState.pFrameBuffer;
        g_AppState.pFrameBuffer = nullptr;
        }

    g_AppState.cxWidth = cxWidth;
    g_AppState.cyHeight = cyHeight;

    g_AppState.pTexture = SDL_CreateTexture(
        g_AppState.pRenderer,
        SDL_PIXELFORMAT_BGRA32,
        SDL_TEXTUREACCESS_STREAMING,
        g_AppState.cxWidth,
        g_AppState.cyHeight
    );

    if (!g_AppState.pTexture)
        {
        log_msg("RecreateFrameBuffer: CreateTexture failed");
        g_AppState.bRunning = false;
        return;
        }

    g_AppState.pFrameBuffer = new uint32_t[g_AppState.cxWidth * g_AppState.cyHeight];
    memset(g_AppState.pFrameBuffer, 0, g_AppState.cxWidth * g_AppState.cyHeight * sizeof(uint32_t));
}

int App_Init(void)
{
    log_msg("App_Init: start");

    if (!kernelInit(KERNEL_FLAG_INTERNETS))
    {
        log_msg("App_Init: kernelInit failed");
        return 0;
    }
    log_msg("App_Init: kernelInit OK");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
        log_msg("App_Init: SDL_Init failed");
        return 0;
    }

    log_msg("App_Init: SDL_Init OK");

    SDL_StartTextInput();

    g_AppState.cxWidth = DEFAULT_WIDTH;
    g_AppState.cyHeight = DEFAULT_HEIGHT;

	SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");

    char buf[256];
    snprintf(buf, sizeof(buf), "App_Init: create window %dx%d", g_AppState.cxWidth, g_AppState.cyHeight);
    log_msg(buf);

    g_AppState.pWindow = SDL_CreateWindow(
        "Transcendence",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        g_AppState.cxWidth,
        g_AppState.cyHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!g_AppState.pWindow)
    {
        log_msg("App_Init: CreateWindow failed");
        SDL_Quit();
        return 0;
    }

    log_msg("App_Init: window created");

    g_AppState.pRenderer = SDL_CreateRenderer(
        g_AppState.pWindow,
        0,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g_AppState.pRenderer)
        g_AppState.pRenderer = SDL_CreateRenderer(
            g_AppState.pWindow,
            0,
            SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC
        );

    if (!g_AppState.pRenderer)
    {
        log_msg("App_Init: CreateRenderer failed");
        SDL_DestroyWindow(g_AppState.pWindow);
        SDL_Quit();
        return 0;
    }

	SDL_RendererInfo rendererInfo;
	if (SDL_GetRendererInfo(g_AppState.pRenderer, &rendererInfo) == 0)
		log_va("App_Init: using SDL renderer: %s", rendererInfo.name);

    RecreateFrameBuffer(g_AppState.cxWidth, g_AppState.cyHeight);
    if (!g_AppState.pTexture || !g_AppState.pFrameBuffer)
    {
        log_msg("App_Init: RecreateFrameBuffer failed");
        SDL_DestroyRenderer(g_AppState.pRenderer);
        SDL_DestroyWindow(g_AppState.pWindow);
        SDL_Quit();
        return 0;
    }

    g_AppState.lastTick = SDL_GetTicks();
    g_AppState.fpsTick = SDL_GetTicks();
    g_AppState.frameCount = 0;
    g_AppState.fps = 0;
    g_AppState.bRunning = true;

    snprintf(buf, sizeof(buf), "App_Init: done %dx%d", g_AppState.cxWidth, g_AppState.cyHeight);
    log_msg(buf);
    return 1;
}

void App_Shutdown(void)
{
    {
        std::lock_guard<std::mutex> lock(g_TimerCS);
        for (auto &entry : g_Timers)
            SDL_RemoveTimer(entry.second);
        g_Timers.clear();
    }

    while (!g_AppState.msgQueue.empty())
        g_AppState.msgQueue.pop();

    if (g_AppState.pFrameBuffer) { delete[] g_AppState.pFrameBuffer; g_AppState.pFrameBuffer = nullptr; }
    if (g_AppState.pTexture) { SDL_DestroyTexture(g_AppState.pTexture); g_AppState.pTexture = nullptr; }
    if (g_AppState.pRenderer) { SDL_DestroyRenderer(g_AppState.pRenderer); g_AppState.pRenderer = nullptr; }
    if (g_AppState.pWindow) { SDL_DestroyWindow(g_AppState.pWindow); g_AppState.pWindow = nullptr; }

    SDL_Quit();
    log_msg("App_Shutdown: done");

    if (g_Log) { fclose(g_Log); g_Log = nullptr; }
}

static unsigned int SDLKeyToVK(SDL_Scancode scanCode)
{
    switch (scanCode)
    {
    case SDL_SCANCODE_UP: return VK_UP;
    case SDL_SCANCODE_DOWN: return VK_DOWN;
    case SDL_SCANCODE_LEFT: return VK_LEFT;
    case SDL_SCANCODE_RIGHT: return VK_RIGHT;
    case SDL_SCANCODE_RETURN: return VK_RETURN;
    case SDL_SCANCODE_ESCAPE: return VK_ESCAPE;
    case SDL_SCANCODE_SPACE: return VK_SPACE;
    case SDL_SCANCODE_BACKSPACE: return VK_BACK;
    case SDL_SCANCODE_TAB: return VK_TAB;
    case SDL_SCANCODE_LSHIFT: return VK_SHIFT;
    case SDL_SCANCODE_RSHIFT: return VK_SHIFT;
    case SDL_SCANCODE_LCTRL: return VK_CONTROL;
    case SDL_SCANCODE_RCTRL: return VK_CONTROL;
    case SDL_SCANCODE_LALT: return VK_MENU;
    case SDL_SCANCODE_RALT: return VK_MENU;
    case SDL_SCANCODE_F1: return VK_F1;
    case SDL_SCANCODE_F2: return VK_F2;
    case SDL_SCANCODE_F3: return VK_F3;
    case SDL_SCANCODE_F4: return VK_F4;
    case SDL_SCANCODE_F5: return VK_F5;
    case SDL_SCANCODE_F6: return VK_F6;
    case SDL_SCANCODE_F7: return VK_F7;
    case SDL_SCANCODE_F8: return VK_F8;
    case SDL_SCANCODE_F9: return VK_F9;
    case SDL_SCANCODE_F10: return VK_F10;
    case SDL_SCANCODE_F11: return VK_F11;
    case SDL_SCANCODE_F12: return VK_F12;
    case SDL_SCANCODE_PAGEUP: return VK_PRIOR;
    case SDL_SCANCODE_PAGEDOWN: return VK_NEXT;
    case SDL_SCANCODE_HOME: return VK_HOME;
    case SDL_SCANCODE_END: return VK_END;
    case SDL_SCANCODE_INSERT: return VK_INSERT;
    case SDL_SCANCODE_DELETE: return VK_DELETE;
    case SDL_SCANCODE_LGUI: return VK_LWIN;
    case SDL_SCANCODE_RGUI: return VK_RWIN;
    case SDL_SCANCODE_A: return 0x41;
    case SDL_SCANCODE_B: return 0x42;
    case SDL_SCANCODE_C: return 0x43;
    case SDL_SCANCODE_D: return 0x44;
    case SDL_SCANCODE_E: return 0x45;
    case SDL_SCANCODE_F: return 0x46;
    case SDL_SCANCODE_G: return 0x47;
    case SDL_SCANCODE_H: return 0x48;
    case SDL_SCANCODE_I: return 0x49;
    case SDL_SCANCODE_J: return 0x4A;
    case SDL_SCANCODE_K: return 0x4B;
    case SDL_SCANCODE_L: return 0x4C;
    case SDL_SCANCODE_M: return 0x4D;
    case SDL_SCANCODE_N: return 0x4E;
    case SDL_SCANCODE_O: return 0x4F;
    case SDL_SCANCODE_P: return 0x50;
    case SDL_SCANCODE_Q: return 0x51;
    case SDL_SCANCODE_R: return 0x52;
    case SDL_SCANCODE_S: return 0x53;
    case SDL_SCANCODE_T: return 0x54;
    case SDL_SCANCODE_U: return 0x55;
    case SDL_SCANCODE_V: return 0x56;
    case SDL_SCANCODE_W: return 0x57;
    case SDL_SCANCODE_X: return 0x58;
    case SDL_SCANCODE_Y: return 0x59;
    case SDL_SCANCODE_Z: return 0x5A;
    case SDL_SCANCODE_0: return 0x30;
    case SDL_SCANCODE_1: return 0x31;
    case SDL_SCANCODE_2: return 0x32;
    case SDL_SCANCODE_3: return 0x33;
    case SDL_SCANCODE_4: return 0x34;
    case SDL_SCANCODE_5: return 0x35;
    case SDL_SCANCODE_6: return 0x36;
    case SDL_SCANCODE_7: return 0x37;
    case SDL_SCANCODE_8: return 0x38;
    case SDL_SCANCODE_9: return 0x39;
    case SDL_SCANCODE_KP_0: return 0x60;
    case SDL_SCANCODE_KP_1: return 0x61;
    case SDL_SCANCODE_KP_2: return 0x62;
    case SDL_SCANCODE_KP_3: return 0x63;
    case SDL_SCANCODE_KP_4: return 0x64;
    case SDL_SCANCODE_KP_5: return 0x65;
    case SDL_SCANCODE_KP_6: return 0x66;
    case SDL_SCANCODE_KP_7: return 0x67;
    case SDL_SCANCODE_KP_8: return 0x68;
    case SDL_SCANCODE_KP_9: return 0x69;
    case SDL_SCANCODE_KP_PLUS: return 0x6B;
    case SDL_SCANCODE_KP_MINUS: return 0x6D;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x6A;
    case SDL_SCANCODE_KP_DIVIDE: return 0x6F;
    case SDL_SCANCODE_KP_PERIOD: return 0x6E;
    default: return (unsigned int)scanCode;
    }
}

static DWORD SDLScancodeToKeyData(SDL_Scancode scanCode)
{
    constexpr DWORD EXTENDED_BIT = (1 << 24);

    switch (scanCode)
    {
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_HOME:
    case SDL_SCANCODE_END:
    case SDL_SCANCODE_PAGEUP:
    case SDL_SCANCODE_PAGEDOWN:
    case SDL_SCANCODE_INSERT:
    case SDL_SCANCODE_DELETE:
    case SDL_SCANCODE_KP_ENTER:
    case SDL_SCANCODE_RCTRL:
    case SDL_SCANCODE_RALT:
        return EXTENDED_BIT;

    default:
        return 0;
    }
}

int App_PumpEvents(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) {
            log_msg("App_PumpEvents: received SDL_QUIT");
			if (RequestGameClose())
				return 0;
			continue;
        }

        switch (event.type)
        {
        case SDL_KEYDOWN:
            PlatformPostMessage(
                WM_KEYDOWN,
                SDLKeyToVK(event.key.keysym.scancode),
                (void *)(uintptr_t)SDLScancodeToKeyData(event.key.keysym.scancode));
            break;
        case SDL_KEYUP:
            PlatformPostMessage(
                WM_KEYUP,
                SDLKeyToVK(event.key.keysym.scancode),
                (void *)(uintptr_t)SDLScancodeToKeyData(event.key.keysym.scancode));
            break;
        case SDL_TEXTINPUT:
            for (const char* p = event.text.text; *p; p++)
                PlatformPostMessage(WM_CHAR, *p, nullptr);
            break;
        case SDL_MOUSEMOTION:
            g_PlatformMouseX = event.motion.x;
            g_PlatformMouseY = event.motion.y;
            PlatformPostMessage(WM_MOUSEMOVE,
                (int)SDLMouseStateToMKFlags(event.motion.state),
                (void*)(uintptr_t)MAKELONG(event.motion.x, event.motion.y));
            break;
        case SDL_MOUSEBUTTONDOWN:
            {
                int msg = WM_LBUTTONDOWN;
                if (event.button.button == SDL_BUTTON_RIGHT) msg = WM_RBUTTONDOWN;
                else if (event.button.button == SDL_BUTTON_MIDDLE) msg = WM_MBUTTONDOWN;
                int x = event.button.x;
                int y = event.button.y;
                g_PlatformMouseX = x;
                g_PlatformMouseY = y;
                PlatformPostMessage(msg,
                    (int)SDLMouseButtonEventToMKFlags(event.button, true),
                    (void*)(uintptr_t)MAKELONG(x, y));
            }
            break;
        case SDL_MOUSEBUTTONUP:
            {
                int msg = WM_LBUTTONUP;
                if (event.button.button == SDL_BUTTON_RIGHT) msg = WM_RBUTTONUP;
                else if (event.button.button == SDL_BUTTON_MIDDLE) msg = WM_MBUTTONUP;
                int x = event.button.x;
                int y = event.button.y;
                g_PlatformMouseX = x;
                g_PlatformMouseY = y;
                PlatformPostMessage(msg,
                    (int)SDLMouseButtonEventToMKFlags(event.button, false),
                    (void*)(uintptr_t)MAKELONG(x, y));
            }
            break;
        case SDL_MOUSEWHEEL:
            {
                int x = 0;
                int y = 0;
                SDL_GetMouseState(&x, &y);

                const int iDelta = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y);
                PlatformPostMessage(WM_MOUSEWHEEL,
                    (int)(uintptr_t)MAKELONG(SDLMouseStateToMKFlags(SDL_GetMouseState(nullptr, nullptr)), (short)(iDelta * 120)),
                    (void*)(uintptr_t)MAKELONG(x, y));
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                log_va("App_PumpEvents: SDL_WINDOWEVENT_RESIZED %d x %d", event.window.data1, event.window.data2);
                PlatformPostMessage(WM_SIZE, 0, (void*)(uintptr_t)MAKELONG(event.window.data1, event.window.data2));
                }
            else if (event.window.event == SDL_WINDOWEVENT_MOVED)
                {
                log_va("App_PumpEvents: SDL_WINDOWEVENT_MOVED %d,%d", event.window.data1, event.window.data2);
                PlatformPostMessage(WM_MOVE, 0, (void*)(uintptr_t)MAKELONG(event.window.data1, event.window.data2));
                }
            else if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                log_msg("App_PumpEvents: SDL_WINDOWEVENT_CLOSE");
				if (RequestGameClose())
					return 0;
                }
            else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                {
                log_msg("App_PumpEvents: SDL_WINDOWEVENT_FOCUS_LOST");
                }
            else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                {
                log_msg("App_PumpEvents: SDL_WINDOWEVENT_FOCUS_GAINED");
                }
            else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                {
                log_msg("App_PumpEvents: SDL_WINDOWEVENT_MINIMIZED");
                }
            else if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                {
                log_msg("App_PumpEvents: SDL_WINDOWEVENT_RESTORED");
                }
            break;
        }
    }
    return 1;
}

const char* PlatformGetGameResourceRoot(void)
{
	static std::string sResourceRoot;
	if (!sResourceRoot.empty())
		return sResourceRoot.c_str();

	char *pBasePath = SDL_GetBasePath();
	if (pBasePath)
		{
		const std::string sBase(pBasePath);
		SDL_free(pBasePath);

		const std::string sBundleGame = sBase + "../Resources/Game";
		sResourceRoot = (access(sBundleGame.c_str(), R_OK) == 0 ? sBundleGame : sBase);
		}

	if (sResourceRoot.empty())
		sResourceRoot = ".";

	return sResourceRoot.c_str();
}

void App_PresentFrameBuffer(void)
{
    if (!g_AppState.pTexture || !g_AppState.pFrameBuffer) return;

    SDL_UpdateTexture(g_AppState.pTexture, nullptr, g_AppState.pFrameBuffer, g_AppState.cxWidth * sizeof(uint32_t));
    SDL_RenderClear(g_AppState.pRenderer);
    SDL_RenderCopy(g_AppState.pRenderer, g_AppState.pTexture, nullptr, nullptr);
    SDL_RenderPresent(g_AppState.pRenderer);

    g_AppState.frameCount++;
    Uint32 currentTick = SDL_GetTicks();
    if (currentTick - g_AppState.fpsTick >= 1000)
    {
        g_AppState.fps = g_AppState.frameCount;
        g_AppState.frameCount = 0;
        g_AppState.fpsTick = currentTick;
    }
}

struct SPlatformScreenInfo PlatformGetScreenInfo(void) { return { g_AppState.pFrameBuffer, g_AppState.cxWidth, g_AppState.cyHeight, g_AppState.cxWidth * (int)sizeof(uint32_t) }; }
void PlatformResizeScreen(int cxWidth, int cyHeight) { RecreateFrameBuffer(cxWidth, cyHeight); }
void PlatformPresentScreen(void) { App_PresentFrameBuffer(); }
uint32_t* App_GetFrameBuffer(void) { return g_AppState.pFrameBuffer; }
int App_GetFrameBufferWidth(void) { return g_AppState.cxWidth; }
int App_GetFrameBufferHeight(void) { return g_AppState.cyHeight; }

bool PlatformPostMessage(int msg, int wParam, void* lParam)
{
    std::lock_guard<std::mutex> lock(g_MessageQueueCS);

    SPlatformMessage message;
    message.msg = msg;
    message.wParam = wParam;
    message.lParam = lParam;
    g_AppState.msgQueue.push(message);
    return true;
}

LRESULT PlatformSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd;

    switch (Msg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
        App_SetRunning(0);
        return 0;

    default:
        PlatformPostMessage((int)Msg, (int)wParam, (void*)lParam);
        return 0;
    }
}

bool PlatformDestroyWindow(HWND hWnd)
{
    (void)hWnd;
    App_SetRunning(0);
    return true;
}

int PlatformPeekMessage(int* pMsg, int* pWParam, void** ppLParam)
{
    std::lock_guard<std::mutex> lock(g_MessageQueueCS);

    if (g_AppState.msgQueue.empty())
        return 0;

    const SPlatformMessage& message = g_AppState.msgQueue.front();
    if (pMsg) *pMsg = message.msg;
    if (pWParam) *pWParam = message.wParam;
    if (ppLParam) *ppLParam = message.lParam;
    g_AppState.msgQueue.pop();

    return 1;
}

unsigned int PlatformSetTimerCompat(void* hWnd, unsigned int timerID, unsigned int elapse, void* callback)
{
    (void)hWnd;
    (void)callback;

    if (elapse == 0)
        return 0;

    std::lock_guard<std::mutex> lock(g_TimerCS);

    auto it = g_Timers.find(timerID);
    if (it != g_Timers.end())
        {
        SDL_RemoveTimer(it->second);
        g_Timers.erase(it);
        }

    SDL_TimerID id = SDL_AddTimer(elapse, TimerThunk, (void *)(uintptr_t)timerID);
    if (id == 0)
        return 0;

    g_Timers.insert({ timerID, id });
    return timerID;
}

int PlatformKillTimerCompat(void* hWnd, unsigned int timerID)
{
    (void)hWnd;

    std::lock_guard<std::mutex> lock(g_TimerCS);

    auto it = g_Timers.find(timerID);
    if (it == g_Timers.end())
        return 0;

    SDL_RemoveTimer(it->second);
    g_Timers.erase(it);
    return 1;
}

// ============================================================================
// Global crash handler — writes crash log to Crash.log
// ============================================================================

static const char CRASH_LOG_FILE[] = "Crash.log";

// Re-entrancy guard: prevents a second (different) signal, delivered while we
// are already writing the crash log, from corrupting the log or nesting into a
// backtrace from a (possibly blown) signal-handler stack frame.
static volatile sig_atomic_t g_InHandler = 0;

static void sigWrite(int fd, const char *s)
	{
	if (!s) return;
	const char *p = s;
	while (*p) p++;
	write(fd, s, (size_t)(p - s));
	}

static void sigWriteHex(int fd, unsigned long long val)
	{
	if (val == 0)
		{
		write(fd, "0x0", 3);
		return;
		}

	char buf[20];
	int pos = 0;
	unsigned long long tmp = val;
	while (tmp > 0 && pos < 19)
		{
		int digit = (int)(tmp % 16);
		buf[pos++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
		tmp /= 16;
		}

	write(fd, "0x", 2);
	for (int i = pos - 1; i >= 0; i--)
		write(fd, &buf[i], 1);
	}

static void sigWriteDec(int fd, int val)
	{
	if (val == 0)
		{
		write(fd, "0", 1);
		return;
		}

	char buf[12];
	int pos = 0;
	int tmp = val;
	bool neg = false;
	if (tmp < 0) { neg = true; tmp = -tmp; }
	while (tmp > 0 && pos < 11)
		{
		buf[pos++] = '0' + (tmp % 10);
		tmp /= 10;
		}

	if (neg) write(fd, "-", 1);
	for (int i = pos - 1; i >= 0; i--)
		write(fd, &buf[i], 1);
	}

static void crashHandler(int sig)
	{
	// Re-entrancy guard: a different signal arriving while we're already
	// handling a crash means we're in an unstable state — just terminate.
	if (g_InHandler)
		_exit(128 + sig);
	g_InHandler = 1;

	int fd = open(CRASH_LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd < 0)
		{
		const char msg[] = "CRASH: signal handler failed to open Crash.log\n";
		write(STDERR_FILENO, msg, sizeof(msg) - 1);
		}
	else
		{
		// Header
		sigWrite(fd, "=== CRASH ===\n");

		// Signal info
		sigWrite(fd, "Signal: ");
		sigWriteDec(fd, sig);
		sigWrite(fd, " (");
		switch (sig)
			{
			case SIGSEGV: sigWrite(fd, "SIGSEGV"); break;
			case SIGABRT: sigWrite(fd, "SIGABRT"); break;
			case SIGBUS:  sigWrite(fd, "SIGBUS");  break;
			case SIGFPE:  sigWrite(fd, "SIGFPE");  break;
			case SIGILL:  sigWrite(fd, "SIGILL");  break;
			case SIGTRAP: sigWrite(fd, "SIGTRAP"); break;
			default:      sigWrite(fd, "unknown"); break;
			}
		sigWrite(fd, ")\n");

		// Backtrace (called last; backtrace/backtrace_symbols_fd are not
		// strictly async-signal-safe but are essential for diagnosis).
		void *frames[64];
		int nFrames = backtrace(frames, 64);
		sigWrite(fd, "Backtrace (");
		sigWriteDec(fd, nFrames);
		sigWrite(fd, " frames):\n");
		backtrace_symbols_fd(frames, nFrames, fd);

		sigWrite(fd, "\n");
		close(fd);
		}

	// Restore the default disposition, then RETURN. The original (still-pending)
	// signal is re-delivered with SIG_DFL once the handler's signal mask is
	// restored on return, producing a core dump. (Calling raise() inside the
	// handler would re-queue onto the signal that is blocked during handler
	// execution and never fire; the old code used _exit, which silently
	// swallowed the crash.)
	struct sigaction sa;
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(sig, &sa, NULL);
	return;
	}

// ============================================================================
// Crash recovery via setjmp/longjmp — allows script crashes to be caught
// ============================================================================

#include <setjmp.h>

static sigjmp_buf g_CrashRecoveryJmp;
static volatile sig_atomic_t g_CrashRecoveryActive = 0;

bool crashRecoveryBegin()
	{
	g_CrashRecoveryActive = 1;
	// sigsetjmp(..., 1) saves the signal mask so siglongjmp restores it. Using
	// plain setjmp/jmp_buf left the crashing signal permanently blocked after a
	// recovery, silently swallowing any later real crash.
	return sigsetjmp(g_CrashRecoveryJmp, 1) == 0;
	}

void crashRecoveryEnd()
	{
	g_CrashRecoveryActive = 0;
	}

static void crashHandlerWithRecovery(int sig)
	{
	if (g_CrashRecoveryActive)
		{
		g_CrashRecoveryActive = 0;
		siglongjmp(g_CrashRecoveryJmp, sig);
		}
	crashHandler(sig);
	}

static void installCrashHandler()
	{
	struct sigaction sa;
	sa.sa_handler = crashHandlerWithRecovery;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGBUS,  &sa, NULL);
	sigaction(SIGFPE,  &sa, NULL);
	sigaction(SIGILL,  &sa, NULL);
	sigaction(SIGTRAP, &sa, NULL);
	}

int App_Run(const char *pszCommandLine)
{
    installCrashHandler();
    log_msg("App_Run: start");

    if (!App_Init())
    {
        log_msg("App_Run: App_Init failed");
        App_Shutdown();
        return 1;
    }

	log_msg("App_Run: App_Init OK, calling InitGameUI");
	if (!InitGameUI(g_AppState, pszCommandLine))
		{
		log_msg("App_Run: InitGameUI failed");
		App_Shutdown();
		return 1;
		}

	log_msg("App_Run: entering main loop");
	while (g_AppState.bRunning)
	{
		if (!App_PumpEvents()) {
            log_msg("App_Run: App_PumpEvents requested exit");
            break;
		}
		UpdateGameUI(g_AppState);
	}

	    log_va("App_Run: exit main loop (bRunning=%d)", (g_AppState.bRunning ? 1 : 0));
	CleanUpGameUI();
	    App_Shutdown();
    return 0;
}

SAppState& GetAppState(void) { return g_AppState; }

int App_IsRunning(void) { return g_AppState.bRunning ? 1 : 0; }
void App_SetRunning(int bRunning) { g_AppState.bRunning = (bRunning != 0); }
void App_SetTitle(const char* pTitle) { if (g_AppState.pWindow) SDL_SetWindowTitle(g_AppState.pWindow, (pTitle ? pTitle : "")); }
void App_GetWindowSize(int* pcxWidth, int* pcyHeight)
{
    if (g_AppState.pWindow)
        SDL_GetWindowSize(g_AppState.pWindow, pcxWidth, pcyHeight);
    else
        {
        if (pcxWidth) *pcxWidth = g_AppState.cxWidth;
        if (pcyHeight) *pcyHeight = g_AppState.cyHeight;
        }
}
