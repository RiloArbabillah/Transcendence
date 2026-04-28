//	CStringStub.cpp
//	Minimal string operations for non-Windows platforms

#include "Alchemy.h"
#include "KernelString.h"

struct CXForm;
struct RECT;

namespace Kernel
{

const CString NULL_STR;

CString strPattern(const CString &sPattern, LPVOID *pArgs) { return CString(); }
bool strEquals(const CString &s1, const CString &s2) { return s1 == s2; }
int mathRound(double x) { return (int)(x >= 0 ? x + 0.5 : x - 0.5); }

int strCompareAbsolute(const Kernel::CString &s1, const Kernel::CString &s2, bool bCaseSensitive) { return 0; }

CString pathAddComponent(const CString &sPath, const CString &sComponent) { CString result = sPath; if (!sPath.IsBlank()) result.Append("/", 1); result.Append(sComponent.GetPointer(), sComponent.GetLength()); return result; }
CString pathGetExtension(const CString &sPath) { return CString(); }
CString pathGetFilename(const CString &sPath) { return sPath; }

int mathRandom(int iMin, int iMax) { return iMin + (rand() % (iMax - iMin + 1)); }

void utlMemCopy(const char *pSource, char *pDest, unsigned int dwCount) { memcpy(pDest, pSource, dwCount); }
void utlMemSet(void *pDest, unsigned int dwCount, unsigned char byValue) { memset(pDest, byValue, dwCount); }
DWORD sysGetTicksElapsed(DWORD dwTick, DWORD *retdwNow) { return 0; }
unsigned int sysGetProcessorsInMask(unsigned long AffinityMask) { return 1; }
SProcessorInfo sysGetProcessorInfo() { SProcessorInfo info = {}; return info; }

void kernelDebugLogPattern(const char *pszLine, ...) { }
void kernelDebugLogString(const Kernel::CString &sLine) { }
Kernel::CString kernelGetSessionDebugLog() { return Kernel::CString(); }
HANDLE kernelCreateThread(LPTHREAD_START_ROUTINE pfStart, LPVOID pData) { return NULL; }

}

struct CG32bitPixel {
    CG32bitPixel(unsigned short val = 0) : m_val(val) {}
    CG32bitPixel(CG32bitPixel const& other) : m_val(other.m_val) {}
    CG32bitPixel& operator=(const CG32bitPixel& other) { m_val = other.m_val; return *this; }
    operator unsigned short() const { return m_val; }
    unsigned short m_val;
};

ALERROR dibGetInfo(void *hDIB, int *retcxWidth, int *retcyHeight, void **retpBase, int *retiStride, BITMAPINFOHEADER *retpBMIH, void **retpBits) { return ERR_FAIL; }
bool dibIs16bit(void *hDIB) { return false; }
bool dibIs24bit(void *hDIB) { return false; }
ALERROR dibLoadFromFile(Kernel::CString sFilename, void **rethDIB, EBitmapTypes *retiType) { if (rethDIB) *rethDIB = nullptr; if (retiType) *retiType = bitmapNone; return ERR_FAIL; }
ALERROR dibLoadToBufferFromFile(Kernel::CString sFilespec, SBMPImageLoad *retImage) { if (retImage) { retImage->cxWidth = 0; retImage->cyHeight = 0; retImage->iPitch = 0; retImage->iType = bitmapNone; } return ERR_FAIL; }
void CalcBltTransform(double xCenter, double yCenter, double xWidth, double yWidth, double rAngle, int cxDest, int cyDest, int cxSrc, int cySrc, void *pSrcToDest, void *pDestToSrc, void *retrcSrc) { }
void CopyBltTransformed(void) { }

