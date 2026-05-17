//	Load.cpp
//
//	Implements loading JPEGs
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.
//
//	MacOS implementation using ImageIO framework

#include "PreComp.h"
#include "JPEGUtil.h"

#include <ImageIO/ImageIO.h>

#ifndef _WIN32
#include "SDLBitmap.h"
#include <SDL2/SDL.h>
#endif
#include <CoreGraphics/CoreGraphics.h>

#ifdef _WIN32

ALERROR JPEGLoadFromFile (CString sFilename, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)
	{
	ALERROR error;
	CFileReadBlock JPEGFile(sFilename);

	if (error = JPEGFile.Open())
		return error;

	error = JPEGLoadFromMemory(JPEGFile.GetPointer(0, JPEGFile.GetLength()), JPEGFile.GetLength(), dwFlags, hPalette, rethBitmap);
	JPEGFile.Close();
	if (error)
		return error;

	return NOERROR;
	}

ALERROR JPEGLoadFromMemory (char *pImage, int iSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)
	{
	ALERROR error;
	IJLERR jerr;

	JPEG_CORE_PROPERTIES jcprops;
	jerr = ijlInit(&jcprops);
	if (jerr != IJL_OK)
		return ERR_FAIL;

	jcprops.JPGFile = NULL;
	jcprops.JPGBytes = (BYTE *)pImage;
	jcprops.JPGSizeBytes = iSize;

	jerr = ijlRead(&jcprops, IJL_JBUFF_READPARAMS);
	if (jerr != IJL_OK)
		{
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	DWORD width = jcprops.JPGWidth;
	DWORD height = jcprops.JPGHeight;
	DWORD nchannels = 3;
	DWORD dib_line_width = width * nchannels;
	DWORD dib_pad_bytes = IJL_DIB_PAD_BYTES(width,nchannels);

	HBITMAP hBitmap;
	BYTE *p24BitPixel;
	if (error = dibCreate24bitDIB(width,
			height,
			0,
			&hBitmap,
			&p24BitPixel))
		{
		ijlFree(&jcprops);
		return error;
		}

	jcprops.DIBWidth = width;
	jcprops.DIBHeight = -(int)height;
	jcprops.DIBChannels = nchannels;
	jcprops.DIBColor = IJL_BGR;
	jcprops.DIBPadBytes = dib_pad_bytes;
	jcprops.DIBBytes = p24BitPixel;

	switch (jcprops.JPGChannels)
		{
		case 1:
			jcprops.JPGColor = IJL_G;
			break;

		case 3:
			jcprops.JPGColor = IJL_YCBCR;
			break;

		default:
			{
			jcprops.DIBColor = (IJL_COLOR)IJL_OTHER;
			jcprops.JPGColor = (IJL_COLOR)IJL_OTHER;
			}
		}

	jerr = ijlRead(&jcprops, IJL_JBUFF_READWHOLEIMAGE);
	if (jerr != IJL_OK)
		{
		::DeleteObject(hBitmap);
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	ijlFree(&jcprops);

	if (dwFlags & JPEG_LFR_DIB)
		*rethBitmap = hBitmap;
	else
		{
		HBITMAP hDDB;
		error = dibConvertToDDB(hBitmap, hPalette, &hDDB);
		::DeleteObject(hBitmap);
		if (error)
			return error;

		*rethBitmap = hDDB;
		}

	return NOERROR;
	}

#else

ALERROR JPEGLoadFromFile (CString sFilename, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)
	{
	SJPEGLoadInfo info;
	ALERROR error = JPEGLoadToRGBAFromFile(sFilename, &info);
	if (error != NOERROR)
		return error;

	SDL_Surface *pSurface = SDL_CreateRGBSurfaceWithFormat(0, info.cxWidth, info.cyHeight, 24, SDL_PIXELFORMAT_BGR24);
	if (pSurface == NULL)
		return ERR_FAIL;

	const BYTE *pSrcRow = (const BYTE *)info.Pixels.GetPointer();
	BYTE *pDestRow = (BYTE *)pSurface->pixels;
	for (int y = 0; y < info.cyHeight; y++)
		{
		const BYTE *pSrc = pSrcRow;
		BYTE *pDest = pDestRow;
		for (int x = 0; x < info.cxWidth; x++)
			{
			pDest[0] = pSrc[0];
			pDest[1] = pSrc[1];
			pDest[2] = pSrc[2];
			pSrc += 4;
			pDest += 3;
			}

		pSrcRow += info.iPitch;
		pDestRow += pSurface->pitch;
		}

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pSurface, bitmapRGB, true);
	if (pBitmap == NULL)
		{
		SDL_FreeSurface(pSurface);
		return ERR_FAIL;
		}

	*rethBitmap = (HBITMAP)pBitmap;
	return NOERROR;
	}

ALERROR JPEGLoadFromMemory (char *pImage, int iSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)
	{
	SJPEGLoadInfo info;
	ALERROR error = JPEGLoadToRGBAFromMemory(pImage, iSize, &info);
	if (error != NOERROR)
		return error;

	SDL_Surface *pSurface = SDL_CreateRGBSurfaceWithFormat(0, info.cxWidth, info.cyHeight, 24, SDL_PIXELFORMAT_BGR24);
	if (pSurface == NULL)
		return ERR_FAIL;

	const BYTE *pSrcRow = (const BYTE *)info.Pixels.GetPointer();
	BYTE *pDestRow = (BYTE *)pSurface->pixels;
	for (int y = 0; y < info.cyHeight; y++)
		{
		const BYTE *pSrc = pSrcRow;
		BYTE *pDest = pDestRow;
		for (int x = 0; x < info.cxWidth; x++)
			{
			pDest[0] = pSrc[0];
			pDest[1] = pSrc[1];
			pDest[2] = pSrc[2];
			pSrc += 4;
			pDest += 3;
			}

		pSrcRow += info.iPitch;
		pDestRow += pSurface->pitch;
		}

	SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pSurface, bitmapRGB, true);
	if (pBitmap == NULL)
		{
		SDL_FreeSurface(pSurface);
		return ERR_FAIL;
		}

	*rethBitmap = (HBITMAP)pBitmap;
	return NOERROR;
	}

