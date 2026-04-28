//	CFileWriteStream.cpp
//	Stub implementation for non-Windows platforms

#include "Kernel.h"

namespace Kernel
{

CFileWriteStream::CFileWriteStream (void) : CObject(NULL), m_hFile(NULL) { }
CFileWriteStream::CFileWriteStream (const CString &sFilename, BOOL bUnique) : CObject(NULL), m_sFilename(sFilename), m_bUnique(bUnique), m_hFile(NULL) { }
CFileWriteStream::~CFileWriteStream (void) { Close(); }

ALERROR CFileWriteStream::Close (void) {
    if (m_hFile == NULL) return NOERROR;
    fclose((FILE *)m_hFile);
    m_hFile = NULL;
    return NOERROR;
}

ALERROR CFileWriteStream::Create (void) {
    ASSERT(m_hFile == NULL);
    FILE *f = fopen(m_sFilename.GetASCIIZPointer(), "wb");
    if (!f) { m_hFile = NULL; return ERR_FAIL; }
    m_hFile = (HANDLE)f;
    return NOERROR;
}

ALERROR CFileWriteStream::Open (void) {
    ASSERT(m_hFile == NULL);
    FILE *f = fopen(m_sFilename.GetASCIIZPointer(), "r+b");
    if (!f) { f = fopen(m_sFilename.GetASCIIZPointer(), "w+b"); if (!f) { m_hFile = NULL; return ERR_FAIL; } }
    m_hFile = (HANDLE)f;
    fseek((FILE *)m_hFile, 0, SEEK_END);
    return NOERROR;
}

ALERROR CFileWriteStream::Write (const char *pData, int iLength, int *retiBytesWritten) {
    if (!m_hFile) throw CException(ERR_FAIL);
    int written = fwrite(pData, 1, iLength, (FILE *)m_hFile);
    if (retiBytesWritten) *retiBytesWritten = written;
    return (written == iLength) ? NOERROR : ERR_FAIL;
}

}