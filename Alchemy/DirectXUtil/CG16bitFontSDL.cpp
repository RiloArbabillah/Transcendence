//	CG16bitFontSDL.cpp
//	Fallback font implementation for macOS SDL platform

#include "Alchemy.h"
#include "DirectXUtil.h"

const int FONT_SAVE_VERSION =					1;

#define STR_ELLIPSIS							CONSTLIT("...")

const int g_iStartChar =						' ';
const int g_iCharCount =						0xff - g_iStartChar + 1;

static const BYTE *GetFallbackGlyph (char chChar)
	{
	static const BYTE BLANK[7] = { 0, 0, 0, 0, 0, 0, 0 };
	static const BYTE UNKNOWN[7] = { 0x0e, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04 };

	switch (chChar)
		{
		case ' ': return BLANK;
		case '!': { static const BYTE G[7] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 }; return G; }
		case '"': { static const BYTE G[7] = { 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00 }; return G; }
		case '#': { static const BYTE G[7] = { 0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a }; return G; }
		case '$': { static const BYTE G[7] = { 0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04 }; return G; }
		case '%': { static const BYTE G[7] = { 0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13 }; return G; }
		case '&': { static const BYTE G[7] = { 0x0c, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0d }; return G; }
		case '\'': { static const BYTE G[7] = { 0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00 }; return G; }
		case '(': { static const BYTE G[7] = { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 }; return G; }
		case ')': { static const BYTE G[7] = { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 }; return G; }
		case '*': { static const BYTE G[7] = { 0x00, 0x04, 0x15, 0x0e, 0x15, 0x04, 0x00 }; return G; }
		case '+': { static const BYTE G[7] = { 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00 }; return G; }
		case ',': { static const BYTE G[7] = { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 }; return G; }
		case '-': { static const BYTE G[7] = { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 }; return G; }
		case '.': { static const BYTE G[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c }; return G; }
		case '/': { static const BYTE G[7] = { 0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10 }; return G; }
		case ':': { static const BYTE G[7] = { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00 }; return G; }
		case ';': { static const BYTE G[7] = { 0x00, 0x0c, 0x0c, 0x00, 0x04, 0x04, 0x08 }; return G; }
		case '<': { static const BYTE G[7] = { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 }; return G; }
		case '=': { static const BYTE G[7] = { 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00 }; return G; }
		case '>': { static const BYTE G[7] = { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 }; return G; }
		case '?': return UNKNOWN;
		case '@': { static const BYTE G[7] = { 0x0e, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0e }; return G; }
		case '[': { static const BYTE G[7] = { 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e }; return G; }
		case '\\': { static const BYTE G[7] = { 0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01 }; return G; }
		case ']': { static const BYTE G[7] = { 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e }; return G; }
		case '^': { static const BYTE G[7] = { 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00 }; return G; }
		case '_': { static const BYTE G[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f }; return G; }
		case '`': { static const BYTE G[7] = { 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 }; return G; }
		case '{': { static const BYTE G[7] = { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02 }; return G; }
		case '|': { static const BYTE G[7] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }; return G; }
		case '}': { static const BYTE G[7] = { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08 }; return G; }
		case '~': { static const BYTE G[7] = { 0x00, 0x00, 0x08, 0x15, 0x02, 0x00, 0x00 }; return G; }

		case '0': { static const BYTE G[7] = { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e }; return G; }
		case '1': { static const BYTE G[7] = { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e }; return G; }
		case '2': { static const BYTE G[7] = { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f }; return G; }
		case '3': { static const BYTE G[7] = { 0x1f, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0e }; return G; }
		case '4': { static const BYTE G[7] = { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 }; return G; }
		case '5': { static const BYTE G[7] = { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e }; return G; }
		case '6': { static const BYTE G[7] = { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e }; return G; }
		case '7': { static const BYTE G[7] = { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 }; return G; }
		case '8': { static const BYTE G[7] = { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e }; return G; }
		case '9': { static const BYTE G[7] = { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c }; return G; }

		case 'A': case 'a': { static const BYTE G[7] = { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 }; return G; }
		case 'B': case 'b': { static const BYTE G[7] = { 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e }; return G; }
		case 'C': case 'c': { static const BYTE G[7] = { 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e }; return G; }
		case 'D': case 'd': { static const BYTE G[7] = { 0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e }; return G; }
		case 'E': case 'e': { static const BYTE G[7] = { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f }; return G; }
		case 'F': case 'f': { static const BYTE G[7] = { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 }; return G; }
		case 'G': case 'g': { static const BYTE G[7] = { 0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0f }; return G; }
		case 'H': case 'h': { static const BYTE G[7] = { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 }; return G; }
		case 'I': case 'i': { static const BYTE G[7] = { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e }; return G; }
		case 'J': case 'j': { static const BYTE G[7] = { 0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0c }; return G; }
		case 'K': case 'k': { static const BYTE G[7] = { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 }; return G; }
		case 'L': case 'l': { static const BYTE G[7] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f }; return G; }
		case 'M': case 'm': { static const BYTE G[7] = { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 }; return G; }
		case 'N': case 'n': { static const BYTE G[7] = { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 }; return G; }
		case 'O': case 'o': { static const BYTE G[7] = { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }; return G; }
		case 'P': case 'p': { static const BYTE G[7] = { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 }; return G; }
		case 'Q': case 'q': { static const BYTE G[7] = { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d }; return G; }
		case 'R': case 'r': { static const BYTE G[7] = { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11 }; return G; }
		case 'S': case 's': { static const BYTE G[7] = { 0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e }; return G; }
		case 'T': case 't': { static const BYTE G[7] = { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }; return G; }
		case 'U': case 'u': { static const BYTE G[7] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e }; return G; }
		case 'V': case 'v': { static const BYTE G[7] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 }; return G; }
		case 'W': case 'w': { static const BYTE G[7] = { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a }; return G; }
		case 'X': case 'x': { static const BYTE G[7] = { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 }; return G; }
		case 'Y': case 'y': { static const BYTE G[7] = { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 }; return G; }
		case 'Z': case 'z': { static const BYTE G[7] = { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f }; return G; }
		}

	return UNKNOWN;
	}

