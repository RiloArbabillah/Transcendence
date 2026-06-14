# Transcendence macOS Port — Bug Fix Plan

> **Last updated:** 2026-06-14
> **Branch:** `osx`
> **Status:** Phase 1–4 complete, Phase 5+ pending

---

## Overview

This document tracks all remaining unfixed bugs in the Windows-to-macOS port.
Bugs are organized into phases by priority. Each phase should be completed before
moving to the next. Within a phase, bugs are grouped by subsystem for efficient
batch commits.

---

## Phase 1: Build Blockers (CRITICAL)

**Goal:** Clean build with zero errors on macOS.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 1.1 | `Transcendence/Transcendence/CDockScreen.cpp` | 871 | `CVisualPalette` cannot convert to `CDockScreenVisuals` — blocks full executable build | Investigate type mismatch, likely need adapter or cast | OPEN |
| 1.2 | `Mammoth/TSE/CPlayerGameStats.cpp` | 1828 | `break` not in loop — already changed to `return` but needs review | Verify the intent was early-return, not break-from-switch | DONE |

**Commit group:** `fix: resolve build blockers on macOS`

---

## Phase 2: Core Runtime Stability (CRITICAL)

**Goal:** Game runs without crashes on macOS. All critical stubs produce correct behavior.

### 2A: Timer System

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 2.1 | `Mammoth/TSUI/CTimerRegistrySDL.cpp` | 7–18 | `AddTimer` didn't store entries, `FireTimer` was empty | Fixed: stores entries, calls `SetTimer`, dispatches `HICommand` | DONE |
| 2.2 | `Transcendence/Platform/CPlatformTimerSystem.cpp` | 78–89 | `FireTimer` finds timer but returns without calling listener | Left as-is: not used in practice, `CTimerRegistrySDL` handles timers | WONTFIX |

### 2B: Keyboard & Mouse Input

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 2.3 | `Alchemy/Include/Kernel.h` | 847 | `GetCursorPos` always returned (0,0) | Fixed: reads from `g_PlatformMouseX/Y` updated by SDL events | DONE |
| 2.4 | `Alchemy/Include/Kernel.h` | 1159 | `GetAsyncKeyState` always returned 0 | Fixed: calls `PlatformGetAsyncKeyState` which queries SDL mod+keyboard state | DONE |
| 2.5 | `Alchemy/Include/Kernel.h` | 1166 | `GetKeyState` always returned 0 | Fixed: calls `PlatformGetKeyState` which returns toggle state for NumLock | DONE |
| 2.6 | `Alchemy/Include/Kernel.h` | 823 | `GetClientRect` hardcoded 1024x768 | Fixed: reads from `g_PlatformWindowWidth/Height` | DONE |

### 2C: Window & Display

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 2.7 | `Alchemy/Include/Kernel.h` | 1028–1029 | `ScreenToClient`/`ClientToScreen` are no-ops | Correct for fullscreen SDL mode — no fix needed | WONTFIX |
| 2.8 | `Alchemy/Include/Kernel.h` | 728 | `GetSystemMetrics` returns 0 (fixed in Phase 3) | Fixed: returns 1920x1080 default | DONE |

### 2D: Memory & Pointers

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 2.9 | `Alchemy/Include/Kernel.h` | 1231 | `GetTickCount` called `mach_timebase_info` every frame | Fixed: caches timebase in static variable | DONE |
| 2.10 | `Alchemy/Include/Kernel.h` | 1241 | `QueryPerformanceCounter` same issue | Fixed: caches timebase | DONE |

**Commit group:** `fix: core runtime stability — timers, input, display`

---

## Phase 3: Audio & Music (HIGH)

**Goal:** Sound effects and music play correctly on macOS.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 3.1 | `Mammoth/TSUI/CMCIMixerStub.h` | 33 | `#define CMCIMixer CMCIMixerStub` conflicted with real SDL implementation | Fixed: removed the `#define` | DONE |
| 3.2 | `Alchemy/DirectXUtil/CSoundMgrSDL.cpp` | 329 | `SDL_RWops` leak on `Mix_LoadWAV_RW` failure (fixed in Phase 2) | Fixed: added `SDL_RWclose` | DONE |
| 3.3 | `Mammoth/TSE/CResourceDb.cpp` | — | External SFX file lookup has no bundle/executable-relative fallback | Add `SDL_GetBasePath()` probing similar to `CMCIMixerStub.cpp::ResolveMusicFilespec` | OPEN |

