//	DIBSDL.cpp
//
//	SDL-based DIB loading support for macOS

#include "PreComp.h"
#include "Graphics.h"
#include "SDLBitmap.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace
	{
	EBitmapTypes DetectBitmapType(SDL_Surface *pSurface)
		{
		if (!pSurface || !pSurface->format)
			return bitmapNone;

		const Uint32 format = pSurface->format->format;
		if (SDL_ISPIXELFORMAT_ALPHA(format))
			return bitmapAlpha;

		const int iBytesPerPixel = pSurface->format->BytesPerPixel;
		if (iBytesPerPixel >= 3)
			{
			bool bMonochrome = true;
			const BYTE *pRow = (const BYTE *)pSurface->pixels;
			for (int y = 0; y < pSurface->h && bMonochrome; y++)
				{
				const BYTE *pPixel = pRow;
				for (int x = 0; x < pSurface->w; x++)
					{
					BYTE byR, byG, byB;
					SDL_GetRGB(*(const Uint32 *)pPixel, pSurface->format, &byR, &byG, &byB);
					if (!((byR == 0x00 && byG == 0x00 && byB == 0x00)
							|| (byR == 0xff && byG == 0xff && byB == 0xff)))
						{
						bMonochrome = false;
						break;
						}

					pPixel += iBytesPerPixel;
					}

				pRow += pSurface->pitch;
				}

			if (bMonochrome)
				return bitmapMonochrome;
			}

		return bitmapRGB;
		}
	}

ALERROR dibLoadFromBlock (IReadBlock &Data, HBITMAP *rethDIB, EBitmapTypes *retiType)

	{
	if (rethDIB)
		*rethDIB = NULL;
	if (retiType)
		*retiType = bitmapNone;

	if (Data.Open() != NOERROR)
		return ERR_FAIL;

	char *pBytes = Data.GetPointer(0, -1);
	const int iLength = Data.GetLength();
	if (pBytes == NULL || iLength <= 0)
		return ERR_FAIL;

	SDL_RWops *pRW = SDL_RWFromConstMem(pBytes, iLength);
	if (pRW == NULL)
		return ERR_FAIL;

	SDL_Surface *pLoaded = IMG_Load_RW(pRW, 1);
	if (pLoaded == NULL)
		return ERR_FAIL;

	SDL_Surface *pNormalized = pLoaded;
	const EBitmapTypes iType = DetectBitmapType(pLoaded);

	if (iType == bitmapAlpha)
		{
		//  Preserve per-pixel alpha for hit masks and transparent sprites.
		pNormalized = SDL_ConvertSurfaceFormat(pLoaded, SDL_PIXELFORMAT_BGRA32, 0);
		if (pNormalized == NULL)
			{
			SDL_FreeSurface(pLoaded);
			return ERR_FAIL;
			}

		SDL_FreeSurface(pLoaded);
		}
	else if (pLoaded->format->format != SDL_PIXELFORMAT_BGR24 && pLoaded->format->format != SDL_PIXELFORMAT_RGB565)
		{
		pNormalized = SDL_ConvertSurfaceFormat(pLoaded, SDL_PIXELFORMAT_BGR24, 0);
		if (pNormalized == NULL)
			{
			SDL_FreeSurface(pLoaded);
			return ERR_FAIL;
			}

		SDL_FreeSurface(pLoaded);
		}

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pNormalized, iType, true);
	if (pBitmap == NULL)
		{
		if (pNormalized && pNormalized != pLoaded)
			SDL_FreeSurface(pNormalized);
		else if (pLoaded)
			SDL_FreeSurface(pLoaded);
		return ERR_FAIL;
		}

	if (retiType)
		*retiType = iType;
	if (rethDIB)
		*rethDIB = (HBITMAP)pBitmap;

	return NOERROR;
	}

//	Creating bitmaps
//
//	The macOS port backs HBITMAP with an SDL_Surface (see SDLBitmap.h). An SDL
//	surface is always top-down: row 0 is the top scanline and the pitch is
//	positive. SDLBitmapGetInfo reports exactly that layout (negative biHeight),
//	so the bitmaps created here are top-down as well. Callers must therefore
//	read them through dibGetInfo instead of assuming the bottom-up layout that
//	the Win32 DIB section uses. The pixel pointer that the create calls return
//	points at the first row of the surface; a caller that walks scanlines must
//	step by the stride from dibGetInfo, because SDL may pad the end of a row
//	exactly as a Win32 DIB section does.

ALERROR dibCreate16bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, WORD **retpPixel)
	{
	if (rethBitmap) *rethBitmap = NULL;
	if (retpPixel) *retpPixel = NULL;

	if (cxWidth <= 0 || cyHeight <= 0)
		return ERR_FAIL;

	//	5-6-5, matching the masks that the Win32 implementation installs.

	SDL_Surface *pSurface = SDL_CreateRGBSurfaceWithFormat(0, cxWidth, cyHeight, 16, SDL_PIXELFORMAT_RGB565);
	if (pSurface == NULL)
		return ERR_FAIL;

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pSurface, bitmapRGB, true);
	if (pBitmap == NULL)
		{
		SDL_FreeSurface(pSurface);
		return ERR_FAIL;
		}

	if (retpPixel)
		*retpPixel = (WORD *)pBitmap->pPixels;

	if (rethBitmap)
		*rethBitmap = (HBITMAP)pBitmap;

	return NOERROR;
	}