static void DrawFallbackGlyph (CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, char chChar, int iScale, bool bBold)
	{
	const BYTE *pGlyph = GetFallbackGlyph(chChar);
	for (int cy = 0; cy < 7; cy++)
		for (int cx = 0; cx < 5; cx++)
			if (pGlyph[cy] & (0x10 >> cx))
				{
				Dest.Fill(x + (cx * iScale), y + (cy * iScale), iScale + (bBold ? 1 : 0), iScale, rgbColor);
				}
	}

static void DrawFallbackGlyph (CG16bitImage &Dest, int x, int y, WORD wColor, char chChar, int iScale, bool bBold)
	{
	const BYTE *pGlyph = GetFallbackGlyph(chChar);
	for (int cy = 0; cy < 7; cy++)
		for (int cx = 0; cx < 5; cx++)
			if (pGlyph[cy] & (0x10 >> cx))
				{
				Dest.Fill(x + (cx * iScale), y + (cy * iScale), iScale + (bBold ? 1 : 0), iScale, wColor);
				}
	}

static int GetFallbackScale (int cyHeight)
	{
	return Max(1, cyHeight / 8);
	}

void FormatLine (char *pPos, int iLen, bool *ioInSmartQuotes, TArray<CString> *retLines);

CG16bitFont::CG16bitFont(void) : m_cyHeight(12), m_cyAscent(10), m_cxAveWidth(8), m_bBold(false), m_bItalic(false), m_bUnderline(false) { }
CG16bitFont::CG16bitFont(const CG16bitFont &Src) :
		m_FontImage(Src.m_FontImage),
		m_cyHeight(Src.m_cyHeight),
		m_cyAscent(Src.m_cyAscent),
		m_cxAveWidth(Src.m_cxAveWidth),
		m_Metrics(Src.m_Metrics),
		m_sTypeface(Src.m_sTypeface),
		m_bBold(Src.m_bBold),
		m_bItalic(Src.m_bItalic),
		m_bUnderline(Src.m_bUnderline)
	{ }

CG16bitFont &CG16bitFont::operator=(const CG16bitFont &Src)
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

