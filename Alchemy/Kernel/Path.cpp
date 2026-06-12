//	Path.cpp
//
//	Path routines
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#ifdef _WIN32
#include "shlobj.h"
#else
#include <copyfile.h>
#include <sys/stat.h>
#include <mach-o/dyld.h>
#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <pwd.h>
#include <string>
#include <vector>
#include <algorithm>
#define SHGFP_TYPE_CURRENT 0
#define CSIDL_LOCAL_APPDATA 28
#define HRESULT long
#define S_OK 0
#define E_FAIL 1
#define LPITEMIDLIST void*
#define FO_DELETE 0x0003
#define FOF_ALLOWUNDO 0x0040
#define FOF_NO_UI 0x0400
#define FILEOP_FLAGS unsigned int
#define FILE_ATTRIBUTE_HIDDEN 0x0002
#define FILE_ATTRIBUTE_SYSTEM 0x0004
#define FILE_ATTRIBUTE_DIRECTORY 0x0010
#define FILETIME unsigned long long
#define SW_SHOWDEFAULT 5
#define CSIDL_APPDATA 26
#define CSIDL_PERSONAL 5
#define CSIDL_MYPICTURES 0x0027
#define CSIDL_MYMUSIC 0x000D
#define MAX_PATH 260
#define LPMALLOC void*
struct WIN32_FIND_DATA { DWORD dwFileAttributes; FILETIME ftCreationTime; FILETIME ftLastAccessTime; FILETIME ftLastWriteTime; DWORD nFileSizeHigh; DWORD nFileSizeLow; DWORD dwReserved0; DWORD dwReserved1; char cFileName[260]; char cAlternateFileName[14]; };
struct SHFILEOPSTRUCT { void* hwnd; UINT wFunc; char* pFrom; char* pTo; FILEOP_FLAGS fFlags; BOOL fAnyOperationsAborted; void* hNameMappings; char* lpszProgressTitle; };
inline HRESULT SHGetFolderPath(void* pToken, int iCSIDL, void* pReserved, DWORD dwFlags, char* pDest)
	{
	//	Map Windows special folders to their macOS equivalents.

	(void)pToken; (void)pReserved; (void)dwFlags;

	const char *pHome = getenv("HOME");
	if (pHome == NULL || pHome[0] == '\0')
		{
		struct passwd *pPasswd = getpwuid(getuid());
		if (pPasswd)
			pHome = pPasswd->pw_dir;
		}

	if (pHome == NULL || pHome[0] == '\0')
		return E_FAIL;

	const char *pSuffix;
	switch (iCSIDL)
		{
		case CSIDL_APPDATA:
		case CSIDL_LOCAL_APPDATA:
			pSuffix = "/Library/Application Support";
			break;

		case CSIDL_PERSONAL:
			pSuffix = "/Documents";
			break;

		case CSIDL_MYPICTURES:
			pSuffix = "/Pictures";
			break;

		case CSIDL_MYMUSIC:
			pSuffix = "/Music";
			break;

		default:
			pSuffix = "";
			break;
		}

	snprintf(pDest, MAX_PATH, "%s%s", pHome, pSuffix);
	return S_OK;
	}

inline int SHFileOperation(SHFILEOPSTRUCT* lpFileOp)
	{
	//	macOS has no recycle bin via this API: fall back to a permanent delete
	//	so that fileDelete(..., bRecycle = true) still works.

	if (lpFileOp == NULL || lpFileOp->wFunc != FO_DELETE || lpFileOp->pFrom == NULL)
		return 1;

	lpFileOp->fAnyOperationsAborted = FALSE;

	std::string sPath = lpFileOp->pFrom;
	std::replace(sPath.begin(), sPath.end(), '\\', '/');
	return (unlink(sPath.c_str()) == 0 ? 0 : 1);
	}

inline BOOL CopyFile(const char* pSrc, const char* pDst, BOOL bFailIfExists) { return copyfile(pSrc, pDst, nullptr, COPYFILE_ALL) == 0; }

//	POSIX implementation of the FindFirstFile/FindNextFile/FindClose family
//	based on opendir/readdir/fnmatch.

struct SPosixFindHandle
	{
	DIR *pDir;
	std::string sDirPath;
	std::string sPattern;
	};

static bool PosixMatchPattern (const char *pPattern, const char *pName)
	{
	//	Windows treats "*.*" as "all files", even those without a dot.

	if (strcmp(pPattern, "*.*") == 0 || strcmp(pPattern, "*") == 0 || pPattern[0] == '\0')
		return true;

	return (fnmatch(pPattern, pName, FNM_CASEFOLD) == 0);
	}

