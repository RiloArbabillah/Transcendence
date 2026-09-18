//	SDLBitmap.cpp
//	SDL-based bitmap implementation for macOS
//	Provides HBITMAP compatibility using SDL2_image

#include "PreComp.h"
#include "SDLBitmap.h"
#include "DirectXUtilCompat.h"
#include <SDL2/SDL_image.h>
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

    SDL_Surface* normalized = loaded;
    EBitmapTypes iType = bitmapRGB;
    Uint32 format = loaded->format->format;

    if (format == SDL_PIXELFORMAT_RGB565) {
        iType = bitmapRGB;
    }
    else if (SDL_ISPIXELFORMAT_ALPHA(format)) {
        //  Preserve the real alpha plane. Hit testing and masks depend on it.
        normalized = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_BGRA32, 0);
        if (!normalized) {
            SDL_FreeSurface(loaded);
            if (retiType) *retiType = bitmapNone;
            return nullptr;
        }

        SDL_FreeSurface(loaded);
        iType = bitmapAlpha;
    }
    else if (format != SDL_PIXELFORMAT_BGR24) {
        normalized = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_BGR24, 0);
        if (!normalized) {
            SDL_FreeSurface(loaded);
            if (retiType) *retiType = bitmapNone;
            return nullptr;
        }

        SDL_FreeSurface(loaded);
        iType = bitmapRGB;
    }

    if (retiType) *retiType = iType;
    return SDLBitmapCreateFromSurface(normalized, iType, true);
}

SDLBitmap* SDLBitmapCreateFromSurface(SDL_Surface* pSurface, EBitmapTypes iType, bool bTakeOwnership) {
    if (!pSurface || !pSurface->pixels) {
        return nullptr;
    }

    SDLBitmap* pBitmap = new SDLBitmap();
    pBitmap->surface = pSurface;
    pBitmap->cxWidth = pSurface->w;
    pBitmap->cyHeight = pSurface->h;
    pBitmap->iStride = pSurface->pitch;
    pBitmap->pPixels = pSurface->pixels;
    pBitmap->iType = iType;
    pBitmap->iBitCount = pSurface->format->BitsPerPixel;
    pBitmap->dwPixelFormat = pSurface->format->format;
    pBitmap->bOwnsSurface = bTakeOwnership;

    void* hBitmap = (void*)pBitmap;
    GetBitmapMap()[hBitmap] = pBitmap;

    return pBitmap;
}

void SDLBitmapDestroy(SDLBitmap* pBitmap) {
    if (!pBitmap) return;

    //  Drop the lookup entry first: the map is keyed by the SDLBitmap pointer,
    //  so leaving it behind would both dangle and grow without bound as
    //  callers create/destroy bitmaps.

    GetBitmapMap().erase((void*)pBitmap);

    if (pBitmap->surface && pBitmap->bOwnsSurface) {
        SDL_FreeSurface(pBitmap->surface);
    }
    delete pBitmap;
}

namespace {

//	Reads one pixel as RGB. The pixel value is assembled through a zeroed
//	buffer rather than an unaligned Uint32 load, so a 3-byte pixel at the end
//	of a row is not read past the end of the pixel buffer. (PDR-027)

bool IsMonochromePixel(const BYTE* pPixel, const SDL_PixelFormat* pFormat) {
    BYTE byValue[4] = { 0, 0, 0, 0 };
    memcpy(byValue, pPixel, (size_t)pFormat->BytesPerPixel);

    Uint32 dwPixel = 0;
    memcpy(&dwPixel, byValue, sizeof(dwPixel));

    BYTE byR = 0;
    BYTE byG = 0;
    BYTE byB = 0;
    SDL_GetRGB(dwPixel, pFormat, &byR, &byG, &byB);

    return ((byR == 0x00 && byG == 0x00 && byB == 0x00)
            || (byR == 0xff && byG == 0xff && byB == 0xff));
}

} // anonymous namespace

