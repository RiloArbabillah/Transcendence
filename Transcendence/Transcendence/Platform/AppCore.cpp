//	AppCore.cpp
//	macOS SDL2 application shell
//
//	Provides SDL2-based platform layer for the game engine

#include "AppCore.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

constexpr int DEFAULT_WIDTH = 1024;
constexpr int DEFAULT_HEIGHT = 768;

struct SAppState
{
    SDL_Window* pWindow = nullptr;
    SDL_Renderer* pRenderer = nullptr;
    SDL_Texture* pTexture = nullptr;
    uint32_t* pFrameBuffer = nullptr;
    bool bRunning = true;
    Uint32 lastTick = 0;
    int frameCount = 0;
    Uint32 fpsTick = 0;
    int fps = 0;
    int cxWidth = DEFAULT_WIDTH;
    int cyHeight = DEFAULT_HEIGHT;
};

static SAppState g_AppState;

int App_Init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    g_AppState.cxWidth = DEFAULT_WIDTH;
    g_AppState.cyHeight = DEFAULT_HEIGHT;

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
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    g_AppState.pRenderer = SDL_CreateRenderer(
        g_AppState.pWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g_AppState.pRenderer)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_AppState.pWindow);
        SDL_Quit();
        return 0;
    }

    // Create framebuffer texture for game rendering
    g_AppState.pTexture = SDL_CreateTexture(
        g_AppState.pRenderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        g_AppState.cxWidth,
        g_AppState.cyHeight
    );

    if (!g_AppState.pTexture)
    {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(g_AppState.pRenderer);
        SDL_DestroyWindow(g_AppState.pWindow);
        SDL_Quit();
        return 0;
    }

    // Allocate software framebuffer for game to render into
    g_AppState.pFrameBuffer = new uint32_t[g_AppState.cxWidth * g_AppState.cyHeight];
    memset(g_AppState.pFrameBuffer, 0, g_AppState.cxWidth * g_AppState.cyHeight * sizeof(uint32_t));

    g_AppState.lastTick = SDL_GetTicks();
    g_AppState.fpsTick = SDL_GetTicks();
    g_AppState.frameCount = 0;
    g_AppState.fps = 0;
    g_AppState.bRunning = true;

    printf("AppCore: Initialized %dx%d\n", g_AppState.cxWidth, g_AppState.cyHeight);
    return 1;
}

void App_Shutdown(void)
{
    if (g_AppState.pFrameBuffer)
    {
        delete[] g_AppState.pFrameBuffer;
        g_AppState.pFrameBuffer = nullptr;
    }

    if (g_AppState.pTexture)
    {
        SDL_DestroyTexture(g_AppState.pTexture);
        g_AppState.pTexture = nullptr;
    }

    if (g_AppState.pRenderer)
    {
        SDL_DestroyRenderer(g_AppState.pRenderer);
        g_AppState.pRenderer = nullptr;
    }

    if (g_AppState.pWindow)
    {
        SDL_DestroyWindow(g_AppState.pWindow);
        g_AppState.pWindow = nullptr;
    }

    SDL_Quit();
    printf("AppCore: Shutdown complete\n");
}

int App_PumpEvents(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            g_AppState.bRunning = false;
            return 0;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                g_AppState.bRunning = false;
                return 0;
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                g_AppState.bRunning = false;
                return 0;
            }
            else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                g_AppState.cxWidth = event.window.data1;
                g_AppState.cyHeight = event.window.data2;
            }
            break;
        }
    }
    return 1;
}

struct SFrameBufferInfo App_GetFrameBufferInfo(void)
{
    struct SFrameBufferInfo info;
    info.pPixels = g_AppState.pFrameBuffer;
    info.cxWidth = g_AppState.cxWidth;
    info.cyHeight = g_AppState.cyHeight;
    info.cbPitch = g_AppState.cxWidth * sizeof(uint32_t);
    return info;
}

void App_PresentFrameBuffer(void)
{
    if (!g_AppState.pRenderer || !g_AppState.pTexture || !g_AppState.pFrameBuffer)
        return;

    // Update texture with framebuffer pixels
    SDL_UpdateTexture(
        g_AppState.pTexture,
        nullptr,
        g_AppState.pFrameBuffer,
        g_AppState.cxWidth * sizeof(uint32_t)
    );

    // Clear and render
    SDL_RenderClear(g_AppState.pRenderer);
    SDL_RenderCopy(g_AppState.pRenderer, g_AppState.pTexture, nullptr, nullptr);
    SDL_RenderPresent(g_AppState.pRenderer);

    // FPS counter
    g_AppState.frameCount++;
    Uint32 currentTick = SDL_GetTicks();
    if (currentTick - g_AppState.fpsTick >= 1000)
    {
        g_AppState.fps = g_AppState.frameCount;
        g_AppState.frameCount = 0;
        g_AppState.fpsTick = currentTick;
        printf("AppCore: %d FPS\n", g_AppState.fps);
    }
}

// Platform screen abstraction for DirectXUtilCompat.h
SPlatformScreenInfo PlatformGetScreenInfo(void)
{
    SPlatformScreenInfo info;
    info.pPixels = g_AppState.pFrameBuffer;
    info.cxWidth = g_AppState.cxWidth;
    info.cyHeight = g_AppState.cyHeight;
    info.cbPitch = g_AppState.cxWidth * (int)sizeof(uint32_t);
    return info;
}

void PlatformPresentScreen(void)
{
    App_PresentFrameBuffer();
}

int App_IsRunning(void)
{
    return g_AppState.bRunning ? 1 : 0;
}

void App_SetRunning(int bRunning)
{
    g_AppState.bRunning = (bRunning != 0);
}

void App_SetTitle(const char* pTitle)
{
    if (g_AppState.pWindow)
        SDL_SetWindowTitle(g_AppState.pWindow, pTitle);
}

void App_GetWindowSize(int* pcxWidth, int* pcyHeight)
{
    if (pcxWidth) *pcxWidth = g_AppState.cxWidth;
    if (pcyHeight) *pcyHeight = g_AppState.cyHeight;
}

// Main entry point - basic loop for milestone-2
// TODO: Integrate with CHumanInterface for full game
int App_Run(void)
{
    if (!App_Init())
    {
        App_Shutdown();
        return 1;
    }

    printf("AppCore: Entering main loop\n");

    while (g_AppState.bRunning)
    {
        if (!App_PumpEvents())
            break;

        // For milestone-2: just render a gradient test pattern
        // Later: this will call into the game engine
        for (int y = 0; y < g_AppState.cyHeight; y++)
        {
            for (int x = 0; x < g_AppState.cxWidth; x++)
            {
                int idx = y * g_AppState.cxWidth + x;
                uint8_t r = (x * 255) / g_AppState.cxWidth;
                uint8_t g = (y * 255) / g_AppState.cyHeight;
                uint8_t b = 128;
                g_AppState.pFrameBuffer[idx] = (r << 16) | (g << 8) | b | 0xFF000000;
            }
        }

        App_PresentFrameBuffer();

        SDL_Delay(16);
    }

    printf("AppCore: Exiting main loop\n");
    App_Shutdown();
    return 0;
}
