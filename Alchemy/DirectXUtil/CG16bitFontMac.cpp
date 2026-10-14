//	CG16bitFontMac.cpp
//
//	macOS replacements for the GDI-dependent parts of CG16bitFont.cpp.
//
//	On Windows, CG16bitFont::Create asks GDI for a font, draws every character
//	from ' ' to 0xFF (Windows-1252) white-on-black into a strip bitmap, one
//	cell of tmHeight pixels per character, and turns the strip into an alpha
//	mask. The rest of CG16bitFont (measuring, wrapping and drawing) works from
//	that strip and the per-character ABC widths. Here we build the same strip
//	and metrics with CoreText, following GDI's metric rules, so the shared
//	code lays out text the way it does on Windows.

#ifndef _WIN32

#include "PreComp.h"
#include <ApplicationServices/ApplicationServices.h>
#include <cmath>

const int g_iMacStartChar =						' ';
const int g_iMacCharCount =						0xff - g_iMacStartChar + 1;

//	Typefaces the game asks for that macOS does not ship. Lucida Grande is the
//	macOS member of the Lucida Sans family; Monaco stands in for the
//	monospaced Lucida Console.

static CString MapTypeface (const CString &sTypeface)
	{
	if (strEquals(sTypeface, CONSTLIT("Lucida Sans")))
		return CONSTLIT("Lucida Grande");
	else if (strEquals(sTypeface, CONSTLIT("Lucida Console")))
		return CONSTLIT("Monaco");
	else if (sTypeface.IsBlank())
		return CONSTLIT("Arial");
	else
		return sTypeface;
	}

static CTFontRef CreateBaseFont (const CString &sTypeface, bool bBold, bool bItalic)
	{
	CFStringRef sFamily = CFStringCreateWithCString(NULL, MapTypeface(sTypeface).GetASCIIZPointer(), kCFStringEncodingWindowsLatin1);
	CTFontRef pFont = CTFontCreateWithName(sFamily, 1.0, NULL);
	CFRelease(sFamily);
	if (pFont == NULL)
		return NULL;

	CTFontSymbolicTraits dwTraits = 0;
	if (bBold)
		dwTraits |= kCTFontBoldTrait;
	if (bItalic)
		dwTraits |= kCTFontItalicTrait;

	if (dwTraits)
		{
		CTFontRef pStyled = CTFontCreateCopyWithSymbolicTraits(pFont, 0.0, NULL, dwTraits, dwTraits);
		if (pStyled)
			{
			CFRelease(pFont);
			pFont = pStyled;
			}
		}

	return pFont;
	}

//	GDI takes its line metrics from the OS/2 table (usWinAscent, usWinDescent,
//	xAvgCharWidth), not from hhea. Returns false if the font has no OS/2 table.

static bool GetWinMetrics (CTFontRef pFont, int *retiWinAscent, int *retiWinDescent, int *retiAvgWidth)
	{
	CFDataRef pTable = CTFontCopyTable(pFont, kCTFontTableOS2, kCTFontTableOptionNoOptions);
	if (pTable == NULL)
		return false;

	bool bOK = (CFDataGetLength(pTable) >= 78);
	if (bOK)
		{
		const UInt8 *pData = CFDataGetBytePtr(pTable);
		*retiAvgWidth = (SInt16)((pData[2] << 8) | pData[3]);
		*retiWinAscent = (UInt16)((pData[74] << 8) | pData[75]);
		*retiWinDescent = (UInt16)((pData[76] << 8) | pData[77]);
		bOK = (*retiWinAscent + *retiWinDescent > 0);
		}

	CFRelease(pTable);
	return bOK;
	}

//	The alpha value CG16bitImage::CreateFromBitmap(..., cfbDesaturateAlpha)
//	computes for a gray pixel of the 16-bit (5-6-5) strip GDI draws into.

static BYTE DesaturatedAlpha (BYTE byGray)
	{
	DWORD dwSource = ((DWORD)(byGray >> 3) << 11) | ((DWORD)(byGray >> 2) << 5) | (DWORD)(byGray >> 3);
	if (dwSource == 0)
		return 0x00;
	else if (dwSource == 0xffff)
		return 0xff;

	DWORD dwBlue = (((dwSource & 0x001f) << 8) / 31);
	DWORD dwGreen = (((dwSource & 0x07e0) << 3) / 63);
	DWORD dwRed = (((dwSource & 0xf800) >> 3) / 31);

	DWORD dwMaxColor = Max(dwRed, Max(dwGreen, dwBlue));
	DWORD dwMinColor = Min(dwRed, Min(dwGreen, dwBlue));
	return (BYTE)((dwMaxColor + dwMinColor) / 2);
	}

