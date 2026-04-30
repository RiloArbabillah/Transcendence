//	AppCore.cpp
//	macOS SDL2 application shell
//
//	Provides SDL2-based platform layer for the game engine

#include "AppCore.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>

constexpr int DEFAULT_WIDTH = 1024;
constexpr int DEFAULT_HEIGHT = 768;

static FILE* g_Log = nullptr;

static void log_msg(const char* pMsg) {
    if (!g_Log) {
        g_Log = fopen("/tmp/trans_app.log", "w");
    }
    if (g_Log) {
        fprintf(g_Log, "%s\n", pMsg);
        fflush(g_Log);
    }
    fprintf(stderr, "%s\n", pMsg);
    fflush(stderr);
}

SAppState g_AppState;

int App_Init(void)
{
    log_msg("App_Init: start");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        log_msg("App_Init: SDL_Init failed");
        return 0;
    }

    log_msg("App_Init: SDL_Init OK");

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
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
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
        SDL_PIXELFORMAT_RGBA32,
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
    while (!g_AppState.msgQueue.empty())
        g_AppState.msgQueue.pop();

    if (g_AppState.pFrameBuffer) { delete[] g_AppState.pFrameBuffer; g_AppState.pFrameBuffer = nullptr; }
    if (g_AppState.pTexture) { SDL_DestroyTexture(g_AppState.pTexture); g_AppState.pTexture = nullptr; }
    if (g_AppState.pRenderer) { SDL_DestroyRenderer(g_AppState.pRenderer); g_AppState.pRenderer = nullptr; }
    if (g_AppState.pWindow) { SDL_DestroyWindow(g_AppState.pWindow); g_AppState.pWindow = nullptr; }

    SDL_Quit();
    log_msg("App_Shutdown: done");
}

int App_PumpEvents(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) { g_AppState.bRunning = false; return 0; }
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
    log_msg("App_Run: InitGameUI returned");

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