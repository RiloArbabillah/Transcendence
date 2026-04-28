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
    else
    {
        static int frame = 0;
        for (int y = 0; y < state.cyHeight; y++)
        {
            for (int x = 0; x < state.cxWidth; x++)
            {
                int idx = y * state.cxWidth + x;
                uint8_t r = (x * 255) / state.cxWidth;
                uint8_t g = (y * 255) / state.cyHeight;
                uint8_t b = 128;
                state.pFrameBuffer[idx] = (r << 16) | (g << 8) | b | 0xFF000000;
            }
        }
        frame++;
    }
}