CG16bitFont::CG16bitFont (const CG16bitFont &Src) :
		m_FontImage(Src.m_FontImage),
		m_cyHeight(Src.m_cyHeight),
		m_cyAscent(Src.m_cyAscent),
		m_cxAveWidth(Src.m_cxAveWidth),
		m_Metrics(Src.m_Metrics),
		m_sTypeface(Src.m_sTypeface),
		m_bBold(Src.m_bBold),
		m_bItalic(Src.m_bItalic),
		m_bUnderline(Src.m_bUnderline)

//	CG16bitFont copy constructor

	{
	}

CG16bitFont &CG16bitFont::operator= (const CG16bitFont &Src)

//	CG16bitFont operator =

	{
	if (this != &Src)
		{
		m_FontImage = Src.m_FontImage;
		m_cyHeight = Src.m_cyHeight;
		m_cyAscent = Src.m_cyAscent;
		m_cxAveWidth = Src.m_cxAveWidth;
		m_Metrics = Src.m_Metrics;
		m_sTypeface = Src.m_sTypeface;
		m_bBold = Src.m_bBold;
		m_bItalic = Src.m_bItalic;
		m_bUnderline = Src.m_bUnderline;
		}

	return *this;
	}

ALERROR CG16bitFont::Create (const CString &sTypeface, int iSize, bool bBold, bool bItalic, bool bUnderline)