#endif

ALERROR JPEGLoadToRGBAFromFile (CString sFilename, SJPEGLoadInfo *retImage)
	{
#ifdef _WIN32
	ALERROR error;
	CFileReadBlock JPEGFile(sFilename);

	if (error = JPEGFile.Open())
		return error;

	error = JPEGLoadToRGBAFromMemory(JPEGFile.GetPointer(0, JPEGFile.GetLength()), JPEGFile.GetLength(), retImage);
	JPEGFile.Close();
	return error;
#else
	if (retImage == NULL)
		return ERR_FAIL;

	CFStringRef cfFilename = CFStringCreateWithCString(kCFAllocatorDefault, sFilename.GetASCIIZPointer(), kCFStringEncodingUTF8);
	if (cfFilename == NULL)
		return ERR_FAIL;

	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfFilename, kCFURLPOSIXPathStyle, false);
	CFRelease(cfFilename);
	if (url == NULL)
		return ERR_FAIL;

	CGImageSourceRef imageSource = CGImageSourceCreateWithURL(url, NULL);
	CFRelease(url);
	if (imageSource == NULL)
		return ERR_FAIL;

	CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
	CFRelease(imageSource);
	if (image == NULL)
		return ERR_FAIL;

	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);

	retImage->cxWidth = (int)width;
	retImage->cyHeight = (int)height;
	retImage->iPitch = (int)width * 4;

	size_t dataSize = retImage->iPitch * retImage->cyHeight;
	retImage->Pixels = "";

	char *pixels = new char[dataSize];

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	if (colorSpace == NULL)
		{
		delete[] pixels;
		CGImageRelease(image);
		return ERR_FAIL;
		}

	CGContextRef context = CGBitmapContextCreate(
		pixels,
		width,
		height,
		8,
		retImage->iPitch,
		colorSpace,
		(CGBitmapInfo)(kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little)
	);
	CGColorSpaceRelease(colorSpace);

	if (context == NULL)
		{
		delete[] pixels;
		CGImageRelease(image);
		return ERR_FAIL;
		}

	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
	CGContextRelease(context);
	CGImageRelease(image);

	retImage->Pixels = CString(pixels, dataSize);
	delete[] pixels;

	return NOERROR;
#endif
	}

