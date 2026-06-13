//	Kernel.h
//
//	Kernel definitions.
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#define _CRT_RAND_S
#include <cstddef>
#include <functional>

#ifdef _WIN32

//	Support Windows 7 and above

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0601 
#define WINVER			0x0601
#endif						

#define NOMINMAX
#include <windows.h>

#include <mmsystem.h>

#else

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <mach/mach_time.h>
#include <dirent.h>
#include <strings.h>
#include <string>
#include <algorithm>
#include <thread>

#undef htons
inline unsigned short htons(unsigned short x) { return (unsigned short)__builtin_bswap16(x); }
#undef ntohs
inline unsigned short ntohs(unsigned short x) { return (unsigned short)__builtin_bswap16(x); }

#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623158e+308
#endif

typedef unsigned char BYTE;
typedef int BOOL;
typedef std::uint32_t DWORD;
typedef std::uint64_t DWORDLONG;
typedef std::uint64_t UINT64;
typedef std::uint64_t KAFFINITY;
typedef long long INT64;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;

//	NOTE: On Windows LONG is always 32-bit, but on macOS/Linux 'long' is 64-bit.
//	We must use a fixed-width type or struct layouts (POINT, RECT, LARGE_INTEGER)
//	and binary file formats (save games, TDB) become incompatible.

typedef std::int32_t LONG;
typedef std::uint32_t ULONG;
typedef short SHORT;

//	LARGE_INTEGER must be 8 bytes so that QuadPart overlaps LowPart/HighPart.
//	Protected from multiple inclusion by #pragma once above.

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

static_assert(sizeof(LONG) == 4, "LONG must be 32-bit to match the Windows ABI and on-disk formats");
static_assert(sizeof(DWORD) == 4, "DWORD must be 32-bit");
static_assert(sizeof(LARGE_INTEGER) == 8, "LARGE_INTEGER must be 8 bytes so QuadPart overlays LowPart/HighPart");

typedef std::uintptr_t SIZE_T;
typedef std::uint64_t ULONG64;
typedef const char *LPCSTR;
typedef const char *LPCTSTR;
typedef void *HKEY;
typedef void *LPVOID;
typedef char *LPSTR;
typedef char *LPTSTR;
typedef void *HANDLE;
typedef void *HBITMAP;
typedef void *HDC;
typedef void *HFONT;
typedef void *HINSTANCE;
typedef void *HMODULE;
typedef void *HPALETTE;
#define MAKEINTRESOURCE(id) ((char *)(intptr_t)(id))
#define IDR_HELP_BACKGROUND 100
inline BOOL DeleteObject(HBITMAP hBitmap) { return 1; }
typedef DWORD COLORREF;
inline COLORREF SetTextColor(HDC hDC, COLORREF crColor) { return 0; }
inline COLORREF SetBkColor(HDC hDC, COLORREF crColor) { return 0; }
#define RGB(r,g,b) ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
inline HFONT CreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, const char* lpszFace) { return nullptr; }
#define CBM_INIT 0x4
#define DIB_RGB_COLORS 0
#define BI_RGB 0
#define BI_BITFIELDS 3
#define RT_BITMAP 2
inline HPALETTE SelectPalette(HDC hDC, HPALETTE hPal, BOOL bForceBackground) { fprintf(stderr, "ERROR: SelectPalette called on macOS\n"); return nullptr; }
inline unsigned int RealizePalette(HDC hDC) { fprintf(stderr, "ERROR: RealizePalette called on macOS\n"); return 0; }
inline HBITMAP CreateDIBitmap(HDC hDC, void* lpInfo, DWORD dwUsage, void* lpInitBits, void* lpColorInfo, DWORD dwColorUsage) { fprintf(stderr, "ERROR: CreateDIBitmap called on macOS\n"); return nullptr; }
inline HBITMAP CreateDIBSection(HDC hDC, void* pInfo, DWORD usage, void** ppBits, HANDLE hSection, DWORD offset) { fprintf(stderr, "ERROR: CreateDIBSection called on macOS\n"); return nullptr; }
inline int SetDIBits(HDC hDC, HBITMAP hBitmap, unsigned int uStartScan, unsigned int cScanLines, void* pBits, void* pInfo, DWORD dwColorUse) { fprintf(stderr, "ERROR: SetDIBits called on macOS\n"); return 0; }
#define SRCAND 0x008800C6
#define SRCCOPY 0x00CC0020
#define SRCPAINT 0x00EE0086
#define FW_BOLD 700
#define FW_NORMAL 400
#define ANSI_CHARSET 0
#define OUT_DEFAULT_PRECIS 0
#define CLIP_EMBEDDED 0x80
#define DEFAULT_QUALITY 0
#define VARIABLE_PITCH 2
#define FF_SWISS 32
typedef void *HRGN;
typedef void *HWND;
typedef void *HANDLE;

//	POSIX sockets are plain file descriptors: int, with -1 as the invalid value.
//	Using an unsigned type breaks error checks against -1.

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct hostent HOSTENT;
typedef struct in_addr IN_ADDR;
#define AF_INET 2
#define SOCK_STREAM 1
#define IPPROTO_TCP 6
#define INADDR_NONE ((unsigned long)-1)
#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)
#define ERROR_IO_PENDING 997
#define ERROR_IO_INCOMPLETE 996
#define ERROR_ALREADY_EXISTS 183
#define ERROR_SUCCESS 0
#define ERROR_INSUFFICIENT_BUFFER 122
#define CP_ACP 0
#define CP_UTF8 65001
typedef std::uint16_t WCHAR;
inline int MultiByteToWideChar(unsigned int CodePage, DWORD dwFlags, const char* lpMultiByteStr, int cbMultiByte, WCHAR* lpWideCharStr, int cchWideChar) {
    (void)dwFlags;
    if (!lpMultiByteStr) return 0;
    int iLen = (cbMultiByte < 0) ? (int)strlen(lpMultiByteStr) : cbMultiByte;
    if (CodePage != CP_UTF8 && CodePage != CP_ACP) return 0;
    int iCount = 0;
    const char* p = lpMultiByteStr;
    const char* pEnd = lpMultiByteStr + iLen;
    while (p < pEnd) {
        unsigned int cp = (unsigned char)*p;
        int bytes = 0;
        if (cp < 0x80) { bytes = 1; }
        else if ((cp & 0xE0) == 0xC0) { cp &= 0x1F; bytes = 2; }
        else if ((cp & 0xF0) == 0xE0) { cp &= 0x0F; bytes = 3; }
        else if ((cp & 0xF8) == 0xF0) { cp &= 0x07; bytes = 4; }
        else { bytes = 1; }
        if (p + bytes > pEnd) break;
        for (int i = 1; i < bytes; i++) {
            if (((unsigned char)p[i] & 0xC0) != 0x80) { bytes = 1; break; }
            cp = (cp << 6) | ((unsigned char)p[i] & 0x3F);
        }
        if (cp < 0x10000) {
            if (lpWideCharStr && iCount < cchWideChar) lpWideCharStr[iCount] = (WCHAR)cp;
            iCount++;
        } else {
            cp -= 0x10000;
            if (lpWideCharStr && iCount + 1 < cchWideChar) {
                lpWideCharStr[iCount] = (WCHAR)(0xD800 + (cp >> 10));
                lpWideCharStr[iCount + 1] = (WCHAR)(0xDC00 + (cp & 0x3FF));
            }
            iCount += 2;
        }
        p += bytes;
    }
    return iCount;
}
inline int WideCharToMultiByte(unsigned int CodePage, DWORD dwFlags, const WCHAR* lpWideCharStr, int cchWideChar, char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, BOOL* lpUsedDefaultChar) {
    (void)dwFlags; (void)lpDefaultChar; (void)lpUsedDefaultChar;
    if (!lpWideCharStr) return 0;
    if (CodePage != CP_UTF8 && CodePage != CP_ACP) return 0;
    int iLen = (cchWideChar < 0) ? (int)wcslen((const wchar_t*)lpWideCharStr) : cchWideChar;
    int iCount = 0;
    for (int i = 0; i < iLen; i++) {
        unsigned int cp = lpWideCharStr[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < iLen) {
            unsigned int lo = lpWideCharStr[i + 1];
            if (lo >= 0xDC00 && lo <= 0xDFFF) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                i++;
            }
        }
        int bytes = 0;
        if (cp < 0x80) bytes = 1;
        else if (cp < 0x800) bytes = 2;
        else if (cp < 0x10000) bytes = 3;
        else bytes = 4;
        if (lpMultiByteStr && iCount + bytes <= cbMultiByte) {
            char* d = lpMultiByteStr + iCount;
            if (bytes == 1) { d[0] = (char)cp; }
            else if (bytes == 2) { d[0] = (char)(0xC0 | (cp >> 6)); d[1] = (char)(0x80 | (cp & 0x3F)); }
            else if (bytes == 3) { d[0] = (char)(0xE0 | (cp >> 12)); d[1] = (char)(0x80 | ((cp >> 6) & 0x3F)); d[2] = (char)(0x80 | (cp & 0x3F)); }
            else { d[0] = (char)(0xF0 | (cp >> 18)); d[1] = (char)(0x80 | ((cp >> 12) & 0x3F)); d[2] = (char)(0x80 | ((cp >> 6) & 0x3F)); d[3] = (char)(0x80 | (cp & 0x3F)); }
        }
        iCount += bytes;
    }
    return iCount;
}
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define REG_OPTION_NON_VOLATILE 0

typedef void* HRSRC;
typedef void* HGLOBAL;
#define PAGE_NOACCESS 0x01
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000
#define MEM_COMMIT 0x1000
inline HRSRC FindResource(HMODULE hModule, const char* pName, const char* pType) { return nullptr; }
inline HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo) { return nullptr; }
inline void* LockResource(HGLOBAL hResData) { return nullptr; }
#ifndef _WIN32
#include <sys/mman.h>
#include <stdlib.h>
#include <cstring>
inline void* VirtualAlloc(void* lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    (void)lpAddress;
    (void)flProtect;
    (void)flAllocationType;
    void* pMem = malloc(dwSize);
    if (pMem && (flAllocationType & MEM_COMMIT))
        memset(pMem, 0, dwSize);
    return pMem;
}
inline BOOL VirtualFree(void* lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
    (void)dwFreeType;
    free(lpAddress);
    return TRUE;
}
#else
inline void* VirtualAlloc(void* lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) { return nullptr; }
inline BOOL VirtualFree(void* lpAddress, SIZE_T dwSize, DWORD dwFreeType) { return TRUE; }
#endif
inline DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo) { return 0; }

typedef void* HKEY;
#define HKEY_CURRENT_USER ((HKEY)1)
#define REG_SZ 1

inline LONG RegCloseKey(HKEY hKey) { return ERROR_SUCCESS; }
inline LONG RegQueryValueEx(HKEY hKey, const char* pValueName, void* pReserved, DWORD* pType, BYTE* pData, DWORD* pcbData) { return 2; }
inline LONG RegOpenKeyEx(HKEY hKey, const char* pSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult) { return 2; }
inline LONG RegCreateKeyEx(HKEY hKey, const char* pSubKey, DWORD Reserved, const char* pClass, DWORD dwOptions, DWORD samDesired, void* pSecurity, HKEY* phkResult, DWORD* pdwDisposition) { return 2; }
inline LONG RegSetValueEx(HKEY hKey, const char* pValueName, DWORD Reserved, DWORD dwType, const BYTE* pData, DWORD cbData) { return 2; }
inline DWORD WSAGetLastError() { return errno; }
typedef struct protoent PROTOENT;
#define closesocket close
inline BOOL CancelIo(HANDLE hFile) { return TRUE; }
struct OVERLAPPED { void *Internal; void *InternalHigh; void *Offset; DWORD OffsetHigh; HANDLE hEvent; };
typedef OVERLAPPED *LPOVERLAPPED;
typedef DWORD *LPDWORD;
inline BOOL GetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait) {
    if (lpNumberOfBytesTransferred)
        *lpNumberOfBytesTransferred = (DWORD)(intptr_t)lpOverlapped->InternalHigh;
    return (lpOverlapped->Internal == 0);
}
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID);

typedef unsigned int UINT;
typedef float FLOAT;
typedef std::uint16_t WORD;
typedef std::int8_t INT8;
typedef std::uint8_t UINT8;
typedef DWORD COLORREF;
typedef void *HICON;
typedef void *HCURSOR;
typedef void *HGDIOBJ;
typedef void *HMENU;
typedef std::uintptr_t UINT_PTR;
typedef UINT_PTR DWORD_PTR;
typedef std::intptr_t LONG_PTR;
typedef std::uintptr_t WPARAM;
typedef std::intptr_t LPARAM;
typedef std::int32_t LONG;

//	On Windows LRESULT is LONG_PTR (pointer-sized); it must be able to hold a
//	pointer, so do not narrow it to 32 bits.

typedef std::intptr_t LRESULT;
typedef int INT;
typedef int WMSG;

#define WM_USER 0x0400
#define APIENTRY __stdcall
#define CALLBACK __stdcall
#define WINAPI __stdcall

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LOBYTE(w) ((BYTE)((w) & 0xFF))
#define HIBYTE(w) ((BYTE)(((w) >> 8) & 0xFF))
#define LOWORD(dw) ((WORD)((dw) & 0xFFFF))
#define HIWORD(dw) ((WORD)(((dw) >> 16) & 0xFFFF))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef INFINITE
#define INFINITE 0xffffffff
#endif

#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 258
#endif

#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)(-1))
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)(-1))
#endif

#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2
#endif

#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND 3
#endif

#ifndef GetLastError
inline DWORD GetLastError() { return errno; }
#endif

#ifndef FILE_BEGIN
#define FILE_BEGIN 0
#endif

#ifndef FILE_CURRENT
#define FILE_CURRENT 1
#endif

#ifndef FILE_END
#define FILE_END 2
#endif

#ifndef GENERIC_READ
#define GENERIC_READ (0x80000000L)
#endif

#ifndef GENERIC_WRITE
#define GENERIC_WRITE (0x40000000L)
#endif

#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif

