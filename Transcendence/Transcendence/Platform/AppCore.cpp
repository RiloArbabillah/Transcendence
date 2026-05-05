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
#include <queue>
#include <map>
#include <mutex>
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

SAppState g_AppState;
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

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
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
        SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g_AppState.pRenderer)
    {
        log_msg("App_Init: CreateRenderer failed");
        SDL_DestroyWindow(g_AppState.pWindow);
        SDL_Quit();
        return 0;
    }

    void* pMetalLayer = SDL_RenderGetMetalLayer(g_AppState.pRenderer);
    if (pMetalLayer)
        log_msg("App_Init: Metal layer OK");
    else
        log_msg("App_Init: No Metal layer");

	g_AppState.pTexture = SDL_CreateTexture(
		g_AppState.pRenderer,
		SDL_PIXELFORMAT_BGRA32,
		SDL_TEXTUREACCESS_STREAMING,
		g_AppState.cxWidth,
		g_AppState.cyHeight
	);

    if (!g_AppState.pTexture)
    {
        log_msg("App_Init: CreateTexture failed");
        SDL_DestroyRenderer(g_AppState.pRenderer);
        SDL_DestroyWindow(g_AppState.pWindow);
        SDL_Quit();
        return 0;
    }

    g_AppState.pFrameBuffer = new uint32_t[g_AppState.cxWidth * g_AppState.cyHeight];
    memset(g_AppState.pFrameBuffer, 0, g_AppState.cxWidth * g_AppState.cyHeight * sizeof(uint32_t));

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

int App_PumpEvents(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) { g_AppState.bRunning = false; return 0; }

        switch (event.type)
        {
        case SDL_KEYDOWN:
            PlatformPostMessage(WM_KEYDOWN, SDLKeyToVK(event.key.keysym.scancode), nullptr);
            break;
        case SDL_KEYUP:
            PlatformPostMessage(WM_KEYUP, SDLKeyToVK(event.key.keysym.scancode), nullptr);
            break;
        case SDL_TEXTINPUT:
            for (const char* p = event.text.text; *p; p++)
                PlatformPostMessage(WM_CHAR, *p, nullptr);
            break;
        case SDL_MOUSEMOTION:
            PlatformPostMessage(WM_MOUSEMOVE,
                (int)(uintptr_t)event.motion.state,
                (void*)(uintptr_t)MAKELONG(event.motion.x, event.motion.y));
            break;
        case SDL_MOUSEBUTTONDOWN:
            {
                int msg = WM_LBUTTONDOWN;
                if (event.button.button == 3) msg = WM_RBUTTONDOWN;
                else if (event.button.button == 2) msg = WM_MBUTTONDOWN;
                int x = event.button.x;
                int y = event.button.y;
                PlatformPostMessage(msg,
                    (int)(uintptr_t)event.button.button,
                    (void*)(uintptr_t)MAKELONG(x, y));
            }
            break;
        case SDL_MOUSEBUTTONUP:
            {
                int msg = WM_LBUTTONUP;
                if (event.button.button == 3) msg = WM_RBUTTONUP;
                else if (event.button.button == 2) msg = WM_MBUTTONUP;
                int x = event.button.x;
                int y = event.button.y;
                PlatformPostMessage(msg,
                    (int)(uintptr_t)event.button.button,
                    (void*)(uintptr_t)MAKELONG(x, y));
            }
            break;
        case SDL_MOUSEWHEEL:
            PlatformPostMessage(WM_MOUSEWHEEL,
                (int)(uintptr_t)MAKELONG(0, event.wheel.y),
                (void*)(uintptr_t)MAKELONG(event.wheel.x, event.wheel.y));
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                PlatformPostMessage(WM_SIZE, 0, (void*)(uintptr_t)MAKELONG(event.window.data1, event.window.data2));
            else if (event.window.event == SDL_WINDOWEVENT_MOVED)
                PlatformPostMessage(WM_MOVE, 0, (void*)(uintptr_t)MAKELONG(event.window.data1, event.window.data2));
            break;
        }
    }
    return 1;
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

int App_Run(void)
{
    log_msg("App_Run: start");

    if (!App_Init())
    {
        log_msg("App_Run: App_Init failed");
        App_Shutdown();
        return 1;
    }

	log_msg("App_Run: App_Init OK, calling InitGameUI");
	InitGameUI(g_AppState);

	log_msg("App_Run: entering main loop");
	while (g_AppState.bRunning)
	{
		if (!App_PumpEvents()) break;
		UpdateGameUI(g_AppState);
		App_PresentFrameBuffer();
		SDL_Delay(16);
	}

    log_msg("App_Run: exit main loop");
    App_Shutdown();
    return 0;
}

SAppState& GetAppState(void) { return g_AppState; }
