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
