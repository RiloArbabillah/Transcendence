//	CObject.cpp
//	Stub implementation for non-Windows platforms

#include "Kernel.h"

#include <mutex>
#include <unordered_set>

namespace Kernel
{

static IObjectClass *g_Classes[10][1000];
static std::mutex g_csLiveObjects;
static std::unordered_set<CObject *> g_LiveObjects;

static inline size_t GetDataDescFieldSize(const DATADESCSTRUCT &Desc)
{
    switch (Desc.iOpCode)
        {
        case DATADESC_OPCODE_INT:
        case DATADESC_OPCODE_ZERO:
        case DATADESC_OPCODE_ALLOC_SIZE32:
            return sizeof(DWORD) * Desc.iCount;

        case DATADESC_OPCODE_REFERENCE:
        case DATADESC_OPCODE_VTABLE:
            return sizeof(LPVOID) * Desc.iCount;

        case DATADESC_OPCODE_ALLOC_MEMORY:
        case DATADESC_OPCODE_ALLOC_OBJ:
            return sizeof(LPVOID);

        default:
            return 0;
        }
}

CObject::CObject (IObjectClass *pClass) : m_pClass(pClass)
    {
    std::lock_guard<std::mutex> lock(g_csLiveObjects);
    g_LiveObjects.insert(this);
    }

CObject::~CObject (void)
    {
    std::lock_guard<std::mutex> lock(g_csLiveObjects);
    g_LiveObjects.erase(this);
    }

CObject *CObject::Copy (void) { return Clone(); }

BOOL CObject::CopyData (PDATADESCSTRUCT pPos, BYTE **iopSource, BYTE **iopDest) {
    BYTE *pSource = *iopSource;
    BYTE *pDest = *iopDest;
    int iAllocSize = -1;

    while (pPos) {
        switch (pPos->iOpCode) {
            case DATADESC_OPCODE_INT:
            case DATADESC_OPCODE_REFERENCE:
            case DATADESC_OPCODE_ALLOC_SIZE32: {
                if (pPos->iOpCode == DATADESC_OPCODE_ALLOC_SIZE32)
                    iAllocSize = sizeof(DWORD) * (*((int *)pSource));

                const size_t iSize = GetDataDescFieldSize(*pPos);
                memcpy(pDest, pSource, iSize);
                pDest += iSize;
                pSource += iSize;
                break;
            }
            case DATADESC_OPCODE_ALLOC_MEMORY: {
                BYTE *pSourceMem = (BYTE *)*(LPVOID *)pSource;
                if (pSourceMem) {
                    ASSERT(iAllocSize != -1);
                    BYTE *pDestMem = (BYTE *)malloc(iAllocSize);
                    if (!pDestMem) return FALSE;
                    *((LPVOID *)pDest) = pDestMem;
                    for (int i = 0; i < iAllocSize; i++) *pDestMem++ = *pSourceMem++;
                } else {
                    *((LPVOID *)pDest) = NULL;
                }
                pSource += sizeof(LPVOID);
                pDest += sizeof(LPVOID);
                iAllocSize = -1;
                break;
            }
            case DATADESC_OPCODE_ALLOC_OBJ: {
                CObject *pObj = *(CObject **)pSource;
                if (pObj) {
                    ASSERT(*((DWORD *)pDest) == 0);
                    CObject *pCopy = pObj->Copy();
                    if (!pCopy) return FALSE;
                    *((LPVOID *)pDest) = pCopy;
                } else {
                    *((LPVOID *)pDest) = NULL;
                }
                pSource += sizeof(LPVOID);
                pDest += sizeof(LPVOID);
                break;
            }
            case DATADESC_OPCODE_EMBED_OBJ: {
                CObject *pObj = (CObject *)pSource;
                CObject *pObjDest = (CObject *)pDest;
                PDATADESCSTRUCT pObjPos = pObj->DataDescStart();
                ASSERT(sizeof(CObject) == 2 * sizeof(LPVOID));
                memcpy(pDest, pSource, sizeof(CObject));
                pDest += sizeof(CObject);
                pSource += sizeof(CObject);
                if (!CopyData(pObjPos, &pSource, &pDest)) return FALSE;
                pObjDest->CopyHandler(pObj);
                break;
            }
            case DATADESC_OPCODE_ZERO: {
                const size_t iSize = sizeof(DWORD) * pPos->iCount;
                pSource += iSize;
                memset(pDest, 0, iSize);
                pDest += iSize;
                break;
            }
            case DATADESC_OPCODE_VTABLE: {
                pSource += pPos->iCount * sizeof(LPVOID);
                pDest += pPos->iCount * sizeof(LPVOID);
                break;
            }
            default:
                ASSERT(FALSE);
        }
        pPos = DataDescNext(pPos);
    }
    *iopSource = pSource;
    *iopDest = pDest;
    return TRUE;
}

PDATADESCSTRUCT CObject::DataDescNext (PDATADESCSTRUCT pPos) {
    pPos++;
    if (pPos->iOpCode == DATADESC_OPCODE_STOP) return NULL;
    return pPos;
}

PDATADESCSTRUCT CObject::DataDescStart (void) { return GetClass()->GetDataDesc(); }

BYTE *CObject::DataStart (void) { return (BYTE *)this + sizeof(CObject); }

ALERROR CObject::Load (CUnarchiver *pUnarchiver) { return LoadHandler(pUnarchiver); }

ALERROR CObject::LoadDone (void) {
    ALERROR error;
    PDATADESCSTRUCT pPos = DataDescStart();
    VerifyDataDesc();
    BYTE *pDest = DataStart();

    while (pPos) {
        switch (pPos->iOpCode) {
            case DATADESC_OPCODE_INT:
            case DATADESC_OPCODE_ALLOC_SIZE32:
            case DATADESC_OPCODE_REFERENCE:
            case DATADESC_OPCODE_ZERO:
            case DATADESC_OPCODE_VTABLE:
                pDest += GetDataDescFieldSize(*pPos);
                break;
            case DATADESC_OPCODE_ALLOC_MEMORY:
                pDest += GetDataDescFieldSize(*pPos);
                break;
            case DATADESC_OPCODE_ALLOC_OBJ: {
                CObject *pObj = *(CObject **)pDest;
                if (pObj) if ((error = pObj->LoadDone()) != NOERROR) return error;
                pDest += sizeof(LPVOID);
                break;
            }
            case DATADESC_OPCODE_EMBED_OBJ: {
                CObject *pObj = (CObject *)pDest;
                if ((error = pObj->LoadDone()) != NOERROR) return error;
                pDest += pObj->m_pClass->GetObjSize();
                break;
            }
            default:
                ASSERT(FALSE);
        }
        pPos = DataDescNext(pPos);
    }
    return LoadDoneHandler();
}

ALERROR CObject::LoadHandler (CUnarchiver *pUnarchiver) {
    ALERROR error;
    PDATADESCSTRUCT pPos = DataDescStart();
    VerifyDataDesc();
    BYTE *pDest = DataStart();
    int iAllocSize = -1;

    while (pPos) {
        switch (pPos->iOpCode) {
            case DATADESC_OPCODE_REFERENCE: {
                for (int i = 0; i < pPos->iCount; i++) {
                    int iID;
                    if ((error = pUnarchiver->ReadData((char *)&iID, sizeof(int))) != NOERROR) goto Fail;
                    if ((error = pUnarchiver->ResolveReference(iID, (void **)pDest)) != NOERROR) goto Fail;
                    pDest += sizeof(LPVOID);
                }
                break;
            }
            case DATADESC_OPCODE_INT:
            case DATADESC_OPCODE_ALLOC_SIZE32: {
                const size_t iSize = sizeof(DWORD) * pPos->iCount;
                if ((error = pUnarchiver->ReadData((char *)pDest, iSize)) != NOERROR) goto Fail;
                if (pPos->iOpCode == DATADESC_OPCODE_ALLOC_SIZE32)
                    iAllocSize = sizeof(DWORD) * (*((int *)pDest));
                pDest += iSize;
                break;
            }
            case DATADESC_OPCODE_ALLOC_MEMORY: {
                ASSERT(iAllocSize != -1);
                char *pBlock = (char *)malloc(iAllocSize);
                if (!pBlock) { error = ERR_MEMORY; goto Fail; }
                *((LPVOID *)pDest) = pBlock;
                if ((error = pUnarchiver->ReadData((char *)pBlock, iAllocSize)) != NOERROR) goto Fail;
                pDest += sizeof(LPVOID);
                iAllocSize = -1;
                break;
            }
            case DATADESC_OPCODE_ALLOC_OBJ: {
                CObject *pObj;
                IObjectClass *pClass;
                OBJCLASSID ObjID;
                if ((error = pUnarchiver->ReadData((char *)&ObjID, sizeof(ObjID))) != NOERROR) goto Fail;
                if (ObjID) {
                    pClass = CObjectClassFactory::GetClass(ObjID);
                    if (!pClass) { error = ERR_CLASSNOTFOUND; goto Fail; }
                    pObj = pClass->Instantiate();
                    if (!pObj) { error = ERR_FAIL; goto Fail; }
                    *((LPVOID *)pDest) = (LPVOID)pObj;
                    if ((error = pObj->Load(pUnarchiver)) != NOERROR) goto Fail;
                } else {
                    *((LPVOID *)pDest) = NULL;
                }
                pDest += sizeof(LPVOID);
                break;
            }
            case DATADESC_OPCODE_EMBED_OBJ: {
                CObject *pObj = (CObject *)pDest;
                if (pPos->dwFlags & DATADESC_FLAG_CUSTOM) {
                    if ((error = LoadCustom(pUnarchiver, pDest)) != NOERROR) goto Fail;
                } else {
                    OBJCLASSID ObjID;
                    if ((error = pUnarchiver->ReadData((char *)&ObjID, sizeof(ObjID))) != NOERROR) goto Fail;
                    if (ObjID != pObj->GetClass()->GetObjID()) goto Fail;
                    if ((error = pObj->Load(pUnarchiver)) != NOERROR) goto Fail;
                }
                pDest += pObj->m_pClass->GetObjSize();
                break;
            }
            case DATADESC_OPCODE_ZERO:
            case DATADESC_OPCODE_VTABLE:
                pDest += GetDataDescFieldSize(*pPos);
                break;
            default:
                ASSERT(FALSE);
        }
        pPos = DataDescNext(pPos);
    }
    return NOERROR;
Fail:
    return error;
}

ALERROR CObject::Save (CArchiver *pArchiver) {
    ALERROR error;
    DWORD dwID = m_pClass->GetObjID();
    if ((error = pArchiver->WriteData((char *)&dwID, sizeof(DWORD))) != NOERROR) return error;
    return SaveHandler(pArchiver);
}

ALERROR CObject::SaveHandler (CArchiver *pArchiver) {
    ALERROR error;
    PDATADESCSTRUCT pPos = DataDescStart();
    VerifyDataDesc();
    BYTE *pSource = DataStart();
    int iAllocSize = -1;

    while (pPos) {
        switch (pPos->iOpCode) {
            case DATADESC_OPCODE_REFERENCE: {
                for (int i = 0; i < pPos->iCount; i++) {
                    int iID;
                    if ((error = pArchiver->Reference2ID(*(void **)pSource, &iID)) != NOERROR) goto Fail;
                    if ((error = pArchiver->WriteData((char *)&iID, sizeof(int))) != NOERROR) goto Fail;
                    pSource += sizeof(LPVOID);
                }
                break;
            }
            case DATADESC_OPCODE_INT:
            case DATADESC_OPCODE_ALLOC_SIZE32: {
                if (pPos->iOpCode == DATADESC_OPCODE_ALLOC_SIZE32)
                    iAllocSize = sizeof(DWORD) * (*((int *)pSource));
                const size_t iSize = sizeof(DWORD) * pPos->iCount;
                if ((error = pArchiver->WriteData((char *)pSource, iSize)) != NOERROR) goto Fail;
                pSource += iSize;
                break;
            }
            case DATADESC_OPCODE_ALLOC_MEMORY: {
                ASSERT(iAllocSize != -1);
                if ((error = pArchiver->WriteData((char *)*(BYTE **)pSource, iAllocSize)) != NOERROR) goto Fail;
                pSource += sizeof(LPVOID);
                iAllocSize = -1;
                break;
            }
            case DATADESC_OPCODE_ALLOC_OBJ: {
                CObject *pObj = *(CObject **)pSource;
                if (pObj == NULL) {
                    DWORD dwZero = 0;
                    if ((error = pArchiver->WriteData((char *)&dwZero, sizeof(DWORD))) != NOERROR) goto Fail;
                } else {
                    if ((error = pObj->Save(pArchiver)) != NOERROR) goto Fail;
                }
                pSource += sizeof(LPVOID);
                break;
            }
            case DATADESC_OPCODE_EMBED_OBJ: {
                CObject *pObj = (CObject *)pSource;
                if (pPos->dwFlags & DATADESC_FLAG_CUSTOM) {
                    if ((error = SaveCustom(pArchiver, pSource)) != NOERROR) goto Fail;
                } else {
                    if ((error = pObj->Save(pArchiver)) != NOERROR) goto Fail;
                }
                pSource += pObj->m_pClass->GetObjSize();
                break;
            }
            case DATADESC_OPCODE_ZERO:
            case DATADESC_OPCODE_VTABLE:
                pSource += GetDataDescFieldSize(*pPos);
                break;
            default:
                ASSERT(FALSE);
        }
        pPos = DataDescNext(pPos);
    }
    return NOERROR;
Fail:
    return error;
}

void CObject::VerifyDataDesc (void) {
#ifdef _DEBUG
    PDATADESCSTRUCT pPos = m_pClass->GetDataDesc();
    int iTotalSize = sizeof(CObject);
    if (pPos) {
        while (pPos) {
            switch (pPos->iOpCode) {
                case DATADESC_OPCODE_INT:
                case DATADESC_OPCODE_ZERO:
                case DATADESC_OPCODE_ALLOC_SIZE32:
                    iTotalSize += sizeof(DWORD) * pPos->iCount;
                    break;

                case DATADESC_OPCODE_REFERENCE:
                case DATADESC_OPCODE_VTABLE:
                    iTotalSize += sizeof(LPVOID) * pPos->iCount;
                    break;

                case DATADESC_OPCODE_ALLOC_MEMORY:
                case DATADESC_OPCODE_ALLOC_OBJ:
                    iTotalSize += sizeof(LPVOID);
                    break;
                case DATADESC_OPCODE_EMBED_OBJ: {
                    CObject *pObj = (CObject *)(((BYTE *)this) + iTotalSize);
                    iTotalSize += pObj->m_pClass->GetObjSize();
                    break;
                }
                default:
                    ASSERT(FALSE);
            }
            pPos = DataDescNext(pPos);
        }
        ASSERT(iTotalSize == m_pClass->GetObjSize());
    }
#endif
}

CObject *CObjectClassFactory::Create (OBJCLASSID ObjID) {
    IObjectClass *pClass = GetClass(ObjID);
    return pClass->Instantiate();
}

IObjectClass *CObjectClassFactory::GetClass (OBJCLASSID ObjID) {
    int iModule = OBJCLASSIDGetModule(ObjID);
    int iID = OBJCLASSIDGetID(ObjID);
    if ((iModule >= 0 && iModule < OBJCLASS_MODULE_COUNT) && (iID >= 0 && iID < 1000))
        return g_Classes[iModule][iID];
    else
        throw CException(ERR_FAIL);
}

bool CObject::IsValidPointer (CObject *pObj)
    {
    if (pObj == nullptr)
        return false;

    std::lock_guard<std::mutex> lock(g_csLiveObjects);
    return (g_LiveObjects.find(pObj) != g_LiveObjects.end());
    }

void CObjectClassFactory::NewClass (IObjectClass *pClass) {
    int iModule = OBJCLASSIDGetModule(pClass->GetObjID());
    int iID = OBJCLASSIDGetID(pClass->GetObjID());
    ASSERT(g_Classes[iModule][iID] == NULL);
    g_Classes[iModule][iID] = pClass;
}

ALERROR CObject::Flatten (CObject *pObject, CString *retsData) { return ERR_FAIL; }
ALERROR CObject::Unflatten (CString sData, CObject **retpObject) { return ERR_FAIL; }

}