//	Save.cpp
//
//	Implements saving JPEGs
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.
//
//	MacOS implementation using ImageIO framework

#include "PreComp.h"

#ifdef _WIN32

ALERROR JPEGSaveToMemory (HBITMAP hBitmap, int iQuality, CString *retsData)
	{
	ALERROR error;
	IJLERR jerr;

	int cxWidth;
	int cyHeight;
	void *pBase;
	int iStride;
	BITMAPINFOHEADER bmih;
	void *pBits;

	if (error = dibGetInfo(hBitmap, &cxWidth, &cyHeight, &pBase, &iStride, &bmih, &pBits))
		return error;

	if (bmih.biBitCount != 24)
		return ERR_FAIL;

	JPEG_CORE_PROPERTIES jcprops;
	jerr = ijlInit(&jcprops);
	if (jerr != IJL_OK)
		return ERR_FAIL;

	char *pBuffer = retsData->GetWritePointer(cxWidth * cyHeight * 3);

	DWORD dwPadBytes = IJL_DIB_PAD_BYTES(bmih.biWidth, 3);

	jcprops.DIBWidth = bmih.biWidth;
	jcprops.DIBHeight = -bmih.biHeight;

	jcprops.DIBBytes = reinterpret_cast<BYTE*>(pBits);
	jcprops.DIBPadBytes = IJL_DIB_PAD_BYTES(bmih.biWidth, 3);
	jcprops.DIBChannels = 3;
	jcprops.DIBColor = IJL_BGR;

	jcprops.JPGWidth = cxWidth;
	jcprops.JPGHeight = cyHeight;
	jcprops.JPGFile = NULL;
	jcprops.JPGBytes = reinterpret_cast<BYTE *>(pBuffer);
	jcprops.JPGSizeBytes = retsData->GetLength();
	jcprops.JPGChannels = 3;
	jcprops.JPGColor = IJL_YCBCR;
	jcprops.JPGSubsampling = IJL_411;
	jcprops.jquality = iQuality;

	jerr = ijlWrite(&jcprops, IJL_JBUFF_WRITEWHOLEIMAGE);
	if (jerr != IJL_OK)
		{
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	retsData->Truncate(jcprops.JPGSizeBytes);

	ijlFree(&jcprops);
	return NOERROR;
	}

#else

ALERROR JPEGSaveToMemory (HBITMAP hBitmap, int iQuality, CString *retsData)
	{
	return ERR_FAIL;
	}

#endif