#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE 0x00000002
#endif

#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS 2
#endif

#ifndef OPEN_EXISTING
#define OPEN_EXISTING 3
#endif

#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS 4
#endif

//	posixResolvePathCase
//
//	Best-effort, case-insensitive resolution of a POSIX path. Windows file
//	systems are case-insensitive, so game data may reference resources with a
//	different case than the actual files on disk. On case-sensitive volumes
//	(APFS can be either) those lookups would fail.
//
//	For each component that does not exist with the given case, we scan the
//	parent directory for a case-insensitive match. If no match is found we
//	keep the original component (so creating new files still works) and
//	continue with the rest of the path.

inline std::string posixResolvePathCase(const std::string &sPath)
	{
	if (sPath.empty() || access(sPath.c_str(), F_OK) == 0)
		return sPath;

	bool bAbsolute = (sPath[0] == '/');
	std::string sResolved = bAbsolute ? "/" : "";
	size_t iPos = bAbsolute ? 1 : 0;

	while (iPos <= sPath.size())
		{
		size_t iNext = sPath.find('/', iPos);
		size_t iEnd = (iNext == std::string::npos) ? sPath.size() : iNext;
		std::string sComp = sPath.substr(iPos, iEnd - iPos);

		if (!sComp.empty())
			{
			std::string sCandidate = sResolved;
			if (!sCandidate.empty() && sCandidate.back() != '/')
				sCandidate += '/';
			sCandidate += sComp;

			if (sComp != "." && sComp != ".."
					&& access(sCandidate.c_str(), F_OK) != 0)
				{
				std::string sDir = sResolved.empty() ? "." : sResolved;
				DIR *pDir = opendir(sDir.c_str());
				if (pDir)
					{
					struct dirent *pEntry;
					while ((pEntry = readdir(pDir)) != NULL)
						{
						if (strcasecmp(pEntry->d_name, sComp.c_str()) == 0)
							{
							fprintf(stderr, "Warning: case mismatch in path: '%s' -> '%s'\n", sComp.c_str(), pEntry->d_name);
							sCandidate = sResolved;
							if (!sCandidate.empty() && sCandidate.back() != '/')
								sCandidate += '/';
							sCandidate += pEntry->d_name;
							break;
							}
						}
					closedir(pDir);
					}
				}

			sResolved = sCandidate;
			}

		if (iNext == std::string::npos)
			break;
		iPos = iNext + 1;
		}

	return sResolved;
	}

#ifndef CreateFile
inline HANDLE CreateFile(const char* pFilename, DWORD dwAccess, DWORD dwShareMode, void* pSecurity, DWORD dwCreationDisposition, DWORD dwFlags, HANDLE hTemplate) {
    (void)pSecurity; (void)hTemplate; (void)dwShareMode; (void)dwFlags;

    std::string sFilename = (pFilename ? pFilename : "");
    std::replace(sFilename.begin(), sFilename.end(), '\\', '/');

    //	Fall back to a case-insensitive lookup (Windows file systems are
    //	case-insensitive; game data may not match the on-disk case).

    if (access(sFilename.c_str(), F_OK) != 0)
        sFilename = posixResolvePathCase(sFilename);

    int flags = 0;
    if ((dwAccess & GENERIC_READ) && (dwAccess & GENERIC_WRITE))
        flags |= O_RDWR;
    else if (dwAccess & GENERIC_WRITE)
        flags |= O_WRONLY;
    else
        flags |= O_RDONLY;

    switch (dwCreationDisposition)
        {
        case CREATE_ALWAYS:
            flags |= O_CREAT | O_TRUNC;
            break;

        case OPEN_ALWAYS:
            flags |= O_CREAT;
            break;

        case OPEN_EXISTING:
            if (access(sFilename.c_str(), F_OK) != 0)
                return INVALID_HANDLE_VALUE;
            break;

        default:
            break;
        }

    int fd = open(sFilename.c_str(), flags, 0666);
    if (fd < 0)
        return INVALID_HANDLE_VALUE;

    return (HANDLE)(intptr_t)fd;
}
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif

#ifndef PAGE_READONLY
#define PAGE_READONLY 0x02
#endif

#ifndef PAGE_READWRITE
#define PAGE_READWRITE 0x04
#endif

#ifndef FILE_MAP_READ
#define FILE_MAP_READ 0x0004
#endif

#ifndef FILE_MAP_WRITE
#define FILE_MAP_WRITE 0x0002
#endif

//	POSIX event synchronization using pipes.
//	A signaled event has a byte in the pipe; an unsignaled event has an empty pipe.

struct SEventHandle { DWORD dwMagic; int fd[2]; bool bManualReset; };
#define EVENT_MAGIC 0x45565448

inline HANDLE CreateEvent(void* pAttrs, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName) {
    (void)pAttrs; (void)lpName;
    SEventHandle *pEvent = new SEventHandle;
    pEvent->dwMagic = EVENT_MAGIC;
    pEvent->bManualReset = (bManualReset != 0);
    if (pipe(pEvent->fd) != 0) { delete pEvent; return nullptr; }
    fcntl(pEvent->fd[0], F_SETFL, O_NONBLOCK);
    fcntl(pEvent->fd[1], F_SETFL, O_NONBLOCK);
    if (bInitialState) {
        char c = 1;
        if (write(pEvent->fd[1], &c, 1) < 0) {}
    }
    return (HANDLE)pEvent;
}

inline BOOL SetEvent(HANDLE h) {
    if (!h || h == INVALID_HANDLE_VALUE) return FALSE;
    SEventHandle *p = (SEventHandle *)h;
    char c = 1;
    if (write(p->fd[1], &c, 1) < 0) {}
    return TRUE;
}

inline BOOL ResetEvent(HANDLE h) {
    if (!h || h == INVALID_HANDLE_VALUE) return FALSE;
    SEventHandle *p = (SEventHandle *)h;
    char buf[64];
    while (read(p->fd[0], buf, sizeof(buf)) > 0) {}
    return TRUE;
}

#ifndef ReadFile
inline BOOL ReadFile(HANDLE hFile, void* buf, DWORD len, DWORD* read_out, void* extra) {
    ssize_t result = read((int)(intptr_t)hFile, buf, len);
    if (read_out) *read_out = (result >= 0) ? (DWORD)result : 0;
    //	For overlapped I/O: store result and signal the event
    if (extra) {
        LPOVERLAPPED pOverlapped = (LPOVERLAPPED)extra;
        pOverlapped->Internal = (void *)(intptr_t)((result >= 0) ? 0 : -1);
        pOverlapped->InternalHigh = (void *)(intptr_t)((result >= 0) ? result : 0);
        if (pOverlapped->hEvent)
            SetEvent(pOverlapped->hEvent);
    }
    return result >= 0;
}
#endif

#ifndef WriteFile
inline BOOL WriteFile(HANDLE hFile, const void* buf, DWORD len, DWORD* written_out, void* extra) {
    ssize_t result = write((int)(intptr_t)hFile, buf, len);
    if (written_out) *written_out = (result >= 0) ? (DWORD)result : 0;
    if (extra) {
        LPOVERLAPPED pOverlapped = (LPOVERLAPPED)extra;
        pOverlapped->Internal = (void *)(intptr_t)((result >= 0) ? 0 : -1);
        pOverlapped->InternalHigh = (void *)(intptr_t)((result >= 0) ? result : 0);
        if (pOverlapped->hEvent)
            SetEvent(pOverlapped->hEvent);
    }
    return result >= 0;
}
#endif

#ifndef SetFilePointer
inline DWORD SetFilePointer(HANDLE hFile, LONG lDist, LONG* pHighWord, DWORD dwWhence) {
    off_t result = lseek((int)(intptr_t)hFile, lDist, (int)dwWhence);
    if (pHighWord && result > 0xFFFFFFFF) *pHighWord = (DWORD)(result >> 32);
    return (DWORD)result;
}
#endif

#ifndef GetFileSize
inline DWORD GetFileSize(HANDLE hFile, DWORD* pHighWord) {
    struct stat st;
    if (fstat((int)(intptr_t)hFile, &st) == 0) {
        if (pHighWord) *pHighWord = (DWORD)((unsigned long long)st.st_size >> 32);
        return (DWORD)st.st_size;
    }
    if (pHighWord) *pHighWord = 0;
    return INVALID_SET_FILE_POINTER;
}
#endif

#ifndef DeleteFile
inline BOOL DeleteFile(const char* pFilename) {
    std::string sFilename = (pFilename ? pFilename : "");
    std::replace(sFilename.begin(), sFilename.end(), '\\', '/');
    return (unlink(sFilename.c_str()) == 0);
}
#endif

#ifndef CreateFileMapping
struct SFileMappingHandle { int fd; size_t size; int prot; };
inline HANDLE CreateFileMapping(HANDLE hFile, void* pAttr, DWORD flProtect, DWORD dwMaxSizeHigh, DWORD dwMaxSizeLow, const char* pName) {
    (void)pAttr; (void)pName;
    SFileMappingHandle* pH = new SFileMappingHandle;
    pH->fd = (int)(intptr_t)hFile;
    pH->size = ((size_t)dwMaxSizeHigh << 32) | dwMaxSizeLow;
    pH->prot = (flProtect == PAGE_READONLY) ? PROT_READ : PROT_READ | PROT_WRITE;
    return (HANDLE)pH;
}
#endif

#ifndef MapViewOfFile
inline void* MapViewOfFile(HANDLE hFileMapping, DWORD dwAccess, DWORD dwOffsetHigh, DWORD dwOffsetLow, SIZE_T dwNumBytes) {
    SFileMappingHandle* pH = (SFileMappingHandle*)hFileMapping;
    int prot = (dwAccess == FILE_MAP_READ) ? PROT_READ : PROT_READ | PROT_WRITE;
    off_t offset = ((off_t)dwOffsetHigh << 32) | dwOffsetLow;
    void* pResult = mmap(NULL, dwNumBytes, prot, MAP_SHARED, pH->fd, offset);
    if (pResult == MAP_FAILED) return NULL;
    return pResult;
}
#endif

#ifndef UnmapViewOfFile
inline BOOL UnmapViewOfFile(void* pBase) {
    return munmap(pBase, 1) == 0;
}
#endif

#ifndef FlushViewOfFile
#define FlushViewOfFile(ptr, size) msync(ptr, size, MS_SYNC)
#endif

struct RECT
	{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
	};

struct POINT
	{
	LONG x;
	LONG y;
	};

struct SIZE
	{
	LONG cx;
	LONG cy;
	};

static_assert(sizeof(POINT) == 8, "POINT must match the Windows 8-byte layout");
static_assert(sizeof(RECT) == 16, "RECT must match the Windows 16-byte layout");

#ifdef TARGET_PLATFORM_MACOS
bool PlatformDestroyWindow(HWND hWnd);
LRESULT PlatformSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
inline HDC GetDC(HWND hWnd) { return nullptr; }
inline int ReleaseDC(HWND hWnd, HDC hDC) { return 0; }
inline int ShowCursor(BOOL bShow) { return 0; }
inline BOOL SetCapture(HWND hWnd) { return TRUE; }
inline BOOL ReleaseCapture() { return TRUE; }
inline BOOL ShowWindow(HWND hWnd, int nCmdShow) { return TRUE; }
inline BOOL UpdateWindow(HWND hWnd) { return TRUE; }
inline HWND GetForegroundWindow() { return nullptr; }
inline BOOL SetForegroundWindow(HWND hWnd) { return TRUE; }
inline BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) { return TRUE; }
inline BOOL GetWindowRect(HWND hWnd, RECT* pRect) { if (pRect) { pRect->left = pRect->top = pRect->right = pRect->bottom = 0; } return TRUE; }
inline BOOL IsWindow(HWND hWnd) { return hWnd != nullptr; }
inline BOOL DestroyWindow(HWND hWnd) { return PlatformDestroyWindow(hWnd) ? TRUE : FALSE; }
inline int GetSystemMetrics(int nIndex) { return 0; }

inline HICON LoadIcon(HINSTANCE hInstance, LPCSTR lpIconName) { return nullptr; }
inline int SetCurrentDirectory(LPCSTR lpPathName) { return (lpPathName && *lpPathName ? chdir(lpPathName) : 1); }
inline BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, LPVOID pvParam, UINT fWinIni) { return TRUE; }
#define SPI_GETWORKAREA 48
#define WS_OVERLAPPEDWINDOW 0x00CF0000
inline BOOL AdjustWindowRect(RECT* lpRect, DWORD dwStyle, BOOL bMenu) { return TRUE; }
#define MB_ICONSTOP 0x10

#define MCIWndGetLength(h) (0)
#define MCIWndGetPosition(h) (0)
#define MCIWndCreate(h, style, flags, file) (nullptr)
#define MCIWndDestroy(h) (0)
#define MCIWndStop(h) (0)
#define MCIWndPlay(h) (0)
#define MCIWndPause(h) (0)
#define MCIWndResume(h) (0)
#define MCIWndSeek(h, pos) (0)
#define MCIWndGetError(h, buf, len) (0)
#define MCIWndOpen(h, file, flags) (0)
#define MCIWndHome(h) (0)
#define MCIWndGetMode(h, buf, len) (0)
#define MCI_MODE_NOT_READY 0
#define MCI_MODE_OPEN 1
#define MCI_MODE_PAUSE 2
#define MCI_MODE_PLAY 3
#define MCI_MODE_RECORD 4
#define MCI_MODE_SEEK 5
#define MCI_MODE_STOP 6
#define MCIWNDF_NOERRORDLG 0x0004
#define MCIWNDF_NOMENU 0x0040
#define MCIWNDF_NOPLAYBAR 0x0080
#define MCIWNDF_NOTIFYALL 0x0010
#define WS_OVERLAPPED 0
#define WS_CHILD 0x40000000
#define SetWindowLong(h, idx, val) (0)
#define GWL_WNDPROC 0
#define GWL_USERDATA (-21)
#define lstrlen(s) ((s) ? strlen(s) : 0)

