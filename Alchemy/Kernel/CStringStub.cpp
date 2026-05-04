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
CString pathGetExtension(const CString &sPath) { int i = 0; for (int n = sPath.GetLength() - 1; n >= 0; n--) { if (sPath.GetPointer()[n] == '.') { i = n + 1; break; } if (sPath.GetPointer()[n] == '/' || sPath.GetPointer()[n] == '\\') break; } return CString(sPath.GetPointer() + i, sPath.GetLength() - i); }
CString pathGetFilename(const CString &sPath) { return sPath; }

int mathRandom(int iMin, int iMax) { return iMin + (rand() % (iMax - iMin + 1)); }

CString strFromInt(int iInteger, bool bSigned) { char buffer[32]; snprintf(buffer, sizeof(buffer), bSigned ? "%d" : "%u", iInteger); return CString(buffer); }

CString strProcess(const CString &sValue, DWORD dwFlags) { return sValue; }
CString strToLower(const CString &sString) { return sString; }
CString strToUpper(const CString &sString) { return sString; }
CString strCapitalize(const CString &sString, int iOffset) { return sString; }
CString strFromDouble(double rValue, int iDecimals) { char buffer[64]; snprintf(buffer, sizeof(buffer), "%.*f", iDecimals, rValue); return CString(buffer); }
CString strSubString(const CString &sString, int iOffset, int iLength) { if (iOffset >= sString.GetLength()) return CString(); int len = (iLength < 0) ? sString.GetLength() - iOffset : iLength; len = Min(len, sString.GetLength() - iOffset); return CString(sString.GetPointer() + iOffset, len); }
CString strToXMLText(const CString &sString, bool bInBody) { return sString; }
double strToDouble(const CString &sString, double rFailResult, bool *retbFailed) { if (retbFailed) *retbFailed = false; return rFailResult; }
int strParseInt(const char *pStart, int iNullResult, DWORD dwFlags, const char **retpEnd, bool *retbNullValue) { if (retpEnd) *retpEnd = pStart; if (retbNullValue) *retbNullValue = false; return iNullResult; }
ALERROR strDelimitEx(const CString &sString, char cDelim, DWORD dwFlags, int iMinParts, TArray<CString> *retList) { if (retList) retList->DeleteAll(); return NOERROR; }
int strFindCount(const CString &sString, const CString &sStringToFind, bool bCaseSensitive) { return 0; }
CString strDelimitGet(const CString &sString, char cDelim, DWORD dwFlags, int iIndex) { return CString(); }
bool strEqualsCase(const CString &sString1, const CString &sString2) { return sString1 == sString2; }

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

namespace Kernel
{

static char s_EmptyString = 0;

CString::CString(void) : m_pStore(NULL) { }
CString::~CString(void) { if (m_pStore) { if (--m_pStore->iRefCount == 0) { free(m_pStore->pString); free(m_pStore); } m_pStore = NULL; } }
CString::CString(const char *pString) : m_pStore(NULL) { if (pString) Transcribe(pString, -1); }
CString::CString(Kernel::CString::CharacterSets iCharSet, const char *pString) : m_pStore(NULL) { if (pString) Transcribe(pString, -1); }
CString::CString(const char *pString, int iLength) : m_pStore(NULL) { if (pString) Transcribe(pString, iLength); }
CString::CString(const char *pString, int iLength, BOOL bExternal) : m_pStore(NULL) {
    if (pString) {
        if (bExternal) { if (iLength == -1) iLength = strlen(pString); GrowToFit(iLength); if (m_pStore) { memcpy(m_pStore->pString, pString, iLength); m_pStore->iLength = iLength; m_pStore->pString[iLength] = '\0'; } }
        else Transcribe(pString, iLength);
    }
}
CString::CString(const Kernel::SConstString &String) : m_pStore(NULL) { if (String.pszString) Transcribe(String.pszString, String.iLen); }
CString::CString(const Kernel::CString &pString) : m_pStore(NULL) { m_pStore = pString.m_pStore; if (m_pStore) m_pStore->iRefCount++; }
Kernel::CString &Kernel::CString::operator=(const Kernel::CString &pString) { if (this != &pString) { if (m_pStore) { if (--m_pStore->iRefCount == 0) { free(m_pStore->pString); free(m_pStore); } } m_pStore = pString.m_pStore; if (m_pStore) m_pStore->iRefCount++; } return *this; }
bool Kernel::CString::operator==(const Kernel::CString &sValue) const { if (GetLength() != sValue.GetLength()) return false; if (IsBlank() && sValue.IsBlank()) return true; return strcmp(GetPointer(), sValue.GetPointer()) == 0; }
bool Kernel::CString::operator!=(const Kernel::CString &sValue) const { return !(*this == sValue); }
void Kernel::CString::Append(LPCSTR pString, int iLength, DWORD dwFlags) { if (!pString) return; if (iLength == -1) iLength = strlen(pString); if (iLength == 0) return; int iOldLen = GetLength(); GrowToFit(iOldLen + iLength); memcpy(m_pStore->pString + iOldLen, pString, iLength); m_pStore->iLength = iOldLen + iLength; m_pStore->pString[m_pStore->iLength] = '\0'; }
void Kernel::CString::Capitalize(Kernel::CString::CapitalizeOptions iOption) { }
char *Kernel::CString::GetASCIIZPointer(void) const { return m_pStore ? m_pStore->pString : &s_EmptyString; }
int Kernel::CString::GetLength(void) const { return m_pStore ? m_pStore->iLength : 0; }
int Kernel::CString::GetMemoryUsage(void) const { return m_pStore ? sizeof(Kernel::CString::STORESTRUCT) + m_pStore->iLength + 1 : 0; }
char *Kernel::CString::GetPointer(void) const { return (!m_pStore || m_pStore->iLength == 0) ? &s_EmptyString : m_pStore->pString; }
char *Kernel::CString::GetWritePointer(int iLength) { GrowToFit(iLength); return m_pStore ? m_pStore->pString : &s_EmptyString; }
void Kernel::CString::GrowToFit(int iLength) { if (m_pStore && m_pStore->iAllocSize >= iLength + 1) return; int iNewAlloc = ((iLength + 256) / 256) * 256; PSTORESTRUCT pNewStore = (PSTORESTRUCT)malloc(sizeof(STORESTRUCT) + iNewAlloc); if (!pNewStore) return; pNewStore->iRefCount = 1; pNewStore->iAllocSize = iNewAlloc; pNewStore->iLength = m_pStore ? m_pStore->iLength : 0; pNewStore->pString = (char *)(pNewStore + 1); if (m_pStore && m_pStore->iLength > 0) { memcpy(pNewStore->pString, m_pStore->pString, m_pStore->iLength); pNewStore->pString[pNewStore->iLength] = '\0'; } if (m_pStore) { if (--m_pStore->iRefCount == 0) { free(m_pStore->pString); free(m_pStore); } } m_pStore = pNewStore; }
void Kernel::CString::ReadFromStream(Kernel::IReadStream *pStream) { }
void Kernel::CString::Transcribe(const char *pString, int iLen) { if (!pString) return; if (iLen == -1) iLen = strlen(pString); if (iLen == 0) { Truncate(0); return; } GrowToFit(iLen); if (m_pStore) { memcpy(m_pStore->pString, pString, iLen); m_pStore->iLength = iLen; m_pStore->pString[iLen] = '\0'; } }
void Kernel::CString::Truncate(int iLength) { if (!m_pStore) return; if (iLength < m_pStore->iLength) { m_pStore->iLength = iLength; m_pStore->pString[iLength] = '\0'; } }

}

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