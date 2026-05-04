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
#include <signal.h>
#include <sys/stat.h>

static int g_logFd = -1;
static CTranscendenceController* g_pController = nullptr;

static const char* GetGameUILogPath()
{
    static char sPath[1024];
    static bool bInit = false;

    if (!bInit)
    {
        const char* pHome = getenv("HOME");
        if (pHome && *pHome)
        {
            char basePath[1024];
            snprintf(basePath, sizeof(basePath), "%s/Library/Application Support", pHome);
            mkdir(basePath, 0755);

            snprintf(basePath, sizeof(basePath), "%s/Library/Application Support/Kronosaur", pHome);
            mkdir(basePath, 0755);

            snprintf(basePath, sizeof(basePath), "%s/Library/Application Support/Kronosaur/Transcendence", pHome);
            mkdir(basePath, 0755);

            snprintf(sPath, sizeof(sPath), "%s/trans_gameui.log", basePath);
        }
        else
            snprintf(sPath, sizeof(sPath), "%s", "/tmp/trans_gameui.log");

        bInit = true;
    }

    return sPath;
}

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

static void sig_handler(int sig) {
    log_msg("SIGSEGV caught!");
    char buf[128];
    snprintf(buf, sizeof(buf), "Signal %d at tick %d", sig, 0);
    log_msg(buf);
    _exit(1);
}



#include <signal.h>

static void sigsegv_handler(int sig) {
    write(STDOUT_FILENO, "!!! SIGSEGV received !!!\n", 25);
    _exit(1);
}

static void sigabrt_handler(int sig) {
    write(STDOUT_FILENO, "!!! SIGABRT received !!!\n", 24);
    _exit(1);
}

void InitGameUI(SAppState& state)
{
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGABRT, sigabrt_handler);

    fprintf(stderr, "GameUIBridge: InitGameUI starting\n");
    fflush(stderr);
    fprintf(stderr, "IG: 4a BEFORE new CTranscendenceController\n");
    fflush(stderr);
    CTranscendenceController* pTemp = new CTranscendenceController();
    fprintf(stderr, "IG: 4b AFTER new, pTemp=%p\n", (void*)pTemp);
    fflush(stderr);
    g_pController = pTemp;
    fprintf(stderr, "IG: 4c AFTER assignment\n");
    fflush(stderr);

    log_msg("IG: 5 SetController (with OnBoot)");
    log_msg("IG: 5a calling SetController");
    fflush(nullptr);
    g_pHI->SetController(g_pController);
    fflush(nullptr);
    log_msg("IG: 5b SetController returned");
    fflush(nullptr);

    log_msg("IG: 6 calling OnBoot (skip for SDL2-only)");
    SHIOptions Options;
    Options.m_bWindowedMode = true;
    Options.m_bNoGPUAcceleration = false;
    CString sError;
    char szCmdLine[1] = { '\0' };
    ALERROR error = g_pController->OnBoot(szCmdLine, &Options, &sError);
    log_va("IG: 6 OnBoot result: %d (error: %s)", error, sError.GetASCIIZPointer());

    log_msg("IG: 7 calling OnInit (testing)");
    error = g_pController->OnInit(&sError);
    log_va("IG: 7 OnInit result: %d (error: %s)", error, sError.GetASCIIZPointer());

    log_msg("IG: 8 done");
}

void UpdateGameUI(SAppState& state)
{
    static int tick = 0;
    tick++;

    int msg;
    int wParam;
    void* lParam;
    while (PlatformPeekMessage(&msg, &wParam, &lParam))
    {
        if (!g_pHI)
            break;

        if (msg == WM_HI_COMMAND)
            g_pHI->OnPostCommand((LPARAM)lParam);
        else if (msg == WM_HI_TASK_COMPLETE)
            g_pHI->OnTaskComplete((DWORD)wParam, (LPARAM)lParam);
        else if (msg == WM_TIMER)
            g_pHI->OnTimer((DWORD)wParam);
        else if (msg == WM_KEYDOWN)
            g_pHI->WMKeyDown(wParam, (DWORD)(uintptr_t)lParam);
        else if (msg == WM_KEYUP)
            g_pHI->WMKeyUp(wParam, (DWORD)(uintptr_t)lParam);
        else if (msg == WM_MOUSEMOVE)
            g_pHI->WMMouseMove(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_LBUTTONDOWN)
            g_pHI->WMLButtonDown(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_RBUTTONDOWN)
            g_pHI->WMRButtonDown(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_MBUTTONDOWN)
            g_pHI->WMMButtonDown(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_LBUTTONUP)
            g_pHI->WMLButtonUp(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_RBUTTONUP)
            g_pHI->WMRButtonUp(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_MBUTTONUP)
            g_pHI->WMMButtonUp(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)(uintptr_t)wParam);
        else if (msg == WM_MOUSEWHEEL)
            g_pHI->WMMouseWheel(
                (int)(short)HIWORD((uintptr_t)wParam),
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (DWORD)LOWORD((uintptr_t)wParam));
        else if (msg == WM_SIZE)
            g_pHI->WMSize(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam),
                (int)(uintptr_t)wParam);
        else if (msg == WM_MOVE)
            g_pHI->WMMove(
                (int)(unsigned short)LOWORD((uintptr_t)lParam),
                (int)(unsigned short)HIWORD((uintptr_t)lParam));
        else if (msg == WM_CHAR)
            g_pHI->WMChar((char)wParam, 0);
    }

    if (tick % 60 == 0) {
        log_va("UG: tick %d", tick);
    }
    if (g_pHI) {
        IHISession* pSession = g_pHI->GetSession();
        if (tick <= 5)
            log_va("UG: tick %d GetSession = %p", tick, (void*)pSession);
        if (tick <= 5)
            log_va("UG: tick %d before OnAnimate", tick);
        g_pHI->OnAnimate();
        if (tick <= 5)
            log_va("UG: tick %d after OnAnimate", tick);
    }
}
