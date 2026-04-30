//	GameUIBridge.cpp
//	Game UI integration

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"
#include <cstdio>
#include <cstdarg>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

static int g_logFd = -1;

static void log_msg(const char* msg) {
    if (g_logFd >= 0) {
        write(g_logFd, msg, strlen(msg));
        write(g_logFd, "\n", 1);
    }
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

static void log_va(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log_msg(buf);
}

void InitGameUI(SAppState& state)
{
    g_logFd = open("/tmp/trans_gameui.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    log_msg("IG: 1 kernelInit");
    kernelInit(0);

    log_msg("IG: 2 CHumanInterface::Create");
    CHumanInterface::Create();

    log_msg("IG: 3 GetScreenMgr().Init");
    g_pHI->GetScreenMgr().Init(state.cxWidth, state.cyHeight, nullptr);

    log_msg("IG: 4 new CTranscendenceController");
    CTranscendenceController* pController = new CTranscendenceController();

    log_msg("IG: 5 HIBoot start");

    SHIOptions Options;
    memset(&Options, 0, sizeof(Options));
    CString sError;

    log_msg("IG: 5b calling OnBoot");
    ALERROR error = pController->OnBoot(NULL, &Options, &sError);
    log_msg("IG: 5c OnBoot returned");

    log_va("IG: OnBoot error=%d", error);

    if (error != NOERROR)
    {
        log_msg("IG: OnBoot failed");
        return;
    }

    log_msg("IG: 6 SetController");
    g_pHI->SetController(pController);

    log_msg("IG: 7 OnInit");
    pController->OnInit(&sError);

    log_msg("IG: 8 done");
}

void UpdateGameUI(SAppState& state)
{
    static int tick = 0;
    tick++;
    if (tick % 60 == 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "UG: tick %d\n", tick);
        write(g_logFd >= 0 ? g_logFd : 2, buf, strlen(buf));
    }
}