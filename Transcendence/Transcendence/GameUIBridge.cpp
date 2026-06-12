//	GameUIBridge.cpp
//	Game UI integration

#include "Alchemy.h"
#include "TSUI.h"
#include "Transcendence.h"
#include "Platform/AppCore.h"
#include <cstdio>
#include <cstdarg>
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <sys/stat.h>

static CTranscendenceController* g_pController = nullptr;

static void log_msg(const char* msg) {
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

static void sigsegv_handler(int sig) {
    (void)sig;
    const char* msg = "!!! SIGSEGV received !!!\nA crash occurred. The save file may be incompatible.\nPlease start a new game.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    void *frames[64];
    int frameCount = backtrace(frames, 64);
    backtrace_symbols_fd(frames, frameCount, STDOUT_FILENO);
    _exit(1);
}

static void sigabrt_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "!!! SIGABRT received !!!\n", 24);
    void *frames[64];
    int frameCount = backtrace(frames, 64);
    backtrace_symbols_fd(frames, frameCount, STDOUT_FILENO);
    _exit(1);
}

void InitGameUI(SAppState& state, const char *pszCommandLine)
{
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGABRT, sigabrt_handler);

    g_pController = new CTranscendenceController();
    g_pHI->SetController(g_pController);

    SHIOptions Options;
    Options.m_bWindowedMode = true;
    Options.m_bNoGPUAcceleration = false;
    CString sError;
    const char *pCmdLine = (pszCommandLine ? pszCommandLine : "");
    ALERROR error = g_pController->OnBoot(const_cast<char *>(pCmdLine), &Options, &sError);

    if (error == NOERROR)
    {
        if (!g_pHI->InitFromSDL((HWND)state.pWindow, Options, &sError))
            error = ERR_FAIL;
    }

    if (error == NOERROR)
        error = g_pController->OnInit(&sError);

    if (error != NOERROR)
        log_va("InitGameUI error: %d (%s)", error, sError.GetASCIIZPointer());
}

void UpdateGameUI(SAppState& state)
{
    (void)state;
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

    if (g_pHI) {
        g_pHI->OnAnimate();
    }
}
