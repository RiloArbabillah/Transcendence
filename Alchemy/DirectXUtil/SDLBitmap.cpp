//	SDLBitmap.cpp
//	SDL-based bitmap implementation for macOS
//	Provides HBITMAP compatibility using SDL2_image

#include "PreComp.h"
#include "SDLBitmap.h"
#include "DirectXUtilCompat.h"
#include <iostream>
#include <cstring>

static std::map<void*, SDLBitmap*>& GetBitmapMap() {
    static std::map<void*, SDLBitmap*> s_map;
    return s_map;
}

std::map<void*, SDLBitmap*>& GetSDLBitmapMap() {
    return GetBitmapMap();
}

SDLBitmap* SDLBitmapCreate(const char* pszFile, EBitmapTypes* retiType) {
    SDL_Surface* loaded = IMG_Load(pszFile);
    if (!loaded) {
        std::cerr << "SDLBitmap: Failed to load " << pszFile << ": " << IMG_GetError() << std::endl;
        if (retiType) *retiType = bitmapNone;
        return nullptr;
    }

    SDLBitmap* pBitmap = new SDLBitmap();
    pBitmap->cxWidth = loaded->w;
    pBitmap->cyHeight = loaded->h;
    pBitmap->iStride = loaded->pitch;
    pBitmap->iType = bitmapRGB;

    Uint32 format = loaded->format->format;

    if (format == SDL_PIXELFORMAT_RGB565) {
        pBitmap->iType = bitmapRGB;
    }
    else if (format == SDL_PIXELFORMAT_RGB24 || format == SDL_PIXELFORMAT_BGR24) {
        pBitmap->iType = bitmapRGB;
    }
    else if (SDL_ISPIXELFORMAT_ALPHA(format)) {
        pBitmap->iType = bitmapAlpha;
    }

    if (retiType) *retiType = pBitmap->iType;

    pBitmap->surface = loaded;
    pBitmap->pPixels = loaded->pixels;

    void* hBitmap = (void*)pBitmap;
    GetBitmapMap()[hBitmap] = pBitmap;

    return pBitmap;
}

void SDLBitmapDestroy(SDLBitmap* pBitmap) {
    if (!pBitmap) return;

    if (pBitmap->surface && pBitmap->bOwnsSurface) {
        SDL_FreeSurface(pBitmap->surface);
    }
    delete pBitmap;
}

ALERROR SDLBitmapGetInfo(SDLBitmap* pBitmap, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits) {
    if (!pBitmap) return ERR_FAIL;

    if (retcxWidth) *retcxWidth = pBitmap->cxWidth;
    if (retcyHeight) *retcyHeight = pBitmap->cyHeight;
    if (retpBase) *retpBase = pBitmap->pPixels;
    if (retiStride) *retiStride = pBitmap->iStride;

    if (retpBMIH) {
        memset(retpBMIH, 0, sizeof(BITMAPINFOHEADER));
        retpBMIH->biSize = sizeof(BITMAPINFOHEADER);
        retpBMIH->biWidth = pBitmap->cxWidth;
        retpBMIH->biHeight = -pBitmap->cyHeight;
        retpBMIH->biPlanes = 1;
        retpBMIH->biBitCount = 32;
        retpBMIH->biCompression = 0;
        retpBMIH->biSizeImage = pBitmap->iStride * pBitmap->cyHeight;
    }

    if (retpBits) *retpBits = pBitmap->pPixels;

    return NOERROR;
}

static SDLBitmap* LookupBitmap(void* hDIB) {
    if (!hDIB) return nullptr;
    auto it = GetBitmapMap().find(hDIB);
    if (it != GetBitmapMap().end()) {
        return it->second;
    }
    return nullptr;
}

ALERROR dibGetInfo(void* hDIB, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits) {
    SDLBitmap* pBitmap = LookupBitmap(hDIB);
    if (!pBitmap) return ERR_FAIL;
    return SDLBitmapGetInfo(pBitmap, retcxWidth, retcyHeight, retpBase, retiStride, retpBMIH, retpBits);
}

bool dibIs16bit(void* hDIB) {
    SDLBitmap* pBitmap = LookupBitmap(hDIB);
    if (!pBitmap) return false;
    return pBitmap->iType == bitmapRGB;
}

bool dibIs24bit(void* hDIB) {
    SDLBitmap* pBitmap = LookupBitmap(hDIB);
    if (!pBitmap) return false;
    return false;
}

ALERROR dibLoadFromFile(Kernel::CString sFilename, void** rethDIB, EBitmapTypes* retiType) {
    if (rethDIB) *rethDIB = nullptr;
    if (retiType) *retiType = bitmapNone;

    const char* pszFile = sFilename.GetPointer();
    if (!pszFile || sFilename.IsBlank()) {
        return ERR_FAIL;
    }

    SDLBitmap* pBitmap = SDLBitmapCreate(pszFile, retiType);
    if (!pBitmap) {
        return ERR_FAIL;
    }

    if (rethDIB) *rethDIB = (void*)pBitmap;
    return NOERROR;
}

ALERROR dibLoadToBufferFromFile(Kernel::CString sFilespec, SBMPImageLoad* retImage) {
    if (!retImage) return ERR_FAIL;

    const char* pszFile = sFilespec.GetPointer();
    if (!pszFile || sFilespec.IsBlank()) {
        retImage->cxWidth = 0;
        retImage->cyHeight = 0;
        retImage->iPitch = 0;
        retImage->iType = bitmapNone;
        return ERR_FAIL;
    }

    SDLBitmap* pBitmap = SDLBitmapCreate(pszFile, &retImage->iType);
    if (!pBitmap) {
        retImage->cxWidth = 0;
        retImage->cyHeight = 0;
        retImage->iPitch = 0;
        retImage->iType = bitmapNone;
        return ERR_FAIL;
    }

    retImage->cxWidth = pBitmap->cxWidth;
    retImage->cyHeight = pBitmap->cyHeight;
    retImage->iPitch = pBitmap->iStride;

    SDLBitmapDestroy(pBitmap);
    return NOERROR;
}