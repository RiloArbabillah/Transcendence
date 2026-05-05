# Transcendence macOS Port - Agent Handoff Document

## For: GPT 5.4 (Next Agent)
**Date:** 2026-05-05
**Current Branch:** `osx` (ahead of `origin/osx` by 89 commits)

---

## Executive Summary

The macOS Apple Silicon port is in a **runnable but unstable** state. A critical `CString::DecRefCount` bug was fixed (crash during construction), but a **SIGSEGV crash still occurs in the main loop** after successful `OnBoot`/`OnInit`. The crash happens ~1 second after entering the main render loop.

---

## What Was Fixed This Session

### P0 Bug: CString::DecRefCount Crash

**File:** `Alchemy/Include/KernelString.h:115-127`

**Root Cause:** `DecRefCount()` incorrectly freed memory for external strings (CONSTLIT strings stored with `iAllocSize < 0`). Two problems:
1. Called `free(m_pStore->pString)` on string literal pointers (not heap-allocated)
2. Called `free(m_pStore)` instead of `FreeStore()` - `m_pStore` is from a `VirtualAlloc` memory pool, not malloc

**Fix Applied:**
```cpp
void DecRefCount (void)
{
    if (m_pStore && (--m_pStore->iRefCount) == 0)
    {
        FreeStore(m_pStore);  // Always use FreeStore - it handles external strings correctly
        m_pStore = NULL;
    }
}
```

**Note:** `FreeStore()` at `Alchemy/Kernel/CString.cpp:536` already handles external strings properly - it only frees `pString` if `iAllocSize >= 0`.

---

## Current Status: SIGSEGV in Main Loop

### Crash Timeline (from stderr output):
```
App_Run: start
App_Init: kernelInit OK
App_Init: SDL_Init OK
App_Init: create window 1024x768
App_Init: window created
App_Init: Metal layer OK
App_Init: done 1024x768
App_Run: App_Init OK, calling InitGameUI
GameUIBridge: InitGameUI starting
IG: 4a BEFORE new CTranscendenceController
IG: 4b AFTER new, pTemp=0x...  // CTranscendenceController constructed
IG: 4c AFTER assignment
IG: 5 SetController (with OnBoot)
IG: 5a calling SetController
IG: 5b SetController returned
IG: 6 calling OnBoot (skip for SDL2-only)
OnBoot: START
OnBoot: after sAppName
IG: 6 OnBoot result: 0 (error: )     // OnBoot SUCCEEDED
IG: 7 calling OnInit (testing)
IG: 7 OnInit result: 0 (error: )     // OnInit SUCCEEDED
IG: 8 done
App_Run: InitGameUI returned
App_Run: entering main loop
!!! SIGSEGV received !!!
Exit: 1
```

### Key Observations:
1. **OnBoot and OnInit both succeed** (return error 0)
2. **Crash happens in the main loop**, not during initialization
3. Signal is SIGSEGV (segmentation fault)
4. Crash occurs ~1 second after entering main loop

### Signal Handler Location:
`Transcendence/Transcendence/GameUIBridge.cpp:80-83` - currently just prints message and exits

### Debugging Suggestions:
1. Add `execinfo.h` backtrace capture to `sigsegv_handler` to get stack trace
2. The crash is likely in `UpdateGameUI()` or `App_Run()` main loop code
3. Could be in `g_pHI->OnAnimate()` or message handling

---

## Code Audit Findings (Stage 1 Complete)

### Input Handling - Appears Correct
**Keyboard:** `AppCore.cpp:345-432` - Complete VK mapping table for A-Z, 0-9, arrows, keypad, function keys

