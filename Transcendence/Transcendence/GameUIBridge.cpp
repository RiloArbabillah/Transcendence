//	GameUIBridge.cpp
//	Full game UI integration - shows intro menu
//	Minimal path to get game UI displaying

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"

void InitGameUI(SAppState& state)
{
    CHumanInterface::Create();

    g_pHI->GetScreenMgr().Init(state.cxWidth, state.cyHeight, nullptr);
}

void UpdateGameUI(SAppState& state)
{
    if (g_pHI && g_pHI->GetSession())
    {
        g_pHI->OnAnimate();
    }
}