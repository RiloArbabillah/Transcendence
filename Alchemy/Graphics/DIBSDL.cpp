//	DIBSDL.cpp
//
//	SDL-based DIB loading stubs for macOS
//	Provides minimal implementation to allow linking

#include "PreComp.h"
#include "Graphics.h"

ALERROR dibLoadFromBlock (IReadBlock &Data, HBITMAP *rethDIB, EBitmapTypes *retiType)
	{
	if (rethDIB)
		*rethDIB = NULL;
	return ERR_FAIL;
	}
