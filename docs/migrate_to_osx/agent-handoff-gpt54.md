# Transcendence macOS Port - Agent Handoff Document

## For: GPT 5.4 (Next Agent)
**Date:** 2026-05-05
**Current Branch:** `osx` (ahead of `origin/osx` by 95 commits)

---

## Executive Summary

The macOS Apple Silicon port now builds reliably and enters the SDL main loop on the active software-renderer safety path. Several major runtime blockers have been removed, including:
- `CString::DecRefCount` external-string cleanup crash
- arm64 pointer-width truncation in dictionary/symbol-table storage
- background `CCodeChain::Boot()` crash
- SDL/Metal texture upload crash in the present path
- hardcoded macOS resource root path and no-op `SetCurrentDirectory`

The current live blocker is still runtime initialization: the app shows a black window and background universe/base-file loading still fails on the `CExtensionCollection::LoadBaseFile` path while processing `Transcendence.xml` and embedded extensions/libraries.

---

## Current Runtime State

### What is proven now
- `transcendence_app` builds successfully from the macOS CMake tree.
- App initializes SDL and enters the main loop.
- The active renderer path is explicitly forced to SDL software renderer.
- The previous SDL/Metal texture upload crash no longer appears to be the primary blocker.
- Background `kernelInit()` is active for task threads.
- Crash backtrace support exists in `GameUIBridge.cpp`.

### Current user-visible symptom
- Window appears black instead of showing title/menu UI.
- A background thread later crashes during base/embedded extension loading.

### Current active crash family
- Crash reports now consistently point to `CExtensionCollection::LoadBaseFile` in the background initialization path.
- The old main-loop `SIGSEGV` and old `CCodeChain::Boot()` crash are no longer the active blockers.

---

## Major Fixes Landed In This Session Chain

### 1. CString external storage cleanup fixed
**File:** `Alchemy/Include/KernelString.h`
- `DecRefCount()` now always uses `FreeStore(m_pStore)`.
- Fixes freeing string literals / wrong allocator path for external strings.

### 2. arm64 pointer-width dictionary fix
**Files:**
- `Alchemy/Include/Kernel.h`
- `Alchemy/Kernel/CIntArray.cpp`
- `Alchemy/Kernel/CDictionary.cpp`
- `Alchemy/Kernel/CIDTable.cpp`
- `Alchemy/Kernel/CSymbolTable.cpp`
- `Alchemy/CodeChain/CCAtomTable.cpp`
- `Alchemy/Kernel/CArchiver.cpp`

This removes 32-bit pointer truncation when dictionary/symbol-table code stores pointers on Apple Silicon.

### 3. Runtime safety path forced to software renderer
**File:** `Transcendence/Transcendence/Platform/AppCore.cpp`
- `SDL_HINT_RENDER_DRIVER` now uses `software`.
- Avoids unstable SDL/Metal texture upload path.

### 4. macOS path handling improved
**Files:**
- `Alchemy/Include/Kernel.h`
- `Transcendence/Transcendence/CResourcePathResolver.cpp`
- `Mammoth/TSE/CUniverse.cpp`

Changes:
- `SetCurrentDirectory` now calls `chdir` on macOS.
- Resource root lookup is dynamic instead of hardcoded to one developer machine.
- Debug/source XML fallback now checks macOS-appropriate repo/build-tree paths before falling back to `.tdb`.

### 5. Logging recursion bug fixed
**File:** `Alchemy/Include/Kernel.h`
- `kernelDebugLogPattern(..., const CString &)` overloads now explicitly dispatch to the variadic base function.
- Prevents recursive self-calls and stack overflow in background threads.

### 6. Assert-to-error conversions for extension load
**File:** `Mammoth/TSE/CExtensionCollection.cpp`
- Replaced some extension-load asserts with actionable `retsError` failures.
- Goal: turn `SIGTRAP` into diagnosable load errors.

### 7. Extension UNID invariant preservation attempt
**File:** `Mammoth/TSE/CExtension.cpp`
- Base file and embedded extension load paths now preserve/restore original UNID invariants after child-content processing.
- This was added because current crashes suggested UNID corruption/mutation during embedded load.

---

## Current Primary Blocker

### Blocker
Black screen plus background failure while loading the base universe definition and embedded extensions from `Transcendence.xml`.

### Most relevant stack area
- `CExtensionCollection::LoadBaseFile`
- `CExtensionCollection::LoadEmbeddedExtension`
- `CExtension::CreateBaseFile`
- `CExtension::CreateExtension`
- `CUniverse::Init`
- `CTranscendenceModel::InitBackground`

### Important interpretation
The app is now far enough along that the next blocker is not shell/bootstrap/renderer infrastructure. It is the correctness of base XML / embedded extension loading during universe initialization.

---

## Plan Alignment

### Stage 0 - Baseline Lock
Complete enough to proceed.
- App builds
- App launches
- App enters main loop
- Current top blocker is runtime initialization / visible first frame

### Stage 1 - Menu/Input Operability
Still not validated manually.
- Input bridge code exists
- Keyboard/mouse/text plumbing is present
- But black screen prevents meaningful menu usability validation

### Stage 2 - First Playable Stability
Still blocked.
- Cannot reach visible menu reliably
- Cannot start `New Game`
- Active blocker is base/embedded extension load correctness

---

## Current Working Assumptions

These appear true based on the latest code and logs:
- The software renderer path is the intended active safety path.
- The old Metal texture upload crash is not the current root blocker.
- The repo-source XML path is being preferred over `.tdb` in current debug runs.
- The next useful diagnostic likely comes from making `LoadBaseFile`/`LoadEmbeddedExtension` report exactly which embedded file or extension resolves to an invalid state.

---

## Best Next Actions

1. Keep working on `Mammoth/TSE/CExtensionCollection.cpp` and `Mammoth/TSE/CExtension.cpp`.
2. Replace any remaining assert-style failure in the current `LoadBaseFile` chain with explicit `retsError` messages.
3. Log or report which embedded file (`CoreTypesLibrary.xml`, `GalaxyLibrary.xml`, `StarsOfThePilgrim.xml`, etc.) is being loaded when the invalid state appears.
4. Once base/embedded loading succeeds, re-check whether the black screen turns into a visible title/menu frame.
5. Only after that, resume Stage 1 manual menu/input validation.

---

## Key Files To Read First

- `docs/macOS_port_status.md`
- `docs/migrate_to_osx/minimax-m27-port-completion-plan.md`
- `Mammoth/TSE/CExtensionCollection.cpp`
- `Mammoth/TSE/CExtension.cpp`
- `Mammoth/TSE/CUniverse.cpp`
- `Transcendence/Transcendence/Platform/AppCore.cpp`
- `Transcendence/Transcendence/CResourcePathResolver.cpp`
- `Alchemy/Include/Kernel.h`

---

## Build / Run Commands

```bash
cmake --preset macos-debug
cmake --build build -j8
perl -e 'alarm 20; exec @ARGV' ./build/macos-debug/Transcendence
```

---

## Known Limitations

- No live GUI inspection from CLI environment
- Audio remains stub-level / silent
- `.app` packaging not validated
- Save/settings path parity still not validated end-to-end
- Current visible-frame/menu problem still unresolved