ALERROR CG16bitFont::Create(const CString &sTypeface, int iSize, bool bBold, bool bItalic, bool bUnderline) { m_sTypeface = sTypeface; m_cyHeight = Max(7, Absolute(iSize)); m_cyAscent = m_cyHeight - Max(1, m_cyHeight / 5); m_cxAveWidth = (GetFallbackScale(m_cyHeight) * 6) + (bBold ? 1 : 0); m_bBold = bBold; m_bItalic = bItalic; m_bUnderline = bUnderline; return NOERROR; }
ALERROR CG16bitFont::CreateFromFile(const CString &sFilespec)
	{
	CFileReadBlock File(sFilespec);
	ALERROR error = File.Open();
	if (error)
		return error;

	CMemoryReadStream Stream(File.GetPointer(0), File.GetLength());
	if ((error = Stream.Open()))
		return error;

	error = ReadFromStream(&Stream);
	Stream.Close();
	File.Close();
	return error;
	}
ALERROR CG16bitFont::CreateFromFont(HFONT hFont) { return NOERROR; }
ALERROR CG16bitFont::CreateFromResource(HINSTANCE hInst, const char *pszRes) { return Create(CONSTLIT("Fallback"), -16); }

int CG16bitFont::BreakText(const CString &sText, int cxWidth, TArray<CString> *retLines, DWORD dwFlags) const
	{
	if (retLines)
		retLines->DeleteAll();

	if (cxWidth <= 0 || sText.IsBlank())
		return 0;

	const int cxEllipsis = (dwFlags & TruncateLine ? MeasureText(STR_ELLIPSIS) : 0);
	const int cxLine = cxWidth - cxEllipsis;
	if (cxLine < 0)
		return 0;

	char *pText = sText.GetASCIIZPointer();
	const int iLength = sText.GetLength();
	int iStart = 0;
	int iLines = 0;
	bool bTruncated = false;
	bool bInSmartQuotes = false;

	while (iStart < iLength)
		{
		if (pText[iStart] == '\n')
			{
			FormatLine(pText + iStart, 0, (dwFlags & SmartQuotes ? &bInSmartQuotes : NULL), retLines);
			iLines++;
			iStart++;
			bInSmartQuotes = false;
			continue;
			}

		int iPos = iStart;
		int iLastBreak = -1;
		int iEnd = iStart;
		while (iPos < iLength && pText[iPos] != '\n')
			{
			const int iCandidateEnd = iPos + 1;
			if (MeasureText(CString(pText + iStart, iCandidateEnd - iStart, true)) > cxLine)
				break;

			iEnd = iCandidateEnd;
			if (pText[iPos] == ' ' || pText[iPos] == '-' || (BYTE)pText[iPos] == 0x97)
				iLastBreak = iCandidateEnd;
			iPos++;
			}

		if (iPos == iLength || pText[iPos] == '\n')
			iEnd = iPos;
		else if (iEnd == iStart)
			iEnd = iStart + 1;
		else if (iLastBreak > iStart)
			iEnd = iLastBreak;

		int iContentEnd = iEnd;
		while (iContentEnd > iStart && pText[iContentEnd - 1] == ' ')
			iContentEnd--;

		FormatLine(pText + iStart, iContentEnd - iStart, (dwFlags & SmartQuotes ? &bInSmartQuotes : NULL), retLines);
		iLines++;

		if ((dwFlags & TruncateLine) && iEnd < iLength)
			{
			bTruncated = (pText[iEnd] != '\n');
			break;
			}

		iStart = iEnd;
		while (iStart < iLength && pText[iStart] == ' ')
			iStart++;
		if (iStart < iLength && pText[iStart] == '\n')
			{
			iStart++;
			bInSmartQuotes = false;
			}
		}

	if (bTruncated && retLines && retLines->GetCount() > 0)
		retLines->GetAt(0).Append(STR_ELLIPSIS);

	return iLines;
	}

int CG16bitFont::CalcHeight(const CString &sText, int cxWidth, DWORD dwFlags) const
	{
	return BreakText(sText, cxWidth, NULL, dwFlags) * m_cyHeight;
	}

