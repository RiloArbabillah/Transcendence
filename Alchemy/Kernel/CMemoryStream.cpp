//	CMemoryStream.cpp
//	Stub implementation for non-Windows platforms

#include "PreComp.h"

namespace Kernel
{

static CObjectClass<CMemoryWriteStream>g_WriteClass(OBJID_CMEMORYWRITESTREAM, NULL);
static CObjectClass<CMemoryReadStream>g_ReadClass(OBJID_CMEMORYREADSTREAM, NULL);

#define ALLOC_SIZE 4096
#define DEFAULT_MAX_SIZE (1024 * 1024)

CMemoryWriteStream::CMemoryWriteStream (int iMaxSize) :
        CObject(&g_WriteClass),
        m_iMaxSize(iMaxSize),
        m_iCommittedSize(0),
        m_iCurrentSize(0),
        m_pBlock(NULL)
{
    if (m_iMaxSize == 0) m_iMaxSize = DEFAULT_MAX_SIZE;
}

CMemoryWriteStream::~CMemoryWriteStream (void) {
    if (m_pBlock) free(m_pBlock);
}

ALERROR CMemoryWriteStream::Close (void) { return NOERROR; }

ALERROR CMemoryWriteStream::Create (void) {
    if (m_pBlock == NULL) {
        m_pBlock = (char *)malloc(m_iMaxSize);
        if (m_pBlock == NULL) return ERR_MEMORY;
        m_iCommittedSize = 0;
    }
    m_iCurrentSize = 0;
    return NOERROR;
}

void CMemoryWriteStream::Seek (int iPos) {
    if (iPos < 0) return;
    else if (iPos <= m_iCommittedSize) m_iCurrentSize = iPos;
    else Write((const char *)NULL, iPos - m_iCurrentSize);
}

ALERROR CMemoryWriteStream::Write (const char *pData, int iLength, int *retiBytesWritten) {
    ASSERT(m_pBlock);
    ASSERT(iLength >= 0);

    if (m_iCurrentSize + iLength > m_iMaxSize) {
        int iNewMaxSize = m_iMaxSize * 2;
        char *pNewBlock = (char *)realloc(m_pBlock, iNewMaxSize);
        if (pNewBlock == NULL) return ERR_MEMORY;
        m_pBlock = pNewBlock;
        m_iMaxSize = iNewMaxSize;
    }

    if (pData) memcpy(m_pBlock + m_iCurrentSize, pData, iLength);
    m_iCurrentSize += iLength;
    if (retiBytesWritten) *retiBytesWritten = iLength;
    return NOERROR;
}

CMemoryReadStream::CMemoryReadStream (void) : CObject(&g_ReadClass), m_pData(NULL), m_iDataSize(0) {
#ifdef DEBUG
    m_iPos = -1;
#endif
}

CMemoryReadStream::CMemoryReadStream (char *pData, int iDataSize) : CObject(&g_ReadClass), m_pData(pData), m_iDataSize(iDataSize) {
#ifdef DEBUG
    m_iPos = 0;
#endif
}

CMemoryReadStream::~CMemoryReadStream (void) { }

ALERROR CMemoryReadStream::Read (char *pData, int iLength, int *retiBytesRead) {
    ASSERT(m_iPos >= 0);
    ASSERT(iLength >= 0);

    ALERROR error = NOERROR;

    if (m_iPos + iLength > m_iDataSize) {
        iLength = m_iDataSize - m_iPos;
        error = ERR_ENDOFFILE;
    }

    if (pData && iLength > 0) memcpy(pData, m_pData + m_iPos, iLength);
    m_iPos += iLength;
    if (retiBytesRead) *retiBytesRead = iLength;
    return error;
}

ALERROR IWriteStream::WriteChar (char chChar, int iLength) {
    if (iLength == 1) {
        Write(&chChar, 1);
    } else {
        char chBuffer[sizeof(DWORD)];
        for (int i = 0; i < sizeof(DWORD); i++) chBuffer[i] = chChar;
        while (iLength > 0) {
            int iChunk = Min((int)sizeof(DWORD), iLength);
            Write(chBuffer, iChunk);
            iLength -= iChunk;
        }
    }
    return NOERROR;
}

}