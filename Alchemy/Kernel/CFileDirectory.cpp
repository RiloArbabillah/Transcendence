//	CFileDirectory.cpp
//
//	Implements CFileDirectory object
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>

#ifndef _WIN32

struct SPosixDirHandle
	{
	DIR *pDir;
	std::string sDirPath;
	std::string sPattern;
	struct dirent *pNextEntry;
	bool bHasMore;
	};

static bool PosixDirMatchPattern (const char *pPattern, const char *pName)
	{
	if (strcmp(pPattern, "*.*") == 0 || strcmp(pPattern, "*") == 0 || pPattern[0] == '\0')
		return true;
	return (fnmatch(pPattern, pName, FNM_CASEFOLD) == 0);
	}

static void PosixDirAdvance (SPosixDirHandle *pHandle)
	{
	while (true)
		{
		pHandle->pNextEntry = readdir(pHandle->pDir);
		if (pHandle->pNextEntry == NULL)
			{
			pHandle->bHasMore = false;
			return;
			}

		if (strcmp(pHandle->pNextEntry->d_name, ".") == 0 || strcmp(pHandle->pNextEntry->d_name, "..") == 0)
			continue;

		if (!PosixDirMatchPattern(pHandle->sPattern.c_str(), pHandle->pNextEntry->d_name))
			continue;

		pHandle->bHasMore = true;
		return;
		}
	}

#endif

CFileDirectory::CFileDirectory (const CString &sFilespec) :
		m_sFilespec(sFilespec),
		m_hSearch(INVALID_HANDLE_VALUE)
	#ifdef _WIN32
	#else
		,m_pFindData(NULL)
	#endif

//	CFileDirectory constructor

	{
	#ifdef _WIN32
	m_hSearch = ::FindFirstFile(sFilespec.GetASCIIZPointer(), &m_FindData);
	#else
	std::string sSpec = sFilespec.GetASCIIZPointer();
	std::replace(sSpec.begin(), sSpec.end(), '\\', '/');

	//	Split into directory and pattern

	size_t iSep = sSpec.rfind('/');
	std::string sDir = (iSep == std::string::npos) ? "." : sSpec.substr(0, (iSep == 0 ? 1 : iSep));
	std::string sPattern = (iSep == std::string::npos) ? sSpec : sSpec.substr(iSep + 1);

	//	Handle case-mismatched directories

	std::string sResolvedDir = posixResolvePathCase(sDir);
	DIR *pDir = opendir(sResolvedDir.c_str());
	if (pDir == NULL)
		return;

	SPosixDirHandle *pHandle = new SPosixDirHandle;
	pHandle->pDir = pDir;
	pHandle->sDirPath = sResolvedDir;
	pHandle->sPattern = sPattern;
	pHandle->pNextEntry = NULL;
	pHandle->bHasMore = false;
	m_pFindData = pHandle;

	PosixDirAdvance(pHandle);
	#endif
	}

CFileDirectory::~CFileDirectory (void)

//	CFileDirectory destructor

	{
	#ifdef _WIN32
	if (m_hSearch != INVALID_HANDLE_VALUE)
		::FindClose(m_hSearch);
	#else
	if (m_pFindData)
		{
		SPosixDirHandle *pHandle = (SPosixDirHandle *)m_pFindData;
		closedir(pHandle->pDir);
		delete pHandle;
		m_pFindData = NULL;
		}
	#endif
	}

bool CFileDirectory::HasMore (void)

//	HasMore
//
//	Returns TRUE if there are more files in the directory

	{
	#ifdef _WIN32
	return (m_hSearch != INVALID_HANDLE_VALUE);
	#else
	if (!m_pFindData)
		return false;
	SPosixDirHandle *pHandle = (SPosixDirHandle *)m_pFindData;
	return pHandle->bHasMore;
	#endif
	}

CString CFileDirectory::GetNext (bool *retbIsFolder)

//	GetNext
//
//	Returns the next filename

	{
	CString sFilename;

	#ifdef _WIN32
	ASSERT(m_hSearch != INVALID_HANDLE_VALUE);

	sFilename = CString(m_FindData.cFileName);
	if (retbIsFolder)
		*retbIsFolder = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);

	//	Get the next file

	if (!::FindNextFile(m_hSearch, &m_FindData))
		{
		::FindClose(m_hSearch);
		m_hSearch = INVALID_HANDLE_VALUE;
		}

	//	Done

	return sFilename;
	#else
	if (!m_pFindData || !HasMore())
		{
		if (retbIsFolder)
			*retbIsFolder = false;
		return sFilename;
		}

	SPosixDirHandle *pHandle = (SPosixDirHandle *)m_pFindData;
	struct dirent *pEntry = pHandle->pNextEntry;

	sFilename = CString(pEntry->d_name);

	//	Get file attributes

	std::string sFull = pHandle->sDirPath + "/" + pEntry->d_name;
	struct stat st;
	bool bIsDir = false;
	if (stat(sFull.c_str(), &st) == 0)
		bIsDir = S_ISDIR(st.st_mode);

	if (retbIsFolder)
		*retbIsFolder = bIsDir;

	//	Advance to next entry

	PosixDirAdvance(pHandle);

	return sFilename;
	#endif
	}

void CFileDirectory::GetNextDesc (SFileDesc *retDesc)

//	GetNextDesc
//
//	Returns the next file descriptor

	{
	#ifdef _WIN32
	ASSERT(m_hSearch != INVALID_HANDLE_VALUE);

	retDesc->sFilename = CString(m_FindData.cFileName);
	retDesc->bFolder = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);
	retDesc->bSystemFile = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ? true : false);
	retDesc->bHiddenFile = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? true : false);
	retDesc->bReadOnly = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? true : false);

	//	Get the next file

	if (!::FindNextFile(m_hSearch, &m_FindData))
		{
		::FindClose(m_hSearch);
		m_hSearch = INVALID_HANDLE_VALUE;
		}
	#else
	if (!m_pFindData || !HasMore())
		{
		retDesc->sFilename = NULL_STR;
		retDesc->bFolder = false;
		retDesc->bSystemFile = false;
		retDesc->bHiddenFile = false;
		retDesc->bReadOnly = false;
		return;
		}

	SPosixDirHandle *pHandle = (SPosixDirHandle *)m_pFindData;
	struct dirent *pEntry = pHandle->pNextEntry;

	retDesc->sFilename = CString(pEntry->d_name);

	std::string sFull = pHandle->sDirPath + "/" + pEntry->d_name;
	struct stat st;
	if (stat(sFull.c_str(), &st) == 0)
		{
		retDesc->bFolder = S_ISDIR(st.st_mode);
		retDesc->bReadOnly = (access(sFull.c_str(), W_OK) != 0);
		}
	else
		{
		retDesc->bFolder = false;
		retDesc->bReadOnly = false;
		}

	retDesc->bSystemFile = false;
	retDesc->bHiddenFile = (pEntry->d_name[0] == '.');

	PosixDirAdvance(pHandle);
	#endif
	}