#define SW_SHOWMAXIMIZED 3
#define SW_SHOW 5
#define SW_RESTORE 9
#define WM_QUIT 0x0012
#define WM_CLOSE 0x0010
#define WM_DESTROY 0x0002
#define WM_SIZE 0x0005
#define WM_MOVE 0x0003
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_MOUSEMOVE 0x0200
#define WM_MOUSEWHEEL 0x020A
#define WM_PAINT 0x000F
#define WM_ERASEBKGND 0x0014
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_ACTIVATE 0x0006
#define WM_ENABLE 0x000A
#define WM_CREATE 0x0001
#define WM_TIMER 0x0113
#define WM_COMMAND 0x0111
#define WM_NOTIFY 0x004E
#define WM_USER 0x0400

#define CS_DBLCLKS 0x0008
#define BLACK_BRUSH 4
#define IDC_ARROW 32512

#define DM_BITMAPWIDTH 1
#define DM_BITMAPHEIGHT 2

#define EWX_LOGOFF 0
#define EWX_SHUTDOWN 1

inline HGDIOBJ GetStockObject(int nIndex) { return nullptr; }
inline int RegisterClassEx(const void* pWndClass) { return 1; }
inline HWND CreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, void* pParam) { return nullptr; }
inline HWND GetCapture() { return nullptr; }
inline HWND SetFocus(HWND hWnd) { return nullptr; }
inline LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return PlatformSendMessage(hWnd, Msg, wParam, lParam); }
bool PlatformPostMessage(int msg, int wParam, void* lParam);
inline bool PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { PlatformPostMessage((int)Msg, (int)wParam, (void*)lParam); return true; }
inline bool PostQuitMessage(int nExitCode) { return true; }
inline HDC BeginPaint(HWND hWnd, void* pPaintStruct) { return nullptr; }
inline bool EndPaint(HWND hWnd, const void* pPaintStruct) { return true; }
inline bool InvalidateRect(HWND hWnd, const RECT* pRect, bool bErase) { return true; }
inline bool GetClientRect(HWND hWnd, RECT* pRect) { if (pRect) { pRect->left = pRect->top = 0; pRect->right = 1024; pRect->bottom = 768; } return true; }
unsigned int PlatformSetTimerCompat(void* hWnd, unsigned int timerID, unsigned int elapse, void* callback);
int PlatformKillTimerCompat(void* hWnd, unsigned int timerID);
#define SetTimer(hwnd, id, elapse, callback) PlatformSetTimerCompat((void*)(hwnd), (unsigned int)(id), (unsigned int)(elapse), (void*)(callback))
#define KillTimer(hwnd, id) PlatformKillTimerCompat((void*)(hwnd), (unsigned int)(id))
inline int LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName) { return 0; }
inline int SetCursor(int hCursor) { return 0; }
int PlatformPeekMessage(int* pMsg, int* pWParam, void** ppLParam);
inline bool PeekMessage(void* pMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) { struct SMsgCompat { void* hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; }; SMsgCompat* p = (SMsgCompat*)pMsg; int msg; int wParam; void* lParam; if (!PlatformPeekMessage(&msg, &wParam, &lParam)) return false; p->message = (UINT)msg; p->wParam = (WPARAM)wParam; p->lParam = (LPARAM)lParam; return true; }
inline bool TranslateMessage(const void* pMsg) { return false; }
inline LRESULT DispatchMessage(const void* pMsg) { return 0; }

struct tagMSG { void* hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; };
typedef tagMSG MSG;
#define PM_REMOVE 0x0001
#define PM_NOYIELD 0x0002

inline int MessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { return 0; }
#define MB_OK 0x00000000
#define MB_YESNO 0x00000004
#define IDYES 6
#define IDNO 7

inline bool GetCursorPos(POINT* pPoint) { if (pPoint) { pPoint->x = 0; pPoint->y = 0; } return true; }
inline void SetCursorPos(int x, int y) { }

#define VK_DOWN 0x28
#define VK_UP 0x26
#define VK_NEXT 0x22
#define VK_PRIOR 0x21
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PLAY_PAUSE 0xB3
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_STOP 0xB2

inline int timeBeginPeriod(int u) { return 0; }
inline DWORD timeGetTime() { return 0; }

typedef void* HBRUSH;
struct WNDCLASSEXA { UINT cbSize; UINT style; void* lpfnWndProc; int cbClsExtra; int cbWndExtra; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCSTR lpszMenuName; LPCSTR lpszClassName; HICON hIconSm; };
struct WNDCLASSEX { UINT cbSize; UINT style; void* lpfnWndProc; int cbClsExtra; int cbWndExtra; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCSTR lpszMenuName; LPCSTR lpszClassName; HICON hIconSm; };
#define WS_POPUP 0x80000000
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1

#define WM_ACTIVATEAPP 0x001C
#define WM_DISPLAYCHANGE 0x007E
#define WM_LBUTTONDBLCLK 0x0203
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_RBUTTONDBLCLK 0x0206
#define WM_PAINT 0x000F
#define WM_ERASEBKGND 0x0014

#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_RETURN 0x0D
#define VK_ESCAPE 0x1B
#define VK_PAUSE 0x13
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F11 0x7A
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_LEFT 0x25
#define VK_RIGHT 0x27
#define VK_TAB 0x09
#define VK_SPACE 0x20
#define VK_MBUTTON 0x04
#define VK_SUBTRACT 0x6D
#define VK_OEM_MINUS 0xBD
#define VK_ADD 0x6B
#define VK_OEM_PLUS 0xBB
#define VK_BACK 0x08
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HOME 0x24
#define VK_END 0x23
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_CLEAR 0x0C
#define VK_MULTIPLY 0x6A
#define VK_DIVIDE 0x6F
#define VK_DECIMAL 0x6E

#define WAIT_TIMEOUT 258

inline void CloseHandle(HANDLE h)
	{
	//	NULL is a valid "no handle" value in caller code (e.g., CFileReadBlock
	//	sets m_hFileMap = NULL on macOS). Without this guard we would call
	//	close(0) and silently close stdin.

	if (h == NULL || h == INVALID_HANDLE_VALUE)
		return;

	//	Event handles are heap-allocated structs (large pointer values).
	//	File descriptors are small integers (typically < 1024).
	//	Check if this looks like a pointer before dereferencing.

	intptr_t iHandle = (intptr_t)h;
	if (iHandle > 1024)
		{
		SEventHandle *pEvent = (SEventHandle *)h;
		if (pEvent->dwMagic == EVENT_MAGIC)
			{
			close(pEvent->fd[0]);
			close(pEvent->fd[1]);
			delete pEvent;
			return;
			}
		}

	close((int)iHandle);
	}

#define TIMER_RESOLUTION 1

inline void ZeroMemory(void* p, size_t n) { memset(p, 0, n); }
inline void Sleep(DWORD dwMs) {
    if (dwMs > 0) {
        struct timespec ts;
        ts.tv_sec = dwMs / 1000;
        ts.tv_nsec = (dwMs % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
}
inline void timeEndPeriod(int u) { }

struct PAINTSTRUCT { void* hdc; int fErase; RECT rcPaint; int fRestore; int fUpdate; int rgbReserved[32]; };
inline HDC BeginPaint(HWND hwnd, PAINTSTRUCT* ps) { if (ps) memset(ps, 0, sizeof(PAINTSTRUCT)); return nullptr; }
inline BOOL EndPaint(HWND hwnd, const PAINTSTRUCT* ps) { return TRUE; }
inline LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return 0; }
struct CREATESTRUCTA { void* lpCreateParams; HINSTANCE hInstance; HMENU hMenu; HWND hwndParent; int cy; int cx; int y; int x; LONG style; LPCSTR lpszName; LPCSTR lpszClass; DWORD dwExStyle; };
typedef const CREATESTRUCTA* LPCREATESTRUCT;
struct SScreenMgrOptions { int cx; int cy; int bWindowed; void* hIcon; };

inline int wsprintf(char* buf, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buf, 4096, format, args);
    va_end(args);
    return result;
}

inline char* CharLower(char* s) {
    if (s) { while (*s) { *s = tolower(*s); s++; } }
    return s;
}
inline char* CharLower(DWORD_PTR p) { static char buf[2]; buf[0] = tolower((char)p); buf[1] = '\0'; return buf; }
inline char* CharUpper(char* s) {
    if (s) { while (*s) { *s = toupper(*s); s++; } }
    return s;
}
inline char* CharUpper(DWORD_PTR p) { static char buf[2]; buf[0] = toupper((char)p); buf[1] = '\0'; return buf; }
inline DWORD CharUpperBuff(char* s, DWORD n) { for (DWORD i = 0; i < n && s[i]; i++) s[i] = toupper(s[i]); return n; }

#define _CVTBUFSIZE 309
inline int _gcvt_s(char* buf, int len, double value, int digits) { snprintf(buf, len, "%.*g", digits, value); return 0; }
inline int _fcvt_s(char* buf, int len, double value, int decimals, int* sign, int* digits) { snprintf(buf, len, "%.*f", decimals, value); return 0; (void)sign; (void)digits; }

inline char* CharLowerA(char* s) { if (s) while (*s) { *s = tolower(*s); s++; } return s; }
inline int LoadString(HINSTANCE hInstance, UINT uID, char* pBuffer, int cchBuffer) { return 0; }
inline BOOL CharLowerBuff(char* s, DWORD n) { for (DWORD i = 0; i < n && s[i]; i++) s[i] = tolower(s[i]); return TRUE; }

#define ERROR_SHARING_VIOLATION 32
inline BOOL SetEndOfFile(HANDLE hFile) { return TRUE; }
inline BOOL FlushFileBuffers(HANDLE hFile) { return TRUE; }
inline unsigned int rand_s(unsigned int* pVal) { *pVal = arc4random(); return 0; }

typedef POINT* LPPOINT;
inline BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) { return TRUE; }
inline BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint) { return TRUE; }

#endif

struct SYSTEMTIME
	{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
	};

inline void GetLocalTime(SYSTEMTIME* lpSystemTime) { time_t t = time(nullptr); struct tm* tm = localtime(&t); lpSystemTime->wYear = tm->tm_year + 1900; lpSystemTime->wMonth = tm->tm_mon + 1; lpSystemTime->wDayOfWeek = tm->tm_wday; lpSystemTime->wDay = tm->tm_mday; lpSystemTime->wHour = tm->tm_hour; lpSystemTime->wMinute = tm->tm_min; lpSystemTime->wSecond = tm->tm_sec; lpSystemTime->wMilliseconds = 0; }
inline void GetSystemTime(SYSTEMTIME* lpSystemTime) { time_t t = time(nullptr); struct tm* tm = gmtime(&t); lpSystemTime->wYear = tm->tm_year + 1900; lpSystemTime->wMonth = tm->tm_mon + 1; lpSystemTime->wDayOfWeek = tm->tm_wday; lpSystemTime->wDay = tm->tm_mday; lpSystemTime->wHour = tm->tm_hour; lpSystemTime->wMinute = tm->tm_min; lpSystemTime->wSecond = tm->tm_sec; lpSystemTime->wMilliseconds = 0; }
struct TIME_ZONE_INFORMATION { LONG Bias; WORD StandardName[32]; SYSTEMTIME StandardDate; LONG StandardBias; WORD DaylightName[32]; SYSTEMTIME DaylightDate; LONG DaylightBias; };
inline BOOL SystemTimeToTzSpecificLocalTime(TIME_ZONE_INFORMATION* pTzInfo, SYSTEMTIME* pUniversalTime, SYSTEMTIME* pLocalTime) { *pLocalTime = *pUniversalTime; return TRUE; }

struct BITMAPINFOHEADER;

struct CRITICAL_SECTION
	{
	pthread_mutex_t Mutex;
	};

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef INFINITE
#define INFINITE 0xffffffff
#endif

#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 258
#endif

#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0
#endif

#define INVALID_HANDLE_VALUE ((HANDLE)(std::intptr_t)-1)

#define CP_ACP 0
#define CP_UTF8 65001
#define RT_RCDATA ((const char *)10)
typedef unsigned short WCHAR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* LPCWSTR;
inline HGLOBAL GlobalAlloc(DWORD flags, SIZE_T size) { return (HGLOBAL)malloc(size); }
inline void* GlobalLock(HGLOBAL hMem) { return (void*)hMem; }
inline BOOL GlobalUnlock(HGLOBAL hMem) { (void)hMem; return TRUE; }
inline HGLOBAL GlobalFree(HGLOBAL hMem) { free((void*)hMem); return NULL; }
#endif
#define GMEM_MOVEABLE 0
#define CF_TEXT 1
struct GROUP_AFFINITY { KAFFINITY Mask; WORD Group; WORD Reserved[3]; };
struct PROCESSOR_RELATIONSHIP { BYTE Flags; KAFFINITY ProcessorMask; BYTE GroupCount; WORD Reserved; GROUP_AFFINITY GroupMask[16]; };
struct GROUP_RELATIONSHIP { BYTE GroupCount; BYTE ActiveGroupCount; WORD MaximumGroupCount; WORD Reserved; GROUP_AFFINITY GroupMask[16]; };
struct SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX { DWORD Relationship; DWORD Size; union { PROCESSOR_RELATIONSHIP Processor; GROUP_RELATIONSHIP Group; BYTE NumaNode; BYTE Cache; ULONGLONG Reserved[16]; }; };
typedef SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;
#define RelationProcessorCore 0
#define RelationGroup 5
#define LTP_PC_SMT 4
struct SYSTEM_INFO { DWORD dwOemId; DWORD dwPageSize; LPVOID lpMinimumApplicationAddress; LPVOID lpMaximumApplicationAddress; DWORD_PTR dwActiveProcessorMask; DWORD dwNumberOfProcessors; DWORD dwProcessorType; DWORD dwAllocationGranularity; WORD wProcessorLevel; WORD wProcessorRevision; };
#define RelationAll 0
inline BOOL GetLogicalProcessorInformationEx(DWORD Type, void* pBuffer, DWORD* pLength) { (void)Type; (void)pBuffer; if (pLength) *pLength = 0; return FALSE; }
inline void GetSystemInfo(SYSTEM_INFO* pInfo) { memset(pInfo, 0, sizeof(SYSTEM_INFO)); pInfo->dwNumberOfProcessors = 1; }
#define SW_SHOWNORMAL 1
inline BOOL GetUserNameA(char* pName, DWORD* pSize) { return FALSE; }
#define GetUserName GetUserNameA

