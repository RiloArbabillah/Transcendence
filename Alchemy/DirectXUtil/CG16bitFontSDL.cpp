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
CG16bitFont::CG16bitFont(const CG16bitFont &Src) : m_cyHeight(Src.m_cyHeight), m_cyAscent(Src.m_cyAscent), m_cxAveWidth(Src.m_cxAveWidth), m_sTypeface(Src.m_sTypeface), m_bBold(Src.m_bBold), m_bItalic(Src.m_bItalic), m_bUnderline(Src.m_bUnderline) { }
CG16bitFont &CG16bitFont::operator=(const CG16bitFont &Src) { if (this != &Src) { m_cyHeight = Src.m_cyHeight; m_cyAscent = Src.m_cyAscent; m_cxAveWidth = Src.m_cxAveWidth; m_sTypeface = Src.m_sTypeface; m_bBold = Src.m_bBold; m_bItalic = Src.m_bItalic; m_bUnderline = Src.m_bUnderline; } return *this; }

ALERROR CG16bitFont::Create(const CString &sTypeface, int iSize, bool bBold, bool bItalic, bool bUnderline) { m_sTypeface = sTypeface; m_cyHeight = Max(7, Absolute(iSize)); m_cyAscent = m_cyHeight - Max(1, m_cyHeight / 5); m_cxAveWidth = (GetFallbackScale(m_cyHeight) * 6) + (bBold ? 1 : 0); m_bBold = bBold; m_bItalic = bItalic; m_bUnderline = bUnderline; return NOERROR; }
ALERROR CG16bitFont::CreateFromFile(const CString &sFilespec) { return Create(CONSTLIT("Fallback"), -16); }
ALERROR CG16bitFont::CreateFromFont(HFONT hFont) { return NOERROR; }
ALERROR CG16bitFont::CreateFromResource(HINSTANCE hInst, const char *pszRes) { return Create(CONSTLIT("Fallback"), -16); }

int CG16bitFont::BreakText(const CString &sText, int cxWidth, TArray<CString> *retLines, DWORD dwFlags) const { if (retLines) retLines->DeleteAll(); if (retLines) retLines->Insert(sText); return 1; }
int CG16bitFont::CalcHeight(const CString &sText, int cxWidth, DWORD dwFlags) const { return m_cyHeight; }

void CG16bitFont::DrawText(CG16bitImage &Dest, int x, int y, WORD wColor, DWORD byOpacity, const CString &sText, DWORD dwFlags, int *retx) const { int xPos = x; if (dwFlags & AlignCenter) xPos -= MeasureText(sText) / 2; else if (dwFlags & AlignRight) xPos -= MeasureText(sText); if (!(dwFlags & MeasureOnly)) { int iScale = GetFallbackScale(m_cyHeight); for (char *pPos = sText.GetASCIIZPointer(); *pPos != '\0'; pPos++) { DrawFallbackGlyph(Dest, xPos, y, wColor, *pPos, iScale, m_bBold); xPos += m_cxAveWidth; } } else xPos += MeasureText(sText); if (retx) *retx = xPos; }
void CG16bitFont::DrawText(CG16bitImage &Dest, const RECT &rcRect, WORD wColor, DWORD byOpacity, const CString &sText, int iLineAdj, DWORD dwFlags, int *retcyHeight) const { int x = (dwFlags & AlignCenter ? rcRect.left + RectWidth(rcRect) / 2 : (dwFlags & AlignRight ? rcRect.right : rcRect.left)); int y = (dwFlags & AlignMiddle ? rcRect.top + (RectHeight(rcRect) - m_cyHeight) / 2 : rcRect.top); DrawText(Dest, x, y, wColor, byOpacity, sText, dwFlags, NULL); if (retcyHeight) *retcyHeight = m_cyHeight; }
void CG16bitFont::DrawText(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const CString &sText, DWORD dwFlags, int *retx) const { int xPos = x; if (dwFlags & AlignCenter) xPos -= MeasureText(sText) / 2; else if (dwFlags & AlignRight) xPos -= MeasureText(sText); if (!(dwFlags & MeasureOnly)) { if (!m_FontImage.IsEmpty() && m_Metrics.GetCount() >= g_iCharCount) { char *pPos = sText.GetASCIIZPointer(); char *pEndPos = pPos + sText.GetLength(); while (pPos < pEndPos) { int iIndex = (int)(BYTE)(*pPos) - g_iStartChar; iIndex = Max(0, iIndex); const CharMetrics &Metrics = m_Metrics[iIndex]; Dest.FillMask(0, iIndex * m_cyHeight, Metrics.cxWidth, m_cyHeight, m_FontImage, rgbColor, xPos, y); pPos++; xPos += Metrics.cxAdvance; } } else { int iScale = GetFallbackScale(m_cyHeight); for (char *pPos = sText.GetASCIIZPointer(); *pPos != '\0'; pPos++) { DrawFallbackGlyph(Dest, xPos, y, rgbColor, *pPos, iScale, m_bBold); xPos += m_cxAveWidth; } } } else xPos += MeasureText(sText); if (retx) *retx = xPos; }
void CG16bitFont::DrawText(CG32bitImage &Dest, const RECT &rcRect, CG32bitPixel rgbColor, const CString &sText, int iLineAdj, DWORD dwFlags, int *retcyHeight) const { int x = (dwFlags & AlignCenter ? rcRect.left + RectWidth(rcRect) / 2 : (dwFlags & AlignRight ? rcRect.right : rcRect.left)); int y = (dwFlags & AlignMiddle ? rcRect.top + (RectHeight(rcRect) - m_cyHeight) / 2 : rcRect.top); DrawText(Dest, x, y, rgbColor, sText, dwFlags, NULL); if (retcyHeight) *retcyHeight = m_cyHeight; }
void CG16bitFont::DrawText(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const TArray<CString> &Lines, int iLineAdj, DWORD dwFlags, int *rety) const { for (int i = 0; i < Lines.GetCount(); i++) { DrawText(Dest, x, y, rgbColor, Lines[i], dwFlags); y += m_cyHeight + iLineAdj; } if (rety) *rety = y; }