**Commit group:** `fix: audio parity — SFX path resolution`

---

## Phase 4: File I/O & Persistence (HIGH)

**Goal:** Settings persist, log files are clean, file operations work correctly.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 4.1 | `Alchemy/Kernel/CTextFileLog.cpp` | 160 | Wrote `\r\n` on macOS | Fixed: uses `\n` on non-Windows | DONE |
| 4.2 | `Alchemy/Kernel/CTextFileLog.cpp` | 17 | `m_dwSessionStart` uninitialized in filename ctor (fixed in Phase 3) | Fixed: added `m_dwSessionStart(0)` | DONE |
| 4.3 | `Alchemy/Include/Kernel.h` | 997 | `SetEndOfFile` was no-op (fixed in Phase 2) | Fixed: uses `ftruncate()` | DONE |
| 4.4 | `Alchemy/Include/Kernel.h` | 998 | `FlushFileBuffers` was no-op (fixed in Phase 2) | Fixed: uses `fsync()` | DONE |
| 4.5 | `Alchemy/Kernel/CRegKey.cpp` | entire | Registry stubs always return error 2 — settings don't persist | By-design: game uses XML file-based settings (`CGameSettings.cpp`) | WONTFIX |
| 4.6 | `Alchemy/Include/Kernel.h` | 676–678 | `UnmapViewOfFile` used hardcoded size 1 (fixed in Phase 2) | Fixed: uses `sysconf(_SC_PAGESIZE)` | DONE |

**Commit group:** `fix: file I/O and persistence`

---

## Phase 5: Graphics & Rendering (HIGH)

**Goal:** All visual output renders correctly on macOS.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 5.1 | `Alchemy/Graphics/DIB.cpp` | entire | Multiple GDI functions stubbed (CreateDIBSection, BitBlt, etc.) — BMP loading fails through this path | Implement BMP loading using raw file parsing + pixel buffer allocation. `dibLoadToBuffer` (line 530) already partial. | OPEN |
| 5.2 | `Alchemy/DirectXUtil/CG16bitFont.cpp` | 325–433 | `CreateFromFont` uses GDI (CreateCompatibleDC, GetTextMetrics, etc.) | Ensure all font creation goes through `CG16bitFontSDL.cpp` on macOS, or implement using Core Text | OPEN |
| 5.3 | `Alchemy/Graphics/DIB.cpp` | 375 | Pointer truncated to `int` on 64-bit (fixed in Phase 3) | Fixed: `(int)` → `(intptr_t)` | DONE |
| 5.4 | `Alchemy/DirectXUtil/CG16BitImage.cpp` | 1416 | Green alpha table only 1/4 copied (fixed in Phase 3) | Fixed: copy size `2*32*32` → `2*64*64` | DONE |
| 5.5 | `Alchemy/DirectXUtil/CG32bitImage.cpp` | 1755 | `delete` vs `delete[]` mismatch (fixed in Phase 3) | Fixed: `delete pbmi` → `delete [] (BYTE *)pbmi` | DONE |
| 5.6 | `Alchemy/Include/Kernel.h` | 122 | `CreateFont` returns nullptr | Low priority: `CG16bitFontSDL.cpp` handles font creation | WONTFIX |
| 5.7 | `Alchemy/Include/Kernel.h` | 130–132 | `CreateDIBitmap`/`CreateDIBSection`/`SetDIBits` return nullptr | Covered by 5.1 — BMP loading through DIB path | OPEN |

**Commit group:** `fix: graphics — DIB loading, font creation`

---

## Phase 6: User-Facing Features (MEDIUM)

**Goal:** All user-facing features work or degrade gracefully.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 6.1 | `Alchemy/Kernel/UI.cpp` | 9–55 | Clipboard operations always fail (`OpenClipboard` returns FALSE) | Implement using `NSPasteboard` via Objective-C++ or `SDL_SetClipboardText`/`SDL_GetClipboardText` | OPEN |
| 6.2 | `Alchemy/Include/Kernel.h` | 1079–1080 | `GetUserName` always returns empty string | Implement using `getlogin()` or `getpwuid(getuid())` | OPEN |
| 6.3 | `Alchemy/Include/Kernel.h` | 1113–1115 | `GetFileVersionInfoSize`/`GetFileVersionInfo`/`VerQueryValue` always return 0 | Read version from `Info.plist` bundle resource using `CFBundleGetValueForInfoDictionaryKey` | OPEN |
| 6.4 | Build config | — | Game version always `0.0.0.0` in Debug.log | Set version in CMakeLists.txt or `Info.plist` | OPEN |
| 6.5 | `Alchemy/Include/Kernel.h` | 837 | `MessageBox` prints to stderr (fixed in Phase 3) | Fixed: prints to stderr, returns IDOK | DONE |

