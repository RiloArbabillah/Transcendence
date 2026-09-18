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

//	Returns the SDLBitmap behind an HBITMAP-style handle, or NULL when the
//	handle was not created by SDLBitmapCreate*/dibCreate*.

SDLBitmap* SDLBitmapLookup(void* hBitmap);

//	PDR-027: returns true when the surface holds only black and white pixels.
//
//	The scan is bounded. A surface whose pixel count fits in the sample budget
//	is examined in full, so the answer is exact whenever it is cheap to be
//	exact; a larger surface is examined on a fixed grid of up to 64x64 pixels.
//	A colour image is therefore only reported as monochrome when its colour
//	lives entirely between the grid lines, which requires a patch smaller than
//	about a thousandth of the image. The bound is what keeps the load path from
//	walking every pixel of every large background image.
//
//	The scan is shared by the DIB loader (DetectBitmapType) and the bitmap
//	loader, so the two cannot drift apart.

bool SDLBitmapSurfaceIsMonochrome(SDL_Surface* pSurface);
ALERROR SDLBitmapGetInfo(SDLBitmap* pBitmap, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits);

ALERROR dibGetInfo(void* hDIB, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits);
bool dibIs16bit(void* hDIB);
bool dibIs24bit(void* hDIB);
bool dibIs32bit(void* hDIB);
ALERROR dibLoadFromFile(Kernel::CString sFilename, void** rethDIB, EBitmapTypes* retiType);
ALERROR dibLoadToBufferFromFile(Kernel::CString sFilespec, struct SBMPImageLoad* retImage);
