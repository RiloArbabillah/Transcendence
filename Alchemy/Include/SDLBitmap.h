//	SDLBitmap.h
//	SDL-based bitmap implementation for macOS
//	Provides HBITMAP compatibility using SDL2_image

#pragma once

#include "Kernel.h"
#include "Graphics.h"
#include <SDL2/SDL.h>
#include <map>

struct SDL_Surface;

struct SDLBitmap {
    SDL_Surface* surface = nullptr;
    int cxWidth = 0;
    int cyHeight = 0;
    int iStride = 0;
    void* pPixels = nullptr;
    EBitmapTypes iType = bitmapNone;
    int iBitCount = 0;
    Uint32 dwPixelFormat = SDL_PIXELFORMAT_UNKNOWN;
    bool bOwnsSurface = true;
};

extern std::map<void*, SDLBitmap*>& GetSDLBitmapMap();

SDLBitmap* SDLBitmapCreate(const char* pszFile, EBitmapTypes* retiType);
SDLBitmap* SDLBitmapCreateFromSurface(SDL_Surface* pSurface, EBitmapTypes iType, bool bTakeOwnership = true);
void SDLBitmapDestroy(SDLBitmap* pBitmap);
ALERROR SDLBitmapGetInfo(SDLBitmap* pBitmap, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits);

ALERROR dibGetInfo(void* hDIB, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits);
bool dibIs16bit(void* hDIB);
bool dibIs24bit(void* hDIB);
ALERROR dibLoadFromFile(Kernel::CString sFilename, void** rethDIB, EBitmapTypes* retiType);
ALERROR dibLoadToBufferFromFile(Kernel::CString sFilespec, struct SBMPImageLoad* retImage);