ALERROR JPEGLoadToRGBAFromMemory (char *pImage, int iSize, SJPEGLoadInfo *retImage)
	{
#ifdef _WIN32
	if (retImage == NULL)
		return ERR_FAIL;

	ALERROR error;
	IJLERR jerr;

	JPEG_CORE_PROPERTIES jcprops;
	jerr = ijlInit(&jcprops);
	if (jerr != IJL_OK)
		return ERR_FAIL;

	jcprops.JPGFile = NULL;
	jcprops.JPGBytes = (BYTE *)pImage;
	jcprops.JPGSizeBytes = iSize;

	jerr = ijlRead(&jcprops, IJL_JBUFF_READPARAMS);
	if (jerr != IJL_OK)
		{
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	int cxWidth = (int)jcprops.JPGWidth;
	int cyHeight = (int)jcprops.JPGHeight;

	retImage->cxWidth = cxWidth;
	retImage->cyHeight = cyHeight;

	jcprops.DIBWidth = cxWidth;
	jcprops.DIBHeight = -cyHeight;
	jcprops.DIBChannels = 4;
	jcprops.DIBColor = IJL_RGBA_FPX;
	jcprops.DIBPadBytes = 0;

	switch (jcprops.JPGChannels)
		{
		case 1:
			jcprops.JPGColor = IJL_G;
			break;

		case 3:
			jcprops.JPGColor = IJL_YCBCR;
			break;

		default:
			jcprops.DIBColor = (IJL_COLOR)IJL_OTHER;
			jcprops.JPGColor = (IJL_COLOR)IJL_OTHER;
			break;
		}

	retImage->iPitch = cxWidth * 4;

	size_t dataSize = retImage->iPitch * cyHeight;
	retImage->Pixels = "";

	char *pixels = new char[dataSize];
	jcprops.DIBBytes = (BYTE *)pixels;

	jerr = ijlRead(&jcprops, IJL_JBUFF_READWHOLEIMAGE);
	if (jerr != IJL_OK)
		{
		delete[] pixels;
		retImage->Pixels = "";
		retImage->cxWidth = 0;
		retImage->cyHeight = 0;
		retImage->iPitch = 0;
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	ijlFree(&jcprops);

	retImage->Pixels = CString(pixels, dataSize);
	delete[] pixels;

	return NOERROR;
#else
	if (retImage == NULL)
		return ERR_FAIL;

	CFDataRef dataRef = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8 *)pImage, iSize, kCFAllocatorNull);
	if (dataRef == NULL)
		return ERR_FAIL;

	CGImageSourceRef imageSource = CGImageSourceCreateWithData(dataRef, NULL);
	CFRelease(dataRef);
	if (imageSource == NULL)
		return ERR_FAIL;

	CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
	CFRelease(imageSource);
	if (image == NULL)
		return ERR_FAIL;

	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);

	retImage->cxWidth = (int)width;
	retImage->cyHeight = (int)height;
	retImage->iPitch = (int)width * 4;

	size_t dataSize = retImage->iPitch * retImage->cyHeight;

	char *pixels = new char[dataSize];

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	if (colorSpace == NULL)
		{
		delete[] pixels;
		CGImageRelease(image);
		return ERR_FAIL;
		}

	CGContextRef context = CGBitmapContextCreate(
		pixels,
		width,
		height,
		8,
		retImage->iPitch,
		colorSpace,
		(CGBitmapInfo)(kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little)
	);
	CGColorSpaceRelease(colorSpace);

	if (context == NULL)
		{
		delete[] pixels;
		CGImageRelease(image);
		return ERR_FAIL;
		}

	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
	CGContextRelease(context);
	CGImageRelease(image);

	retImage->Pixels = CString(pixels, dataSize);
	delete[] pixels;

	return NOERROR;
#endif
	}

#ifdef _WIN32
ALERROR JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)
	{
	HRSRC hRes;
	HGLOBAL hGlobalRes;
	void *pImage;
	int iSize;

	hRes = FindResource(hInst, pszRes, "JPEG");
	if (hRes == NULL)
		return ERR_NOTFOUND;

	iSize = SizeofResource(hInst, hRes);
	if (iSize == 0)
		return ERR_FAIL;

	hGlobalRes = LoadResource(hInst, hRes);
	if (hGlobalRes == NULL)
		return ERR_FAIL;

	pImage = LockResource(hGlobalRes);
	if (pImage == NULL)
		return ERR_FAIL;

	return JPEGLoadFromMemory((char *)pImage, iSize, dwFlags, hPalette, rethBitmap);
	}
#else
ALERROR JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)
{
    return ERR_NOTFOUND;
}
#endif
