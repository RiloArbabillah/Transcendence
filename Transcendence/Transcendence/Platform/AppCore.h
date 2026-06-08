//	AppCore.h
//	macOS SDL2 application shell interface
//
//	Provides platform abstraction for the game engine

#pragma once

#include <stdint.h>
#include <SDL2/SDL.h>
#include <queue>

#ifndef SPlatformScreenInfoDefined
#define SPlatformScreenInfoDefined
struct SPlatformScreenInfo {
    void* pPixels;
    int cxWidth;
    int cyHeight;
    int cbPitch;
};
#endif

#ifndef SFrameBufferInfoDefined
#define SFrameBufferInfoDefined
struct SFrameBufferInfo
{
    uint32_t* pPixels;
    int cxWidth;
    int cyHeight;
    int cbPitch;
};
#endif

struct SPlatformMessage
{
    int msg;
    int wParam;
    void* lParam;
};

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
    int cxWidth = 1024;
    int cyHeight = 768;
    std::queue<SPlatformMessage> msgQueue;
};

struct SPlatformScreenInfo PlatformGetScreenInfo(void);
void PlatformResizeScreen(int cxWidth, int cyHeight);
void PlatformPresentScreen(void);

int App_Init(void);
void App_Shutdown(void);
int App_Run(const char *pszCommandLine = nullptr);

struct SFrameBufferInfo App_GetFrameBufferInfo(void);
void App_PresentFrameBuffer(void);

int App_IsRunning(void);
void App_SetRunning(int bRunning);
void App_SetTitle(const char* pTitle);
void App_GetWindowSize(int* pcxWidth, int* pcyHeight);
int App_PumpEvents(void);

typedef void (*TimerCallback)(int timerID, void* userData);
int PlatformAddTimer(int dwMilliseconds, TimerCallback callback, void* userData);
void PlatformRemoveTimer(int timerID);

#define PLATFORM_MSG_TIMER         1
#define PLATFORM_MSG_COMMAND       2
#define PLATFORM_MSG_TASK_COMPLETE 3

bool PlatformPostMessage(int msg, int wParam, void* lParam);
int PlatformPeekMessage(int* pMsg, int* pWParam, void** ppLParam);
unsigned int PlatformSetTimerCompat(void* hWnd, unsigned int timerID, unsigned int elapse, void* callback);
int PlatformKillTimerCompat(void* hWnd, unsigned int timerID);

uint32_t* App_GetFrameBuffer(void);
int App_GetFrameBufferWidth(void);
int App_GetFrameBufferHeight(void);

struct SAppState& GetAppState(void);

void InitGameUI(SAppState& state, const char *pszCommandLine = nullptr);
void UpdateGameUI(SAppState& state);