namespace Kernel
{

Kernel::CString::CString(void) : m_pStore(NULL) { }
Kernel::CString::~CString(void) { if (m_pStore && --m_pStore->iRefCount == 0) free(m_pStore); }
Kernel::CString::CString(const char *pString) : m_pStore(NULL) { if (pString) Transcribe(pString, -1); }
Kernel::CString::CString(Kernel::CString::CharacterSets iCharSet, const char *pString) : m_pStore(NULL) { if (pString) Transcribe(pString, -1); }
Kernel::CString::CString(const char *pString, int iLength) : m_pStore(NULL) { if (pString) Transcribe(pString, iLength); }
Kernel::CString::CString(const char *pString, int iLength, BOOL bExternal) : m_pStore(NULL) {
    if (pString) {
        if (bExternal) { if (iLength == -1) iLength = strlen(pString); GrowToFit(iLength); memcpy(m_pStore->pString, pString, iLength); m_pStore->iLength = iLength; m_pStore->pString[iLength] = '\0'; }
        else Transcribe(pString, iLength);
    }
}
Kernel::CString::CString(const Kernel::SConstString &String) : m_pStore(NULL) { if (String.pszString) Transcribe(String.pszString, String.iLen); }
Kernel::CString::CString(const Kernel::CString &pString) : m_pStore(NULL) { if (!pString.IsBlank()) { GrowToFit(pString.GetLength()); memcpy(m_pStore->pString, pString.GetPointer(), pString.GetLength()); m_pStore->iLength = pString.GetLength(); m_pStore->pString[m_pStore->iLength] = '\0'; } }
Kernel::CString &Kernel::CString::operator=(const Kernel::CString &pString) { if (this != &pString) { Truncate(0); if (!pString.IsBlank()) { GrowToFit(pString.GetLength()); memcpy(m_pStore->pString, pString.GetPointer(), pString.GetLength()); m_pStore->iLength = pString.GetLength(); m_pStore->pString[m_pStore->iLength] = '\0'; } } return *this; }
bool Kernel::CString::operator==(const Kernel::CString &sValue) const { if (GetLength() != sValue.GetLength()) return false; if (IsBlank() && sValue.IsBlank()) return true; return strcmp(GetPointer(), sValue.GetPointer()) == 0; }
bool Kernel::CString::operator!=(const Kernel::CString &sValue) const { return !(*this == sValue); }
void Kernel::CString::Append(LPCSTR pString, int iLength, DWORD dwFlags) { if (!pString) return; if (iLength == -1) iLength = strlen(pString); if (iLength == 0) return; int iOldLen = GetLength(); GrowToFit(iOldLen + iLength); memcpy(m_pStore->pString + iOldLen, pString, iLength); m_pStore->iLength = iOldLen + iLength; m_pStore->pString[m_pStore->iLength] = '\0'; }
void Kernel::CString::Capitalize(Kernel::CString::CapitalizeOptions iOption) { }
char *Kernel::CString::GetASCIIZPointer(void) const { return GetPointer(); }
int Kernel::CString::GetLength(void) const { return m_pStore ? m_pStore->iLength : 0; }
int Kernel::CString::GetMemoryUsage(void) const { return m_pStore ? sizeof(Kernel::CString::STORESTRUCT) + m_pStore->iLength + 1 : 0; }
char *Kernel::CString::GetPointer(void) const { static char sEmpty = 0; return (!m_pStore || m_pStore->iLength == 0) ? &sEmpty : m_pStore->pString; }
char *Kernel::CString::GetWritePointer(int iLength) { GrowToFit(iLength); return m_pStore->pString; }
void Kernel::CString::GrowToFit(int iLength) { if (m_pStore && m_pStore->iAllocSize >= iLength + 1) return; int iNewAlloc = ((iLength + 256) / 256) * 256; PSTORESTRUCT pNewStore = (PSTORESTRUCT)malloc(sizeof(STORESTRUCT) + iNewAlloc); if (!pNewStore) return; pNewStore->iRefCount = 1; pNewStore->iAllocSize = iNewAlloc; pNewStore->iLength = 0; pNewStore->pString = (char *)(pNewStore + 1); pNewStore->pString[0] = '\0'; if (m_pStore && m_pStore->iLength > 0) { memcpy(pNewStore->pString, m_pStore->pString, m_pStore->iLength); pNewStore->iLength = m_pStore->iLength; pNewStore->pString[pNewStore->iLength] = '\0'; } free(m_pStore); m_pStore = pNewStore; }
void Kernel::CString::ReadFromStream(Kernel::IReadStream *pStream) { }
void Kernel::CString::Transcribe(const char *pString, int iLen) { if (!pString) return; if (iLen == -1) iLen = strlen(pString); if (iLen == 0) { Truncate(0); return; } GrowToFit(iLen); memcpy(m_pStore->pString, pString, iLen); m_pStore->iLength = iLen; m_pStore->pString[iLen] = '\0'; }
void Kernel::CString::Truncate(int iLength) { if (!m_pStore) return; if (iLength < m_pStore->iLength) { m_pStore->iLength = iLength; m_pStore->pString[iLength] = '\0'; } }
void Kernel::CString::WriteToStream(Kernel::IWriteStream *pStream) const { }
void Kernel::CString::INTStringCleanUp(void) { }
Kernel::ALERROR Kernel::CString::INTStringInit(void) { return NOERROR; }
void *Kernel::CString::INTCopyStorage(void *pvStore) { return pvStore; }
void *Kernel::CString::INTGetStorage(const Kernel::CString &sString) { return (void *)sString.GetPointer(); }
void Kernel::CString::INTFreeStorage(void *pStore) { }
Kernel::CString Kernel::CString::INTMakeString(void *pvStore) { return Kernel::CString(); }
void Kernel::CString::INTSetStorage(Kernel::CString &sString, void *pvStore) { }
void Kernel::CString::INTTakeStorage(void *pvStore) { }
void Kernel::CString::InitLowerCaseAbsoluteTable(void) { }
Kernel::ALERROR Kernel::CString::LoadHandler(Kernel::CUnarchiver *pUnarchiver) { return NOERROR; }
Kernel::ALERROR Kernel::CString::SaveHandler(Kernel::CArchiver *pArchiver) { return NOERROR; }

}