**Mouse Button Mapping:** `AppCore.cpp:462-463, 474-475`
- Uses hardcoded `2`/`3` but values ARE correct: SDL_BUTTON_MIDDLE=2, SDL_BUTTON_RIGHT=3
- Could use `SDL_BUTTON_RIGHT`/`SDL_BUTTON_MIDDLE` constants for clarity (Plan item #3)

**Coordinate Packing:** `AppCore.cpp:457,468,480,486` - Uses `MAKELONG(x,y)` correctly

**Text Input:** `AppCore.cpp:450-453` - Posts one `WM_CHAR` per byte (ASCII-safe)

### Win32 API Stubs (macOS compatibility)
`Alchemy/Include/Kernel.h:778-794` - Has proper macOS implementations for:
- `GetModuleFileName` using `_NSGetExecutablePath`
- `SetCurrentDirectory` returns 1 (no-op, but not crashing)

---

## Execution Plan Reference

**File:** `docs/migrate_to_osx/minimax-m27-port-completion-plan.md`

### Stage 0 - Baseline Lock: ✅ COMPLETE
- App builds and runs to main loop
- Window creates successfully
- Signal handlers active

### Stage 1 - Menu/Input Operability: IN PROGRESS
- Code review shows input handling appears correct
- **No live GUI testing performed** (CLI environment)
- Need manual QA validation

### Stage 2 - First Playable Stability: BLOCKED
- **P0: SIGSEGV crash in main loop**
- OnBoot/OnInit succeed, but crash occurs after

### Remaining Stages (3-8): NOT STARTED
- Stage 3: macOS Runtime Paths (resources, saves, settings)
- Stage 4: Native Audio Backend
- Stage 5: `.app` Bundle Packaging
- Stage 6: Lifecycle, Retina, Fullscreen
- Stage 7: QA Gate
- Stage 8: Performance Profiling

---

## Files Modified This Session

| File | Change |
|------|--------|
| `Alchemy/Include/KernelString.h` | Fixed DecRefCount to use FreeStore() |
| `Transcendence/Transcendence/CTranscendenceController.cpp` | Added debug logging to OnBoot |
| `Transcendence/Transcendence/GameUIBridge.cpp` | Signal handlers for crash capture |

---

## Next Actions for GPT 5.4

### Immediate Priority 1: Fix Main Loop SIGSEGV

1. **Add backtrace to signal handler:**
```cpp
#include <execinfo.h>
static void sigsegv_handler(int sig) {
    write(STDOUT_FILENO, "!!! SIGSEGV received !!!\n", 25);
    void* buffer[100];
    int n = backtrace(buffer, 100);
    backtrace_symbols_fd(buffer, n, STDOUT_FILENO);
    _exit(1);
}
```

2. **Run with longer timeout to capture crash in debugger:**
```bash
cd build/macos-debug
lldb ./Transcendence
(lldb) run
# Wait for crash, then: bt
```

3. **Suspected crash locations to investigate:**
   - `UpdateGameUI()` at `GameUIBridge.cpp:130`
   - `App_Run()` at `AppCore.cpp` main loop
   - `g_pHI->OnAnimate()` call chain

### Priority 2: Manual Menu/Input QA

Once crash is fixed, validate:
- Keyboard navigation in menu (arrow keys, enter, escape)
- Mouse hover and click on menu items
- Text input if applicable

### Priority 3: Continue Execution Plan
After stable runtime:
- Stage 3: Resource/save path parity
- Stage 4: Native audio
- Stage 5: `.app` packaging

---

## Build Commands

```bash
# Debug build
cmake --preset macos-debug
cmake --build build -j8

# Run
./build/Transcendence

# Or with longer timeout
perl -e 'alarm 15; exec @ARGV' ./build/Transcendence
```

---

## Key Files Reference

| File | Purpose |
|------|---------|
| `Transcendence/Transcendence/Platform/AppCore.cpp` | SDL event bridge, main loop |
| `Transcendence/Transcendence/GameUIBridge.cpp` | GameUI init, signal handlers |
| `Alchemy/Include/KernelString.h` | CString DecRefCount |
| `Alchemy/Kernel/CString.cpp` | CString FreeStore implementation |
| `docs/migrate_to_osx/minimax-m27-port-completion-plan.md` | Full execution plan |
| `docs/migrate_to_osx/qa-test-matrix.md` | QA validation checklist |

---

## Known Limitations

1. **No live GUI testing possible** from CLI - need manual QA
2. **Audio is stub-level** - silent on macOS
3. **No `.app` packaging** yet
4. **Save/settings paths** may not be macOS-native
5. **SDL software renderer** used instead of Metal (safety path)

---

## Contact/Context

- **Project:** Transcendence Dev (kronosaur/TranscendenceDev)
- **Platform:** macOS Apple Silicon (arm64)
- **Build System:** CMake with Xcode generator
- **Last Working Commit:** `79590058` (kernelInit consolidation)
