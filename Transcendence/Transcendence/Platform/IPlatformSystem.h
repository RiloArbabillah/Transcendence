//	IPlatformSystem.h
//
//	Platform abstraction layer for macOS/Linux
//	Replaces Windows-specific dependencies in TSUI

#pragma once

#include <cstdint>
#include <string>

struct SPlatformTimerEntry
{
    uint32_t dwID;
    uint32_t dwIntervalMs;
    void* pListener;
    std::string sCmd;
    bool bRecurring;
};

class IPlatformTimerSystem
{
public:
    virtual ~IPlatformTimerSystem() = default;
    virtual uint32_t AddTimer(uint32_t dwIntervalMs, void* pListener, const char* pszCmd, bool bRecurring) = 0;
    virtual void DeleteTimer(uint32_t dwID) = 0;
    virtual void FireTimer(uint32_t dwID) = 0;
    virtual void ListenerDestroyed(void* pListener) = 0;
};

class IPlatformMessageQueue
{
public:
    virtual ~IPlatformMessageQueue() = default;
    virtual void PostCommand(const char* pszCmd, void* pData) = 0;
    virtual void PostTaskComplete(void* pData) = 0;
    virtual bool PeekMessage(int* pMsg, int* pWParam, void** ppLParam) = 0;
};

extern IPlatformTimerSystem* g_pTimerSystem;
extern IPlatformMessageQueue* g_pMessageQueue;

void PlatformSystem_Init();
void PlatformSystem_Shutdown();