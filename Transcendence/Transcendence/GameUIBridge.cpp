//	GameUIBridge.cpp
//	Game UI integration

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"

int g_cxScreen = 1024;
int g_cyScreen = 768;

static CTranscendenceController* g_pController = nullptr;

void InitGameUI(SAppState& state)
{
    CHumanInterface::Create();
    g_pHI->GetScreenMgr().Init(state.cxWidth, state.cyHeight, nullptr);

    g_pController = new CTranscendenceController();
    g_pHI->SetController(g_pController);

    SHIOptions Options;
    CString sError;
    if (g_pController->HIBoot("", &Options, &sError) != NOERROR)
        {
        printf("HIBoot failed: %s\n", sError.GetASCIIZPointer());
        delete g_pController;
        g_pController = nullptr;
        return;
        }

    if (g_pController->HIInit(&sError) != NOERROR)
        {
        printf("HIInit failed: %s\n", sError.GetASCIIZPointer());
        }
}

void UpdateGameUI(SAppState& state)
{
    if (g_pHI)
        g_pHI->OnAnimate();
}