#ifndef _WIN32
typedef unsigned short WORD;
struct WSAData { int wVersion; int wHighVersion; char szDescription[257]; char szSystemStatus[129]; int iMaxSockets; int iMaxUdpDg; char* lpVendorInfo; };
inline int WSAStartup(WORD wVersionRequested, WSAData* lpWSAData) { return 0; }
inline int WSACleanup() { return 0; }
inline void OutputDebugString(const char* pStr) { }
inline void _set_se_translator(void* pFunc) { }
typedef void* LPEXCEPTION_POINTERS;
inline long InterlockedIncrement(long* p) { return ++(*p); }
inline long InterlockedDecrement(long* p) { return --(*p); }
#define _beginthreadex(pSec, stack, start, arg, flags, id) ((HANDLE)0)
#define QS_ALLINPUT 0x04FF
#ifndef _WIN32
#include <mach-o/dyld.h>
#include <cstdio>
#include <cstring>
inline DWORD GetModuleFileName(HMODULE hModule, char* pFilename, DWORD nSize) {
    (void)hModule;
    uint32_t size = nSize;
    int result = _NSGetExecutablePath(pFilename, &size);
    if (result == 0) {
        size = strlen(pFilename);
        return size;
    }
    return 0;
}
#else
inline DWORD GetModuleFileName(HMODULE hModule, char* pFilename, DWORD nSize) { return 0; }
#endif
inline BOOL MoveFile(const char* pSrc, const char* pDst) { return rename(pSrc, pDst) == 0; }
inline void* ShellExecute(void* hwnd, const char* pOp, const char* pFile, const char* pParams, const char* pDir, int nShow) { return nullptr; }
inline DWORD GetFileVersionInfoSize(const char* pFilename, void* pHandle) { return 0; }
inline BOOL GetFileVersionInfo(const char* pFilename, DWORD handle, DWORD len, void* pData) { return FALSE; }
inline BOOL VerQueryValue(const void* pData, const char* pSubBlock, void** ppBuf, UINT* puLen) { return FALSE; }
inline DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* pHandles, BOOL bWaitAll, DWORD dwTimeout);

inline int MsgWaitForMultipleObjects(DWORD nCount, HANDLE* pHandles, BOOL bWaitAll, DWORD dwMilliseconds, DWORD dwWakeMask) { return WaitForMultipleObjects(nCount, pHandles, bWaitAll, dwMilliseconds); }
struct VS_FIXEDFILEINFO { DWORD dwSignature; DWORD dwStrucVersion; DWORD dwFileVersionMS; DWORD dwFileVersionLS; DWORD dwProductVersionMS; DWORD dwProductVersionLS; DWORD dwFileFlagsMask; DWORD dwFileFlags; DWORD dwFileOS; DWORD dwFileType; DWORD dwFileSubtype; DWORD dwFileDateMS; DWORD dwFileDateLS; };
typedef VS_FIXEDFILEINFO* LPVSFIXEDFILEINFO;
#define PUINT unsigned int*
#endif

#define WINAPI
#define VK_CONTROL 0x11
#define VK_NUMLOCK 0x90
#define VK_SHIFT 0x10
#define MAPVK_VK_TO_CHAR 2

inline void DebugBreak (void) { }
inline int GetAsyncKeyState (int) { return 0; }
inline DWORD GetCurrentThreadId (void) { return (DWORD)(uintptr_t)pthread_self(); }
inline SHORT GetKeyState (int) { return 0; }
inline BOOL IsCharAlpha (char chChar) { return (((chChar >= 'a' && chChar <= 'z') || (chChar >= 'A' && chChar <= 'Z')) ? TRUE : FALSE); }
inline BOOL IsCharAlphaNumeric (char chChar) { return (((chChar >= 'a' && chChar <= 'z') || (chChar >= 'A' && chChar <= 'Z') || (chChar >= '0' && chChar <= '9')) ? TRUE : FALSE); }
inline UINT MapVirtualKey (UINT, UINT) { return 0; }
inline HANDLE GetProcessHeap (void) { return nullptr; }
inline LPVOID HeapAlloc (HANDLE, DWORD, size_t iSize) { return std::malloc(iSize); }
inline BOOL HeapFree (HANDLE, DWORD, LPVOID pMem) { std::free(pMem); return TRUE; }
inline void InitializeCriticalSection (CRITICAL_SECTION *pCS)
	{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	int err = pthread_mutex_init(&pCS->Mutex, &attr);
	if (err != 0) { fprintf(stderr, "pthread_mutex_init failed: %d\n", err); }
	pthread_mutexattr_destroy(&attr);
	}
inline void DeleteCriticalSection (CRITICAL_SECTION *pCS) { pthread_mutex_destroy(&pCS->Mutex); }
inline void EnterCriticalSection (CRITICAL_SECTION *pCS) {
	int err = pthread_mutex_lock(&pCS->Mutex);
	if (err != 0) { fprintf(stderr, "pthread_mutex_lock failed: %d\n", err); }
}
inline void LeaveCriticalSection (CRITICAL_SECTION *pCS) { pthread_mutex_unlock(&pCS->Mutex); }
inline DWORD WaitForSingleObject(HANDLE h, DWORD dwTimeout) {
    if (!h || h == INVALID_HANDLE_VALUE) return WAIT_OBJECT_0;
    SEventHandle *p = (SEventHandle *)h;
    struct pollfd pfd;
    pfd.fd = p->fd[0];
    pfd.events = POLLIN;
    int timeout_ms = (dwTimeout == INFINITE) ? -1 : (int)dwTimeout;
    int rc = poll(&pfd, 1, timeout_ms);
    if (rc > 0 && (pfd.revents & POLLIN)) {
        if (!p->bManualReset) {
            char buf[64];
            while (read(p->fd[0], buf, sizeof(buf)) > 0) {}
        }
        return WAIT_OBJECT_0;
    }
    return WAIT_TIMEOUT;
}

inline DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* pHandles, BOOL bWaitAll, DWORD dwTimeout) {
    if (nCount == 0) return WAIT_TIMEOUT;
    struct pollfd *pFds = (struct pollfd *)alloca(nCount * sizeof(struct pollfd));
    for (DWORD i = 0; i < nCount; i++) {
        SEventHandle *p = (SEventHandle *)pHandles[i];
        pFds[i].fd = (p ? p->fd[0] : -1);
        pFds[i].events = POLLIN;
    }
    int timeout_ms = (dwTimeout == INFINITE) ? -1 : (int)dwTimeout;
    int rc = poll(pFds, nCount, timeout_ms);
    if (rc > 0) {
        for (DWORD i = 0; i < nCount; i++) {
            if (pFds[i].revents & POLLIN) {
                SEventHandle *p = (SEventHandle *)pHandles[i];
                if (p && !p->bManualReset) {
                    char buf[64];
                    while (read(p->fd[0], buf, sizeof(buf)) > 0) {}
                }
                return WAIT_OBJECT_0 + i;
            }
        }
    }
    return WAIT_TIMEOUT;
}

inline DWORD GetTickCount (void)
{
    struct mach_timebase_info timebase;
    mach_timebase_info(&timebase);
    uint64_t time = mach_absolute_time();
    return (DWORD)((time * timebase.numer) / timebase.denom / 1000000);
}

#ifndef QueryPerformanceCounter
inline void QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t time = mach_absolute_time();
    lpPerformanceCount->QuadPart = (LONGLONG)((time * timebase.numer) / timebase.denom);
}
#endif

#ifndef QueryPerformanceFrequency
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    lpFrequency->QuadPart = (LONGLONG)((1000000000ULL * timebase.denom) / timebase.numer);
    return TRUE;
}
#endif

#ifndef MAKELONG
#define MAKELONG(a, b) ((DWORD)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)((UINT_PTR)(a) & 0xff)) | ((WORD)((BYTE)((UINT_PTR)(b) & 0xff))) << 8))
#endif

inline BOOL UnionRect (RECT *prcDest, const RECT *prcSrc1, const RECT *prcSrc2)
{
    prcDest->left = std::min(prcSrc1->left, prcSrc2->left);
    prcDest->top = std::min(prcSrc1->top, prcSrc2->top);
    prcDest->right = std::max(prcSrc1->right, prcSrc2->right);
    prcDest->bottom = std::max(prcSrc1->bottom, prcSrc2->bottom);
    return TRUE;
}

inline BOOL OffsetRect (RECT *prc, int dx, int dy)
{
    prc->left += dx;
    prc->top += dy;
    prc->right += dx;
    prc->bottom += dy;
    return TRUE;
}

inline BOOL InflateRect (RECT *prc, int dx, int dy)
{
    prc->left -= dx;
    prc->top -= dy;
    prc->right += dx;
    prc->bottom += dy;
    return TRUE;
}

inline BOOL IsRectEmpty (const RECT *prc)
{
    return (prc->right <= prc->left || prc->bottom <= prc->top) ? TRUE : FALSE;
}

inline BOOL SetRect (RECT *prc, int left, int top, int right, int bottom)
{
    prc->left = left;
    prc->top = top;
    prc->right = right;
    prc->bottom = bottom;
    return TRUE;
}

inline BOOL SetRectEmpty (RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    return TRUE;
}

inline BOOL CopyRect (RECT *prcDest, const RECT *prcSrc)
{
    prcDest->left = prcSrc->left;
    prcDest->top = prcSrc->top;
    prcDest->right = prcSrc->right;
    prcDest->bottom = prcSrc->bottom;
    return TRUE;
}

inline BOOL IntersectRect (RECT *prcDest, const RECT *prcSrc1, const RECT *prcSrc2)
{
    prcDest->left = std::max(prcSrc1->left, prcSrc2->left);
    prcDest->top = std::max(prcSrc1->top, prcSrc2->top);
    prcDest->right = std::min(prcSrc1->right, prcSrc2->right);
    prcDest->bottom = std::min(prcSrc1->bottom, prcSrc2->bottom);
    return (prcDest->right >= prcDest->left && prcDest->bottom >= prcDest->top) ? TRUE : FALSE;
}

inline int MulDiv (int nMultiplicand, int nMultiplier, int nDivisor)
{
    if (nDivisor == 0) return -1;
    int64_t result = (int64_t)nMultiplicand * nMultiplier / nDivisor;
    return (int)result;
}

//	For some reason, <kernelspecs.h> defines HIGH_LEVEL, which ends up 
//	conflicting with a lot of other definitions.

#ifdef HIGH_LEVEL
#undef HIGH_LEVEL
#endif

//	Debugging defines

#ifdef DEBUG
//#define DEBUG_MEMORY_LEAKS
//#define DEBUG_STRING_LEAKS
//#define DEBUG_ARRAY_STATS
#endif

//	Undefine some common names

#undef DrawText
#undef LoadImage
#undef PlaySound

//	Explicit placement operator
struct placement_new_class { };
extern placement_new_class placement_new;
inline void *operator new (size_t, ::placement_new_class, void *p) { return p; }

#ifdef DEBUG_MEMORY_LEAKS
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

//	HACK: Declare _alloca so that we don't have to include malloc.h
extern "C" void *          __cdecl _alloca(size_t);

