//	CFileReadBlock.cpp
//
//	Implements CFileReadBlock object
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

static CObjectClass<CFileReadBlock>g_Class(OBJID_CFILEREADBLOCK, NULL);

CFileReadBlock::CFileReadBlock (void) :
		CObject(&g_Class),
		m_pFile(NULL),
		m_hFileMap(NULL),
		m_hFile(NULL),
		m_dwFileSize(0)

//	CFileReadBlock constructor

	{
	}

CFileReadBlock::CFileReadBlock (const CString &sFilename) :
		CObject(&g_Class),
		m_sFilename(sFilename),
		m_pFile(NULL),
		m_hFileMap(NULL),
		m_hFile(NULL),
		m_dwFileSize(0)

//	CFileReadBlock constructor

	{
	}

CFileReadBlock::~CFileReadBlock (void)

//	CFileReadBlock destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CFileReadBlock::Close (void)

//	CFileReadBlock
//
//	Close the stream

	{
	if (m_hFile == NULL)
		return NOERROR;

	//	Close the file

	if (m_pFile && m_pFile != MAP_FAILED)
		{
#ifdef TARGET_PLATFORM_MACOS
		munmap(m_pFile, m_dwFileSize);
#else
		UnmapViewOfFile(m_pFile);
#endif
		}

	CloseHandle(m_hFileMap);
	CloseHandle(m_hFile);
	m_pFile = NULL;
	m_hFileMap = NULL;
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CFileReadBlock::Open (void)

//	Open
//
//	Opens the stream for reading

	{
	if (m_hFile)
		return NOERROR;

	m_hFile = CreateFile(m_sFilename.GetASCIIZPointer(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;

		switch (::GetLastError())
			{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				return ERR_NOTFOUND;

			default:
				return ERR_FAIL;
			}
		}

	//	Open a file mapping

	//	Figure out the size of the file before mapping.

	m_dwFileSize = ::GetFileSize(m_hFile, NULL);

#ifdef TARGET_PLATFORM_MACOS
	if (m_dwFileSize == 0 || m_dwFileSize == INVALID_SET_FILE_POINTER)
		{
		if (m_dwFileSize == INVALID_SET_FILE_POINTER)
			{
			CloseHandle(m_hFile);
			m_hFile = NULL;
			return ERR_FAIL;
			}
		//	Empty file: close the file handle and set m_pFile to a safe
		//	non-NULL address so that GetPointer() doesn't return NULL + offset.

		m_pFile = (char *)"";
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return NOERROR;
		}

	m_pFile = (char *)mmap(NULL,
			m_dwFileSize,
			PROT_READ,
			MAP_PRIVATE,
			(int)(intptr_t)m_hFile,
			0);
	if (m_pFile == MAP_FAILED)
		{
		CloseHandle(m_hFile);
		m_pFile = NULL;
		m_hFile = NULL;
		return ERR_FAIL;
		}

	m_hFileMap = NULL;
	return NOERROR;
#endif

	m_hFileMap = CreateFileMapping(m_hFile,
			NULL,
			PAGE_READONLY,
			0,
			0,
			NULL);
	if (!m_hFileMap)
		{
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Map a view of the file

	m_pFile = (char *)MapViewOfFile(m_hFileMap,
			FILE_MAP_READ,
			0,
			0,
			0);
	if (m_pFile == NULL)
		{
		CloseHandle(m_hFileMap);
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	return NOERROR;
	}