ALERROR dibCreate24bitDIB (int cxWidth, int cyHeight, DWORD dwFlags, HBITMAP *rethBitmap, BYTE **retpPixel)
	{
	(void)dwFlags;		//	Reserved, as in the Win32 implementation.

	if (rethBitmap) *rethBitmap = NULL;
	if (retpPixel) *retpPixel = NULL;

	if (cxWidth <= 0 || cyHeight <= 0)
		return ERR_FAIL;

	SDL_Surface *pSurface = SDL_CreateRGBSurfaceWithFormat(0, cxWidth, cyHeight, 24, SDL_PIXELFORMAT_BGR24);
	if (pSurface == NULL)
		return ERR_FAIL;

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pSurface, bitmapRGB, true);
	if (pBitmap == NULL)
		{
		SDL_FreeSurface(pSurface);
		return ERR_FAIL;
		}

	if (retpPixel)
		*retpPixel = (BYTE *)pBitmap->pPixels;

	if (rethBitmap)
		*rethBitmap = (HBITMAP)pBitmap;

	return NOERROR;
	}

ALERROR dibCreate32bitDIB (int cxWidth, int cyHeight, DWORD dwFlags, HBITMAP *rethBitmap, DWORD **retpPixel)
	{
	(void)dwFlags;		//	Reserved, as in the Win32 implementation.

	if (rethBitmap) *rethBitmap = NULL;
	if (retpPixel) *retpPixel = NULL;

	if (cxWidth <= 0 || cyHeight <= 0)
		return ERR_FAIL;

	//	BGRA byte order, which is what a BI_RGB 32-bit DIB section holds.

	SDL_Surface *pSurface = SDL_CreateRGBSurfaceWithFormat(0, cxWidth, cyHeight, 32, SDL_PIXELFORMAT_BGRA32);
	if (pSurface == NULL)
		return ERR_FAIL;

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pSurface, bitmapAlpha, true);
	if (pBitmap == NULL)
		{
		SDL_FreeSurface(pSurface);
		return ERR_FAIL;
		}

	if (retpPixel)
		*retpPixel = (DWORD *)pBitmap->pPixels;

	if (rethBitmap)
		*rethBitmap = (HBITMAP)pBitmap;

	return NOERROR;
	}

ALERROR dibConvertToDDB (HBITMAP hDIB, HPALETTE hPalette, HBITMAP *rethBitmap)
	{
	(void)hPalette;

	if (rethBitmap) *rethBitmap = NULL;

	//	This port has no separate device-dependent bitmap: the DIB is already
	//	an SDL surface, so hand back the same handle. Note that the Win32
	//	callers DeleteObject() the DIB afterwards, which is a no-op here.

	if (dibGetInfo(hDIB, NULL, NULL, NULL, NULL, NULL, NULL) != NOERROR)
		return ERR_FAIL;

	if (rethBitmap)
		*rethBitmap = hDIB;

	return NOERROR;
	}

ALERROR dibCrop (HBITMAP hDIB, int x, int y, int cxWidth, int cyHeight, HBITMAP *rethBitmap)
	{
	if (rethBitmap) *rethBitmap = NULL;

	SDLBitmap *pSource = SDLBitmapLookup(hDIB);
	if (pSource == NULL || pSource->surface == NULL)
		return ERR_FAIL;

	//	Reject a crop that falls outside the source instead of reading past
	//	the end of the bitmap.

	if (x < 0 || y < 0 || cxWidth <= 0 || cyHeight <= 0
			|| x + cxWidth > pSource->cxWidth
			|| y + cyHeight > pSource->cyHeight)
		return ERR_FAIL;

	//	The Win32 implementation always produces a 24-bit result.

	SDL_Surface *pCrop = SDL_CreateRGBSurfaceWithFormat(0, cxWidth, cyHeight, 24, SDL_PIXELFORMAT_BGR24);
	if (pCrop == NULL)
		return ERR_FAIL;

	SDL_Rect rcSource;
	rcSource.x = x;
	rcSource.y = y;
	rcSource.w = cxWidth;
	rcSource.h = cyHeight;

	if (SDL_BlitSurface(pSource->surface, &rcSource, pCrop, NULL) != 0)
		{
		SDL_FreeSurface(pCrop);
		return ERR_FAIL;
		}

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pCrop, bitmapRGB, true);
	if (pBitmap == NULL)
		{
		SDL_FreeSurface(pCrop);
		return ERR_FAIL;
		}

	if (rethBitmap)
		*rethBitmap = (HBITMAP)pBitmap;

	return NOERROR;
	}

ALERROR dibLoadFromResource (HINSTANCE hInst, char *szResource, HBITMAP *rethDIB, EBitmapTypes *retiType)
	{
	(void)hInst;
	(void)szResource;

	if (rethDIB) *rethDIB = NULL;
	if (retiType) *retiType = bitmapNone;

	//	Win32 resources are not part of the macOS build; the port loads its
	//	bitmaps through dibLoadFromFile/dibLoadFromBlock instead. Report the
	//	feature once so that a caller does not fail silently.

	PlatformReportUnsupportedFeature("dibLoadFromResource (Win32 resource)");

	return ERR_FAIL;
	}
