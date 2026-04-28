//	CPlatformTimerSystem.cpp
//
//	SDL2-based timer system implementation
//	Replaces Windows SetTimer/KillTimer for macOS

#include "AppCore.h"
#include "IPlatformSystem.h"
#include <mutex>
#include <vector>
#include <string>

class CPlatformTimerSystem : public IPlatformTimerSystem
{
public:
    CPlatformTimerSystem();
    virtual ~CPlatformTimerSystem();

    virtual uint32_t AddTimer(uint32_t dwIntervalMs, void* pListener, const char* pszCmd, bool bRecurring) override;
    virtual void DeleteTimer(uint32_t dwID) override;
    virtual void FireTimer(uint32_t dwID) override;
    virtual void ListenerDestroyed(void* pListener) override;

private:
    struct STimerEntry
    {
        uint32_t dwID;
        uint32_t dwIntervalMs;
        void* pListener;
        std::string sCmd;
        bool bRecurring;
    };

    std::vector<STimerEntry> m_Timers;
    uint32_t m_dwNextID;
    std::mutex m_cs;
};

CPlatformTimerSystem::CPlatformTimerSystem()
    : m_dwNextID(1)
{
}

CPlatformTimerSystem::~CPlatformTimerSystem()
{
    std::lock_guard<std::mutex> lock(m_cs);
    m_Timers.clear();
}

uint32_t CPlatformTimerSystem::AddTimer(uint32_t dwIntervalMs, void* pListener, const char* pszCmd, bool bRecurring)
{
    std::lock_guard<std::mutex> lock(m_cs);

    STimerEntry entry;
    entry.dwID = m_dwNextID++;
    entry.dwIntervalMs = dwIntervalMs;
    entry.pListener = pListener;
    entry.sCmd = pszCmd ? pszCmd : "";
    entry.bRecurring = bRecurring;

    m_Timers.push_back(entry);
    return entry.dwID;
}

void CPlatformTimerSystem::DeleteTimer(uint32_t dwID)
{
    std::lock_guard<std::mutex> lock(m_cs);

    for (auto it = m_Timers.begin(); it != m_Timers.end(); ++it)
    {
        if (it->dwID == dwID)
        {
            m_Timers.erase(it);
            return;
        }
    }
}

void CPlatformTimerSystem::FireTimer(uint32_t dwID)
{
    std::lock_guard<std::mutex> lock(m_cs);

    for (auto& timer : m_Timers)
    {
        if (timer.dwID == dwID)
        {
            return;
        }
    }
}

void CPlatformTimerSystem::ListenerDestroyed(void* pListener)
{
    std::lock_guard<std::mutex> lock(m_cs);

    for (auto it = m_Timers.begin(); it != m_Timers.end(); )
    {
        if (it->pListener == pListener)
            it = m_Timers.erase(it);
        else
            ++it;
    }
}

IPlatformTimerSystem* g_pTimerSystem = nullptr;

void PlatformSystem_Init()
{
    if (!g_pTimerSystem)
        g_pTimerSystem = new CPlatformTimerSystem();
}

void PlatformSystem_Shutdown()
{
    if (g_pTimerSystem)
    {
        delete g_pTimerSystem;
        g_pTimerSystem = nullptr;
    }
}