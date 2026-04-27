//	AppCore.h
//	macOS SDL2 application shell interface
//
//	Provides platform abstraction for the game engine

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Framebuffer info structure for game engine
// This is also defined in DirectXUtilCompat.h for CScreenMgrSDL
#ifndef SPlatformScreenInfoDefined
#define SPlatformScreenInfoDefined
struct SPlatformScreenInfo {
    void* pPixels;
    int cxWidth;
    int cyHeight;
    int cbPitch;
};
#endif

// Platform screen functions - implemented in AppCore.cpp
// Declared here so CScreenMgrSDL can use them via DirectXUtilCompat.h
struct SPlatformScreenInfo PlatformGetScreenInfo(void);
void PlatformPresentScreen(void);

// Framebuffer info structure
struct SFrameBufferInfo {
    uint32_t* pPixels;
    int cxWidth;
    int cyHeight;
    int cbPitch;
};

// Initialize SDL2 app - returns true on success
int App_Init(void);

// Shutdown SDL2 app
void App_Shutdown(void);

// Run the main loop - returns exit code
int App_Run(void);

// Get the framebuffer for game rendering
struct SFrameBufferInfo App_GetFrameBufferInfo(void);

// Present the framebuffer to screen
void App_PresentFrameBuffer(void);

// Check if app is running
int App_IsRunning(void);

// Set running state
void App_SetRunning(int bRunning);

// Set window title
void App_SetTitle(const char* pTitle);

// Get window size
void App_GetWindowSize(int* pcxWidth, int* pcyHeight);

// Event pump - returns true if should continue
int App_PumpEvents(void);

#ifdef __cplusplus
}
#endif