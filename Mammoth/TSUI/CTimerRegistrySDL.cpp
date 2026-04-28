//	CTimerRegistrySDL.cpp
//	Stub timer registry for macOS SDL platform

#include "Alchemy.h"
#include "TSUI.h"

DWORD CTimerRegistry::AddTimer(HWND hWnd, DWORD dwMilliseconds, IHICommand *pListener, const CString &sCmd, bool bRecurring)
{
    return ++m_dwNextID;
}

void CTimerRegistry::DeleteTimer(HWND hWnd, DWORD dwID)
{
}

void CTimerRegistry::FireTimer(HWND hWnd, DWORD dwID)
{
}

void CTimerRegistry::ListenerDestroyed(HWND hWnd, IHICommand *pListener)
{
    for (int i = 0; i < m_Timers.GetCount(); i++)
    {
        if (m_Timers[i].pListener == pListener)
        {
            m_Timers.Delete(i);
            i--;
        }
    }
}