static bool PosixFindNext (SPosixFindHandle *pHandle, WIN32_FIND_DATA *pData)
	{
	struct dirent *pEntry;
	while ((pEntry = readdir(pHandle->pDir)) != NULL)
		{
		if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0)
			continue;

		if (!PosixMatchPattern(pHandle->sPattern.c_str(), pEntry->d_name))
			continue;

		memset(pData, 0, sizeof(*pData));
		strlcpy(pData->cFileName, pEntry->d_name, sizeof(pData->cFileName));

		std::string sFull = pHandle->sDirPath + "/" + pEntry->d_name;
		struct stat st;
		if (stat(sFull.c_str(), &st) == 0)
			{
			if (S_ISDIR(st.st_mode))
				pData->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			pData->nFileSizeLow = (DWORD)((unsigned long long)st.st_size & 0xffffffff);
			pData->nFileSizeHigh = (DWORD)((unsigned long long)st.st_size >> 32);
			pData->ftLastWriteTime = (FILETIME)st.st_mtime;
			}

		if (pData->dwFileAttributes == 0)
			pData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

		//	Mimic Windows semantics: treat Unix dotfiles as hidden so the game
		//	does not pick up .DS_Store and similar files.

		if (pEntry->d_name[0] == '.')
			pData->dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;

		return true;
		}

	return false;
	}

inline void* FindFirstFile(const char* pPattern, WIN32_FIND_DATA* pData)
	{
	std::string sPattern = (pPattern ? pPattern : "");
	std::replace(sPattern.begin(), sPattern.end(), '\\', '/');

	//	Split into directory and filename pattern.

	size_t iSep = sPattern.rfind('/');
	std::string sDir = (iSep == std::string::npos) ? "." : sPattern.substr(0, (iSep == 0 ? 1 : iSep));
	std::string sFilePattern = (iSep == std::string::npos) ? sPattern : sPattern.substr(iSep + 1);

	//	Handle case-mismatched directories (Windows is case-insensitive).

	DIR *pDir = opendir(sDir.c_str());
	if (pDir == NULL)
		{
		sDir = posixResolvePathCase(sDir);
		pDir = opendir(sDir.c_str());
		}

	if (pDir == NULL)
		{
		//	Callers check GetLastError() == ERROR_FILE_NOT_FOUND (== ENOENT).

		errno = ENOENT;
		return INVALID_HANDLE_VALUE;
		}

	SPosixFindHandle *pHandle = new SPosixFindHandle;
	pHandle->pDir = pDir;
	pHandle->sDirPath = sDir;
	pHandle->sPattern = sFilePattern;

	if (!PosixFindNext(pHandle, pData))
		{
		closedir(pHandle->pDir);
		delete pHandle;
		errno = ENOENT;
		return INVALID_HANDLE_VALUE;
		}

	return pHandle;
	}

