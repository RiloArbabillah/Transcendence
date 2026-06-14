//	CTimerRegistrySDL.cpp
//	SDL-compatible timer registry for macOS

#include "Alchemy.h"
#include "TSUI.h"

DWORD CTimerRegistry::AddTimer(HWND hWnd, DWORD dwMilliseconds, IHICommand *pListener, const CString &sCmd, bool bRecurring)
{
    SEntry *pEntry = m_Timers.Insert();
    pEntry->dwID = m_dwNextID++;
    pEntry->pListener = pListener;
    pEntry->sCmd = sCmd;
    pEntry->bRecurring = bRecurring;

    ::SetTimer(hWnd, pEntry->dwID, dwMilliseconds, NULL);

    return pEntry->dwID;
}

void CTimerRegistry::DeleteTimer(HWND hWnd, DWORD dwID)
{
    for (int i = 0; i < m_Timers.GetCount(); i++)
    {
        if (m_Timers[i].dwID == dwID)
        {
            ::KillTimer(hWnd, dwID);
            m_Timers.Delete(i);
            return;
        }
    }
}

void CTimerRegistry::FireTimer(HWND hWnd, DWORD dwID)
{
    for (int i = 0; i < m_Timers.GetCount(); i++)
    {
        if (m_Timers[i].dwID == dwID)
        {
            m_Timers[i].pListener->HICommand(m_Timers[i].sCmd);
            if (!m_Timers[i].bRecurring)
            {
                ::KillTimer(hWnd, dwID);
                m_Timers.Delete(i);
            }
            return;
        }
    }
}

void CTimerRegistry::ListenerDestroyed(HWND hWnd, IHICommand *pListener)
{
    for (int i = 0; i < m_Timers.GetCount(); i++)
    {
        if (m_Timers[i].pListener == pListener)
        {
            ::KillTimer(hWnd, m_Timers[i].dwID);
            m_Timers.Delete(i);
            i--;
        }
    }
}