namespace Kernel {

template <class T> inline T max(T a, T b) { return a > b ? a : b; }
template <class T> inline T min(T a, T b) { return a < b ? a : b; }

//	Define ASSERT macro, if necessary

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(exp)						\
			{							\
			if (!(exp))					\
				DebugBreak();			\
			}
#else
#define ASSERT(exp)
#endif
#endif

//	Call stack logging

#define DEBUG_TRY					try {
#define DEBUG_CATCH					} catch (...) { kernelDebugLogPattern("Crash in %s", __FUNCTION__); throw; }
#define DEBUG_CATCH_MT				} catch (...) { m_cs.Lock(); kernelDebugLogPattern("Crash in %s", __FUNCTION__); m_cs.Unlock(); throw; }
#define DEBUG_CATCH_CONTINUE		} catch (...) { kernelDebugLogPattern("Crash in %s", __FUNCTION__); }
#define DEBUG_CATCH_CONTINUE_MT		} catch (...) { m_cs.Lock(); kernelDebugLogPattern("Crash in %s", __FUNCTION__); m_cs.Unlock(); }
#define DEBUG_CATCH_MSG(msg)		} catch (...) { kernelDebugLogPattern((msg)); throw; }
#define DEBUG_CATCH_MSG_MT(msg)		} catch (...) { m_cs.Lock(); kernelDebugLogPattern((msg)); m_cs.Unlock(); throw; }
#define DEBUG_CATCH_MSG1(msg,p1)	} catch (...) { kernelDebugLogPattern((msg),(p1)); throw; }
#define DEBUG_CATCH_MSG1_MT(msg,p1)	} catch (...) { m_cs.Lock(); kernelDebugLogPattern((msg),(p1)); m_cs.Unlock(); throw; }

#define INLINE_DECREF				TRUE

//	Error definitions

typedef DWORD ALERROR;

#ifndef NOERROR
#define NOERROR									0
#endif

#define ERR_FAIL								1	//	Generic failure
#define ERR_MEMORY								2	//	Out of memory
#define ERR_ENDOFFILE							3	//	Read past end of file
#define ERR_CANCEL								4	//	User canceled operation
#define ERR_NOTFOUND							5	//	Entry not found
#define ERR_FILEOPEN							6	//	Unable to open file
#define ERR_CLASSNOTFOUND						7	//	Constructor for class not found
#define ERR_OUTOFDATE							8	//	Not latest version
#define ERR_MORE								9	//	More needed
#define ERR_WIN32_EXCEPTION						10	//	Win32 exception
#define ERR_OUTOFROOM							11	//	Unable to insert
#define ERR_FILE_IN_USE							12	//	File is in use

#define ERR_MODULE						0x00010000	//	First module error message (see GlobalErr.h)
#define ERR_APPL						0x01000000	//	First application error message
#define ERR_FLAG_DISPLAYED				0x80000000	//	Error message already displayed

inline BOOL ErrorWasDisplayed (ALERROR error) { return (error & ERR_FLAG_DISPLAYED) ? TRUE : FALSE; }
inline ALERROR ErrorSetDisplayed (ALERROR error) { return error | ERR_FLAG_DISPLAYED; }
inline ALERROR ErrorCode (ALERROR error) { return error & ~ERR_FLAG_DISPLAYED; }

//	Miscellaneous macros

inline int Absolute (int iValue) { return (iValue < 0 ? -iValue : iValue); }
inline double Absolute (double rValue) { return (rValue < 0.0 ? -rValue : rValue); }
inline int AlignDown (int iValue, int iGranularity) { return (iValue / iGranularity) * iGranularity; }
inline int AlignDownSigned (int iValue, int iGranularity) { return ((iValue + (iValue < 0 ? (1 - iGranularity) : 0)) / iGranularity) * iGranularity; }
inline int AlignUp (int iValue, int iGranularity) { return ((iValue + (iGranularity - 1)) / iGranularity) * iGranularity; }
inline int AlignUpSigned (int iValue, int iGranularity) { return ((iValue + (iValue < 0 ? 0 : (iGranularity - 1))) / iGranularity) * iGranularity; }
inline int ClockMod (int iValue, int iDivisor) { int iResult = (iValue % iDivisor); return (iResult < 0 ? iResult + iDivisor : iResult); }
inline int ClockDiff (int iValue, int iOrigin, int iDivisor)
	{
	int iDiff = ClockMod(iValue - iOrigin, iDivisor);
	int iHalfDiv = iDivisor / 2;
	if (iDiff <= iHalfDiv)
		return iDiff;
	else
		return iDiff - iDivisor;
	}
inline BOOL IsShiftDown (void) { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE; }
inline BOOL IsControlDown (void) { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? TRUE : FALSE; }
inline int Sign (int iValue) { return (iValue == 0 ? 0 : (iValue > 0 ? 1 : -1)); }
template <class VALUE> VALUE Clamp (VALUE x, VALUE a, VALUE b)
	{
	return (x < a ? a : (x > b ? b : x));
	}
template <class VALUE> VALUE Max (VALUE a, VALUE b)
	{
	return (a > b ? a : b);
	}
template <class VALUE> VALUE Min (VALUE a, VALUE b)
	{
	return (a < b ? a : b);
	}
template <class VALUE> void Swap (VALUE &a, VALUE &b)
	{
	VALUE temp = a;
	a = b;
	b = temp;
	}

#ifdef NOMINMAX
template <class VALUE> VALUE max (VALUE a, VALUE b)
	{
	return (a > b ? a : b);
	}
template <class VALUE> VALUE min (VALUE a, VALUE b)
	{
	return (a < b ? a : b);
	}

inline int max (int a, LONG b) { return (a > b ? a : b); }
inline int max (LONG a, int b) { return (a > b ? a : b); }
inline size_t max (int a, size_t b) { return ((size_t)max(0,a) > b ? (size_t)max(0,a) : b); }
inline size_t max (size_t a, int b) { return (a > (size_t)max(0,b) ? a : (size_t)max(0,b)); }
inline int min (int a, LONG b) { return (a < b ? a : b); }
inline int min (LONG a, int b) { return (a < b ? a : b); }
inline size_t min (int a, size_t b) { return ((size_t)max(0,a) < b ? (size_t)max(0,a) : b); }
inline size_t min (size_t a, int b) { return (a < (size_t)max(0,b) ? a : (size_t)max(0,b)); }
#endif

inline int RectHeight(RECT *pRect) { return pRect->bottom - pRect->top; }
inline int RectHeight(const RECT &Rect) { return Rect.bottom - Rect.top; }
inline int RectWidth(RECT *pRect) { return pRect->right - pRect->left; }
inline int RectWidth(const RECT &Rect) { return Rect.right - Rect.left; }
inline bool PtInRect(const RECT *pRect, int x, int y) { return (x >= pRect->left && x <= pRect->right && y >= pRect->top && y <= pRect->bottom); }
inline bool PtInRect(const RECT *pRect, const POINT& pt) { return PtInRect(pRect, pt.x, pt.y); }
inline void RectCenter (const RECT &rcRect, int *retx, int *rety) { *retx = rcRect.left + (RectWidth(rcRect) / 2); *rety = rcRect.top + (RectHeight(rcRect) / 2); }
inline bool RectsIntersect(const RECT &R1, const RECT &R2)
	{
	return (R1.right >= R2.left)
			&& (R1.left < R2.right)
			&& (R1.bottom >= R2.top)
			&& (R1.top < R2.bottom);
	}
inline bool RectEncloses (RECT *pR1, RECT *pR2)
	{
	return (pR1->left <= pR2->left)
			&& (pR1->right >= pR2->right)
			&& (pR1->top <= pR2->top)
			&& (pR1->bottom >= pR2->bottom);
	}
inline bool RectEquals (const RECT &rc1, const RECT &rc2)
	{
	return (rc1.left == rc2.left
			&& rc1.top == rc2.top
			&& rc1.right == rc2.right
			&& rc1.bottom == rc2.bottom);
	}
inline void RectInit (RECT *pRect)
	{
	pRect->left = 0;
	pRect->top = 0;
	pRect->right = 0;
	pRect->bottom = 0;
	}

const int MAX_SHORT =					32767;

//	API flags

#define API_FLAG_MASKBLT				0x00000001	//	MaskBlt is available
#define API_FLAG_WINNT					0x00000002	//	Running on Windows NT
#define API_FLAG_DWM					0x00000004	//	Desktop Window Manager running (Vista or Win7)

//	Forward class definitions

class CArchiver;
class CObject;
class CUnarchiver;
class CIDTable;
class IReadStream;
class IWriteStream;

//	Templates and Structures

#include "KernelString.h"
#include "KernelExceptions.h"

#include "TSmartPtr.h"
#include "TArray.h"
#include "TLinkedList.h"
#include "TMap.h"
#include "TQueue.h"
#include "TStack.h"

//	TimeDate classes

#define SECONDS_PER_DAY					(60 * 60 * 24)

class CTimeDate
	{
	public:
		enum Constants
			{
			Now,
			Today,
			};

		CTimeDate (void);
		CTimeDate (Constants Init);
		CTimeDate (const SYSTEMTIME &Time);
		CTimeDate (int iDaysSince1AD, int iMillisecondsSinceMidnight);

		operator SYSTEMTIME () const { return m_Time; }

		int Year (void) const { return m_Time.wYear; }
		int Month (void) const { return m_Time.wMonth; }
		int Day (void) const { return m_Time.wDay; }
		int Hour (void) const { return m_Time.wHour; }
		int Minute (void) const { return m_Time.wMinute; }
		int Second (void) const { return m_Time.wSecond; }
		int Millisecond (void) const { return m_Time.wMilliseconds; }

		int Compare (const CTimeDate &Src) const;
		int DayOfWeek (void) const;
		int DaysSince1AD (void) const;
		CString Format (const CString &sFormat) const;
		int MillisecondsSinceMidnight (void) const;
		bool Parse (const CString &sFormat, const CString &sValue, CString *retsError = NULL);
		CTimeDate ToLocalTime (void) const;

	private:
		SYSTEMTIME m_Time;
	};

class CTimeSpan
	{
	public:
		CTimeSpan (void);
		CTimeSpan (int iMilliseconds);
		CTimeSpan (int iDays, int iMilliseconds);

		static bool Parse (const CString &sValue, CTimeSpan *retValue);

		int Days (void) const { return (int)m_Days; }
		int Seconds (void) const { return (SECONDS_PER_DAY * m_Days) + (m_Milliseconds / 1000); }
		int Milliseconds (void) const { return (SECONDS_PER_DAY * 1000 * m_Days) + m_Milliseconds; }
		int MillisecondsSinceMidnight (void) const { return (int)m_Milliseconds; }

		CString Encode (void) const;
		CString Format (const CString &sFormat) const;
		bool IsBlank (void) const { return (m_Days == 0 && m_Milliseconds == 0); }
		void ReadFromStream (IReadStream *pStream);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		static bool ParsePartial (const char *pPos, DWORD *retdwDays, DWORD *retdwMilliseconds, const char **retpPos);

		DWORD m_Days;
		DWORD m_Milliseconds;
	};

const CTimeSpan operator+ (const CTimeSpan &op1, const CTimeSpan &op2);
const CTimeSpan operator- (const CTimeSpan &op1, const CTimeSpan &op2);

CTimeDate timeAddTime (const CTimeDate &StartTime, const CTimeSpan &Addition);
CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime);
CTimeDate timeSubtractTime (const CTimeDate &StartTime, const CTimeSpan &Subtraction);
bool timeIsLeapYear (int iYear);

//	High-performance timer classes --------------------------------------------

class CPeriodicWaiter
	{
	public:
		CPeriodicWaiter (DWORD dwPeriod);

		void Wait (void);

	private:
		DWORD m_dwPeriod;
		LONGLONG m_LastCounter;

		LONGLONG m_PCFreq;
		LONGLONG m_PCCountsPerPeriod;
	};

//	Object class ID definitions

typedef DWORD OBJCLASSID;

#define OBJCLASS_MODULE_MASK					0xFFF00000
#define OBJCLASS_MODULE_SHIFT					20

#define OBJCLASS_MODULE_KERNEL					0
#define OBJCLASS_MODULE_APPLICATION				1
#define OBJCLASS_MODULE_GLOBAL					2
#define OBJCLASS_MODULE_COUNT					3

inline constexpr OBJCLASSID MakeOBJCLASSIDExt (int iModule, int iID) { return (((DWORD)iModule) << OBJCLASS_MODULE_SHIFT) + (DWORD)iID; }
inline constexpr OBJCLASSID MakeOBJCLASSID (int iID) { return MakeOBJCLASSIDExt(OBJCLASS_MODULE_APPLICATION, iID); }
inline constexpr OBJCLASSID MakeGlobalOBJCLASSID (int iID) { return MakeOBJCLASSIDExt(OBJCLASS_MODULE_GLOBAL, iID); }
inline constexpr int OBJCLASSIDGetID (OBJCLASSID ObjID) { return (int)(ObjID & ~OBJCLASS_MODULE_MASK); }
inline constexpr int OBJCLASSIDGetModule (OBJCLASSID ObjID) { return (int)((ObjID & OBJCLASS_MODULE_MASK) >> OBJCLASS_MODULE_SHIFT); }

//	Object data description

#define DATADESC_OPCODE_STOP				0	//	No more entries
#define DATADESC_OPCODE_INT					1	//	32-bit integer (iCount valid)
#define DATADESC_OPCODE_REFERENCE			2	//	Reference to memory location or object (iCount valid)
#define DATADESC_OPCODE_ALLOC_OBJ			3	//	Pointer to owned object (derived from CObject)
#define DATADESC_OPCODE_EMBED_OBJ			4	//	Embedded object (derived from CObject)
#define DATADESC_OPCODE_ZERO				5	//	32-bit of zero-init data (iCount valid)
#define DATADESC_OPCODE_VTABLE				6	//	This is a vtable (which is initialized by new) (iCount is valid)
#define DATADESC_OPCODE_ALLOC_SIZE32		7	//	Number of 32-bit words allocated in the following memory block
#define DATADESC_OPCODE_ALLOC_MEMORY		8	//	Block of memory; previous must be ALLOC_SIZE

#define DATADESC_FLAG_CUSTOM		0x00000001	//	Object handles saving this part

typedef struct
	{
	int iOpCode:8;								//	Op-code
	int iCount:8;								//	Count
	DWORD dwFlags:16;							//	Miscellaneous flags
	} DATADESCSTRUCT, *PDATADESCSTRUCT;

//	Abstract object class

class IObjectClass
	{
	public:
		IObjectClass (OBJCLASSID ObjID, PDATADESCSTRUCT pDataDesc) : m_ObjID(ObjID), m_pDataDesc(pDataDesc) { }

		PDATADESCSTRUCT GetDataDesc (void) { return m_pDataDesc; }
		OBJCLASSID GetObjID (void) { return m_ObjID; }
		virtual CObject *Instantiate (void) = 0;
		virtual int GetObjSize (void) = 0;

	private:
		OBJCLASSID m_ObjID;
		PDATADESCSTRUCT m_pDataDesc;
	};

//	Base object class

class CObject
	{
	public:
		CObject (IObjectClass *pClass);
		virtual ~CObject (void);

		CObject *Copy (void);
		IObjectClass *GetClass (void) { return m_pClass; }
		static bool IsValidPointer (CObject *pObj);
		ALERROR Load (CUnarchiver *pUnarchiver);
		ALERROR LoadDone (void);
		ALERROR Save (CArchiver *pArchiver);

		static ALERROR Flatten (CObject *pObject, CString *retsData);
		static ALERROR Unflatten (CString sData, CObject **retpObject);

	protected:
		virtual void CopyHandler (CObject *pOriginal) { }
		virtual ALERROR LoadCustom (CUnarchiver *pUnarchiver, BYTE *pDest) { return NOERROR; }
		virtual ALERROR LoadDoneHandler (void) { return NOERROR; }
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual LPVOID MemAlloc (int iSize) { return (BYTE *)HeapAlloc(GetProcessHeap(), 0, iSize); }
		virtual void MemFree (LPVOID pMem) { HeapFree(GetProcessHeap(), 0, pMem); }
		virtual ALERROR SaveCustom (CArchiver *pArchiver, BYTE *pSource) { return NOERROR; }
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		CObject *Clone (void) { return m_pClass->Instantiate(); }
		BOOL CopyData (PDATADESCSTRUCT pPos, BYTE **iopSource, BYTE **iopDest);
		PDATADESCSTRUCT DataDescNext (PDATADESCSTRUCT pPos);
		PDATADESCSTRUCT DataDescStart (void);
		BYTE *DataStart (void);
		void VerifyDataDesc (void);

		IObjectClass *m_pClass;
	};

//	Factory for creating objects

