//	CG16bitFontSDL.cpp
//	Stub font implementation for macOS SDL platform

#include "Alchemy.h"
#include "DirectXUtil.h"

CG16bitFont::CG16bitFont(void) : m_cyHeight(12), m_cyAscent(10), m_cxAveWidth(8), m_bBold(false), m_bItalic(false), m_bUnderline(false) { }
CG16bitFont::CG16bitFont(const CG16bitFont &Src) : m_cyHeight(Src.m_cyHeight), m_cyAscent(Src.m_cyAscent), m_cxAveWidth(Src.m_cxAveWidth), m_sTypeface(Src.m_sTypeface), m_bBold(Src.m_bBold), m_bItalic(Src.m_bItalic), m_bUnderline(Src.m_bUnderline) { }
CG16bitFont &CG16bitFont::operator=(const CG16bitFont &Src) { if (this != &Src) { m_cyHeight = Src.m_cyHeight; m_cyAscent = Src.m_cyAscent; m_cxAveWidth = Src.m_cxAveWidth; m_sTypeface = Src.m_sTypeface; m_bBold = Src.m_bBold; m_bItalic = Src.m_bItalic; m_bUnderline = Src.m_bUnderline; } return *this; }

ALERROR CG16bitFont::Create(const CString &sTypeface, int iSize, bool bBold, bool bItalic, bool bUnderline) { m_sTypeface = sTypeface; m_bBold = bBold; m_bItalic = bItalic; m_bUnderline = bUnderline; return NOERROR; }
ALERROR CG16bitFont::CreateFromFile(const CString &sFilespec) { return NOERROR; }
ALERROR CG16bitFont::CreateFromFont(HFONT hFont) { return NOERROR; }
ALERROR CG16bitFont::CreateFromResource(HINSTANCE hInst, const char *pszRes) { return NOERROR; }

int CG16bitFont::BreakText(const CString &sText, int cxWidth, TArray<CString> *retLines, DWORD dwFlags) const { if (retLines) retLines->DeleteAll(); if (retLines) retLines->Insert(sText); return 1; }
int CG16bitFont::CalcHeight(const CString &sText, int cxWidth, DWORD dwFlags) const { return m_cyHeight; }

void CG16bitFont::DrawText(CG16bitImage &Dest, int x, int y, WORD wColor, DWORD byOpacity, const CString &sText, DWORD dwFlags, int *retx) const { if (retx) *retx = x + MeasureText(sText); }
void CG16bitFont::DrawText(CG16bitImage &Dest, const RECT &rcRect, WORD wColor, DWORD byOpacity, const CString &sText, int iLineAdj, DWORD dwFlags, int *retcyHeight) const { if (retcyHeight) *retcyHeight = m_cyHeight; }
void CG16bitFont::DrawText(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const CString &sText, DWORD dwFlags, int *retx) const { if (retx) *retx = x + MeasureText(sText); }
void CG16bitFont::DrawText(CG32bitImage &Dest, const RECT &rcRect, CG32bitPixel rgbColor, const CString &sText, int iLineAdj, DWORD dwFlags, int *retcyHeight) const { if (retcyHeight) *retcyHeight = m_cyHeight; }
void CG16bitFont::DrawText(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const TArray<CString> &Lines, int iLineAdj, DWORD dwFlags, int *rety) const { if (rety) *rety = y + (int)Lines.GetCount() * m_cyHeight; }

void CG16bitFont::DrawTextEffect(CG16bitImage &Dest, int x, int y, WORD wColor, const CString &sText, int iEffectsCount, const SEffectDesc *pEffects, DWORD dwFlags, int *retx) const { }
void CG16bitFont::DrawTextEffect(CG32bitImage &Dest, int x, int y, CG32bitPixel rgbColor, const CString &sText, int iEffectsCount, const SEffectDesc *pEffects, DWORD dwFlags, int *retx) const { }

const CG16bitImage &CG16bitFont::GetCharacterImage(char chChar, int *retx, int *rety, int *retcxWidth, int *retcyHeight, int *retcxAdvance) const { static CG16bitImage empty; if (retx) *retx = 0; if (rety) *rety = 0; if (retcxWidth) *retcxWidth = m_cxAveWidth; if (retcyHeight) *retcyHeight = m_cyHeight; if (retcxAdvance) *retcxAdvance = m_cxAveWidth; return empty; }
int CG16bitFont::MeasureText(const CString &sText, int *retcyHeight, bool bAlwaysAdvance) const { if (retcyHeight) *retcyHeight = m_cyHeight; return sText.GetLength() * m_cxAveWidth; }
bool CG16bitFont::ParseFontDesc(const CString &sDesc, CString *retsTypeface, int *retiSize, bool *retbBold, bool *retbItalic) { return false; }
ALERROR CG16bitFont::ReadFromStream(IReadStream *pStream) { return NOERROR; }
void CG16bitFont::WriteToStream(IWriteStream *pStream) { }

const CG16bitFont &CG16bitFont::GetDefault(void) { return m_DefaultFont; }

CG16bitFont CG16bitFont::m_DefaultFont;

void FormatLine(char *pPos, int iLen, bool *ioInSmartQuotes, TArray<CString> *retLines) { }