void CG16bitFont::DrawText(CG16bitImage &Dest, int x, int y, WORD wColor, DWORD byOpacity, const CString &sText, DWORD dwFlags, int *retx) const
	{
	const int cxText = MeasureText(sText, NULL, true);
	int xPos = x;
	if (dwFlags & AlignCenter)
		xPos -= cxText / 2;
	else if (dwFlags & AlignRight)
		xPos -= cxText;

	if (dwFlags & AdjustToFit)
		{
		const RECT &rcClip = Dest.GetClipRect();
		xPos = Min(Max((int)rcClip.left, xPos), Max((int)rcClip.left, (int)rcClip.right - cxText));
		}

	const bool bAtlas = !m_FontImage.IsEmpty() && m_Metrics.GetCount() >= g_iCharCount;
	const int iScale = GetFallbackScale(m_cyHeight);
	char *pPos = sText.GetASCIIZPointer();
	char *pEnd = pPos + sText.GetLength();
	while (pPos < pEnd)
		{
		if (bAtlas)
			{
			const int iIndex = Min(g_iCharCount - 1, Max(0, (int)(BYTE)*pPos - g_iStartChar));
			const CharMetrics &Metrics = m_Metrics[iIndex];
			if (!(dwFlags & MeasureOnly))
				Dest.FillMask(0, iIndex * m_cyHeight, Metrics.cxWidth, m_cyHeight, m_FontImage, wColor, xPos, y, (BYTE)byOpacity);
			xPos += Metrics.cxAdvance;
			}
		else
			{
			if (!(dwFlags & MeasureOnly))
				DrawFallbackGlyph(Dest, xPos, y, wColor, *pPos, iScale, m_bBold);
			xPos += m_cxAveWidth;
			}
		pPos++;
		}

	if (retx)
		*retx = xPos;
	}

void CG16bitFont::DrawText(CG16bitImage &Dest, const RECT &rcRect, WORD wColor, DWORD byOpacity, const CString &sText, int iLineAdj, DWORD dwFlags, int *retcyHeight) const
	{
	TArray<CString> Lines;
	BreakText(sText, RectWidth(rcRect), &Lines, dwFlags);
	int iLines = Lines.GetCount();
	int cyHeight = (iLines > 0 ? iLines * m_cyHeight + ((iLines - 1) * iLineAdj) : 0);
	while ((dwFlags & TruncateBlock) && iLines > 1 && cyHeight > RectHeight(rcRect))
		{
		iLines--;
		cyHeight -= m_cyHeight + iLineAdj;
		}

	if ((dwFlags & TruncateBlock) && iLines > 0 && iLines < Lines.GetCount())
		{
		CString &sLast = Lines[iLines - 1];
		while (!sLast.IsBlank() && MeasureText(sLast) + MeasureText(STR_ELLIPSIS) > RectWidth(rcRect))
			sLast = strSubString(sLast, 0, sLast.GetLength() - 1);
		sLast.Append(STR_ELLIPSIS);
		}

	int y = (dwFlags & AlignMiddle ? rcRect.top + (RectHeight(rcRect) - cyHeight) / 2 : rcRect.top);
	const int yStart = y;
	for (int i = 0; i < iLines; i++)
		{
		int x = rcRect.left;
		if (dwFlags & AlignCenter)
			x += (RectWidth(rcRect) - MeasureText(Lines[i])) / 2;
		else if (dwFlags & AlignRight)
			x = rcRect.right - MeasureText(Lines[i]);
		DrawText(Dest, x, y, wColor, byOpacity, Lines[i], dwFlags & (MeasureOnly | AdjustToFit));
		y += m_cyHeight + iLineAdj;
		}

	if (retcyHeight)
		*retcyHeight = y - yStart;
	}