class CObjectClassFactory
	{
	public:
		static CObject *Create (OBJCLASSID ObjID);
		static IObjectClass *GetClass (OBJCLASSID ObjID);
		static void NewClass (IObjectClass *pClass);
	};

//	Template for object classes

template <class T>
class CObjectClass : public IObjectClass
	{
	public:
		CObjectClass (OBJCLASSID ObjID, PDATADESCSTRUCT pDataDesc = NULL)
				: IObjectClass(ObjID, pDataDesc)
				{ CObjectClassFactory::NewClass(this); }
 
		virtual CObject *Instantiate (void) { return new T; }
		virtual int GetObjSize (void) { return sizeof(T); }
	};

//	Synchronization -----------------------------------------------------------

class CCriticalSection
	{
	public:
		CCriticalSection (void) { ::InitializeCriticalSection(&m_cs); }
		~CCriticalSection (void) { ::DeleteCriticalSection(&m_cs); }

		void Lock (void) { ::EnterCriticalSection(&m_cs); }
		void Unlock (void) { ::LeaveCriticalSection(&m_cs); }

	private:
		CRITICAL_SECTION m_cs;
	};

class CSmartLock
	{
	public:
		CSmartLock(CCriticalSection &cs) : m_cs(cs) { m_cs.Lock(); }
		~CSmartLock (void) { m_cs.Unlock(); }

	private:
		CCriticalSection &m_cs;
	};

class COSObject
	{
	public:
		COSObject (void) : m_hHandle(INVALID_HANDLE_VALUE) { }
		~COSObject (void) { if (m_hHandle != INVALID_HANDLE_VALUE) ::CloseHandle(m_hHandle); }

		void Close (void) { if (m_hHandle != INVALID_HANDLE_VALUE) { ::CloseHandle(m_hHandle); m_hHandle = INVALID_HANDLE_VALUE; } }
		HANDLE GetWaitObject (void) const { return m_hHandle; }
		void TakeHandoff (COSObject &Obj) { Close(); m_hHandle = Obj.m_hHandle; Obj.m_hHandle = INVALID_HANDLE_VALUE; }
		bool Wait (DWORD dwTimeout = INFINITE) const { return (::WaitForSingleObject(m_hHandle, dwTimeout) != WAIT_TIMEOUT); }

	protected:
		HANDLE m_hHandle;
	};

class CManualEvent : public COSObject
	{
	public:
		void Create (void);
		void Create (const CString &sName, bool *retbExists = NULL);
		bool IsSet (void) { return (::WaitForSingleObject(m_hHandle, 0) == WAIT_OBJECT_0); }
		void Reset (void) { ::ResetEvent(m_hHandle); }
		void Set (void) { ::SetEvent(m_hHandle); }
	};

//	CINTDynamicArray. Implementation of a dynamic array.
//	(NOTE: To save space, this class does not have a virtual
//	destructor. Do not sub-class this class without taking that into account).

class CINTDynamicArray
	{
	public:
		CINTDynamicArray (void);
		CINTDynamicArray (HANDLE hHeap);
		~CINTDynamicArray (void);

		ALERROR Append (BYTE *pData, int iLength, int iAllocQuantum)
			{ return Insert(-1, pData, iLength, iAllocQuantum); }
		ALERROR Delete (int iOffset, int iLength);
		ALERROR DeleteAll (void) { return Delete(0, m_iLength); }
		int GetLength (void) const { return m_iLength; }
		void SetLength (int iLength) { m_iLength = iLength; }
		BYTE *GetPointer (int iOffset) const { return (m_pArray ? m_pArray + iOffset : NULL); }
		ALERROR Insert (int iOffset, BYTE *pData, int iLength, int iAllocQuantum);
		ALERROR Resize (int iNewSize, BOOL bPreserve, int iAllocQuantum);

	private:
		int m_iLength;							//	Length of the array in bytes
		int m_iAllocSize;						//	Allocated size of the array
		HANDLE m_hHeap;							//	Heap to use
		BYTE *m_pArray;							//	Array data
	};

//	CIntArray. Implementation of a dynamic array of integers

class CIntArray : public CObject
	{
	public:
		CIntArray (void);
		virtual ~CIntArray (void);

		CIntArray &operator= (const CIntArray &Obj);

		ALERROR AppendElement (intptr_t iElement, int *retiIndex = NULL);
		ALERROR CollapseArray (int iPos, int iCount) { return RemoveRange(iPos, iPos + iCount - 1); }
		ALERROR ExpandArray (int iPos, int iCount);
		int FindElement (intptr_t iElement) const;
		int GetCount (void) const;
		intptr_t GetElement (int iIndex) const;
		ALERROR InsertElement (intptr_t iElement, int iPos, int *retiIndex);
		ALERROR InsertRange (CIntArray *pList, int iStart, int iEnd, int iPos);
		ALERROR MoveRange (int iStart, int iEnd, int iPos);
		ALERROR Set (int iCount, intptr_t *pData);
		ALERROR RemoveAll (void);
		ALERROR RemoveElement (int iPos) { return RemoveRange(iPos, iPos); }
		ALERROR RemoveRange (int iStart, int iEnd);
		void ReplaceElement (int iPos, intptr_t iElement);
		void Shuffle (void);

	private:
		int m_iAllocSize;					//	Number of integers allocated
		intptr_t *m_pData;					//	Pointer to integer array
		int m_iLength;						//	Number of integers used
	};

//	CString. Implementation of a standard string class

static constexpr char CHAR_LEFT_DOUBLE_QUOTE =		'\x93';
static constexpr char CHAR_RIGHT_DOUBLE_QUOTE =		'\x94';

static constexpr SConstString CSTR_LEFT_DOUBLE_QUOTE =	CONSTDEFS("\x93");
static constexpr SConstString CSTR_RIGHT_DOUBLE_QUOTE =	CONSTDEFS("\x94");
static constexpr SConstString CSTR_BULLET =				CONSTDEFS("\x95");
static constexpr SConstString CSTR_MDASH =				CONSTDEFS("\x97");
static constexpr SConstString CSTR_TRADEMARK =			CONSTDEFS("\x99");
static constexpr SConstString CSTR_COPYRIGHT =			CONSTDEFS("\xA9");
static constexpr SConstString CSTR_REGISTERED =			CONSTDEFS("\xAE");
static constexpr SConstString CSTR_DEGREE =				CONSTDEFS("\xB0");
static constexpr SConstString CSTR_PLUS_MINUS =			CONSTDEFS("\xB1");

static constexpr SConstString CSTR_CAP_AACUTE =			CONSTDEFS("\xC1");
static constexpr SConstString CSTR_CAP_EACUTE =			CONSTDEFS("\xC9");
static constexpr SConstString CSTR_CAP_IACUTE =			CONSTDEFS("\xCD");
static constexpr SConstString CSTR_CAP_NTILDE =			CONSTDEFS("\xD1");
static constexpr SConstString CSTR_CAP_OACUTE =			CONSTDEFS("\xD3");
static constexpr SConstString CSTR_TIMES =				CONSTDEFS("\xD7");
static constexpr SConstString CSTR_CAP_UACUTE =			CONSTDEFS("\xDA");
static constexpr SConstString CSTR_CAP_UUML =			CONSTDEFS("\xDC");
static constexpr SConstString CSTR_AACUTE =				CONSTDEFS("\xE1");
static constexpr SConstString CSTR_EACUTE =				CONSTDEFS("\xE9");
static constexpr SConstString CSTR_IACUTE =				CONSTDEFS("\xED");
static constexpr SConstString CSTR_NTILDE =				CONSTDEFS("\xF1");
static constexpr SConstString CSTR_OACUTE =				CONSTDEFS("\xF3");
static constexpr SConstString CSTR_UACUTE =				CONSTDEFS("\xFA");
static constexpr SConstString CSTR_UUML =				CONSTDEFS("\xFC");

static constexpr SConstString CSTR_WINGDING_LEFT_ARROW =	CONSTDEFS("\xE7");
static constexpr SConstString CSTR_WINGDING_RIGHT_ARROW =	CONSTDEFS("\xE8");

//	CDictionary. Implementation of a dynamic array of entries

class CDictionary : public CObject
	{
	public:
		CDictionary (void);
		CDictionary (IObjectClass *pClass);
		virtual ~CDictionary (void);

		ALERROR AddEntry (intptr_t iKey, intptr_t iValue);
		ALERROR Find (intptr_t iKey, intptr_t *retiValue) const;
		ALERROR FindEx (intptr_t iKey, int *retiEntry) const;
		ALERROR FindOrAdd (intptr_t iKey, intptr_t iValue, bool *retbFound, intptr_t *retiValue);
		int GetCount (void) const { return m_Array.GetCount() / 2; }
		void GetEntry (int iEntry, intptr_t *retiKey, intptr_t *retiValue) const;
		ALERROR ReplaceEntry (intptr_t iKey, intptr_t iValue, bool bAdd, bool *retbAdded, intptr_t *retiOldValue);
		ALERROR RemoveAll (void) { return m_Array.RemoveAll(); }
		ALERROR RemoveEntryByOrdinal (int iEntry, intptr_t *retiOldValue = NULL);
		ALERROR RemoveEntry (intptr_t iKey, intptr_t *retiOldValue);

	protected:
		virtual int Compare (intptr_t iKey1, intptr_t iKey2) const;
		ALERROR ExpandArray (int iPos, int iCount) { return m_Array.ExpandArray(2 * iPos, 2 * iCount); }
		void SetEntry (int iEntry, intptr_t iKey, intptr_t iValue);

		bool FindSlot (intptr_t iKey, int *retiPos) const;

		CIntArray m_Array;
	};

//	CIDTable. Implementation of a table that matches IDs with objects

class CIDTable : public CDictionary
	{
	public:
		CIDTable (void);
		CIDTable (BOOL bOwned, BOOL bNoReference);
		virtual ~CIDTable (void);

		ALERROR AddEntry (int iKey, CObject *pValue);
		int GetKey (int iEntry) const;
		CObject *GetValue (int iEntry) const;
		ALERROR Lookup (int iKey, CObject **retpValue) const;
		ALERROR LookupEx (int iKey, int *retiEntry) const;
		ALERROR RemoveAll (void);
		ALERROR RemoveEntry (int iKey, CObject **retpOldValue);
		ALERROR ReplaceEntry (int iKey, CObject *pValue, bool bAdd, CObject **retpOldValue);
		void SetValue (int iEntry, CObject *pValue, CObject **retpOldValue);

	protected:
		virtual int Compare (intptr_t iKey1, intptr_t iKey2) const;
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		BOOL m_bNoReference;
	};

//	CSymbolTable. Implementation of a symbol table

class CSymbolTable : public CDictionary
	{
	public:
		CSymbolTable (void);
		CSymbolTable (BOOL bOwned, BOOL bNoReference);
		virtual ~CSymbolTable (void);
		CSymbolTable &operator= (const CSymbolTable &Obj);

		ALERROR AddEntry (const CString &sKey, CObject *pValue);
		CString GetKey (int iEntry) const;
		CObject *GetValue (int iEntry) const;
		ALERROR Lookup (const CString &sKey, CObject **retpValue = NULL) const;
		ALERROR LookupEx (const CString &sKey, int *retiEntry) const;
		ALERROR RemoveAll (void);
		ALERROR RemoveEntry (int iEntry, CObject **retpOldValue = NULL);
		ALERROR RemoveEntry (const CString &sKey, CObject **retpOldValue);
		ALERROR ReplaceEntry (const CString &sKey, CObject *pValue, bool bAdd, CObject **retpOldValue);
		void SetValue (int iEntry, CObject *pValue, CObject **retpOldValue);

	protected:
		virtual int Compare (intptr_t iKey1, intptr_t iKey2) const;
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		BOOL m_bNoReference;
	};

//	CAtomTable. Implementation of a string hash table

class CAtomTable : public CObject
	{
	public:
		CAtomTable (void);
		CAtomTable (int iHashSize);
		virtual ~CAtomTable (void);

		ALERROR AppendAtom (const CString &sString, int *retiAtom);
		int Atomize (const CString &sString);

	private:
		CSymbolTable *Hash (const CString &sString);

		int m_iHashSize;
		int m_iNextAtom;
		CSymbolTable *m_pBackbone;
	};

//	CLargeSet

class CLargeSet
	{
	public:
		enum EConstants
			{
			INVALID_VALUE = 0xFFFFFFFF,
			};

		CLargeSet (int iSize = -1);

		void Clear (DWORD dwValue);
		void ClearAll (void);
		DWORD GetNextValue (DWORD dwStart = 0) const;
		bool InitFromString (const CString &sValue, DWORD dwMaxValue = 0, CString *retsError = NULL);
		bool IsEmpty (void) const;
		bool IsSet (DWORD dwValue) const;
		void Set (DWORD dwValue);

	private:
		static DWORD GetBitFromMask (DWORD dwMask);

		TArray<DWORD> m_Set;
	};

//	Atomizer

class CAtomizer
	{
	public:
		CAtomizer (void);

		DWORD Atomize (const CString &sIdentifier);
		int GetCount (void) const { return m_StringToAtom.GetCount(); }
		const CString &GetIdentifier (DWORD dwAtom) const;
		int GetMemoryUsage (void) const;

	private:
		DWORD m_dwNextID;
		TSortMap<CString, DWORD> m_StringToAtom;
		TArray<CString> m_AtomToString;
	};

//	Memory Blocks

class IReadBlock
	{
	public:
		virtual ~IReadBlock (void) { }

		virtual ALERROR Close (void) = 0;
		virtual ALERROR Open (void) = 0;
		virtual int GetLength (void) = 0;
		virtual char *GetPointer (int iOffset, int iLength = -1) = 0;
	};

class CFileReadBlock : public CObject, public IReadBlock
	{
	public:
		CFileReadBlock (void);
		CFileReadBlock (const CString &sFilename);
		virtual ~CFileReadBlock (void);

		const CString &GetFilename (void) const { return m_sFilename; }

		//	IReadBlock virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual int GetLength (void) { return (int)m_dwFileSize; }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_pFile + iOffset; }

	private:
		CString m_sFilename;
		HANDLE m_hFile;
		HANDLE m_hFileMap;
		char *m_pFile;
		DWORD m_dwFileSize;
	};

