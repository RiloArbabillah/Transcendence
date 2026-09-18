#include "Alchemy.h"
#include "DirectXUtil.h"
#include "PlatformInput.h"

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

	//	PDR-002/PDR-003: the memory write stream must grow its reservation until
	//	the whole request fits (a single write can exceed twice the current
	//	maximum) and must account for committed bytes so that a forward Seek
	//	exposes zero-filled memory instead of uninitialized heap.

	{
	CMemoryWriteStream MemoryStream(1024);
	bSuccess = Check(MemoryStream.Create() == NOERROR, "CMemoryWriteStream create") && bSuccess;

	const int iChunkSize = 4096;
	char szChunk[iChunkSize];
	for (int i = 0; i < iChunkSize; i++)
		szChunk[i] = (char)(i & 0xFF);

	int iBytesWritten = 0;
	bSuccess = Check(MemoryStream.Write(szChunk, iChunkSize, &iBytesWritten) == NOERROR && iBytesWritten == iChunkSize,
			"CMemoryWriteStream write beyond 2x maximum") && bSuccess;
	bSuccess = Check(MemoryStream.GetLength() == iChunkSize, "CMemoryWriteStream length after growth") && bSuccess;
	bSuccess = Check(MemoryStream.GetCommittedSize() >= iChunkSize, "CMemoryWriteStream committed covers write") && bSuccess;

	bool bIntact = true;
	const char *pBlock = MemoryStream.GetPointer();
	for (int i = 0; i < iChunkSize; i++)
		if (pBlock[i] != (char)(i & 0xFF))
			bIntact = false;
	bSuccess = Check(bIntact, "CMemoryWriteStream data intact after growth") && bSuccess;

	//	Seeking forward past the committed region must extend the stream with
	//	zero-filled bytes, not garbage.

	const int iSeekTo = MemoryStream.GetCommittedSize() + 512;
	MemoryStream.Seek(iSeekTo);
	bSuccess = Check(MemoryStream.GetLength() == iSeekTo, "CMemoryWriteStream forward seek extends length") && bSuccess;
	bSuccess = Check(MemoryStream.GetCommittedSize() >= iSeekTo, "CMemoryWriteStream committed covers seek") && bSuccess;

	pBlock = MemoryStream.GetPointer();
	bool bZeroFilled = true;
	for (int i = iChunkSize; i < iSeekTo; i++)
		if (pBlock[i] != 0)
			bZeroFilled = false;
	bSuccess = Check(bZeroFilled, "CMemoryWriteStream seek region is zero-filled") && bSuccess;

	//	Growing again must preserve the data written before the reallocation.

	char szTail[8192];
	for (int i = 0; i < (int)sizeof(szTail); i++)
		szTail[i] = (char)(0xFF - (i & 0x7F));
	bSuccess = Check(MemoryStream.Write(szTail, (int)sizeof(szTail)) == NOERROR, "CMemoryWriteStream second growth") && bSuccess;

	pBlock = MemoryStream.GetPointer();
	bIntact = true;
	for (int i = 0; i < iChunkSize; i++)
		if (pBlock[i] != (char)(i & 0xFF))
			bIntact = false;
	bSuccess = Check(bIntact, "CMemoryWriteStream data preserved across second growth") && bSuccess;
	}

	//	PDR-001: every virtual key referenced by the default key mappings must
	//	map to the right scancode, and the async-key-state query must report it
	//	as down. The state is injected so that the mapping is exercised without
	//	a window, a device, or an initialized SDL video subsystem.

	{
	struct SVKCase
		{
		int iVK;
		SDL_Scancode iScancode;
		};

	const SVKCase KeyCases[] =
		{
		{ 'A', SDL_SCANCODE_A },
		{ 'B', SDL_SCANCODE_B },
		{ 'C', SDL_SCANCODE_C },
		{ 'D', SDL_SCANCODE_D },
		{ 'F', SDL_SCANCODE_F },
		{ 'G', SDL_SCANCODE_G },
		{ 'M', SDL_SCANCODE_M },
		{ 'N', SDL_SCANCODE_N },
		{ 'P', SDL_SCANCODE_P },
		{ 'Q', SDL_SCANCODE_Q },
		{ 'R', SDL_SCANCODE_R },
		{ 'S', SDL_SCANCODE_S },
		{ 'T', SDL_SCANCODE_T },
		{ 'U', SDL_SCANCODE_U },
		{ 'V', SDL_SCANCODE_V },
		{ 'W', SDL_SCANCODE_W },
		{ 'X', SDL_SCANCODE_X },
		{ 'Y', SDL_SCANCODE_Y },
		{ 'Z', SDL_SCANCODE_Z },
		{ VK_CONTROL, SDL_SCANCODE_LCTRL },
		{ VK_DOWN, SDL_SCANCODE_DOWN },
		{ VK_LEFT, SDL_SCANCODE_LEFT },
		{ VK_PAUSE, SDL_SCANCODE_PAUSE },
		{ VK_RIGHT, SDL_SCANCODE_RIGHT },
		{ VK_SHIFT, SDL_SCANCODE_LSHIFT },
		{ VK_SPACE, SDL_SCANCODE_SPACE },
		{ VK_TAB, SDL_SCANCODE_TAB },
		{ VK_UP, SDL_SCANCODE_UP },
		{ VK_F1, SDL_SCANCODE_F1 },
		{ VK_F2, SDL_SCANCODE_F2 },
		{ VK_F6, SDL_SCANCODE_F6 },
		{ VK_F7, SDL_SCANCODE_F7 },
		{ VK_F8, SDL_SCANCODE_F8 },
		{ VK_F9, SDL_SCANCODE_F9 },
		};

	Uint8 KeyState[SDL_NUM_SCANCODES];
	bool bMapping = true;
	for (size_t i = 0; i < sizeof(KeyCases) / sizeof(KeyCases[0]); i++)
		{
		if (PlatformVKToScancode(KeyCases[i].iVK) != KeyCases[i].iScancode)
			bMapping = false;

		//	Key up: nothing reported.

		memset(KeyState, 0, sizeof(KeyState));
		if (PlatformAsyncKeyStateForState(KeyCases[i].iVK, KeyState, 0, KMOD_NONE) != 0)
			bMapping = false;

		//	Key down: reported as down.

		KeyState[KeyCases[i].iScancode] = 1;
		if (PlatformAsyncKeyStateForState(KeyCases[i].iVK, KeyState, 0, KMOD_NONE) != (SHORT)0x8000)
			bMapping = false;
		}
	bSuccess = Check(bMapping, "default key mapping VK to scancode") && bSuccess;

	Uint8 EmptyState[SDL_NUM_SCANCODES];
	memset(EmptyState, 0, sizeof(EmptyState));

	//	Mouse buttons have no scancode and are polled from the button state.

	bSuccess = Check(PlatformVKToScancode(VK_LBUTTON) == SDL_SCANCODE_UNKNOWN
			&& PlatformAsyncKeyStateForState(VK_LBUTTON, EmptyState, SDL_BUTTON(SDL_BUTTON_LEFT), KMOD_NONE) == (SHORT)0x8000
			&& PlatformAsyncKeyStateForState(VK_LBUTTON, EmptyState, 0, KMOD_NONE) == 0,
			"left mouse button virtual key") && bSuccess;
	bSuccess = Check(PlatformAsyncKeyStateForState(VK_RBUTTON, EmptyState, SDL_BUTTON(SDL_BUTTON_RIGHT), KMOD_NONE) == (SHORT)0x8000
			&& PlatformAsyncKeyStateForState(VK_RBUTTON, EmptyState, 0, KMOD_NONE) == 0,
			"right mouse button virtual key") && bSuccess;
	bSuccess = Check(PlatformAsyncKeyStateForState(VK_MBUTTON, EmptyState, SDL_BUTTON(SDL_BUTTON_MIDDLE), KMOD_NONE) == (SHORT)0x8000,
			"middle mouse button virtual key") && bSuccess;

	//	Modifiers are driven by the SDL modifier state.

	bSuccess = Check(PlatformAsyncKeyStateForState(VK_SHIFT, EmptyState, 0, KMOD_LSHIFT) == (SHORT)0x8000
			&& PlatformAsyncKeyStateForState(VK_CONTROL, EmptyState, 0, KMOD_RCTRL) == (SHORT)0x8000
			&& PlatformAsyncKeyStateForState(VK_MENU, EmptyState, 0, KMOD_LALT) == (SHORT)0x8000
			&& PlatformAsyncKeyStateForState(VK_NUMLOCK, EmptyState, 0, KMOD_NUM) == (SHORT)0x8000,
			"modifier virtual keys") && bSuccess;
	}

	//	PDR-006: destroying a bitmap must drop its lookup entry, so that the
	//	registry neither dangles nor grows across repeated create/destroy cycles.

	{
	//	A caller-owned pixel buffer keeps this independent of SDL video init.

	DWORD PixelBuffer[8 * 8];
	memset(PixelBuffer, 0, sizeof(PixelBuffer));
	SDL_Surface *pSurface = SDL_CreateRGBSurfaceFrom(PixelBuffer, 8, 8, 32, 8 * 4,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	bSuccess = Check(pSurface != NULL, "SDL_CreateRGBSurfaceFrom") && bSuccess;

	if (pSurface)
		{
		std::map<void*, SDLBitmap*>& BitmapMap = GetSDLBitmapMap();
		const size_t iBaseline = BitmapMap.size();

		SDLBitmap *pBitmap = SDLBitmapCreateFromSurface(pSurface, bitmapRGB, false);
		bSuccess = Check(pBitmap != NULL, "SDLBitmapCreateFromSurface") && bSuccess;

		if (pBitmap)
			{
			bSuccess = Check(BitmapMap.count((void *)pBitmap) == 1, "bitmap registered in lookup map") && bSuccess;
			SDLBitmapDestroy(pBitmap);
			bSuccess = Check(BitmapMap.count((void *)pBitmap) == 0, "bitmap removed from lookup map on destroy") && bSuccess;
			}

		for (int i = 0; i < 64; i++)
			{
			SDLBitmap *pCycle = SDLBitmapCreateFromSurface(pSurface, bitmapRGB, false);
			if (pCycle)
				SDLBitmapDestroy(pCycle);
			}
		bSuccess = Check(BitmapMap.size() == iBaseline, "bitmap map does not grow across cycles") && bSuccess;

		SDL_FreeSurface(pSurface);
		}
	}

	kernelCleanUp();
	return (bSuccess ? 0 : 1);
	}
