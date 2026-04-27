//	AppCore.cpp
//	Minimal macOS SDL2 application shell for milestone-1
//
//	Provides:
//	- SDL2 window and event loop
//	- Software framebuffer for initial rendering
//	- Basic HI (Human Interface) scaffolding

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr int FRAMEBUFFER_WIDTH = 1024;
constexpr int FRAMEBUFFER_HEIGHT = 768;

struct SAppState
{
    SDL_Window* pWindow = nullptr;
    SDL_Renderer* pRenderer = nullptr;
    Uint32* pFrameBuffer = nullptr;
    bool bRunning = true;
    Uint32 lastTick = 0;
    int frameCount = 0;
    Uint32 fpsTick = 0;
    int fps = 0;
};

static SAppState g_AppState;

bool AppInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    g_AppState.pWindow = SDL_CreateWindow(
        "Transcendence",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!g_AppState.pWindow)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    g_AppState.pRenderer = SDL_CreateRenderer(
        g_AppState.pWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g_AppState.pRenderer)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }

    g_AppState.pFrameBuffer = new Uint32[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
    memset(g_AppState.pFrameBuffer, 0, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(Uint32));

    g_AppState.lastTick = SDL_GetTicks();
    g_AppState.fpsTick = SDL_GetTicks();
    g_AppState.frameCount = 0;
    g_AppState.fps = 0;

    printf("AppCore: Initialized %dx%d window\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

void AppShutdown()
{
    if (g_AppState.pFrameBuffer)
    {
        delete[] g_AppState.pFrameBuffer;
        g_AppState.pFrameBuffer = nullptr;
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

bool AppPumpEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            g_AppState.bRunning = false;
            return false;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                g_AppState.bRunning = false;
                return false;
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                g_AppState.bRunning = false;
                return false;
            }
            break;
        }
    }
    return true;
}

void AppRender()
{
    if (!g_AppState.pRenderer || !g_AppState.pFrameBuffer)
        return;

    SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(
        g_AppState.pFrameBuffer,
        FRAMEBUFFER_WIDTH,
        FRAMEBUFFER_HEIGHT,
        32,
        FRAMEBUFFER_WIDTH * sizeof(Uint32),
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    );

    if (pSurface)
    {
        SDL_Texture* pTexture = SDL_CreateTextureFromSurface(g_AppState.pRenderer, pSurface);
        if (pTexture)
        {
            SDL_RenderClear(g_AppState.pRenderer);
            SDL_RenderCopy(g_AppState.pRenderer, pTexture, nullptr, nullptr);
            SDL_RenderPresent(g_AppState.pRenderer);
            SDL_DestroyTexture(pTexture);
        }
        SDL_FreeSurface(pSurface);
    }

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

Uint32* AppGetFrameBuffer()
{
    return g_AppState.pFrameBuffer;
}

int AppGetFrameBufferPitch()
{
    return FRAMEBUFFER_WIDTH * sizeof(Uint32);
}

bool AppIsRunning()
{
    return g_AppState.bRunning;
}

void AppSetRunning(bool bRunning)
{
    g_AppState.bRunning = bRunning;
}

extern "C" {

int App_Run()
{
    if (!AppInit())
    {
        AppShutdown();
        return 1;
    }

    printf("AppCore: Entering main loop\n");

    while (g_AppState.bRunning)
    {
        if (!AppPumpEvents())
            break;

        AppRender();

        SDL_Delay(16);
    }

    printf("AppCore: Exiting main loop\n");
    AppShutdown();
    return 0;
}

void* App_GetFrameBuffer()
{
    return AppGetFrameBuffer();
}

int App_GetFrameBufferWidth() { return FRAMEBUFFER_WIDTH; }
int App_GetFrameBufferHeight() { return FRAMEBUFFER_HEIGHT; }
int App_GetFrameBufferPitch() { return AppGetFrameBufferPitch(); }

void App_SetTitle(const char* pTitle)
{
    if (g_AppState.pWindow)
        SDL_SetWindowTitle(g_AppState.pWindow, pTitle);
}

}