void CG16bitFont::DrawTextEffect(CG16bitImage &Dest, int x, int y, WORD wColor, const CString &sText, int iEffectsCount, const SEffectDesc *pEffects, DWORD dwFlags, int *retx) const { }
void CG16bitFont::DrawTextEffect(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const CString &sText, int iEffectsCount, const SEffectDesc *pEffects, DWORD dwFlags, int *retx) const { }

const CG16bitImage &CG16bitFont::GetCharacterImage(char chChar, int *retx, int *rety, int *retcxWidth, int *retcyHeight, int *retcxAdvance) const { static CG16bitImage empty; if (retx) *retx = 0; if (rety) *rety = 0; if (retcxWidth) *retcxWidth = m_cxAveWidth; if (retcyHeight) *retcyHeight = m_cyHeight; if (retcxAdvance) *retcxAdvance = m_cxAveWidth; return empty; }
int CG16bitFont::MeasureText(const CString &sText, int *retcyHeight, bool bAlwaysAdvance) const { if (retcyHeight) *retcyHeight = m_cyHeight; if (m_FontImage.IsEmpty() || m_Metrics.GetCount() < g_iCharCount) return sText.GetLength() * m_cxAveWidth; int cxWidth = 0; char *pPos = sText.GetASCIIZPointer(); char *pEndPos = pPos + sText.GetLength(); while (pPos != pEndPos) { int iIndex = (int)(BYTE)(*pPos) - g_iStartChar; iIndex = Max(0, iIndex); const CharMetrics &Metrics = m_Metrics[iIndex]; pPos++; if (pPos == pEndPos && iIndex != 0 && !bAlwaysAdvance) cxWidth += Metrics.cxWidth; else cxWidth += Metrics.cxAdvance; } return cxWidth; }
bool CG16bitFont::ParseFontDesc(const CString &sDesc, CString *retsTypeface, int *retiSize, bool *retbBold, bool *retbItalic) { return false; }
ALERROR CG16bitFont::ReadFromStream(IReadStream *pStream) { ALERROR error; DWORD dwVersion; DWORD dwLoad; pStream->Read((char *)&dwVersion, sizeof(DWORD)); if (dwVersion > FONT_SAVE_VERSION) return ERR_FAIL; m_sTypeface.ReadFromStream(pStream); pStream->Read((char *)&m_cyHeight, sizeof(DWORD)); pStream->Read((char *)&m_cyAscent, sizeof(DWORD)); pStream->Read((char *)&m_cxAveWidth, sizeof(DWORD)); pStream->Read((char *)&dwLoad, sizeof(DWORD)); pStream->Read((char *)&dwLoad, sizeof(DWORD)); m_Metrics.DeleteAll(); m_Metrics.InsertEmpty(dwLoad); for (int i = 0; i < m_Metrics.GetCount(); i++) { CharMetrics *pMetrics = &m_Metrics[i]; pStream->Read((char *)&pMetrics->cxWidth, sizeof(DWORD)); pStream->Read((char *)&pMetrics->cxAdvance, sizeof(DWORD)); } if ((error = m_FontImage.ReadFromStream(pStream))) return error; return NOERROR; }
void CG16bitFont::WriteToStream(IWriteStream *pStream) { }

const CG16bitFont &CG16bitFont::GetDefault(void) { return m_DefaultFont; }

CG16bitFont CG16bitFont::m_DefaultFont;

void FormatLine(char *pPos, int iLen, bool *ioInSmartQuotes, TArray<CString> *retLines) { if (retLines) retLines->Insert(CString(pPos, iLen)); }