//	Create
//
//	Creates a basic font (see the file header for how this mirrors GDI)

	{
	//	GDI picks a default size for a height of 0.

	if (iSize == 0)
		iSize = 16;

	CTFontRef pUnitFont = CreateBaseFont(sTypeface, bBold, bItalic);
	if (pUnitFont == NULL)
		return ERR_FAIL;

	//	CreateFont semantics: a negative height is the em (character) height,
	//	a positive height is the cell height (ascent + descent).

	const double rUnitsPerEm = (double)CTFontGetUnitsPerEm(pUnitFont);
	int iWinAscent, iWinDescent, iAvgWidth;
	if (!GetWinMetrics(pUnitFont, &iWinAscent, &iWinDescent, &iAvgWidth))
		{
		iWinAscent = (int)std::ceil(CTFontGetAscent(pUnitFont) * rUnitsPerEm);
		iWinDescent = (int)std::ceil(CTFontGetDescent(pUnitFont) * rUnitsPerEm);
		iAvgWidth = 0;
		}

	const double rEmPixels = (iSize < 0 ? (double)-iSize : (double)iSize * rUnitsPerEm / (iWinAscent + iWinDescent));
	const double rScale = rEmPixels / rUnitsPerEm;

	CTFontRef pFont = CTFontCreateCopyWithAttributes(pUnitFont, (CGFloat)rEmPixels, NULL, NULL);
	CFRelease(pUnitFont);
	if (pFont == NULL)
		return ERR_FAIL;

	m_cyAscent = (int)std::lround(iWinAscent * rScale);
	m_cyHeight = m_cyAscent + (int)std::lround(iWinDescent * rScale);
	if (m_cyHeight <= 0)
		{
		CFRelease(pFont);
		return ERR_FAIL;
		}

	//	Map the Windows-1252 character range to glyphs. Unassigned code points
	//	get glyph 0 (.notdef), which is what GDI draws for them.

	CGGlyph Glyphs[g_iMacCharCount];
	for (int i = 0; i < g_iMacCharCount; i++)
		{
		UInt8 byChar = (UInt8)(g_iMacStartChar + i);
		UniChar wChar = 0;
		CFStringRef sChar = CFStringCreateWithBytes(NULL, &byChar, 1, kCFStringEncodingWindowsLatin1, false);
		if (sChar && CFStringGetLength(sChar) == 1)
			wChar = CFStringGetCharacterAtIndex(sChar, 0);
		if (sChar)
			CFRelease(sChar);

		Glyphs[i] = 0;
		if (wChar)
			CTFontGetGlyphsForCharacters(pFont, &wChar, &Glyphs[i], 1);
		}

	CGSize Advances[g_iMacCharCount];
	CGRect Bounds[g_iMacCharCount];
	CTFontGetAdvancesForGlyphs(pFont, kCTFontOrientationHorizontal, Glyphs, Advances, g_iMacCharCount);
	CTFontGetBoundingRectsForGlyphs(pFont, kCTFontOrientationHorizontal, Glyphs, Bounds, g_iMacCharCount);

	//	ABC widths: A + B is the right edge of the glyph's ink (the strip is
	//	drawn at x = 0, so ink left of the origin is clipped, as with TextOut)
	//	and A + B + C is the integer advance.

	m_Metrics.DeleteAll();
	m_Metrics.GrowToFit(g_iMacCharCount);

	int cxStrip = 1;
	for (int i = 0; i < g_iMacCharCount; i++)
		{
		CharMetrics Metrics;
		Metrics.cxAdvance = (int)std::lround(Advances[i].width);
		Metrics.cxWidth = (CGRectIsEmpty(Bounds[i]) ? 0 : Max(0, (int)std::ceil(CGRectGetMaxX(Bounds[i]))));
		m_Metrics.Insert(Metrics);

		cxStrip = Max(cxStrip, Max(Metrics.cxWidth, Metrics.cxAdvance));
		}

	m_cxAveWidth = (iAvgWidth > 0 ? (int)std::lround(iAvgWidth * rScale) : m_Metrics[(int)'x' - g_iMacStartChar].cxAdvance);

	//	Draw every character white on black, one cell per character.

	const int cyStrip = m_cyHeight * g_iMacCharCount;
	CGColorSpaceRef pGray = CGColorSpaceCreateDeviceGray();
	CGContextRef pContext = CGBitmapContextCreate(NULL, cxStrip, cyStrip, 8, 0, pGray, kCGImageAlphaNone);
	CGColorSpaceRelease(pGray);
	if (pContext == NULL)
		{
		CFRelease(pFont);
		return ERR_MEMORY;
		}

	CGContextSetGrayFillColor(pContext, 0.0, 1.0);
	CGContextFillRect(pContext, CGRectMake(0, 0, cxStrip, cyStrip));
	CGContextSetGrayFillColor(pContext, 1.0, 1.0);
	CGContextSetShouldAntialias(pContext, true);
	CGContextSetShouldSmoothFonts(pContext, false);

	const CGFloat rUnderlinePos = CTFontGetUnderlinePosition(pFont);
	const CGFloat rUnderlineThickness = Max((CGFloat)1.0, (CGFloat)std::round(CTFontGetUnderlineThickness(pFont)));

	for (int i = 0; i < g_iMacCharCount; i++)
		{
		//	CoreGraphics has its origin at the bottom left.

		const CGFloat yBaseline = (CGFloat)(cyStrip - (i * m_cyHeight) - m_cyAscent);
		CGPoint Pos = CGPointMake(0, yBaseline);
		CTFontDrawGlyphs(pFont, &Glyphs[i], &Pos, 1, pContext);

		if (bUnderline)
			CGContextFillRect(pContext, CGRectMake(0, std::round(yBaseline + rUnderlinePos - rUnderlineThickness), m_Metrics[i].cxAdvance, rUnderlineThickness));
		}

	CGContextFlush(pContext);
	CFRelease(pFont);

	//	Convert the strip into the alpha-only image the drawing code expects.

	ALERROR error;
	if (error = m_FontImage.CreateBlankAlpha(cxStrip, cyStrip))
		{
		CGContextRelease(pContext);
		return error;
		}

	const BYTE *pPixels = (const BYTE *)CGBitmapContextGetData(pContext);
	const size_t iStride = CGBitmapContextGetBytesPerRow(pContext);
	for (int y = 0; y < cyStrip; y++)
		{
		const BYTE *pSrc = pPixels + (y * iStride);
		for (int x = 0; x < cxStrip; x++)
			*m_FontImage.GetAlphaValue(x, y) = DesaturatedAlpha(pSrc[x]);
		}

	CGContextRelease(pContext);

	m_sTypeface = sTypeface;
	m_bBold = bBold;
	m_bItalic = bItalic;
	m_bUnderline = bUnderline;

	return NOERROR;
	}

ALERROR CG16bitFont::CreateFromFont (HFONT hFont)

//	CreateFromFont
//
//	There are no GDI font handles on macOS.

	{
	return ERR_FAIL;
	}

ALERROR CG16bitFont::CreateFromResource (HINSTANCE hInst, const char *pszRes)

//	CreateFromResource
//
//	macOS has no DXFN resources; callers load the .dxfn files from the
//	resource folder with CreateFromFile instead.

	{
	return ERR_FAIL;
	}

#endif