void CG16bitFont::DrawText(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const CString &sText, DWORD dwFlags, int *retx) const
	{
	const int cxText = MeasureText(sText, NULL, true);
	int xPos = x;
	if (dwFlags & AlignCenter)
		xPos -= cxText / 2;
	else if (dwFlags & AlignRight)
		xPos -= cxText;

	if (dwFlags & AdjustToFit)
		{
		const RECT &rcClip = Dest.GetClipRect();
		xPos = Min(Max((int)rcClip.left, xPos), Max((int)rcClip.left, (int)rcClip.right - cxText));
		}

	const bool bAtlas = !m_FontImage.IsEmpty() && m_Metrics.GetCount() >= g_iCharCount;
	const int iScale = GetFallbackScale(m_cyHeight);
	char *pPos = sText.GetASCIIZPointer();
	char *pEnd = pPos + sText.GetLength();
	while (pPos < pEnd)
		{
		if (bAtlas)
			{
			const int iIndex = Min(g_iCharCount - 1, Max(0, (int)(BYTE)*pPos - g_iStartChar));
			const CharMetrics &Metrics = m_Metrics[iIndex];
			if (!(dwFlags & MeasureOnly))
				Dest.FillMask(0, iIndex * m_cyHeight, Metrics.cxWidth, m_cyHeight, m_FontImage, rgbColor, xPos, y);
			xPos += Metrics.cxAdvance;
			}
		else
			{
			if (!(dwFlags & MeasureOnly))
				DrawFallbackGlyph(Dest, xPos, y, rgbColor, *pPos, iScale, m_bBold);
			xPos += m_cxAveWidth;
			}
		pPos++;
		}

	if (retx)
		*retx = xPos;
	}

void CG16bitFont::DrawText(CG32bitImage &Dest, const RECT &rcRect, CG32bitPixel rgbColor, const CString &sText, int iLineAdj, DWORD dwFlags, int *retcyHeight) const
	{
	TArray<CString> Lines;
	BreakText(sText, RectWidth(rcRect), &Lines, dwFlags);
	int iLines = Lines.GetCount();
	int cyHeight = (iLines > 0 ? iLines * m_cyHeight + ((iLines - 1) * iLineAdj) : 0);
	while ((dwFlags & TruncateBlock) && iLines > 1 && cyHeight > RectHeight(rcRect))
		{
		iLines--;
		cyHeight -= m_cyHeight + iLineAdj;
		}

	if ((dwFlags & TruncateBlock) && iLines > 0 && iLines < Lines.GetCount())
		{
		CString &sLast = Lines[iLines - 1];
		while (!sLast.IsBlank() && MeasureText(sLast) + MeasureText(STR_ELLIPSIS) > RectWidth(rcRect))
			sLast = strSubString(sLast, 0, sLast.GetLength() - 1);
		sLast.Append(STR_ELLIPSIS);
		}

	int y = (dwFlags & AlignMiddle ? rcRect.top + (RectHeight(rcRect) - cyHeight) / 2 : rcRect.top);
	const int yStart = y;
	for (int i = 0; i < iLines; i++)
		{
		int x = rcRect.left;
		if (dwFlags & AlignCenter)
			x += (RectWidth(rcRect) - MeasureText(Lines[i])) / 2;
		else if (dwFlags & AlignRight)
			x = rcRect.right - MeasureText(Lines[i]);
		DrawText(Dest, x, y, rgbColor, Lines[i], dwFlags & (MeasureOnly | AdjustToFit));
		y += m_cyHeight + iLineAdj;
		}

	if (retcyHeight)
		*retcyHeight = y - yStart;
	}

void CG16bitFont::DrawText(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const TArray<CString> &Lines, int iLineAdj, DWORD dwFlags, int *rety) const
	{
	for (int i = 0; i < Lines.GetCount(); i++)
		{
		DrawText(Dest, x, y, rgbColor, Lines[i], dwFlags);
		y += m_cyHeight + iLineAdj;
		}
	if (rety)
		*rety = y;
	}

void CG16bitFont::DrawTextEffect(CG16bitImage &Dest, int x, int y, WORD wColor, const CString &sText, int iEffectsCount, const SEffectDesc *pEffects, DWORD dwFlags, int *retx) const
	{
	for (int i = 0; i < iEffectsCount; i++)
		if (pEffects[i].iType == effectShadow)
			DrawText(Dest, x + Max(1, m_cyHeight / 16), y + Max(1, m_cyHeight / 16), CG16bitImage::RGBValue(0, 0, 0), sText, dwFlags);
	DrawText(Dest, x, y, wColor, sText, dwFlags, retx);
	}

