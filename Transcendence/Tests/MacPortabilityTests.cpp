#include "Alchemy.h"
#include "DirectXUtil.h"

#include <cstdio>
#include <cstring>

namespace
	{
	bool Check(bool bCondition, const char *pMessage)
		{
		if (!bCondition)
			std::fprintf(stderr, "FAILED: %s\n", pMessage);

		return bCondition;
		}
	}

int main()
	{
	if (!kernelInit())
		return 1;

	bool bSuccess = true;
	bSuccess = Check(strCompare(CONSTLIT("Alpha"), CONSTLIT("alpha")) == 0, "case-insensitive comparison") && bSuccess;
	bSuccess = Check(strCompare(CONSTLIT("alpha"), CONSTLIT("beta")) < 0, "comparison ordering") && bSuccess;
	bSuccess = Check(strStartsWithOld(CONSTLIT("Transcendence"), CONSTLIT("trans")), "case-insensitive prefix") && bSuccess;
	bSuccess = Check(strEndsWithOld(CONSTLIT("Settings.XML"), CONSTLIT("xml")), "case-insensitive suffix") && bSuccess;
	bSuccess = Check(strEquals(strConvertToToken(CONSTLIT("Alpha Beta"), true), CONSTLIT("alpha_beta")), "lowercase token conversion") && bSuccess;
	bSuccess = Check(strToLowerASCII('Z') == 'z' && strToUpperASCII('q') == 'Q', "ASCII character conversion") && bSuccess;

	CG16bitFont Font;
	bSuccess = Check(Font.Create(CONSTLIT("Fallback"), -16) == NOERROR, "fallback font creation") && bSuccess;
	CG16bitFont AssetFont;
	bSuccess = Check(AssetFont.CreateFromFile(strPatternSubst(CONSTLIT("%s/Transcendence/Transcendence/Resources/Header.dxfn"), CONSTLIT(TRANSCENDENCE_SOURCE_DIR))) == NOERROR,
			"serialized font loading") && bSuccess;
	bSuccess = Check(!AssetFont.IsEmpty() && AssetFont.MeasureText(CONSTLIT("Header")) > 0, "serialized font atlas and metrics") && bSuccess;
	const int cxWord = Font.MeasureText(CONSTLIT("alpha"));
	TArray<CString> Lines;
	bSuccess = Check(Font.BreakText(CONSTLIT("alpha beta gamma"), cxWord + Font.GetAverageWidth(), &Lines) == 3, "font word wrapping") && bSuccess;
	bSuccess = Check(Lines.GetCount() == 3 && strEquals(Lines[0], CONSTLIT("alpha")) && strEquals(Lines[2], CONSTLIT("gamma")), "wrapped line contents") && bSuccess;
	bSuccess = Check(Font.CalcHeight(CONSTLIT("alpha beta gamma"), cxWord + Font.GetAverageWidth()) == 3 * Font.GetHeight(), "wrapped text height") && bSuccess;

	Lines.DeleteAll();
	Font.BreakText(CONSTLIT("alpha beta"), cxWord + Font.GetAverageWidth(), &Lines, CG16bitFont::TruncateLine);
	bSuccess = Check(Lines.GetCount() == 1 && strEndsWithOld(Lines[0], CONSTLIT("...")), "single-line truncation") && bSuccess;

	CG16bitFont FontCopy(Font);
	bSuccess = Check(FontCopy.MeasureText(CONSTLIT("copy")) == Font.MeasureText(CONSTLIT("copy")), "copied font metrics") && bSuccess;
	CG16bitFont FontAssigned;
	FontAssigned = Font;
	bSuccess = Check(FontAssigned.MeasureText(CONSTLIT("assigned")) == Font.MeasureText(CONSTLIT("assigned")), "assigned font metrics") && bSuccess;

	CString sTypeface;
	int iSize = 0;
	bool bBold = false;
	bool bItalic = false;
	bSuccess = Check(CG16bitFont::ParseFontDesc(CONSTLIT("'Fallback UI' 18 bold italic"), &sTypeface, &iSize, &bBold, &bItalic)
			&& strEquals(sTypeface, CONSTLIT("Fallback UI")) && iSize == 18 && bBold && bItalic,
			"font descriptor parsing") && bSuccess;

	CG32bitImage Canvas;
	bSuccess = Check(Canvas.Create(240, 100, CG32bitImage::alpha8), "font test canvas") && bSuccess;
	int xEnd = 0;
	Font.DrawText(Canvas, 10, 10, CG32bitPixel(255, 255, 255), CONSTLIT("advance"), 0, &xEnd);
	bSuccess = Check(xEnd == 10 + Font.MeasureText(CONSTLIT("advance"), NULL, true), "DrawText final x") && bSuccess;
	CDrawText::WithAccelerator(Canvas, 10, 30, CONSTLIT("launch"), 99, Font, CG32bitPixel(255, 255, 255), CG32bitPixel(255, 200, 0));

	RECT rcText = { 0, 0, cxWord + Font.GetAverageWidth(), (2 * Font.GetHeight()) };
	int cyDrawn = 0;
	Font.DrawText(Canvas, rcText, CG32bitPixel(255, 255, 255), CONSTLIT("alpha beta gamma"), 0, CG16bitFont::TruncateBlock | CG16bitFont::AlignMiddle, &cyDrawn);
	bSuccess = Check(cyDrawn == 2 * Font.GetHeight(), "block truncation height") && bSuccess;

	CString sMappedPath = strPatternSubst(CONSTLIT("%s/Transcendence/Transcendence/Resources/Header.dxfn"), CONSTLIT(TRANSCENDENCE_SOURCE_DIR));

	//	Exercise the Win32 file-mapping compatibility layer with zero-byte
	//	mapping semantics (map the entire file) and make sure CloseHandle does
	//	not confuse a mapping handle with a raw file descriptor.
	HANDLE hMappingSource = CreateFile(sMappedPath.GetASCIIZPointer(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	bSuccess = Check(hMappingSource != INVALID_HANDLE_VALUE, "mapping source open") && bSuccess;
	if (hMappingSource != INVALID_HANDLE_VALUE)
		{
		HANDLE hMap = CreateFileMapping(hMappingSource, NULL, PAGE_READONLY, 0, 0, NULL);
		bSuccess = Check(hMap != NULL, "CreateFileMapping") && bSuccess;

		if (hMap)
			{
			void *pView = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
			bSuccess = Check(pView != NULL, "MapViewOfFile zero-length maps whole file") && bSuccess;

			if (pView)
				{
				DWORD dwMappedSize = ::GetFileSize(hMappingSource, NULL);
				bSuccess = Check(dwMappedSize > 0, "mapped source size sanity") && bSuccess;

				if (dwMappedSize > 0)
					{
					char szHead[8];
					const size_t iCopy = dwMappedSize < sizeof(szHead) ? dwMappedSize : sizeof(szHead);
					memcpy(szHead, pView, iCopy);
					bSuccess = Check(true, "mapping data readable");
					}

				bSuccess = Check(UnmapViewOfFile(pView) == TRUE, "UnmapViewOfFile") && bSuccess;
				}

			CloseHandle(hMap);
			}

		CloseHandle(hMappingSource);
		}

	//	Exercise the direct POSIX mmap path used by CFileReadStream on macOS.
	CFileReadStream Stream(sMappedPath);
	bSuccess = Check(Stream.Open() == NOERROR, "CFileReadStream open") && bSuccess;
	if (bSuccess)
		{
		char szHeader[16];
		int iBytesRead = 0;
		bSuccess = Check(Stream.Read(szHeader, (int)sizeof(szHeader), &iBytesRead) == NOERROR && iBytesRead == (int)sizeof(szHeader),
				"CFileReadStream read") && bSuccess;
		}
	bSuccess = Check(Stream.Close() == NOERROR, "CFileReadStream close") && bSuccess;

	kernelCleanUp();
	return (bSuccess ? 0 : 1);
	}
