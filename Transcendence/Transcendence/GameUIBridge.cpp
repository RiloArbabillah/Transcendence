//	GameUIBridge.cpp
//	Game UI integration

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"
#include <cstdio>

void InitGameUI(SAppState& state)
{
    kernelInit(0);
    CHumanInterface::Create();
    g_pHI->GetScreenMgr().Init(state.cxWidth, state.cyHeight, nullptr);
}

void UpdateGameUI(SAppState& state)
{
}