void CG16bitFont::DrawTextEffect(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const CString &sText, int iEffectsCount, const SEffectDesc *pEffects, DWORD dwFlags, int *retx) const
	{
	for (int i = 0; i < iEffectsCount; i++)
		if (pEffects[i].iType == effectShadow)
			DrawText(Dest, x + Max(1, m_cyHeight / 16), y + Max(1, m_cyHeight / 16), CG32bitPixel(0, 0, 0), sText, dwFlags);
	DrawText(Dest, x, y, rgbColor, sText, dwFlags, retx);
	}

const CG16bitImage &CG16bitFont::GetCharacterImage(char chChar, int *retx, int *rety, int *retcxWidth, int *retcyHeight, int *retcxAdvance) const
	{
	static CG16bitImage Empty;
	const bool bAtlas = !m_FontImage.IsEmpty() && m_Metrics.GetCount() >= g_iCharCount;
	const int iIndex = Min(g_iCharCount - 1, Max(0, (int)(BYTE)chChar - g_iStartChar));
	const int cxWidth = (bAtlas ? m_Metrics[iIndex].cxWidth : m_cxAveWidth);
	const int cxAdvance = (bAtlas ? m_Metrics[iIndex].cxAdvance : m_cxAveWidth);

	if (retx) *retx = 0;
	if (rety) *rety = (bAtlas ? iIndex * m_cyHeight : 0);
	if (retcxWidth) *retcxWidth = cxWidth;
	if (retcyHeight) *retcyHeight = m_cyHeight;
	if (retcxAdvance) *retcxAdvance = cxAdvance;
	return (bAtlas ? m_FontImage : Empty);
	}

const CG16bitFont::CharMetrics &CG16bitFont::GetCharMetrics(char chChar) const
	{
	if (m_Metrics.GetCount() >= g_iCharCount)
		return m_Metrics[Min(g_iCharCount - 1, Max(0, (int)(BYTE)chChar - g_iStartChar))];

	static CharMetrics Fallback;
	Fallback.cxWidth = m_cxAveWidth;
	Fallback.cxAdvance = m_cxAveWidth;
	return Fallback;
	}

int CG16bitFont::MeasureText(const CString &sText, int *retcyHeight, bool bAlwaysAdvance) const
	{
	if (retcyHeight)
		*retcyHeight = m_cyHeight;

	if (m_FontImage.IsEmpty() || m_Metrics.GetCount() < g_iCharCount)
		return sText.GetLength() * m_cxAveWidth;

	int cxWidth = 0;
	char *pPos = sText.GetASCIIZPointer();
	char *pEndPos = pPos + sText.GetLength();
	while (pPos != pEndPos)
		{
		const int iIndex = Min(g_iCharCount - 1, Max(0, (int)(BYTE)*pPos - g_iStartChar));
		const CharMetrics &Metrics = m_Metrics[iIndex];
		pPos++;
		cxWidth += (pPos == pEndPos && iIndex != 0 && !bAlwaysAdvance ? Metrics.cxWidth : Metrics.cxAdvance);
		}
	return cxWidth;
	}

bool CG16bitFont::ParseFontDesc(const CString &sDesc, CString *retsTypeface, int *retiSize, bool *retbBold, bool *retbItalic)
	{
	const char *pPos = sDesc.GetASCIIZPointer();
	while (*pPos == ' ' || *pPos == '\t') pPos++;

	const bool bQuoted = (*pPos == '\'' || *pPos == '"');
	const char chQuote = (bQuoted ? *pPos++ : '\0');
	const char *pStart = pPos;
	while (*pPos != '\0' && (bQuoted ? *pPos != chQuote : *pPos != ' ' && *pPos != '\t')) pPos++;
	if (pPos == pStart)
		return false;
	CString sTypeface(pStart, (int)(pPos - pStart));
	if (bQuoted && *pPos == chQuote) pPos++;

	while (*pPos != '\0' && (*pPos < '0' || *pPos > '9')) pPos++;
	if (*pPos == '\0')
		return false;
	int iSize = strParseInt(pPos, -1, &pPos);
	if (iSize < 0)
		return false;

	bool bBold = false;
	bool bItalic = false;
	while (*pPos != '\0')
		{
		while (*pPos == ' ' || *pPos == '\t') pPos++;
		pStart = pPos;
		while (*pPos != '\0' && *pPos != ' ' && *pPos != '\t') pPos++;
		CString sStyle(pStart, (int)(pPos - pStart));
		if (strEquals(sStyle, CONSTLIT("bold"))) bBold = true;
		else if (strEquals(sStyle, CONSTLIT("italic"))) bItalic = true;
		}

	if (retsTypeface) *retsTypeface = sTypeface;
	if (retiSize) *retiSize = iSize;
	if (retbBold) *retbBold = bBold;
	if (retbItalic) *retbItalic = bItalic;
	return true;
	}