bool SDLBitmapSurfaceIsMonochrome(SDL_Surface* pSurface) {
    if (!pSurface || !pSurface->format || !pSurface->pixels)
        return false;

    const SDL_PixelFormat* pFormat = pSurface->format;
    const int iBytesPerPixel = pFormat->BytesPerPixel;
    if (iBytesPerPixel < 3)
        return false;

    const int cxWidth = pSurface->w;
    const int cyHeight = pSurface->h;
    if (cxWidth <= 0 || cyHeight <= 0)
        return false;

    const BYTE* pPixels = (const BYTE*)pSurface->pixels;

    const int kMaxSamples = 4096;
    const int kMaxGridSide = 64;

    //  Small surface: examine every pixel.

    if ((long long)cxWidth * (long long)cyHeight <= kMaxSamples) {
        for (int y = 0; y < cyHeight; y++) {
            const BYTE* pPixel = pPixels + (size_t)y * (size_t)pSurface->pitch;
            for (int x = 0; x < cxWidth; x++) {
                if (!IsMonochromePixel(pPixel, pFormat))
                    return false;

                pPixel += iBytesPerPixel;
            }
        }

        return true;
    }

    //  Large surface: sample a fixed grid.

    const int cxGrid = (cxWidth < kMaxGridSide ? cxWidth : kMaxGridSide);
    const int cyGrid = (cyHeight < kMaxGridSide ? cyHeight : kMaxGridSide);

    for (int iy = 0; iy < cyGrid; iy++) {
        const int y = (int)(((long long)iy * (long long)cyHeight) / (long long)cyGrid);
        const BYTE* pRow = pPixels + (size_t)y * (size_t)pSurface->pitch;

        for (int ix = 0; ix < cxGrid; ix++) {
            const int x = (int)(((long long)ix * (long long)cxWidth) / (long long)cxGrid);
            if (!IsMonochromePixel(pRow + (size_t)x * (size_t)iBytesPerPixel, pFormat))
                return false;
        }
    }

    return true;
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
        retpBMIH->biBitCount = (WORD)(pBitmap->iBitCount > 0 ? pBitmap->iBitCount : 32);
        retpBMIH->biCompression = 0;
        retpBMIH->biSizeImage = pBitmap->iStride * pBitmap->cyHeight;
    }

    if (retpBits) *retpBits = pBitmap->pPixels;

    return NOERROR;
}

SDLBitmap* SDLBitmapLookup(void* hDIB) {
    if (!hDIB) return nullptr;
    auto it = GetBitmapMap().find(hDIB);
    if (it != GetBitmapMap().end()) {
        return it->second;
    }
    return nullptr;
}

ALERROR dibGetInfo(void* hDIB, int* retcxWidth, int* retcyHeight, void** retpBase, int* retiStride, BITMAPINFOHEADER* retpBMIH, void** retpBits) {
    SDLBitmap* pBitmap = SDLBitmapLookup(hDIB);
    if (!pBitmap) return ERR_FAIL;
    return SDLBitmapGetInfo(pBitmap, retcxWidth, retcyHeight, retpBase, retiStride, retpBMIH, retpBits);
}

bool dibIs16bit(void* hDIB) {
    SDLBitmap* pBitmap = SDLBitmapLookup(hDIB);
    if (!pBitmap) return false;
    return pBitmap->iBitCount == 16;
}

bool dibIs24bit(void* hDIB) {
    SDLBitmap* pBitmap = SDLBitmapLookup(hDIB);
    if (!pBitmap) return false;
    return pBitmap->iBitCount == 24;
}

bool dibIs32bit(void* hDIB) {
    SDLBitmap* pBitmap = SDLBitmapLookup(hDIB);
    if (!pBitmap) return false;
    return pBitmap->iBitCount == 32;
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

    SDL_Surface* pSurface = pBitmap->surface;
    SDL_Surface* pConverted = nullptr;
    if (!pSurface || !pSurface->pixels) {
        retImage->cxWidth = 0;
        retImage->cyHeight = 0;
        retImage->iPitch = 0;
        retImage->iType = bitmapNone;
        retImage->Pixels = "";
        SDLBitmapDestroy(pBitmap);
        return ERR_FAIL;
    }

    if (pSurface->format->format != SDL_PIXELFORMAT_BGRA32) {
        pConverted = SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_BGRA32, 0);
        if (!pConverted) {
            retImage->cxWidth = 0;
            retImage->cyHeight = 0;
            retImage->iPitch = 0;
            retImage->iType = bitmapNone;
            retImage->Pixels = "";
            SDLBitmapDestroy(pBitmap);
            return ERR_FAIL;
        }

        pSurface = pConverted;
    }

    retImage->cxWidth = pSurface->w;
    retImage->cyHeight = pSurface->h;
    retImage->iPitch = pSurface->pitch;

    const int iDataSize = retImage->iPitch * retImage->cyHeight;
    retImage->Pixels = CString((const char*)pSurface->pixels, iDataSize);

    //  PDR-027: the same bounded scan that the DIB loader uses. This used to be
    //  a second, unbounded copy of the monochrome test.

    if (retImage->iType == bitmapRGB && SDLBitmapSurfaceIsMonochrome(pSurface))
        retImage->iType = bitmapMonochrome;

    if (pConverted)
        SDL_FreeSurface(pConverted);

    SDLBitmapDestroy(pBitmap);
    return NOERROR;
}
