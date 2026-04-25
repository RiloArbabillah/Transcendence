//	CFileDirectory.cpp
//
//	Implements CFileDirectory object
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

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
	#endif
	}

CFileDirectory::~CFileDirectory (void)

//	CFileDirectory destructor

	{
	#ifdef _WIN32
	if (m_hSearch != INVALID_HANDLE_VALUE)
		::FindClose(m_hSearch);
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
	return false;
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
	if (retbIsFolder)
		*retbIsFolder = false;

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
	retDesc->sFilename = NULL_STR;
	retDesc->bFolder = false;
	retDesc->bSystemFile = false;
	retDesc->bHiddenFile = false;
	retDesc->bReadOnly = false;
	#endif
	}