ALERROR CG16bitFont::ReadFromStream(IReadStream *pStream) { ALERROR error; DWORD dwVersion; DWORD dwLoad; pStream->Read((char *)&dwVersion, sizeof(DWORD)); if (dwVersion > FONT_SAVE_VERSION) return ERR_FAIL; m_sTypeface.ReadFromStream(pStream); pStream->Read((char *)&m_cyHeight, sizeof(DWORD)); pStream->Read((char *)&m_cyAscent, sizeof(DWORD)); pStream->Read((char *)&m_cxAveWidth, sizeof(DWORD)); pStream->Read((char *)&dwLoad, sizeof(DWORD)); pStream->Read((char *)&dwLoad, sizeof(DWORD)); m_Metrics.DeleteAll(); m_Metrics.InsertEmpty(dwLoad); for (int i = 0; i < m_Metrics.GetCount(); i++) { CharMetrics *pMetrics = &m_Metrics[i]; pStream->Read((char *)&pMetrics->cxWidth, sizeof(DWORD)); pStream->Read((char *)&pMetrics->cxAdvance, sizeof(DWORD)); } if ((error = m_FontImage.ReadFromStream(pStream))) return error; return NOERROR; }
void CG16bitFont::WriteToStream(IWriteStream *pStream)
	{
	DWORD dwSave = FONT_SAVE_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_sTypeface.WriteToStream(pStream);
	pStream->Write((char *)&m_cyHeight, sizeof(DWORD));
	pStream->Write((char *)&m_cyAscent, sizeof(DWORD));
	pStream->Write((char *)&m_cxAveWidth, sizeof(DWORD));
	dwSave = g_iStartChar;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = m_Metrics.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	for (int i = 0; i < m_Metrics.GetCount(); i++)
		{
		pStream->Write((char *)&m_Metrics[i].cxWidth, sizeof(DWORD));
		pStream->Write((char *)&m_Metrics[i].cxAdvance, sizeof(DWORD));
		}
	m_FontImage.WriteToStream(pStream);
	}

const CG16bitFont &CG16bitFont::GetDefault(void)
	{
	if (m_DefaultFont.GetTypeface().IsBlank())
		m_DefaultFont.Create(CONSTLIT("Fallback"), -16);
	return m_DefaultFont;
	}

CG16bitFont CG16bitFont::m_DefaultFont;

void FormatLine(char *pPos, int iLen, bool *ioInSmartQuotes, TArray<CString> *retLines)
	{
	if (retLines == NULL)
		return;

	if (ioInSmartQuotes)
		{
		retLines->Insert(NULL_STR);
		CString &sLine = retLines->GetAt(retLines->GetCount() - 1);
		char *pEnd = pPos + iLen;
		char *pStart = pPos;
		while (pPos < pEnd)
			{
			if (*pPos == '"')
				{
				if (pStart != pPos)
					sLine.Append(CString(pStart, (int)(pPos - pStart), true));
				sLine.Append(*ioInSmartQuotes ? CSTR_RIGHT_DOUBLE_QUOTE : CSTR_LEFT_DOUBLE_QUOTE);
				*ioInSmartQuotes = !*ioInSmartQuotes;
				pStart = ++pPos;
				}
			else
				pPos++;
			}
		if (pStart != pPos)
			sLine.Append(CString(pStart, (int)(pPos - pStart), true));
		}
	else
		retLines->Insert(CString(pPos, iLen));
	}
