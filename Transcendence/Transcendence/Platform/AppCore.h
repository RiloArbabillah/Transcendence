//	AppCore.h
//	Minimal macOS SDL2 application shell interface

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int App_Run();
void* App_GetFrameBuffer();
int App_GetFrameBufferWidth();
int App_GetFrameBufferHeight();
int App_GetFrameBufferPitch();
void App_SetTitle(const char* pTitle);
void App_SetRunning(bool bRunning);

#ifdef __cplusplus
}
#endif