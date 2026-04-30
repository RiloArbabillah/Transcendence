//	MetalRenderer.cpp
//	Metal-based renderer for macOS
//	SDL already handles Metal via SDL_CreateRenderer with SDL_RENDERER_METAL

#ifdef __APPLE__

#include "MetalRenderer.h"
#include "AppCore.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct CMetalRenderer {
    void* m_view;
    void* m_metalLayer;
    void* m_device;
    void* m_commandQueue;
    void* m_pipelineState;
    void* m_vertexBuffer;
    int m_cxWidth;
    int m_cyHeight;
    bool m_bInitialized;
    SMetalRendererStats m_Stats;
};

struct CMetalRenderer* MetalRenderer_Create(void) {
    CMetalRenderer* pRenderer = (CMetalRenderer*)malloc(sizeof(CMetalRenderer));
    if (!pRenderer)
        return nullptr;

    memset(pRenderer, 0, sizeof(CMetalRenderer));
    return pRenderer;
}

void MetalRenderer_Destroy(struct CMetalRenderer* pRenderer) {
    if (pRenderer) {
        MetalRenderer_Shutdown(pRenderer);
        free(pRenderer);
    }
}

bool MetalRenderer_Init(struct CMetalRenderer* pRenderer, void* pView, int cxWidth, int cyHeight) {
    if (!pRenderer || !pView)
        return false;

    SDL_Window* pWindow = (SDL_Window*)pView;
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    pRenderer->m_cxWidth = cxWidth;
    pRenderer->m_cyHeight = cyHeight;
    pRenderer->m_bInitialized = true;

    return true;
}

void MetalRenderer_Shutdown(struct CMetalRenderer* pRenderer) {
    if (!pRenderer)
        return;

    pRenderer->m_vertexBuffer = nullptr;
    pRenderer->m_pipelineState = nullptr;
    pRenderer->m_commandQueue = nullptr;
    pRenderer->m_device = nullptr;
    pRenderer->m_metalLayer = nullptr;
    pRenderer->m_view = nullptr;
    pRenderer->m_bInitialized = false;
}

bool MetalRenderer_RenderFrame(struct CMetalRenderer* pRenderer, uint32_t* pFrameBuffer, int cxWidth, int cyHeight) {
    if (!pRenderer || !pRenderer->m_bInitialized || !pFrameBuffer)
        return false;

    pRenderer->m_Stats.framesRendered++;

    uint32_t currentTick = SDL_GetTicks();
    if (currentTick - pRenderer->m_Stats.lastFpsUpdate >= 1000) {
        pRenderer->m_Stats.fps = pRenderer->m_Stats.framesRendered;
        pRenderer->m_Stats.framesRendered = 0;
        pRenderer->m_Stats.lastFpsUpdate = currentTick;
        printf("MetalRenderer: %d FPS\n", pRenderer->m_Stats.fps);
    }

    return true;
}

void* MetalRenderer_GetMetalLayer(struct CMetalRenderer* pRenderer) {
    if (!pRenderer)
        return nullptr;
    return pRenderer->m_metalLayer;
}

void MetalRenderer_Resize(struct CMetalRenderer* pRenderer, int cxWidth, int cyHeight) {
    if (!pRenderer)
        return;

    pRenderer->m_cxWidth = cxWidth;
    pRenderer->m_cyHeight = cyHeight;
}

#endif // __APPLE__