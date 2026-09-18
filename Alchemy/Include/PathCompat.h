//	PathCompat.h
//
//	macOS declarations for the Win32 path/time surface.
//
//	Alchemy/Kernel/Path.cpp implements a handful of Win32 helpers for the
//	macOS port: special folders, file times and file copy. They used to be
//	inline helpers private to that translation unit, which meant that no other
//	translation unit (including the portability tests) could reach them. They
//	are declared here so that Path.cpp and its callers agree on one contract.
//
//	Include this header only on the macOS port. On Windows the real Win32
//	declarations apply and this header does nothing.

#pragma once

#ifdef TARGET_PLATFORM_MACOS

#include "Kernel.h"

//	FILETIME is a 64-bit count of 100-nanosecond intervals since 1601-01-01
//	UTC, which is why it is a plain integer rather than a struct in this port.

#ifndef FILETIME
#define FILETIME unsigned long long
#endif

#ifndef HRESULT
#define HRESULT long
#endif

#ifndef S_OK
#define S_OK 0
#endif

#ifndef E_FAIL
#define E_FAIL 1
#endif

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA 26
#endif

#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA 28
#endif

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL 5
#endif

#ifndef CSIDL_MYPICTURES
#define CSIDL_MYPICTURES 0x0027
#endif

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC 0x000D
#endif

#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

//	Maps the CSIDL_* constants above onto the macOS directories and creates
//	the result (mkdir -p semantics). CSIDL_APPDATA and CSIDL_LOCAL_APPDATA
//	both resolve to the single canonical application-data root, so that the
//	game does not end up with two of them.

HRESULT SHGetFolderPath (void *pToken, int iCSIDL, void *pReserved, DWORD dwFlags, char *pDest);

//	Fills the requested FILETIME values from the open file handle. Any of the
//	three outputs may be NULL. Returns FALSE if the handle is not usable.

BOOL GetFileTime (HANDLE hFile, FILETIME *pCreation, FILETIME *pLastAccess, FILETIME *pLastWrite);

//	Converts a FILETIME (100-nanosecond intervals since 1601-01-01 UTC) into
//	the UTC calendar time that Win32 callers expect.

BOOL FileTimeToSystemTime (FILETIME *pFileTime, SYSTEMTIME *pSystemTime);

//	Win32-compatible copy that honours bFailIfExists.

BOOL CopyFile (const char *pSrc, const char *pDst, BOOL bFailIfExists);

#endif