**Commit group:** `fix: user-facing features — clipboard, username, version`

---

## Phase 7: Networking (MEDIUM)

**Goal:** Network features work or are properly disabled.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 7.1 | `Alchemy/NetUtil/CNetServer.cpp` | 85–200 | OVERLAPPED I/O stubs don't signal events — async networking broken | Rewrite using POSIX sockets with `poll()`/`select()` or macOS `kqueue` | OPEN |
| 7.2 | `Alchemy/NetUtil/CNetClient.cpp` | 55–200 | Same OVERLAPPED I/O issue | Same fix as 7.1 | OPEN |
| 7.3 | `Mammoth/TSUI/CHexarcServiceStub.cpp` | 10 | Returns NULL — online features disabled | By-design: caller null-checks. Implement when Hexarc API is needed | WONTFIX |

**Commit group:** `fix: networking — POSIX socket async I/O`

---

## Phase 8: Debug & Diagnostics (LOW)

**Goal:** Debug tools work correctly on macOS.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 8.1 | `Alchemy/Include/Kernel.h` | 1162 | `DebugBreak()` uses `__builtin_debugtrap()` — crashes in release builds | By-design: same behavior as Windows `DebugBreak()` | WONTFIX |
| 8.2 | `Mammoth/TSE/CUniverse.cpp` | 567 | `wvsprintf` undefined on macOS (fixed in Phase 4) | Fixed: replaced with `vsnprintf`+`va_start` | DONE |
| 8.3 | `Alchemy/Include/Kernel.h` | 1164 | `MapVirtualKey` returns 0 | Low priority: keyboard mapping through SDL events | WONTFIX |

**Commit group:** N/A (mostly by-design or already fixed)

---

## Phase 9: Code Quality & Cleanup (LOW)

**Goal:** Clean code, no warnings from our changes.

| # | File | Line | Bug | Fix | Status |
|---|------|------|-----|-----|--------|
| 9.1 | `Transcendence/CTranscendenceWnd.cpp` | 30–41 | Many members uninitialized in constructor (fixed in Phase 4) | Fixed: added initializers + memset for arrays | DONE |
| 9.2 | `Alchemy/Include/TSmartPtr.h` | 128–138 | Pointer-to-int truncation on 64-bit (fixed in Phase 2) | Fixed: `(int)` → `(intptr_t)` | DONE |
| 9.3 | `Alchemy/Kernel/CArchiver.cpp` | 30 | `CArchiver`/`CUnarchiver` default ctors leave members uninitialized (fixed in Phase 3) | Fixed: initialized all members | DONE |
| 9.4 | `Alchemy/Kernel/CResourceReadBlock.cpp` | 10–16 | Uninitialized members (fixed in Phase 3) | Fixed: initialized all members | DONE |

**Commit group:** N/A (already fixed)

---

## Summary by Status

| Status | Count |
|--------|-------|
| DONE | 28 |
| OPEN | 9 |
| WONTFIX | 6 |

### Remaining OPEN items (9 total)

| Priority | # | Description |
|----------|---|-------------|
| CRITICAL | 1.1 | CDockScreen.cpp type conversion error (build blocker) |
| HIGH | 3.3 | SFX file path resolution needs bundle fallback |
| HIGH | 5.1 | DIB.cpp BMP loading — GDI functions stubbed |
| HIGH | 5.2 | CG16bitFont::CreateFromFont — GDI code |
| MEDIUM | 6.1 | Clipboard operations |
| MEDIUM | 6.2 | GetUserName |
| MEDIUM | 6.3 | GetFileVersionInfo |
| MEDIUM | 6.4 | Game version 0.0.0.0 |
| MEDIUM | 7.1–7.2 | OVERLAPPED I/O networking |

### Suggested Next Steps

1. **Fix 1.1** (CDockScreen.cpp) — unblocks full build
2. **Fix 5.1 + 5.2** (DIB + Font) — unblocks visual correctness
3. **Fix 3.3** (SFX paths) — unblocks audio parity
4. **Fix 6.1** (Clipboard) — quick win using SDL API
5. **Fix 6.2** (GetUserName) — quick win using POSIX API
6. **Fix 7.1–7.2** (Networking) — complex, defer if not needed for single-player