class CResourceReadBlock : public CObject, public IReadBlock
	{
	public:
		CResourceReadBlock (void);
		CResourceReadBlock (HMODULE hInst, const char *pszRes, const char *pszType = RT_RCDATA);
		virtual ~CResourceReadBlock (void);

		//	IReadBlock virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual int GetLength (void) { return m_dwLength; }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_pData + iOffset; }

	private:
		HMODULE m_hModule;
		const char *m_pszRes;
		const char *m_pszType;

		char *m_pData;
		DWORD m_dwLength;
	};

class CBufferReadBlock : public CObject, public IReadBlock
	{
	public:
		CBufferReadBlock (void) : CObject(NULL) { }
		CBufferReadBlock (const CString &sData) : CObject(NULL), m_sData(sData) { }

		//	IReadBlock virtuals

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { return NOERROR; }
		virtual int GetLength (void) { return m_sData.GetLength(); }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_sData.GetPointer() + iOffset; }

	private:
		CString m_sData;
	};

//	File System

class IWriteStream
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Create (void) = 0;
		virtual ALERROR Write (const char *pData, int iLength, int *retiBytesWritten = NULL) = 0;

		ALERROR Write (char chChar, int iLength = 1) { return WriteChar(chChar, iLength); }
		ALERROR Write (int iValue) { return Write((char *)&iValue, sizeof(DWORD)); }
		ALERROR Write (DWORD dwValue) { return Write((char *)&dwValue, sizeof(DWORD)); }
		ALERROR Write (double rValue) { return Write((char *)&rValue, sizeof(double)); }
		ALERROR Write (LONGLONG iValue) { return Write((char *)&iValue, sizeof(LONGLONG)); }
		ALERROR Write (DWORDLONG iValue) { return Write((char *)&iValue, sizeof(DWORDLONG)); }
		ALERROR Write (const CString &sString) { return Write(sString.GetPointer(), sString.GetLength()); }

		ALERROR WriteChar (char chChar, int iLength = 1);
		ALERROR WriteChars (const CString &sString, int *retiBytesWritten = NULL) { return Write(sString.GetASCIIZPointer(), sString.GetLength(), retiBytesWritten); }
	};

class IReadStream
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Open (void) = 0;
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL) = 0;

		ALERROR Read (int &iValue) { return Read((char *)&iValue, sizeof(DWORD)); }
		ALERROR Read (DWORD &dwValue) { return Read((char *)&dwValue, sizeof(DWORD)); }
		ALERROR Read (double &rValue) { return Read((char *)&rValue, sizeof(double)); }
		ALERROR Read (LONGLONG &iValue) { return Read((char *)&iValue, sizeof(LONGLONG)); }
		ALERROR Read (DWORDLONG &iValue) { return Read((char *)&iValue, sizeof(DWORDLONG)); }
	};

//	CMemoryWriteStream. This object is used to write variable length
//	data to a memory block.

class CMemoryWriteStream : public CObject, public IWriteStream
	{
	public:
		CMemoryWriteStream (int iMaxSize = DEFAULT_MAX_SIZE);
		virtual ~CMemoryWriteStream (void);

		char *GetPointer (void) { return m_pBlock; }
		int GetLength (void) { return m_iCurrentSize; }
		void Seek (int iPos);

		//	IWriteStream virtuals

		virtual ALERROR Close (void) override;
		virtual ALERROR Create (void) override;
		virtual ALERROR Write (const char *pData, int iLength, int *retiBytesWritten = NULL) override;

		//	We want to inherit all the overloaded versions of Write.

		using IWriteStream::Write;

	private:
		enum Constants
			{
			DEFAULT_MAX_SIZE = 			(1024 * 1024),
			};

		int m_iMaxSize;
		int m_iCommittedSize;
		int m_iCurrentSize;
		char *m_pBlock;
	};

class CMemoryReadBlockWrapper : public IReadBlock
	{
	public:
		CMemoryReadBlockWrapper (CMemoryWriteStream &Stream) : 
				m_pPointer(Stream.GetPointer()),
				m_iLength(Stream.GetLength())
			{ }

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { return NOERROR; }
		virtual int GetLength (void) { return m_iLength; }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_pPointer + iOffset; }

	private:
		char *m_pPointer;
		int m_iLength;
	};

//	CMemoryReadStream. This object is used to read variable length data

class CMemoryReadStream : public CObject, public IReadStream
	{
	public:
		CMemoryReadStream (void);
		CMemoryReadStream (char *pData, int iDataSize);
		virtual ~CMemoryReadStream (void);

		//	IReadStream virtuals

		virtual ALERROR Close (void) override { return NOERROR; }
		virtual ALERROR Open (void) override { m_iPos = 0; return NOERROR; }
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL) override;

		//	We want to inherit all the overloaded versions of Read.

		using IReadStream::Read;

	private:
		char *m_pData;
		int m_iDataSize;
		int m_iPos;
	};

//	CFileWriteStream. This object is used to write a file out

class CFileWriteStream : public CObject, public IWriteStream
	{
	public:
		CFileWriteStream (void);
		CFileWriteStream (const CString &sFilename, BOOL bUnique = FALSE);
		virtual ~CFileWriteStream (void);

		ALERROR Open (void);

		//	IWriteStream virtuals

		virtual ALERROR Close (void) override;
		virtual ALERROR Create (void) override;
		virtual ALERROR Write (const char *pData, int iLength, int *retiBytesWritten = NULL) override;

		//	We want to inherit all the overloaded versions of Write.

		using IWriteStream::Write;

	private:
		CString m_sFilename;
		BOOL m_bUnique;
		HANDLE m_hFile;
	};

//	CFileReadStream. This object is used to read a file in

class CFileReadStream : public CObject, public IReadStream
	{
	public:
		CFileReadStream (void);
		CFileReadStream (const CString &sFilename);
		virtual ~CFileReadStream (void);

		DWORD GetFileSize (void) { return m_dwFileSize; }

		//	IReadStream virtuals

		virtual ALERROR Close (void) override;
		virtual ALERROR Open (void) override;
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL) override;

		//	We want to inherit all the overloaded versions of Read.

		using IReadStream::Read;

	private:
		CString m_sFilename;
		HANDLE m_hFile;
		HANDLE m_hFileMap;
		char *m_pFile;
		char *m_pPos;
		DWORD m_dwFileSize;
	};

//	CArchive. This is an object that knows how to archive objects to a 
//	stream.

class CArchiver : public CObject
	{
	public:
		CArchiver (void);
		CArchiver (IWriteStream *pStream);
		virtual ~CArchiver (void);

		ALERROR AddExternalReference (CString sTag, void *pReference);
		ALERROR AddObject (CObject *pObject);
		ALERROR BeginArchive (void);
		ALERROR EndArchive (void);
		void SetVersion (DWORD dwVersion) { m_dwVersion = dwVersion; }

		//	These methods should only be called by objects
		//	that are being saved

		ALERROR Reference2ID (void *pReference, int *retiID);
		ALERROR SaveObject (CObject *pObject);
		ALERROR SaveObject (CString *pObject);
		ALERROR WriteData (char *pData, int iLength);

	private:
		IWriteStream *m_pStream;					//	Stream to save to
		TArray<CObject *> m_List;					//	List of objects to save
		CDictionary m_ReferenceList;				//	Pointer references
		CSymbolTable m_ExternalReferences;			//	List of external references
		int m_iNextID;								//	Next ID to use for references
		DWORD m_dwVersion;							//	User-defined version
	};

//	CUnarchiver. This is an object that knows how to load objects from
//	a stream.

class CUnarchiver : public CObject
	{
	public:
		CUnarchiver (void);
		CUnarchiver (IReadStream *pStream);
		virtual ~CUnarchiver (void);

		ALERROR BeginUnarchive (void);
		ALERROR EndUnarchive (void);
		TArray<CObject *> &GetList (void) { return m_List; }
		CObject *GetObject (int iIndex);
		DWORD GetVersion (void) { return m_dwVersion; }
		ALERROR ResolveExternalReference (CString sTag, void *pReference);
		void SetMinVersion (DWORD dwVersion) { m_dwMinVersion = dwVersion; }

		//	These methods should only be called by objects
		//	that are being loaded

		ALERROR LoadObject (CObject **retpObject);
		ALERROR LoadObject (CString **retpString);
		ALERROR ReadData (char *pData, int iLength);
		ALERROR ResolveReference (int iID, void **pReference);

	private:
		IReadStream *m_pStream;
		TArray<CObject *> m_List;
		CSymbolTable *m_pExternalReferences;
		CIntArray m_ReferenceList;
		CIntArray m_FixupTable;
		DWORD m_dwVersion;
		DWORD m_dwMinVersion;
	};

//	CDataFile. This is a file-based collection of variable-sized records.

#define DFOPEN_FLAG_READ_ONLY					0x00000001

class CDataFile : public CObject
	{
	public:
		struct SVersionInfo
			{
			DWORD dwVersion;
			int iEntry;
			};

		CDataFile (const CString &sFilename = NULL_STR);
		virtual ~CDataFile (void);

		ALERROR AddEntry (const CString &sData, int *retiEntry);
		ALERROR Close (void);
		ALERROR DeleteEntry (int iEntry);
		ALERROR Flush (void);
		CString GetFilename (void) const { return m_sFilename; }
		int GetDefaultEntry (void);
		int GetEntryLength (int iEntry);
		BOOL IsOpen (void) { return (m_hFile != INVALID_HANDLE_VALUE || m_pFile); }
		ALERROR Open (DWORD dwFlags = 0) { return Open(NULL_STR, dwFlags); }
		ALERROR Open (const CString &sFilename, DWORD dwFlags = 0);
		ALERROR OpenFromResource (HMODULE hInst, char *pszRes, DWORD dwFlags = 0);
		ALERROR ReadEntry (int iEntry, CString *retsData);
		ALERROR ReadEntryPartial (int iEntry, int iPos, int iLength, CString *retsData);
		ALERROR ReadHistory (int iEntry, TArray<SVersionInfo> *retHistory);
		void SetDefaultEntry (int iEntry);
		ALERROR WriteEntry (int iEntry, const CString &sData);
		ALERROR WriteVersion (int iEntry, const CString &sData, DWORD *retdwVersion = NULL);

		static ALERROR Create (const CString &sFilename,
							   int iBlockSize,
							   int iInitialEntries);

	private:
		typedef struct
			{
			DWORD dwBlock;								//	Block Number (-1 = unused)
			DWORD dwBlockCount;							//	Number of blocks reserved for entry
			DWORD dwSize;								//	Size of entry
			DWORD dwVersion;							//	Version number
			DWORD dwPrevEntry;							//	Previous version
			DWORD dwLatestEntry;						//	Latest entry (-1 if this is latest)
			DWORD dwFlags;								//	Misc flags
			} ENTRYSTRUCT, *PENTRYSTRUCT;

		struct SEntryV1
			{
			DWORD dwBlock;								//	Block Number (-1 = unused)
			DWORD dwBlockCount;							//	Number of blocks reserved for entry
			DWORD dwSize;								//	Size of entry
			DWORD dwFlags;								//	Misc flags
			};

		ALERROR AllocBlockChain (DWORD dwBlockCount, DWORD *retdwStartingBlock);
		ALERROR FreeBlockChain (DWORD dwStartingBlock, DWORD dwBlockCount);
		ALERROR GrowEntryTable (int *retiEntry);
		ALERROR OpenInt (void);
		ALERROR ReadBuffer (DWORD dwFilePos, DWORD dwLen, void *pBuffer);
		ALERROR ResizeEntry (int iEntry, DWORD dwSize, DWORD *retdwBlockCount);
		ALERROR WriteBlockChain (DWORD dwStartingBlock, char *pData, DWORD dwSize);

		CString m_sFilename;							//	Filename of data file
		HANDLE m_hFile;									//	Open file handle
		IReadBlock *m_pFile;							//	Memory file

		int m_iBlockSize;								//	Size of each block
		int m_iBlockCount;								//	Number of blocks in file
		int m_iDefaultEntry;							//	Default entry

		int m_iEntryTableCount;							//	Number of entries
		PENTRYSTRUCT m_pEntryTable;						//	Entry table

		DWORD m_fHeaderModified:1;						//	TRUE if header has changed
		DWORD m_fEntryTableModified:1;					//	TRUE if entry table has changed
		DWORD m_fFlushing:1;							//	TRUE if we're inside ::Flush
		DWORD m_fReadOnly:1;							//	TRUE if we're open read-only
	};

//	Directory classes

struct SFileDesc
	{
	CString sFilename;

	bool bFolder;
	bool bReadOnly;
	bool bSystemFile;
	bool bHiddenFile;
	};

class CFileDirectory
	{
	public:
		CFileDirectory (const CString &sFilespec);
		~CFileDirectory (void);

		bool HasMore (void);
		CString GetNext (bool *retbIsFolder = NULL);
		void GetNextDesc (SFileDesc *retDesc);

	private:
		CString m_sFilespec;
		HANDLE m_hSearch;
	#ifdef _WIN32
		WIN32_FIND_DATA m_FindData;
	#else
		void *m_pFindData;
	#endif
	};

//	Logging classes

#define ILOG_FLAG_WARNING					0x00000001	//	Warning log entry
#define ILOG_FLAG_ERROR						0x00000002	//	Error log entry
#define ILOG_FLAG_FATAL						0x00000004	//	Fatal error log entry
#define ILOG_FLAG_TIMEDATE					0x00000008	//	Include time date

class ILog
	{
	public:
		void LogOutput (DWORD dwFlags, char *pszLine, ...) const;
		void LogOutput (DWORD dwFlags, const CString &sLine) const;
		virtual void Print (const CString &sLine) const = 0;
		virtual void Progress (const CString &sLine, int iPercent = -1) const { Print(sLine); }
	};

