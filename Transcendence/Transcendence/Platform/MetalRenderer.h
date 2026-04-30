//	MetalRenderer.h
//	Metal-based renderer for macOS
//	Replaces SDL_Renderer for hardware-accelerated presentation

#pragma once

#ifdef __APPLE__

#include <stdint.h>

#ifndef METAL_RENDERER_DEFINED
#define METAL_RENDERER_DEFINED

struct SMetalRendererStats {
    int framesRendered;
    int fps;
    uint32_t lastFpsUpdate;
};

typedef struct CMetalRenderer CMetalRenderer;

struct CMetalRenderer* MetalRenderer_Create(void);
void MetalRenderer_Destroy(struct CMetalRenderer* pRenderer);
bool MetalRenderer_Init(struct CMetalRenderer* pRenderer, void* pView, int cxWidth, int cyHeight);
void MetalRenderer_Shutdown(struct CMetalRenderer* pRenderer);
bool MetalRenderer_RenderFrame(struct CMetalRenderer* pRenderer, uint32_t* pFrameBuffer, int cxWidth, int cyHeight);
void* MetalRenderer_GetMetalLayer(struct CMetalRenderer* pRenderer);
void MetalRenderer_Resize(struct CMetalRenderer* pRenderer, int cxWidth, int cyHeight);

#endif // METAL_RENDERER_DEFINED

#endif // __APPLE__