inline BOOL FindNextFile(void* hFind, WIN32_FIND_DATA* pData)
	{
	if (hFind == NULL || hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	return (PosixFindNext((SPosixFindHandle *)hFind, pData) ? TRUE : FALSE);
	}

inline BOOL FindClose(void* hFind)
	{
	if (hFind == NULL || hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	SPosixFindHandle *pHandle = (SPosixFindHandle *)hFind;
	closedir(pHandle->pDir);
	delete pHandle;
	return TRUE;
	}
inline BOOL GetFileTime(HANDLE hFile, FILETIME* pCreation, FILETIME* pLastAccess, FILETIME* pLastWrite) { return TRUE; }
inline BOOL FileTimeToSystemTime(FILETIME* pFileTime, SYSTEMTIME* pSystemTime) { return TRUE; }
inline DWORD GetTempPath(DWORD nBufferLength, char* lpBuffer) { strcpy(lpBuffer, "/tmp"); return strlen(lpBuffer); }
inline DWORD GetFileAttributes(const char* lpFileName) {
#ifndef _WIN32
    std::string sFilename = (lpFileName ? lpFileName : "");
    std::replace(sFilename.begin(), sFilename.end(), '\\', '/');

    //	Fall back to a case-insensitive lookup (Windows is case-insensitive).

    if (access(sFilename.c_str(), F_OK) != 0)
        sFilename = posixResolvePathCase(sFilename);

    struct stat st;
    if (stat(sFilename.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) return FILE_ATTRIBUTE_DIRECTORY;
        return FILE_ATTRIBUTE_NORMAL;
    }
    return 0xffffffff;
#else
    return FILE_ATTRIBUTE_NORMAL;
#endif
}
inline BOOL CreateDirectory(const char* lpPathName, void* lpSecurityAttributes)
	{
	(void)lpSecurityAttributes;
	std::string sPath = (lpPathName ? lpPathName : "");
	std::replace(sPath.begin(), sPath.end(), '\\', '/');
	return (mkdir(sPath.c_str(), 0755) == 0);
	}
inline BOOL RemoveDirectory(const char* lpPathName) { return rmdir(lpPathName) == 0; }
inline void* CoTaskMemAlloc(DWORD cb) { return malloc(cb); }
inline void CoTaskMemFree(void* pv) { free(pv); }
inline DWORD GetFullPathName(const char* lpFileName, DWORD nBufferLength, char* lpBuffer, char** lpFilePart)
	{
	if (lpBuffer == NULL || nBufferLength == 0)
		return 0;

	std::string sPath = (lpFileName ? lpFileName : "");
	std::replace(sPath.begin(), sPath.end(), '\\', '/');

	std::string sAbsolute;
	char szResolved[PATH_MAX];
	if (!sPath.empty() && realpath(sPath.c_str(), szResolved) != NULL)
		sAbsolute = szResolved;
	else
		{
		//	The path may not exist yet: anchor relative paths to the current
		//	directory and normalize '.' and '..' components manually.

		if (sPath.empty() || sPath[0] != '/')
			{
			char szCwd[PATH_MAX];
			if (getcwd(szCwd, sizeof(szCwd)) == NULL)
				return 0;

			sAbsolute = szCwd;
			if (!sPath.empty())
				{
				sAbsolute += '/';
				sAbsolute += sPath;
				}
			}
		else
			sAbsolute = sPath;

		std::vector<std::string> Parts;
		size_t iPos = 1;
		while (iPos <= sAbsolute.size())
			{
			size_t iNext = sAbsolute.find('/', iPos);
			size_t iEnd = (iNext == std::string::npos) ? sAbsolute.size() : iNext;
			std::string sComp = sAbsolute.substr(iPos, iEnd - iPos);

			if (sComp == "..")
				{
				if (!Parts.empty())
					Parts.pop_back();
				}
			else if (!sComp.empty() && sComp != ".")
				Parts.push_back(sComp);

			if (iNext == std::string::npos)
				break;
			iPos = iNext + 1;
			}

		sAbsolute.clear();
		for (size_t i = 0; i < Parts.size(); i++)
			{
			sAbsolute += '/';
			sAbsolute += Parts[i];
			}

		if (sAbsolute.empty())
			sAbsolute = "/";
		}

	//	Windows semantics: if the buffer is too small, return the required size
	//	(including the NUL terminator).

	if (sAbsolute.size() + 1 > (size_t)nBufferLength)
		return (DWORD)(sAbsolute.size() + 1);

	memcpy(lpBuffer, sAbsolute.c_str(), sAbsolute.size() + 1);

	if (lpFilePart)
		{
		char *pSep = strrchr(lpBuffer, '/');
		*lpFilePart = (pSep ? pSep + 1 : lpBuffer);
		}

	return (DWORD)sAbsolute.size();
	}
inline void* SHGetMalloc() { return nullptr; }
inline HRESULT SHGetMalloc(void** ppMalloc) { *ppMalloc = (void*)1; return S_OK; }
#endif

#ifdef _WIN32
#define STR_PATH_SEPARATOR				CONSTLIT("\\")
#else
#define STR_PATH_SEPARATOR				CONSTLIT("/")
#endif

#define STR_COMPANY_NAME				CONSTLIT("CompanyName")
#define STR_COPYRIGHT					CONSTLIT("LegalCopyright")
#define STR_PRODUCT_NAME				CONSTLIT("ProductName")
#define STR_PRODUCT_VERSION				CONSTLIT("ProductVersion")

#define STR_DOT							CONSTLIT(".")
#define STR_DOT_DOT						CONSTLIT("..")
#define STR_STAR						CONSTLIT("*")

void FreePIDL (LPITEMIDLIST pidl);
CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString);

bool Kernel::fileCopy (const CString &sSourceFilespec, const CString &sDestFilespec)

//	fileMove
//
//	Moves a file

	{
	if (!::CopyFile(sSourceFilespec.GetASCIIZPointer(), sDestFilespec.GetASCIIZPointer(), FALSE))
		return false;

	return true;
	}

bool Kernel::fileDelete (const CString &sFilespec, bool bRecycle)

//	fileDelete
//
//	Delete the file permanently from disk.
//	NOTE: Path must be absolute.

	{
	//	If we're recycling, send to recycle bin

	if (bRecycle)
		{
		SHFILEOPSTRUCT FileOp;
		::ZeroMemory(&FileOp, sizeof(FileOp));
		FileOp.hwnd = NULL;
		FileOp.wFunc = FO_DELETE;

		CString sFrom = strPatternSubst(CONSTLIT("%s\0"), pathMakeAbsolute(sFilespec));
		FileOp.pFrom = sFrom.GetASCIIZPointer();
		FileOp.pTo = NULL;
		FileOp.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;

		int iResult = ::SHFileOperation(&FileOp);
		return (iResult == 0 && !FileOp.fAnyOperationsAborted);
		}

	//	Otherwise, just delete

	else
		{
		if (!::DeleteFile(sFilespec.GetASCIIZPointer()))
			return false;
		}

	return true;
	}

bool Kernel::fileGetFileList (const CString &sRoot, const CString &sPath, const CString &sSearch, DWORD dwFlags, TArray<CString> *retFiles)

//	fileGetFileList
//
//	Returns a list of filespecs that match the given filespec

	{
	WIN32_FIND_DATA FileInfo;

	CString sFullPath = pathAddComponent(sRoot, sPath);
	CString sFilespec = pathAddComponent(sFullPath, sSearch);

	HANDLE hFind = ::FindFirstFile(sFilespec.GetASCIIZPointer(), &FileInfo);
	if (hFind == INVALID_HANDLE_VALUE)
		{
		//	If ERROR_FILE_NOT_FOUND then there are no files that match

		if (::GetLastError() == ERROR_FILE_NOT_FOUND)
			return true;

		//	Otherwise this is an error of some sort

		else
			return false;
		}

	bool bDirectoriesOnly = ((dwFlags & FFL_FLAG_DIRECTORIES_ONLY) ? true : false);

	do
		{
		CString sFound(FileInfo.cFileName, -1);

		//	Skip . and ..

		if (strEquals(sFound, STR_DOT) || strEquals(sFound, STR_DOT_DOT))
			continue;

		//	Skip system and hidden files

		if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
				|| (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
			continue;

		//	Is this a directory?

		bool bIsDirectory = ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);

		//	Add to the list (but only if not a directory)

		if (bDirectoriesOnly == bIsDirectory)
			{
			if (dwFlags & FFL_FLAG_RELATIVE_FILESPEC)
				retFiles->Insert(pathAddComponent(sPath, sFound));
			else
				retFiles->Insert(pathAddComponent(sFullPath, sFound));
			}

		//	Recurse, if necessary

		if (bIsDirectory && (dwFlags & FFL_FLAG_RECURSIVE))
			{
			if (!fileGetFileList(sRoot, pathAddComponent(sPath, sFound), sSearch, dwFlags, retFiles))
				return false;
			}
		}
	while (::FindNextFile(hFind, &FileInfo));

	//	Done

	::FindClose(hFind);
	return true;
	}

CTimeDate Kernel::fileGetModifiedTime (const CString &sFilespec)

//	fileGetModifiedTime
//
//	Returns the modified time

	{
	HANDLE hFile = ::CreateFile(sFilespec.GetASCIIZPointer(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return CTimeDate();

	//	Get modified time

	FILETIME ftWrite;
	if (!::GetFileTime(hFile, NULL, NULL, &ftWrite))
		{
		::CloseHandle(hFile);
		return CTimeDate();
		}

	//	Convert

	SYSTEMTIME SystemTime;
	::FileTimeToSystemTime(&ftWrite, &SystemTime);

	//	Done

	::CloseHandle(hFile);
	return CTimeDate(SystemTime);
	}

CString Kernel::fileGetProductName (void)

//	fileGetProductName
//
//	Returns the name of the product.

	{
	SFileVersionInfo VerInfo;
	fileGetVersionInfo(NULL_STR, &VerInfo);
	return VerInfo.sProductName;
	}

DWORD Kernel::fileGetProductVersion (void)

//	fileGetProductVersion
//
//	Returns the version encoded as a DWORD.
//
//	NOTE: This is not very accurate, since it tries to pack a DWORDLONG into
//	a DWORD. Do not use unless you know that your version numbers fit.

	{
	SFileVersionInfo VerInfo;
	fileGetVersionInfo(NULL_STR, &VerInfo);

	BYTE b1 = (BYTE)HIWORD((DWORD)(VerInfo.dwProductVersion >> 32));
	BYTE b2 = (BYTE)LOWORD((DWORD)(VerInfo.dwProductVersion >> 32));
	BYTE b3 = (BYTE)HIWORD((DWORD)VerInfo.dwProductVersion);
	BYTE b4 = (BYTE)LOWORD((DWORD)VerInfo.dwProductVersion);

	return MAKELONG(MAKEWORD(b4, b3), MAKEWORD(b2, b1));
	}

ALERROR Kernel::fileGetVersionInfo (const CString &sFilename, SFileVersionInfo *retInfo)

//	fileGetVersionInfo
//
//	Returns version information for the file (if sFilename is NULL_STRING then
//	we return information for the current module.)

	{
	CString sPath = sFilename;
	if (sPath.IsBlank())
		{
		char szBuffer[1024];
		int iLen = ::GetModuleFileName(NULL, szBuffer, sizeof(szBuffer)-1);
		sPath = CString(szBuffer, iLen);
		}

	//	Initialize

	retInfo->dwFileVersion = 0;
	retInfo->dwProductVersion = 0;

	//	Figure out how big the version information is

	DWORD dwDummy;
	DWORD dwSize = ::GetFileVersionInfoSize(sPath.GetASCIIZPointer(), &dwDummy);
	if (dwSize == 0)
		return ERR_FAIL;

	//	Load the info

	CString sData;
	char *pData = sData.GetWritePointer(dwSize);
	if (!::GetFileVersionInfo(sPath.GetASCIIZPointer(), 0, dwSize, pData))
		return ERR_FAIL;

	//	Get the fixed-size portion

	VS_FIXEDFILEINFO *pFileInfo;
	DWORD dwFileInfoSize;
	if (::VerQueryValue(pData, "\\", (LPVOID *)&pFileInfo, (PUINT)&dwFileInfoSize))
		{
		retInfo->dwFileVersion = ((ULONG64)pFileInfo->dwFileDateMS << 32) | (ULONG64)pFileInfo->dwFileDateLS;
		retInfo->dwProductVersion = ((ULONG64)pFileInfo->dwProductVersionMS << 32) | (ULONG64)pFileInfo->dwProductVersionLS;
		}

	//	Get language information

	WORD *pLangInfo;
	DWORD dwLangInfoSize;
	::VerQueryValue(pData, "\\VarFileInfo\\Translation", (LPVOID *)&pLangInfo, (PUINT)&dwLangInfoSize);

	//	Get the strings

	retInfo->sCompanyName = GetVersionString(pData, pLangInfo, STR_COMPANY_NAME);
	retInfo->sCopyright = GetVersionString(pData, pLangInfo, STR_COPYRIGHT);
	retInfo->sProductName = GetVersionString(pData, pLangInfo, STR_PRODUCT_NAME);
	retInfo->sProductVersion = GetVersionString(pData, pLangInfo, STR_PRODUCT_VERSION);

	return NOERROR;
	}

bool Kernel::fileMove (const CString &sSourceFilespec, const CString &sDestFilespec)

//	fileMove
//
//	Moves a file

	{
	if (!::MoveFile(sSourceFilespec.GetASCIIZPointer(), sDestFilespec.GetASCIIZPointer()))
		return false;

	return true;
	}

bool Kernel::fileOpen (const CString &sFile, const CString &sParameters, const CString &sCurrentFolder, CString *retsError)

//	fileOpen
//
//	Launches the current file.

	{
	intptr_t iResult = (intptr_t)::ShellExecute(NULL,
			NULL,
			pathMakeAbsolute(sFile.GetASCIIZPointer()).GetASCIIZPointer(),
			(!sParameters.IsBlank() ? sParameters.GetASCIIZPointer() : NULL),
			(!sCurrentFolder.IsBlank() ? pathMakeAbsolute(sCurrentFolder).GetASCIIZPointer() : NULL),
			SW_SHOWDEFAULT);
	if (iResult <= 32)
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to launch program: %s; error = %d"), sFile, iResult);
		return false;
		}

	return true;
	}

CString Kernel::pathAddComponent (const CString &sPath, const CString &sComponent)

//	pathAddComponent
//
//	Concatenates the given component to the given path and returns
//	the result
//
//	sPath: full pathname to a directory (e.g. "c:\", "\\lawrence\cdrom", "d:\test")
//	sComponent: directory, filename, or wildcard.

	{
	if (sPath.IsBlank())
		return sComponent;
	else if (sComponent.IsBlank())
		return sPath;
	else
		{
		CString sResult = sPath;
		int iPathLength = sResult.GetLength();
		char *pString = sResult.GetPointer();
		const char *pszSeparator = "\\";

#ifdef TARGET_PLATFORM_MACOS
		pszSeparator = "/";
#endif

		//	If the path name does not have a trailing backslash, add one

		if (!sPath.IsBlank() && !pathIsPathSeparator(pString + iPathLength - 1))
			sResult.Append(pszSeparator);

		//	Now concatenate the component

		sResult.Append(sComponent);

		return sResult;
		}
	}

CString Kernel::pathAddExtensionIfNecessary (const CString &sPath, const CString &sExtension)

//	pathAddExtensionIfNecessary
//
//	If the path has not extension and if it doesn't end in '.' then add
//	the extension

	{
	int iLength = sPath.GetLength();
	char *pStart = sPath.GetASCIIZPointer();
	char *pPos = pStart + iLength;

	while (pPos >= pStart)
		{
		if (*pPos-- == '.')
			return sPath;
		}

	if (*sExtension.GetASCIIZPointer() == '.')
		return strPatternSubst(CONSTLIT("%s%s"), sPath, sExtension);
	else
		return strPatternSubst(CONSTLIT("%s.%s"), sPath, sExtension);
	}

bool Kernel::pathCreate (const CString &sPath)

//	pathCreate
//
//	Makes sure that the given path exists. Creates all intermediate folders.
//	Returns TRUE if successful.

	{
	CString sTest(sPath.GetASCIIZPointer(), sPath.GetLength());
	char *pPos = sTest.GetASCIIZPointer();

	//	Make sure the path exists from the top down.

	while (*pPos != '\0')
		{
		//	Skip over this backslash

		while (pathIsPathSeparator(pPos))
			pPos++;

		//	Skip to the next backslash

		while (!pathIsPathSeparator(pPos) && *pPos != '\0')
			pPos++;

		//	Trim the path here and see if it exists so far

		char chSaved = *pPos;
		*pPos = '\0';
		
		//	If the path doesn't exist, create it.

		if (!pathExists(sTest))
			{
			if (!::CreateDirectory(sTest.GetASCIIZPointer(), NULL))
				return false;
			}

		*pPos = chSaved;
		}

	return true;
	}

bool Kernel::pathDeleteAll (const CString &sPath)

//  pathDeleteAll
//
//  Recursively deletes all file in the given directory (including the directory).
//  We return FALSE if we could not delete everything.

    {
    bool bFailed = false;

	CFileDirectory Dir(pathAddComponent(sPath, CONSTLIT("*.*")));
    while (Dir.HasMore())
        {
        SFileDesc FileDesc;
        Dir.GetNextDesc(&FileDesc);

        //	Skip special files

        if (strEquals(FileDesc.sFilename, STR_DOT) || strEquals(FileDesc.sFilename, STR_DOT_DOT))
            continue;

        //	Get path and extension

        CString sFilepath = pathAddComponent(sPath, FileDesc.sFilename);

        //	If this is a folder, then recurse

        if (FileDesc.bFolder)
            {
            if (!pathDeleteAll(sFilepath))
                bFailed = true;
            }

        //	Otherwise, delete the file

        else
            {
            if (!fileDelete(sFilepath))
                bFailed = true;
            }
        }

    //  We're not going to be able to delete the directory, since it is not 
    //  empty.

    if (bFailed)
        return false;

    //  Delete the directory

    if (!::RemoveDirectory(sPath.GetASCIIZPointer()))
        return false;

    return true;
    }

CString Kernel::pathGetExecutablePath (HINSTANCE hInstance)

//	pathGetExecutablePath
//
//	Returns the path of the given executable. This is the path
//	(without the filename) of the executable (e.g., c:\bin\windows)

	{
	char szBuffer[1024];
	int iLen;
	char *pPos;
	CString sPath;

	//	Get the path

	iLen = GetModuleFileName(hInstance, szBuffer, sizeof(szBuffer)-1);

	//	Skip backwards to the first backslash

	pPos = szBuffer + iLen;
	while (!pathIsPathSeparator(pPos) && pPos != szBuffer)
		pPos--;

	*pPos = '\0';

	//	Create string

	sPath.Transcribe(szBuffer, -1);
	return sPath;
	}

CString Kernel::pathGetTempPath (void)

//  pathGetTempPath
//
//  Returns a path to a temporary directory

    {
	char szBuffer[1024];

    if (GetTempPath(sizeof(szBuffer), szBuffer) == 0)
        return NULL_STR;

    return CString(szBuffer);
    }

bool Kernel::pathExists (const CString &sPath)

//	pathExists
//
//	Returns TRUE if the given path exists

	{
	DWORD dwResult = ::GetFileAttributes(sPath.GetASCIIZPointer());
	return (dwResult != 0xffffffff);
	}

CString Kernel::pathGetExtension (const CString &sPath)

//	pathGetExtension
//
//	Returns the extension (without dot)

	{
	char *pPos;
	int iLength;

	pPos = sPath.GetASCIIZPointer();
	iLength = sPath.GetLength();

	//	Look for the extension

	while (iLength > 0)
		{
		if (pPos[iLength] == '.')
			break;
		iLength--;
		}

	//	Return the extension

	if (iLength == 0)
		return NULL_STR;
	else
		return strSubString(sPath, iLength + 1, -1);
	}

CString Kernel::pathGetFilename (const CString &sPath)

//	pathGetFilename
//
//	Returns the filename (without the path)

	{
	char *pStart = sPath.GetASCIIZPointer();
	char *pPos = pStart + sPath.GetLength() + 1;

	//	Look for the first backslash

	while (pPos > pStart && !pathIsPathSeparator(pPos - 1))
		pPos--;

	return CString(pPos);
	}

CString Kernel::pathGetPath (const CString &sPath)

//	pathGetPath
//
//	Returns the path without the filename

	{
	char *pStart = sPath.GetASCIIZPointer();
	char *pPos = pStart + sPath.GetLength() + 1;

	//	Look for the first backslash

	while (pPos > pStart && !pathIsPathSeparator(pPos - 1))
		pPos--;

	return CString(pStart, pPos - pStart);
	}

CString Kernel::pathGetResourcePath (char *pszResID)

//	pathGetResourcePath
//
//	Converts a resource identifier into a path-like string
//	of the form:
//
//	resID:\@id
//
//	OR
//
//	resID:\{resID}

	{
	if ((uintptr_t)pszResID < 65536)
		return strPatternSubst(CONSTLIT("resID:\\@%d"), (uintptr_t)pszResID);
	else
		return strPatternSubst(CONSTLIT("resID:\\%s"), CString(pszResID));
	}

CString Kernel::pathGetSpecialFolder (ESpecialFolders iFolder)

//	pathGetSpecialFolder
//
//	Returns the path of various user folders on the machine

	{
	//	Figure out the CSIDL

	int iCSIDL;
	switch (iFolder)
		{
		case folderAppData:
			iCSIDL = CSIDL_APPDATA;
			break;

		case folderDocuments:
			iCSIDL = CSIDL_PERSONAL;
			break;

		case folderPictures:
			iCSIDL = CSIDL_MYPICTURES;
			break;

		case folderMusic:
			iCSIDL = CSIDL_MYMUSIC;
			break;

		default:
			ASSERT(false);
			return NULL_STR;
		}

	//	Get the path

	CString sPath;
	char *pDest = sPath.GetWritePointer(MAX_PATH);
	HRESULT hr = ::SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, pDest);
	if (hr != S_OK)
		{
#ifndef _WIN32
		const char* home = getenv("HOME");
		if (home)
			{
			switch (iFolder)
				{
				case folderAppData:
					strncpy(pDest, home, MAX_PATH - 1);
					pDest[MAX_PATH - 1] = '\0';
					strncat(pDest, "/Library/Application Support", MAX_PATH - strlen(pDest) - 1);
					break;

				case folderDocuments:
					strncpy(pDest, home, MAX_PATH - 1);
					pDest[MAX_PATH - 1] = '\0';
					strncat(pDest, "/Documents", MAX_PATH - strlen(pDest) - 1);
					break;

				case folderPictures:
					strncpy(pDest, home, MAX_PATH - 1);
					pDest[MAX_PATH - 1] = '\0';
					strncat(pDest, "/Pictures", MAX_PATH - strlen(pDest) - 1);
					break;

				case folderMusic:
					strncpy(pDest, home, MAX_PATH - 1);
					pDest[MAX_PATH - 1] = '\0';
					strncat(pDest, "/Music", MAX_PATH - strlen(pDest) - 1);
					break;

				default:
					strncpy(pDest, home, MAX_PATH - 1);
					pDest[MAX_PATH - 1] = '\0';
					break;
				}
			}
		else
			pDest[0] = '\0';
#else
		return NULL_STR;
#endif
		}

	//	Truncate to the correct size

	sPath.Truncate(lstrlen(pDest));

	//	Done

	return sPath;
	}

bool Kernel::pathIsAbsolute (const CString &sPath)

//	pathIsAbsolute
//
//	Returns TRUE if the path is absolute

	{
	char *pPos = sPath.GetASCIIZPointer();

#ifndef _WIN32
	//	A leading slash means this is an absolute POSIX path

	if (*pPos == '/')
		return true;
#endif

	//	A double back-slash means this is an absolute network path

	if (*pPos == '\\')
		{
		pPos++;
		return (*pPos == '\\');
		}

	//	A drive letter means this is absolute

	else if ((*pPos >= 'a' && *pPos <= 'z') || (*pPos >= 'A' && *pPos <= 'Z'))
		{
		pPos++;
		return (*pPos == ':');
		}

	//	Otherwise, relative

	else
		return false;
	}

bool Kernel::pathIsFolder (const CString &sFilespec)

//	pathIsFolder
//
//	Returns TRUE if filespec is a folder.

	{
	DWORD dwResult = ::GetFileAttributes(sFilespec.GetASCIIZPointer());
	return (dwResult != 0xffffffff
			&& (dwResult & FILE_ATTRIBUTE_DIRECTORY));
	}

bool Kernel::pathIsResourcePath (const CString &sPath, char **retpszResID)

//	pathIsResourcePath
//
//	If this path is a resource path (see pathGetResourcePath) then
//	we return TRUE and retpszResID is initialized with a pointer to
//	the identifier (this may be a pointer into sPath)

	{
	if (strStartsWith(sPath, CONSTLIT("resID:\\")))
		{
		char *pPos = sPath.GetASCIIZPointer() + 7;
		if (*pPos == '@')
			*retpszResID = MAKEINTRESOURCE(strParseInt(pPos + 1, 0));
		else
			*retpszResID = pPos;

		return true;
		}
	else
		return false;
	}

bool Kernel::pathIsWritable (const CString &sFilespec)

//	pathIsWritable
//
//	If sFilespec is a directory, we try to create a temporary file. If it
//	succeeds, then we're writable. If sFilespec is an existing file, we try
//	to open it in write mode.

	{
	if (!pathExists(sFilespec))
		{
		HANDLE hFile = ::CreateFile(sFilespec.GetASCIIZPointer(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		fileDelete(sFilespec);
		return true;
		}
	else if (pathIsFolder(sFilespec))
		{
		CString sTestFile = pathAddComponent(sFilespec, CONSTLIT("~temp.txt"));
		HANDLE hFile = ::CreateFile(sTestFile.GetASCIIZPointer(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		fileDelete(sTestFile);
		return true;
		}
	else
		{
		HANDLE hFile = ::CreateFile(sFilespec.GetASCIIZPointer(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		return true;
		}
	}

CString Kernel::pathMakeAbsolute (const CString &sPath, const CString &sRoot)

//	pathMakeAbsolute
//
//	Converts a relative path to an absolute path. If the path is already 
//	absolute then we return it unchanged.
//
//	If sRoot is empty then we use the current directory.

	{
	CString sResult;

	DWORD dwResult = ::GetFullPathName(sPath.GetASCIIZPointer(),
			MAX_PATH,
			sResult.GetWritePointer(MAX_PATH),
			NULL);
	if (dwResult == 0)
		return NULL_STR;
	else if (dwResult > MAX_PATH)
		return NULL_STR;
	
	sResult.Truncate(dwResult);
	return sResult;
	}

CString Kernel::pathMakeRelative (const CString &sFilespec, const CString &sRoot, bool bNoCheck)

//	pathMakeRelative
//
//	Converts an absolute path to a relative path. If the path does not start
//	with the given root then we return NULL_STR.
//
//	We strip out any leading / from the remaining path.

	{
	if (sRoot.GetLength() >= sFilespec.GetLength())
		return NULL_STR;

	if (!bNoCheck && !strStartsWith(sFilespec, sRoot))
		return NULL_STR;

	//	Do we have to strip a slash?

	char *pPos = sFilespec.GetASCIIZPointer() + sRoot.GetLength();
	if (pathIsPathSeparator(pPos))
		pPos++;

	return CString(pPos);
	}

CString Kernel::pathStripExtension (const CString &sPath)

//	pathStripExtension
//
//	Returns the path without the extension on the filename

	{
	char *pPos;
	int iLength;

	pPos = sPath.GetASCIIZPointer();
	iLength = sPath.GetLength();

	//	Look for the extension

	while (iLength > 0)
		{
		if (pPos[iLength] == '.')
			break;
		iLength--;
		}

	if (iLength == 0)
		return sPath;

	//	Return everything except the extension

	return strSubString(sPath, 0, iLength);
	}

bool Kernel::pathValidateFilename (const CString &sFilename, CString *retsValidFilename)

//	pathValidateFilename
//
//	Return TRUE if the given filename is valid. If an output parameter is also
//	supplied then we return a valid filename.

	{
	bool bOriginalValid = true;
	CString sValid = sFilename;
	char *pPos = sFilename.GetASCIIZPointer();
	char *pDest = sValid.GetASCIIZPointer();
	while (*pPos)
		{
		switch (*pPos)
			{
			case '<':
			case '>':
			case ':':
			case '/':
			case '\\':
			case '|':
			case '*':
			case '?':
				bOriginalValid = false;
				*pDest = '_';
				break;

			default:
				{
				if (*pPos < ' ')
					{
					bOriginalValid = false;
					*pDest = '_';
					}
				}
			}

		pPos++;
		pDest++;
		}

	if (retsValidFilename)
		*retsValidFilename = sValid;

	return bOriginalValid;
	}

//	HELPERS

void FreePIDL (LPITEMIDLIST pidl)
	{
	(void)pidl;
	}

CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString)
	{
	char szBuffer[1024];

	wsprintf(szBuffer, 
			"\\StringFileInfo\\%04x%04x\\%s",
			pLangInfo[0],
			pLangInfo[1],
			sString.GetASCIIZPointer());

	char *pPos;
	DWORD dwSize;
	if (::VerQueryValue(pData, szBuffer, (LPVOID *)&pPos, (PUINT)&dwSize))
		return CString(pPos, (int)dwSize-1);
	else
		return NULL_STR;
	}
