//	ClipboardMacOS.mm
//
//	macOS clipboard support for images (the equivalent of SetClipboardData with
//	CF_BITMAP on Windows).

#import <AppKit/AppKit.h>

//	Copies 32-bit pixels laid out as 0xAARRGGBB DWORDs (CG32bitPixel) to the
//	general pasteboard. Like the Win32 path, which blts the image into an
//	opaque device bitmap, the alpha channel is ignored. Returns false on
//	failure.

extern "C" bool macosCopyImageToClipboard (const void *pPixels, int cxWidth, int cyHeight, int iPitch)
	{
	if (pPixels == NULL || cxWidth <= 0 || cyHeight <= 0)
		return false;

	@autoreleasepool
		{
		CGColorSpaceRef pColorSpace = CGColorSpaceCreateDeviceRGB();
		CGContextRef pContext = CGBitmapContextCreate((void *)pPixels, cxWidth, cyHeight, 8, iPitch, pColorSpace,
				kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little);
		CGColorSpaceRelease(pColorSpace);
		if (pContext == NULL)
			return false;

		CGImageRef pImage = CGBitmapContextCreateImage(pContext);
		CGContextRelease(pContext);
		if (pImage == NULL)
			return false;

		NSBitmapImageRep *pRep = [[NSBitmapImageRep alloc] initWithCGImage:pImage];
		CGImageRelease(pImage);

		NSData *pData = [pRep TIFFRepresentation];
		if (pData == nil)
			return false;

		NSPasteboard *pPasteboard = [NSPasteboard generalPasteboard];
		[pPasteboard clearContents];
		return ([pPasteboard setData:pData forType:NSPasteboardTypeTIFF] == YES);
		}
	}
