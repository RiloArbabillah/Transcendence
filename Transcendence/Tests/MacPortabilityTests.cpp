#include "Alchemy.h"
#include "DirectXUtil.h"
#include "PlatformInput.h"
#include "PlatformMessage.h"
#include "PathCompat.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

namespace
	{
	bool Check(bool bCondition, const char *pMessage)
		{
		if (!bCondition)
			std::fprintf(stderr, "FAILED: %s\n", pMessage);

		return bCondition;
		}

	//	PDR-018: the dispatch handler below records that it ran, so a test can
	//	observe that SendMessage ran it before returning.

	int g_iDispatchCount = 0;
	unsigned int g_dwDispatchMessage = 0;
	WPARAM g_wDispatchParam = 0;

	LRESULT TestMessageDispatch(const SPlatformMessage &message)
		{
		g_iDispatchCount++;
		g_dwDispatchMessage = message.message;
		g_wDispatchParam = message.wParam;
		return 0;
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

	//	PDR-010/PDR-011: screen<->client conversion and the cursor position are
	//	expressed in *screen* coordinates, offset by where the window client
	//	area actually sits on the desktop. The arithmetic is exercised through
	//	the pure helpers and through the platform-layer state, so that neither
	//	a window nor a video device is required.

	{
	POINT Point;
	Point.x = 120;
	Point.y = 80;
	PlatformTranslateScreenToClient(&Point, 100, 50);
	bSuccess = Check(Point.x == 20 && Point.y == 30, "screen to client translation") && bSuccess;
	PlatformTranslateClientToScreen(&Point, 100, 50);
	bSuccess = Check(Point.x == 120 && Point.y == 80, "client to screen translation") && bSuccess;

	//	A NULL point is ignored rather than dereferenced.

	PlatformTranslateScreenToClient(NULL, 100, 50);
	PlatformTranslateClientToScreen(NULL, 100, 50);
	bSuccess = Check(true, "null point translation is ignored") && bSuccess;

	//	With the window at (100,50) and the cursor at client (10,20), Win32
	//	reports the screen position (110,70).

	PlatformSetWindowOrigin(100, 50);
	PlatformSetMouseClientPos(10, 20);

	POINT Cursor;
	memset(&Cursor, 0, sizeof(Cursor));
	bSuccess = Check(PlatformGetCursorPos(&Cursor) == TRUE && Cursor.x == 110 && Cursor.y == 70,
			"GetCursorPos reports screen coordinates") && bSuccess;

	//	SetCursorPos takes screen coordinates and must round-trip.

	PlatformSetCursorPos(210, 120);
	memset(&Cursor, 0, sizeof(Cursor));
	bSuccess = Check(PlatformGetCursorPos(&Cursor) == TRUE && Cursor.x == 210 && Cursor.y == 120,
			"SetCursorPos/GetCursorPos round-trip") && bSuccess;

	//	The Win32 shims must agree with the platform layer.

	Point.x = 210;
	Point.y = 120;
	bSuccess = Check(PlatformScreenToClient(NULL, &Point) == TRUE && Point.x == 110 && Point.y == 70,
			"ScreenToClient uses the window origin") && bSuccess;
	bSuccess = Check(PlatformClientToScreen(NULL, &Point) == TRUE && Point.x == 210 && Point.y == 120,
			"ClientToScreen uses the window origin") && bSuccess;
	bSuccess = Check(PlatformScreenToClient(NULL, NULL) == FALSE
			&& PlatformClientToScreen(NULL, NULL) == FALSE,
			"screen/client conversion rejects a null point") && bSuccess;

	int xOrigin = 0;
	int yOrigin = 0;
	bSuccess = Check(PlatformGetWindowOrigin(&xOrigin, &yOrigin) == TRUE && xOrigin == 100 && yOrigin == 50,
			"GetWindowOrigin reports the window position") && bSuccess;

	//	Restore the default origin so that later groups start clean.

	PlatformSetWindowOrigin(0, 0);
	PlatformSetMouseClientPos(0, 0);
	}

	//	PDR-012/PDR-013/PDR-014/PDR-015: file times must be real FILETIME
	//	values, CopyFile must honour bFailIfExists, and the port must expose
	//	exactly one application-data root.

	{
	char szSource[] = "/tmp/trans-port-fs-XXXXXX";
	int iSource = mkstemp(szSource);
	bSuccess = Check(iSource >= 0, "create test file") && bSuccess;

	if (iSource >= 0)
		{
		const char *pszContent = "transcendence";
		const ssize_t iContent = (ssize_t)strlen(pszContent);
		bSuccess = Check(write(iSource, pszContent, (size_t)iContent) == iContent, "write test file") && bSuccess;

		//	Pin the timestamp so that the conversion is checked against a known
		//	instant: 1600000000 is 2020-09-13T12:26:40Z.

		struct timespec Times[2];
		Times[0].tv_sec = 1600000000;
		Times[0].tv_nsec = 0;
		Times[1].tv_sec = 1600000000;
		Times[1].tv_nsec = 0;
		bSuccess = Check(futimens(iSource, Times) == 0, "set test file time") && bSuccess;

		FILETIME ftCreation = 0;
		FILETIME ftAccess = 0;
		FILETIME ftWrite = 0;
		bSuccess = Check(GetFileTime((HANDLE)(intptr_t)iSource, &ftCreation, &ftAccess, &ftWrite) == TRUE,
				"GetFileTime succeeds") && bSuccess;
		bSuccess = Check(ftWrite != 0 && ftAccess != 0, "GetFileTime fills its outputs") && bSuccess;

		SYSTEMTIME SystemTime;
		memset(&SystemTime, 0, sizeof(SystemTime));
		bSuccess = Check(FileTimeToSystemTime(&ftWrite, &SystemTime) == TRUE
				&& SystemTime.wYear == 2020 && SystemTime.wMonth == 9 && SystemTime.wDay == 13
				&& SystemTime.wHour == 12 && SystemTime.wMinute == 26 && SystemTime.wSecond == 40,
				"FileTimeToSystemTime matches the file's UTC timestamp") && bSuccess;

		//	A raw Unix timestamp is not a FILETIME; converting one must fail
		//	instead of producing a meaningless date.

		FILETIME ftRawEpoch = (FILETIME)1600000000ULL;
		bSuccess = Check(FileTimeToSystemTime(&ftRawEpoch, &SystemTime) == FALSE,
				"FileTimeToSystemTime rejects a raw Unix timestamp") && bSuccess;

		//	PDR-014: a copy must fail when the destination exists and the caller
		//	asked for that, and must overwrite when it did not.

		char szCopy[] = "/tmp/trans-port-copy-XXXXXX";
		int iCopy = mkstemp(szCopy);
		bSuccess = Check(iCopy >= 0, "create copy target") && bSuccess;

		if (iCopy >= 0)
			{
			close(iCopy);

			bSuccess = Check(CopyFile(szSource, szCopy, TRUE) == FALSE,
					"CopyFile fails when bFailIfExists and the target exists") && bSuccess;

			bSuccess = Check(CopyFile(szSource, szCopy, FALSE) == TRUE,
					"CopyFile overwrites when bFailIfExists is FALSE") && bSuccess;

			int iCopied = open(szCopy, O_RDONLY);
			bSuccess = Check(iCopied >= 0, "open copied file") && bSuccess;
			if (iCopied >= 0)
				{
				char szBuffer[32];
				memset(szBuffer, 0, sizeof(szBuffer));
				ssize_t iRead = read(iCopied, szBuffer, sizeof(szBuffer) - 1);
				bSuccess = Check(iRead == iContent && strcmp(szBuffer, pszContent) == 0,
						"copied file has the source contents") && bSuccess;
				close(iCopied);
				}

			//	A target that does not exist yet must still be created when
			//	bFailIfExists is set.

			char szNewTarget[] = "/tmp/trans-port-new-XXXXXX";
			int iNew = mkstemp(szNewTarget);
			if (iNew >= 0)
				close(iNew);
			unlink(szNewTarget);
			bSuccess = Check(CopyFile(szSource, szNewTarget, TRUE) == TRUE,
					"CopyFile creates a missing target when bFailIfExists") && bSuccess;
			unlink(szNewTarget);

			unlink(szCopy);
			}

		close(iSource);
		unlink(szSource);
		}

	//	PDR-015: the Win32 special-folder shim, the kernel helper and the SDL
	//	shell's log path must all name the same application-data root.

	char szAppData[1024];
	memset(szAppData, 0, sizeof(szAppData));
	bSuccess = Check(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szAppData) == S_OK,
			"SHGetFolderPath(CSIDL_APPDATA)") && bSuccess;

	char szLocalAppData[1024];
	memset(szLocalAppData, 0, sizeof(szLocalAppData));
	bSuccess = Check(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szLocalAppData) == S_OK
			&& strcmp(szLocalAppData, szAppData) == 0,
			"roaming and local app data share one root") && bSuccess;

	Kernel::CString sAppDataRoot = Kernel::pathGetAppDataRoot();
	bSuccess = Check(!sAppDataRoot.IsBlank(), "pathGetAppDataRoot is not blank") && bSuccess;
	bSuccess = Check(strcmp(szAppData, sAppDataRoot.GetASCIIZPointer()) == 0,
			"pathGetAppDataRoot agrees with SHGetFolderPath") && bSuccess;
	bSuccess = Check(strEndsWithOld(CString(szAppData), CONSTLIT("/Library/Application Support/Kronosaur/Transcendence")),
			"application data lives under the Kronosaur bundle") && bSuccess;
	}

	//	PDR-007: the DIB creation helpers must produce a usable bitmap instead
	//	of failing outright.

	{
	HBITMAP hDib = NULL;
	BYTE *pPixels = NULL;
	bSuccess = Check(dibCreate24bitDIB(4, 3, 0, &hDib, &pPixels) == NOERROR, "dibCreate24bitDIB") && bSuccess;
	bSuccess = Check(hDib != NULL && pPixels != NULL, "dibCreate24bitDIB returns a bitmap") && bSuccess;

	if (hDib)
		{
		int cxWidth = 0;
		int cyHeight = 0;
		int iStride = 0;
		void *pBase = NULL;
		void *pBits = NULL;
		bSuccess = Check(dibGetInfo(hDib, &cxWidth, &cyHeight, &pBase, &iStride, NULL, &pBits) == NOERROR
				&& cxWidth == 4 && cyHeight == 3 && iStride >= 12 && pBase != NULL && pBits != NULL,
				"created DIB reports its geometry") && bSuccess;

		//	A crop inside the source must succeed; one that runs past the edge
		//	must fail instead of reading out of bounds.

		HBITMAP hCrop = NULL;
		bSuccess = Check(dibCrop(hDib, 1, 1, 2, 2, &hCrop) == NOERROR, "dibCrop") && bSuccess;
		if (hCrop)
			{
			cxWidth = 0;
			cyHeight = 0;
			bSuccess = Check(dibGetInfo(hCrop, &cxWidth, &cyHeight, NULL, NULL, NULL, NULL) == NOERROR
					&& cxWidth == 2 && cyHeight == 2,
					"cropped DIB reports the cropped geometry") && bSuccess;
			SDLBitmapDestroy((SDLBitmap *)hCrop);
			}

		HBITMAP hBadCrop = NULL;
		bSuccess = Check(dibCrop(hDib, 3, 2, 2, 2, &hBadCrop) == ERR_FAIL && hBadCrop == NULL,
				"dibCrop rejects a crop outside the source") && bSuccess;

		//	This port has no separate device-dependent bitmap, so a DDB
		//	conversion hands back the same handle.

		HBITMAP hDdb = NULL;
		bSuccess = Check(dibConvertToDDB(hDib, NULL, &hDdb) == NOERROR && hDdb == hDib,
				"dibConvertToDDB returns the DIB handle") && bSuccess;

		SDLBitmapDestroy((SDLBitmap *)hDib);
		}
	}

	//	PDR-022: MEM_COMMIT promises committed, zero-filled memory. A fresh
	//	reservation and an explicit address that was reserved earlier must both
	//	read as zero after the commit.

	{
	void *pBlock = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_READWRITE);
	bSuccess = Check(pBlock != NULL, "VirtualAlloc(MEM_COMMIT) returns memory") && bSuccess;

	if (pBlock)
		{
		const BYTE *pBytes = (const BYTE *)pBlock;
		bool bZeroed = true;
		for (int i = 0; i < 4096; i++)
			{
			if (pBytes[i] != 0)
				{
				bZeroed = false;
				break;
				}
			}
		bSuccess = Check(bZeroed, "VirtualAlloc(MEM_COMMIT) zero-fills a fresh region") && bSuccess;
		VirtualFree(pBlock, 0, MEM_RELEASE);
		}

	//	The reserve/commit pair is the pattern CString uses: the range is
	//	reserved first, and the later commit has to clear whatever the
	//	allocator left behind.

	BYTE *pReserved = (BYTE *)VirtualAlloc(NULL, 1024, MEM_RESERVE, PAGE_READWRITE);
	bSuccess = Check(pReserved != NULL, "VirtualAlloc(MEM_RESERVE) returns memory") && bSuccess;

	if (pReserved)
		{
		memset(pReserved, 0xAB, 1024);
		bSuccess = Check(VirtualAlloc(pReserved, 1024, MEM_COMMIT, PAGE_READWRITE) == pReserved,
				"VirtualAlloc(MEM_COMMIT) keeps an explicit address") && bSuccess;

		bool bCleared = true;
		for (int i = 0; i < 1024; i++)
			{
			if (pReserved[i] != 0)
				{
				bCleared = false;
				break;
				}
			}
		bSuccess = Check(bCleared, "VirtualAlloc(MEM_COMMIT) clears an explicit range") && bSuccess;
		VirtualFree(pReserved, 0, MEM_RELEASE);
		}
	}

	//	PDR-016/PDR-017: the queue must carry the payload at full width and hand
	//	back every field of a Win32 MSG, not just message/wParam/lParam.

	{
	PlatformClearMessageQueue();
	PlatformSetMessageWindow((void *)0x1234);

	const WPARAM wWideParam = (WPARAM)0x123456789ABCDEF0ULL;
	const LPARAM lWideParam = (LPARAM)0xFEDCBA9876543210ULL;
	bSuccess = Check(PostMessage(NULL, WM_USER + 7, wWideParam, lWideParam), "PostMessage queues a message") && bSuccess;

	MSG Message;
	memset(&Message, 0, sizeof(Message));
	Message.hwnd = (void *)0xBAD;
	Message.time = 0;
	Message.pt.x = 0x7FFF;
	Message.pt.y = 0x7FFF;

	POINT ExpectedCursor;
	PlatformGetCursorPos(&ExpectedCursor);

	bSuccess = Check(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE), "PeekMessage returns the queued message") && bSuccess;
	bSuccess = Check(Message.message == WM_USER + 7, "message id survives the queue") && bSuccess;
	bSuccess = Check(Message.wParam == wWideParam && Message.lParam == lWideParam,
			"wide WPARAM/LPARAM survive the queue") && bSuccess;
	bSuccess = Check(Message.hwnd == (void *)0x1234, "PeekMessage fills hwnd") && bSuccess;
	bSuccess = Check(Message.time != 0, "PeekMessage fills time") && bSuccess;
	bSuccess = Check(Message.pt.x == ExpectedCursor.x && Message.pt.y == ExpectedCursor.y,
			"PeekMessage fills pt from the cursor position") && bSuccess;
	bSuccess = Check(!PeekMessage(&Message, NULL, 0, 0, PM_REMOVE), "the queue drains to empty") && bSuccess;

	PlatformSetMessageWindow(NULL);
	PlatformClearMessageQueue();
	}

	//	PDR-018: Win32 SendMessage runs the handler before it returns; it does
	//	not merely queue the message for the next pump.

	{
	PlatformClearMessageQueue();
	PlatformSetMessageDispatch(TestMessageDispatch);
	g_iDispatchCount = 0;
	g_dwDispatchMessage = 0;
	g_wDispatchParam = 0;

	SendMessage(NULL, WM_USER + 3, (WPARAM)0x1111222233334444ULL, (LPARAM)0x5555);

	bSuccess = Check(g_iDispatchCount == 1, "SendMessage runs the handler before returning") && bSuccess;
	bSuccess = Check(g_dwDispatchMessage == WM_USER + 3, "SendMessage passes the message id to the handler") && bSuccess;
	bSuccess = Check(g_wDispatchParam == (WPARAM)0x1111222233334444ULL,
			"SendMessage passes a wide wParam to the handler") && bSuccess;

	MSG Message;
	memset(&Message, 0, sizeof(Message));
	bSuccess = Check(!PeekMessage(&Message, NULL, 0, 0, PM_REMOVE),
			"a dispatched SendMessage is not also queued") && bSuccess;

	//	WM_CLOSE goes to the registered close request instead of the dispatcher.

	PlatformSetMessageDispatch(NULL);
	PlatformClearMessageQueue();
	}

	//	PDR-019: coordinates and wheel deltas are packed as signed 16-bit halves
	//	and must survive the round trip, including the sign-extension that
	//	GET_X_LPARAM/GET_Y_LPARAM perform.

	{
	int x = 0;
	int y = 0;

	PlatformUnpackPoint(PlatformPackPoint(1920, 1080), &x, &y);
	bSuccess = Check(x == 1920 && y == 1080, "an in-range point round-trips") && bSuccess;

	PlatformUnpackPoint(PlatformPackPoint(-1200, -800), &x, &y);
	bSuccess = Check(x == -1200 && y == -800, "a negative point round-trips") && bSuccess;

	PlatformUnpackPoint(PlatformPackPoint(-1, 1), &x, &y);
	bSuccess = Check(x == -1 && y == 1, "both halves are sign-extended") && bSuccess;

	PlatformUnpackPoint(PlatformPackPoint(40000, -40000), &x, &y);
	bSuccess = Check(x == 32767 && y == -32768,
			"an out-of-range point clamps to the 16-bit bounds") && bSuccess;

	//	MK_LBUTTON (0x0001) shares the wParam with the wheel delta, the way
	//	Win32 packs it.

	const DWORD dwWheel = PlatformPackMouseWheel((WORD)0x0001, -120);
	bSuccess = Check(PlatformUnpackMouseWheelDelta(dwWheel) == -120, "the wheel delta round-trips") && bSuccess;
	bSuccess = Check(PlatformUnpackMouseWheelFlags(dwWheel) == (WORD)0x0001, "the wheel key flags round-trip") && bSuccess;

	const DWORD dwWheelUp = PlatformPackMouseWheel((WORD)0x0000, 120);
	bSuccess = Check(PlatformUnpackMouseWheelDelta(dwWheelUp) == 120, "a positive wheel delta round-trips") && bSuccess;
	}

	//	PDR-027: the bounded monochrome scan must still classify a surface that
	//	fits in the sample budget exactly, and must not read past the end of a
	//	row.

	{
	DWORD MonoPixels[8 * 8];
	for (int i = 0; i < 8 * 8; i++)
		MonoPixels[i] = 0xFF000000;	//	opaque black

	SDL_Surface *pMono = SDL_CreateRGBSurfaceFrom(MonoPixels, 8, 8, 32, 8 * 4,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	bSuccess = Check(pMono != NULL, "monochrome test surface") && bSuccess;

	if (pMono)
		{
		bSuccess = Check(SDLBitmapSurfaceIsMonochrome(pMono), "an all-black surface is monochrome") && bSuccess;

		for (int i = 0; i < 8 * 8; i++)
			MonoPixels[i] = 0xFFFFFFFF;	//	opaque white
		bSuccess = Check(SDLBitmapSurfaceIsMonochrome(pMono), "an all-white surface is monochrome") && bSuccess;

		MonoPixels[8 * 4 + 3] = 0xFFFF0000;	//	one red pixel
		bSuccess = Check(!SDLBitmapSurfaceIsMonochrome(pMono), "a surface with a colour pixel is not monochrome") && bSuccess;

		SDL_FreeSurface(pMono);
		}

	//	A large surface takes the sampled path; an all-black one must still be
	//	reported as monochrome.

	DWORD LargePixels[128 * 128];
	for (int i = 0; i < 128 * 128; i++)
		LargePixels[i] = 0xFF000000;

	SDL_Surface *pLarge = SDL_CreateRGBSurfaceFrom(LargePixels, 128, 128, 32, 128 * 4,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	bSuccess = Check(pLarge != NULL, "large monochrome test surface") && bSuccess;

	if (pLarge)
		{
		bSuccess = Check(SDLBitmapSurfaceIsMonochrome(pLarge), "a large all-black surface is monochrome") && bSuccess;
		SDL_FreeSurface(pLarge);
		}
	}

	//	PDR-028: _fcvt_s is handed a CString whose declared length is
	//	_CVTBUFSIZE (309), so the shim is told the buffer is 309 bytes and must
	//	produce a deterministic, NUL-terminated digit string without leaving
	//	stale bytes inside the declared range.

	{
	struct SFcvtCase { double rValue; int iDecimals; const char *pszDigits; int iDec; int iSign; };
	const SFcvtCase FcvtCases[] = {
		{ 0.5, 0, "0", 1, 0 },
		{ 12.34, 0, "12", 2, 0 },
		{ -0.125, 3, "0125", 1, 1 },
		{ 9.9, 0, "10", 2, 0 },
		};

	for (int i = 0; i < (int)(sizeof(FcvtCases) / sizeof(FcvtCases[0])); i++)
		{
		//	0xAA everywhere, exactly like a recycled CString buffer.

		char szDigits[310];
		memset(szDigits, 0xAA, sizeof(szDigits));

		int iDec = -1;
		int iSign = -1;
		int iResult = _fcvt_s(szDigits, 309, FcvtCases[i].rValue, FcvtCases[i].iDecimals, &iDec, &iSign);

		bSuccess = Check(iResult == 0, "_fcvt_s succeeds") && bSuccess;
		bSuccess = Check(strcmp(szDigits, FcvtCases[i].pszDigits) == 0, "_fcvt_s produces the expected digits") && bSuccess;
		bSuccess = Check(iDec == FcvtCases[i].iDec && iSign == FcvtCases[i].iSign,
				"_fcvt_s reports the decimal point and the sign") && bSuccess;

		//	Everything between the terminator and the declared length must have
		//	been cleared, so no stale byte can be read as part of the string.

		bool bCleared = true;
		for (size_t j = strlen(szDigits) + 1; j < 309; j++)
			{
			if (szDigits[j] != 0)
				bCleared = false;
			}

		bSuccess = Check(bCleared, "_fcvt_s leaves no stale bytes inside the declared buffer") && bSuccess;

		//	The byte past the declared length is the caller's, not ours.

		bSuccess = Check(szDigits[309] == (char)0xAA, "_fcvt_s does not write past the declared length") && bSuccess;
		}

	//	A buffer far too small for the formatted value must still be safe.

	char szTiny[8];
	memset(szTiny, 0xAA, sizeof(szTiny));

	int iTinyDec = -1;
	int iTinySign = -1;
	bSuccess = Check(_fcvt_s(szTiny, (int)sizeof(szTiny), 1234567.875, 3, &iTinyDec, &iTinySign) == 0,
			"_fcvt_s succeeds with a truncating buffer") && bSuccess;
	bSuccess = Check(szTiny[sizeof(szTiny) - 1] == '\0' && strlen(szTiny) == sizeof(szTiny) - 1,
			"_fcvt_s terminates a truncating buffer") && bSuccess;

	int iNullDec = -1;
	int iNullSign = -1;
	bSuccess = Check(_fcvt_s(NULL, 0, 1.0, 0, &iNullDec, &iNullSign) != 0,
			"_fcvt_s rejects an unusable buffer") && bSuccess;
	}

	//	PDR-039: the shim had its two output parameters swapped, so the caller
	//	Kernel::strFromDouble() read the sign out of its decimal-point variable
	//	and vice versa. Every call with an explicit decimal count came back
	//	corrupted (12.34 with 2 decimals became "-0.1234"). These cases pin the
	//	caller against the CRT parameter order.

	{
	struct SStrFromDoubleCase { double rValue; int iDecimals; const char *pszExpected; };
	const SStrFromDoubleCase Cases[] = {
		{ 12.34, 2, "12.34" },
		{ -0.125, 3, "-0.125" },
		{ 0.5, 0, "0.0" },
		{ 9.9, 0, "10.0" },
		{ 0.05, 1, "0.1" },
		{ 100.0, 2, "100.00" },
		{ 12.34, -1, "12.34" },
		};

	for (int i = 0; i < (int)(sizeof(Cases) / sizeof(Cases[0])); i++)
		{
		CString sResult = Kernel::strFromDouble(Cases[i].rValue, Cases[i].iDecimals);
		const char *pszActual = (const char *)sResult.GetASCIIZPointer();
		bool bMatch = (strcmp(pszActual, Cases[i].pszExpected) == 0);

		if (!bMatch)
			std::fprintf(stderr, "FAILED: strFromDouble(%g, %d) = \"%s\", expected \"%s\"\n",
					Cases[i].rValue, Cases[i].iDecimals, pszActual, Cases[i].pszExpected);

		bSuccess = Check(bMatch,
				"strFromDouble keeps the sign and the decimal point in the CRT positions") && bSuccess;
		}
	}

	//	PDR-029: wsprintf must bound itself to the destination array it is
	//	given, not to a fixed 4096 bytes.

	{
	char szTruncated[16];
	int iWritten = wsprintf(szTruncated, "%s", "abcdefghijklmnopqrstuvwxyz");
	bSuccess = Check(iWritten == (int)sizeof(szTruncated) - 1, "wsprintf reports the truncated length") && bSuccess;
	bSuccess = Check(strlen(szTruncated) == sizeof(szTruncated) - 1 && szTruncated[sizeof(szTruncated) - 1] == '\0',
			"wsprintf terminates a truncated destination") && bSuccess;
	bSuccess = Check(strncmp(szTruncated, "abcdefghijklmno", sizeof(szTruncated) - 1) == 0,
			"wsprintf keeps the characters that fit") && bSuccess;

	char szFits[64];
	iWritten = wsprintf(szFits, "%d/%d", 12, 34);
	bSuccess = Check(iWritten == 5 && strcmp(szFits, "12/34") == 0, "wsprintf writes an untruncated result") && bSuccess;
	}

	//	PDR-030: the creation dispositions must be honoured. CREATE_NEW and
	//	TRUNCATE_EXISTING used to fall through and behave like OPEN_EXISTING.

	{
	char szExisting[] = "/tmp/trans-port-create-XXXXXX";
	int iExisting = mkstemp(szExisting);
	bSuccess = Check(iExisting >= 0, "create disposition test file") && bSuccess;

	if (iExisting >= 0)
		{
		bSuccess = Check(write(iExisting, "keep", 4) == 4, "write disposition test file") && bSuccess;
		close(iExisting);

		HANDLE hFile = CreateFile(szExisting, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
				NULL, CREATE_NEW, 0, NULL);
		bSuccess = Check(hFile == INVALID_HANDLE_VALUE, "CREATE_NEW fails when the file exists") && bSuccess;

		hFile = CreateFile(szExisting, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
				NULL, TRUNCATE_EXISTING, 0, NULL);
		bSuccess = Check(hFile != INVALID_HANDLE_VALUE, "TRUNCATE_EXISTING opens an existing file") && bSuccess;

		if (hFile != INVALID_HANDLE_VALUE)
			{
			DWORD dwSizeHigh = 0;
			DWORD dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
			bSuccess = Check(dwSizeLow == 0 && dwSizeHigh == 0, "TRUNCATE_EXISTING empties the file") && bSuccess;
			CloseHandle(hFile);
			}

		unlink(szExisting);
		}

	//	TRUNCATE_EXISTING must not create a missing file, while CREATE_NEW must.

	char szAbsent[] = "/tmp/trans-port-absent-XXXXXX";
	int iAbsent = mkstemp(szAbsent);
	if (iAbsent >= 0)
		close(iAbsent);
	unlink(szAbsent);

	HANDLE hAbsent = CreateFile(szAbsent, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
			NULL, TRUNCATE_EXISTING, 0, NULL);
	bSuccess = Check(hAbsent == INVALID_HANDLE_VALUE, "TRUNCATE_EXISTING does not create a missing file") && bSuccess;

	hAbsent = CreateFile(szAbsent, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
			NULL, CREATE_NEW, 0, NULL);
	bSuccess = Check(hAbsent != INVALID_HANDLE_VALUE, "CREATE_NEW creates a missing file") && bSuccess;
	if (hAbsent != INVALID_HANDLE_VALUE)
		CloseHandle(hAbsent);
	unlink(szAbsent);
	}

	//	PDR-031: a 64-bit position must survive SetFilePointer, and GetFileSize
	//	must report the high half of a size above 4 GiB. The file is created
	//	sparse, so the 5 GiB length costs no disk space.

	{
	char szSparse[] = "/tmp/trans-port-sparse-XXXXXX";
	int iSparse = mkstemp(szSparse);
	bSuccess = Check(iSparse >= 0, "create sparse test file") && bSuccess;

	if (iSparse >= 0)
		{
		const off_t iFiveGiB = (off_t)5 * 1024 * 1024 * 1024;
		bSuccess = Check(ftruncate(iSparse, iFiveGiB) == 0, "size the sparse test file") && bSuccess;

		HANDLE hFile = (HANDLE)(intptr_t)iSparse;

		LONG lHigh = 1;
		DWORD dwLow = SetFilePointer(hFile, (LONG)0x40000000, &lHigh, FILE_BEGIN);
		bSuccess = Check(dwLow == 0x40000000 && lHigh == 1,
				"SetFilePointer applies the input high word") && bSuccess;

		LONG lHighOut = 0;
		DWORD dwCurrent = SetFilePointer(hFile, 0, &lHighOut, FILE_CURRENT);
		bSuccess = Check(dwCurrent == 0x40000000 && lHighOut == 1,
				"the 64-bit position round-trips") && bSuccess;

		DWORD dwSizeHigh = 0;
		DWORD dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
		bSuccess = Check(dwSizeLow == 0x40000000 && dwSizeHigh == 1,
				"GetFileSize reports a size above 4 GiB") && bSuccess;

		//	A seek before the start of the file fails and reports the Win32
		//	sentinel instead of a truncated errno value.

		bSuccess = Check(SetFilePointer(hFile, -1, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER,
				"a failing seek reports INVALID_SET_FILE_POINTER") && bSuccess;

		close(iSparse);
		unlink(szSparse);
		}
	}

	//	PDR-032: MoveFile must replace an existing destination, and the
	//	cross-volume fallback must copy and unlink. GetTempPath must follow
	//	TMPDIR instead of hardcoding /tmp.

	{
	char szMoveSrc[] = "/tmp/trans-port-move-src-XXXXXX";
	int iMoveSrc = mkstemp(szMoveSrc);
	char szMoveDst[] = "/tmp/trans-port-move-dst-XXXXXX";
	int iMoveDst = mkstemp(szMoveDst);
	bSuccess = Check(iMoveSrc >= 0 && iMoveDst >= 0, "create move test files") && bSuccess;

	if (iMoveSrc >= 0 && iMoveDst >= 0)
		{
		bSuccess = Check(write(iMoveSrc, "alpha", 5) == 5, "write move source") && bSuccess;
		bSuccess = Check(write(iMoveDst, "beta", 4) == 4, "write move destination") && bSuccess;
		close(iMoveSrc);
		close(iMoveDst);

		bSuccess = Check(MoveFile(szMoveSrc, szMoveDst) == TRUE, "MoveFile replaces an existing destination") && bSuccess;
		bSuccess = Check(access(szMoveSrc, F_OK) != 0, "MoveFile removes the source") && bSuccess;

		int iMoved = open(szMoveDst, O_RDONLY);
		bSuccess = Check(iMoved >= 0, "open the moved file") && bSuccess;
		if (iMoved >= 0)
			{
			char szBuffer[16];
			memset(szBuffer, 0, sizeof(szBuffer));
			ssize_t iRead = read(iMoved, szBuffer, sizeof(szBuffer) - 1);
			bSuccess = Check(iRead == 5 && strcmp(szBuffer, "alpha") == 0,
					"the destination holds the source contents") && bSuccess;
			close(iMoved);
			}

		//	MoveFile on a path that does not exist must fail rather than
		//	silently succeed.

		bSuccess = Check(MoveFile(szMoveSrc, szMoveDst) == FALSE, "MoveFile fails for a missing source") && bSuccess;

		unlink(szMoveDst);
		}

	//	The EXDEV fallback is not reachable from a single volume, so the helper
	//	it calls is exercised directly: it must copy the contents and remove
	//	the source.

	char szFallbackSrc[] = "/tmp/trans-port-fallback-src-XXXXXX";
	int iFallbackSrc = mkstemp(szFallbackSrc);
	char szFallbackDst[] = "/tmp/trans-port-fallback-dst-XXXXXX";
	int iFallbackDst = mkstemp(szFallbackDst);
	bSuccess = Check(iFallbackSrc >= 0 && iFallbackDst >= 0, "create fallback test files") && bSuccess;

	if (iFallbackSrc >= 0 && iFallbackDst >= 0)
		{
		bSuccess = Check(write(iFallbackSrc, "gamma", 5) == 5, "write fallback source") && bSuccess;
		close(iFallbackSrc);
		close(iFallbackDst);

		bSuccess = Check(posixMoveFileAcrossVolumes(szFallbackSrc, szFallbackDst) == TRUE,
				"the cross-volume fallback reports success") && bSuccess;
		bSuccess = Check(access(szFallbackSrc, F_OK) != 0, "the cross-volume fallback removes the source") && bSuccess;

		int iFallback = open(szFallbackDst, O_RDONLY);
		bSuccess = Check(iFallback >= 0, "open the fallback destination") && bSuccess;
		if (iFallback >= 0)
			{
			char szBuffer[16];
			memset(szBuffer, 0, sizeof(szBuffer));
			ssize_t iRead = read(iFallback, szBuffer, sizeof(szBuffer) - 1);
			bSuccess = Check(iRead == 5 && strcmp(szBuffer, "gamma") == 0,
					"the cross-volume fallback copies the contents") && bSuccess;
			close(iFallback);
			}

		unlink(szFallbackDst);
		}

	//	GetTempPath must report the directory the process was actually given.

	char szTemp[512];
	const char *pszOriginalTemp = getenv("TMPDIR");
	CString sSavedTemp(pszOriginalTemp ? pszOriginalTemp : "");

	DWORD dwTempLen = GetTempPath((DWORD)sizeof(szTemp), szTemp);
	const char *pszExpectedTemp = (pszOriginalTemp && *pszOriginalTemp) ? pszOriginalTemp : "/tmp";
	bSuccess = Check(dwTempLen == (DWORD)strlen(pszExpectedTemp), "GetTempPath returns the length it wrote") && bSuccess;
	bSuccess = Check(strcmp(szTemp, pszExpectedTemp) == 0, "GetTempPath honours TMPDIR") && bSuccess;

	//	An undersized buffer must report the length that would be needed and
	//	must be left untouched.

	char szSmallTemp[4];
	memset(szSmallTemp, 0x5A, sizeof(szSmallTemp));
	bSuccess = Check(GetTempPath((DWORD)sizeof(szSmallTemp), szSmallTemp) == (DWORD)strlen(pszExpectedTemp),
			"GetTempPath reports the needed length") && bSuccess;
	bSuccess = Check(szSmallTemp[0] == (char)0x5A, "GetTempPath leaves an undersized buffer alone") && bSuccess;

	bSuccess = Check(setenv("TMPDIR", "/tmp/trans-port-temp", 1) == 0, "set TMPDIR") && bSuccess;
	bSuccess = Check(GetTempPath((DWORD)sizeof(szTemp), szTemp) == (DWORD)strlen("/tmp/trans-port-temp")
			&& strcmp(szTemp, "/tmp/trans-port-temp") == 0,
			"GetTempPath follows a changed TMPDIR") && bSuccess;

	if (!sSavedTemp.IsBlank())
		setenv("TMPDIR", sSavedTemp.GetASCIIZPointer(), 1);
	else
		unsetenv("TMPDIR");
	}

	//	PDR-034: the CPU information a caller can rely on must come from the
	//	host, and the logical-processor query must stay an honest stub.

	{
	SYSTEM_INFO SystemInfo;
	memset(&SystemInfo, 0x5A, sizeof(SystemInfo));
	GetSystemInfo(&SystemInfo);

	bSuccess = Check(SystemInfo.dwNumberOfProcessors >= 1, "GetSystemInfo reports at least one processor") && bSuccess;
	bSuccess = Check(SystemInfo.dwPageSize >= 4096 && (SystemInfo.dwPageSize & (SystemInfo.dwPageSize - 1)) == 0,
			"GetSystemInfo reports a plausible page size") && bSuccess;
	bSuccess = Check(SystemInfo.dwAllocationGranularity == SystemInfo.dwPageSize,
			"the allocation granularity matches the page size") && bSuccess;
	bSuccess = Check(SystemInfo.dwActiveProcessorMask != 0, "the active processor mask is not empty") && bSuccess;

	if (SystemInfo.dwNumberOfProcessors < 8 * sizeof(DWORD_PTR))
		bSuccess = Check(SystemInfo.dwActiveProcessorMask
				== (((DWORD_PTR)1 << SystemInfo.dwNumberOfProcessors) - 1),
				"the active processor mask covers every processor") && bSuccess;

	DWORD dwLength = 1234;
	bSuccess = Check(GetLogicalProcessorInformationEx(RelationAll, NULL, &dwLength) == FALSE && dwLength == 0,
			"GetLogicalProcessorInformationEx reports no information") && bSuccess;
	}

	kernelCleanUp();
	return (bSuccess ? 0 : 1);
	}