class CTextFileLog : public ILog
	{
	public:
		CTextFileLog (void);
		CTextFileLog (const CString &sFilename);

		ALERROR Close (void);
		ALERROR Create (bool bAppend);
		CString GetSessionLog (void);
		void SetFilename (const CString &sFilename);
		void SetSessionStart (void);
		virtual ~CTextFileLog (void);

		//	ILog virtuals

		virtual void Print (const CString &sLine) const override;

	private:
		HANDLE m_hFile = NULL;
		CString m_sFilename;

		DWORD m_dwSessionStart = 0;			//	Offset to file at start of session
	};

//	Registry classes

class CRegKey
	{
	public:
		CRegKey (void);
		~CRegKey (void);

		static ALERROR OpenUserAppKey (const CString &sCompany, 
									   const CString &sAppName,
									   CRegKey *retKey);

		operator HKEY() const { return m_hKey; }

		bool FindStringValue (const CString &sValue, CString *retsData);
		void SetStringValue (const CString &sValue, const CString &sData);

	private:
		void CleanUp (void);

		HKEY m_hKey;
	};

//	Thread pool

class IThreadPoolTask
	{
	public:
		virtual ~IThreadPoolTask (void) { }
		virtual void Run (void) { }
	};

class CThreadPool
	{
	public:
		enum EThreadPoolState
			{
			eNotBooted,
			eBooting,
			eBooted,
			eDeleting,
			eDeleted
			};

		~CThreadPool (void) { CleanUp(); }

		void AddTask (IThreadPoolTask *pTask);
		bool Boot (int iThreadCount);
		void CleanUp (void);
		EThreadPoolState GetState (void) { return m_iState; }
		int GetThreadCount (void) const { return m_Threads.GetCount() + 1; }
		void Run (void);

	private:
		struct SThreadDesc
			{
			HANDLE hThread;
			};

		void AssertInOwnerThread (void) const { ASSERT(m_dwOwner == GetCurrentThreadId()); }
		void AssertNotInOurThreads () const;
		IThreadPoolTask *GetTaskToRun (void);
		void RunTask (IThreadPoolTask *pTask);
		void WorkerThread (void);

		static DWORD WINAPI WorkerThreadStub (LPVOID pData) { ((CThreadPool *)pData)->WorkerThread(); return 0; }

		EThreadPoolState m_iState = eNotBooted;			//	We can only boot once
		DWORD m_dwOwner = GetCurrentThreadId();			//	We only permit the owner thread to call our interfaces
		CCriticalSection m_cs = CCriticalSection();		//	Ensure our critical sections are initialized
		int m_iTasksRemaining = 0;

		//	This section is initialized in Boot

		TArray<SThreadDesc> m_Threads;
		TQueue<IThreadPoolTask *> m_Tasks;
		TArray<IThreadPoolTask *> m_Completed;
		CManualEvent m_WorkAvail;
		CManualEvent m_WorkCompleted;
		CManualEvent m_Quit;
	};

//	Initialization functions (Kernel.cpp)

void kernelCleanUp (void);
void kernelClearDebugLog (void);
void kernelDebugLogPattern (const char *pszLine, ...);
void kernelDebugLogString (const CString &sLine);
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, const CString &s2) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, s2)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, int i2) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, i2)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, int i2, const CString &s3) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, i2, s3)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, int i2, const CString &s3, int i4) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, i2, s3, i4)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, int i2, const CString &s3, int i4, const CString &s5) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, i2, s3, i4, s5)); }
inline void kernelDebugLogPattern (const char *pszLine, int i1, const CString &s2) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), i1, s2)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, const CString &s2, int i3) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, s2, i3)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, int i2, int i3) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, i2, i3)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, const CString &s2, const CString &s3) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, s2, s3)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, const CString &s2, int i3, const CString &s4) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, s2, i3, s4)); }
inline void kernelDebugLogPattern (const char *pszLine, const CString &s1, int i2, const CString &s3, const CString &s4) { kernelDebugLogString(strPatternSubst(CString(pszLine, -1, TRUE), s1, i2, s3, s4)); }
CString kernelGetSessionDebugLog (void);

#define KERNEL_FLAG_INTERNETS					0x00000001
BOOL kernelInit (DWORD dwFlags = 0);
ALERROR kernelSetDebugLog (const CString &sFilespec, bool bAppend = true);
ALERROR kernelSetDebugLog (CTextFileLog *pLog, bool bAppend = true, bool bFreeLog = false);
HANDLE kernelCreateThread (LPTHREAD_START_ROUTINE pfStart, LPVOID pData);
bool kernelDispatchUntilEventSet (HANDLE hEvent, DWORD dwTimeout = INFINITE);

//	String and Arrays

static const DWORD DELIMIT_TRIM_WHITESPACE =		0x00000001;
static const DWORD DELIMIT_ALLOW_BLANK_STRINGS =	0x00000002;
static const DWORD DELIMIT_COMMA =					0x00000004;
static const DWORD DELIMIT_SEMI_COLON =				0x00000008;
static const DWORD DELIMIT_QUOTE_ESCAPE =			0x00000010;
static const DWORD DELIMIT_AUTO_COMMA =				0x00000020;
ALERROR strDelimitEx (const CString &sString, char cDelim, DWORD dwFlags, int iMinParts, TArray<CString> *retList);

inline ALERROR strDelimit (const CString &sString, char cDelim, int iMinParts, TArray<CString> *pStringList)
	{ return strDelimitEx(sString, cDelim, 0, iMinParts, pStringList); }

CString strJoin (const TArray<CString> &List, const CString &sConjunction);


//	Path functions (Path.cpp)

enum ESpecialFolders
	{
	folderAppData,
	folderDocuments,
	folderPictures,
	folderMusic,
	};

struct SFileVersionInfo
	{
	SFileVersionInfo (void) :
			dwFileVersion(0),
			dwProductVersion(0)
		{ }

	CString sProductName;
	CString sProductVersion;
	CString sCompanyName;
	CString sCopyright;

	ULONG64 dwFileVersion;
	ULONG64 dwProductVersion;
	};

bool fileCopy (const CString &sSourceFilespec, const CString &sDestFilespec);
bool fileDelete (const CString &sFilespec, bool bRecycle = false);

const DWORD FFL_FLAG_DIRECTORIES_ONLY =		0x00000001;
const DWORD FFL_FLAG_RELATIVE_FILESPEC =	0x00000002;
const DWORD FFL_FLAG_RECURSIVE =			0x00000004;
bool fileGetFileList (const CString &sRoot, const CString &sPath, const CString &sSearch, DWORD dwFlags, TArray<CString> *retFiles);

CTimeDate fileGetModifiedTime (const CString &sFilespec);
CString fileGetProductName (void);
DWORD fileGetProductVersion (void);
ALERROR fileGetVersionInfo (const CString &sFilename, SFileVersionInfo *retInfo);
bool fileMove (const CString &sSourceFilespec, const CString &sDestFilespec);
bool fileOpen (const CString &sFile, const CString &sParameters = NULL_STR, const CString &sCurrentFolder = NULL_STR, CString *retsError = NULL);

CString pathAddComponent (const CString &sPath, const CString &sComponent);
CString pathAddExtensionIfNecessary (const CString &sPath, const CString &sExtension);
bool pathCreate (const CString &sPath);
bool pathDeleteAll (const CString &sPath);
bool pathExists (const CString &sPath);
CString pathGetExecutablePath (HINSTANCE hInstance);
CString pathGetExtension (const CString &sPath);
CString pathGetFilename (const CString &sPath);
CString pathGetPath (const CString &sPath);
CString pathGetResourcePath (char *pszResID);
CString pathGetSpecialFolder (ESpecialFolders iFolder);
CString pathGetTempPath (void);
bool pathIsAbsolute (const CString &sPath);
bool pathIsFolder (const CString &sFilespec);
inline bool pathIsPathSeparator (char *pPos) { return (*pPos == '\\' || *pPos == '/'); }

//	pathNormalizeSeparators
//
//	Replaces backslashes with forward slashes on non-Windows platforms.
//	Call this when reading paths from XML/TDB that may have been authored on Windows.

inline CString pathNormalizeSeparators (const CString &sPath)
	{
#ifdef _WIN32
	return sPath;
#else
	CString sResult = sPath;
	char *pPos = sResult.GetASCIIZPointer();
	char *pEnd = pPos + sResult.GetLength();
	while (pPos < pEnd)
		{
		if (*pPos == '\\')
			*pPos = '/';
		pPos++;
		}
	return sResult;
#endif
	}
bool pathIsResourcePath (const CString &sPath, char **retpszResID);
bool pathIsWritable (const CString &sFilespec);
CString pathMakeAbsolute (const CString &sPath, const CString &sRoot = NULL_STR);
CString pathMakeRelative (const CString &sFilespec, const CString &sRoot, bool bNoCheck = false);
CString pathStripExtension (const CString &sPath);
bool pathValidateFilename (const CString &sFilename, CString *retsValidFilename = NULL);

//	RegEx functions (RegEx.cpp)

struct SRegExMatch
	{
	char *pPos;
	CString sMatch;
	};

bool strRegEx (char *pStart, const CString &sPattern, TArray<SRegExMatch> *retMatches = NULL, char **retpEnd = NULL);

//	Math functions (Math.cpp)

int mathAdjust (int iValue, int iPercent);
int mathAdjustRound (int iValue, int iPercent);
DWORD mathGetSeed (void);
DWORD mathMakeSeed (DWORD dwValue);
int mathNearestPowerOf2 (int x);
int mathPower (int x, int n);
DWORD mathRandom (void);
inline double mathRandomDouble (void) { return (mathRandom() / 2147483648.0); }
int mathRandom (int iFrom, int iTo);
double mathRandomGaussian (void);
double mathRandomMinusOneToOne (void);
int mathRound (double x);
int mathRoundStochastic (double x);
int mathSeededRandom (int iSeed, int iFrom, int iTo);
void mathSetSeed (DWORD dwSeed);
int mathSqrt (int x);

#include "TMath.h"

//	Compression functions

void CompressRunLengthByte (IWriteStream *pOutput, IReadBlock *pInput);
void UncompressRunLengthByte (IWriteStream *pOutput, IReadBlock *pInput);

//	System functions

DWORD sysGetAPIFlags (void);
DWORD sysGetTicksElapsed (DWORD dwTick, DWORD *retdwNow = NULL);
CString sysGetUserName (void);
bool sysIsBigEndian (void);
bool sysOpenURL (const CString &sURL);

//	Processor info functions

struct SProcessorInfo
	{
	DWORD dwNumLogical = 0;
	DWORD dwNumPhysical = 0;
	DWORD dwNumProcessorGroups = 0;

	//	flags

	DWORD fReliablePhysicalProcessorCount : 1 = 0;
	DWORD fReliableLogicalProcessorCount : 1 = 0;
	DWORD fReliableProcessorGroups : 1 = 0;
	DWORD fSuccess : 1 = 0;
	DWORD fCanAddProcessorGroups : 1 = 0;
	DWORD dwSpare : 27 = 0;
	};

#ifdef WIN32
#define RELIABLE_AFFINITY_MASK false
#else
#define RELIABLE_AFFINITY_MASK true
#endif

DWORD sysGetProcessorsInMask(KAFFINITY& AffinityMask);
SProcessorInfo sysGetProcessorInfo(void);
int sysGetProcessorCountLegacy(void);
int sysGetProcessorCount(void);

//	Utility functions (Utilities.cpp)

DWORD utlHashFunctionCase (BYTE *pKey, int iKeyLen);
void utlMemSet (LPVOID pDest, DWORD Count, BYTE Value);
void utlMemCopy (const char *pSource, char *pDest, DWORD dwCount);
BOOL utlMemCompare (char *pSource, char *pDest, DWORD dwCount);
inline LPVOID MemAlloc (int iSize) { return (BYTE *)HeapAlloc(GetProcessHeap(), 0, iSize); }
inline void MemFree (LPVOID pMem) { HeapFree(GetProcessHeap(), 0, pMem); }

//	UI functions

ALERROR uiCopyTextToClipboard (HWND hWnd, const CString &sText);
void uiGetCenteredWindowRect (int cxWidth, 
							  int cyHeight, 
							  RECT *retrcRect,
							  bool bClip = true);
inline bool uiIsControlDown (void) { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) ? true : false; }
inline bool uiIsKeyDown (int iVirtKey) { return ((::GetAsyncKeyState(iVirtKey) & 0x8000) ? true : false); }
inline bool uiIsKeyRepeat (DWORD dwKeyData) { return ((dwKeyData & 0x40000000) ? true : false); }
inline bool uiIsNumLockOn (void) { return (::GetKeyState(VK_NUMLOCK) & 0x0001) ? true : false; }
inline bool uiIsShiftDown (void) { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) ? true : false; }
inline char uiGetCharFromKeyCode (int iVirtKey) { DWORD dwChar = ::MapVirtualKey((UINT)iVirtKey, MAPVK_VK_TO_CHAR); return (dwChar < 256 ? (char)(BYTE)dwChar : 0); }

//	Note: This cannot be an inline because it will fail if the inline is
//	ever compiled as a function call
#define MemStackAlloc(iSize) (_alloca(iSize))

//	Comparison functions

template<>
inline int KeyCompare<LPCSTR> (const LPCSTR &Key1, const LPCSTR &Key2)
	{
	return strCompareAbsolute(Key1, Key2);
	}

template<>
inline int KeyCompare<CString> (const CString &sKey1, const CString &sKey2)
	{
	return strCompareAbsolute(sKey1, sKey2);
	}

template<>
inline int KeyCompare<CTimeDate> (const CTimeDate &Key1, const CTimeDate &Key2)
	{
	return Key1.Compare(Key2);
	}

//	The following macro "NoEmptyFile()" can be put into a file 
//	in order suppress the MS Visual C++ Linker warning 4221 
// 
//	warning LNK4221: no public symbols found; archive member will be inaccessible 
// 
//	Thanks to: http://stackoverflow.com/users/14904/adisak
//	See: http://stackoverflow.com/questions/1822887/what-is-the-best-way-to-eliminate-ms-visual-c-linker-warning-warning-lnk422
 
#define NoEmptyFile()   namespace { char NoEmptyFileDummy##__LINE__; } 

};
