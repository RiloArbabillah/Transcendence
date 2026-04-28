//	GameUIBridge.cpp
//	Game UI integration

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"

int g_cxScreen = 1024;
int g_cyScreen = 768;

void InitGameUI(SAppState& state)
{
    CHumanInterface::Create();
    g_pHI->GetScreenMgr().Init(state.cxWidth, state.cyHeight, nullptr);
}

void UpdateGameUI(SAppState& state)
{
    if (g_pHI)
        g_pHI->OnAnimate();
}