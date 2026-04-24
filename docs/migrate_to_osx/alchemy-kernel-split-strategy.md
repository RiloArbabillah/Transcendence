# Alchemy Kernel Split Strategy

## Document Status

- Version: v1.1
- Last Updated: 2026-04-15
- Derived From: `alchemy-kernel-portability-fallout.md`, `milestone-1-source-subset.md`, `cmake-build-plan.md`
- Purpose: recommend a safer path for separating portable kernel utilities from the Win32-heavy service surface currently exposed by `Alchemy/Include/Kernel.h`

## Why this document exists

The current `alchemy_kernel` build attempt shows that the target is not simply “portable core plus a few Win32 includes.”

Instead, `Alchemy/Include/Kernel.h` currently exposes a mixed layer containing:

- portable foundation types and containers
- Win32 heap and synchronization primitives
- Win32 registry and file-enumeration surface
- Win32 input helpers and key translation
- Win32 internet and HTTP dependencies via adjacent include flow

This document recommends a split strategy so the macOS port can move forward without turning `Kernel.h` into a giant compatibility shim.

## High-level recommendation

Treat `alchemy_kernel` as two conceptual layers:

1. `kernel_portable_core`
2. `kernel_win32_services`

The goal is not necessarily to rename targets immediately.

The goal is to make the build and code changes follow this separation.

## Layer A - Portable kernel core

This layer should contain the pieces most likely to remain in the earliest macOS build path.

### Intended contents

- string and container utilities that do not require Win32 APIs
- portable math/geometry helpers
- archive/data structures after 64-bit correctness cleanup
- portable file/stream abstractions where the implementation does not require Win32 handles directly in the public header surface
- basic object model and utility templates

### Build intention

- this is the part that should eventually satisfy the “portable early foundation target” role in the macOS build

## Layer B - Win32 service surface

This layer is currently leaking through `Kernel.h` and should be treated as its own portability problem.

### Intended contents

- heap and handle ownership patterns tied to Win32 APIs
- event handles and wait semantics
- critical-section implementation tied to Win32
- registry helpers (`HKEY` and related behavior)
- Win32 file-enumeration structures (`WIN32_FIND_DATA`)
- input helpers relying on Win32 key state and virtual-key translation
- thread and affinity helpers using Win32-specific concepts
- internet stack pieces that rely on `winhttp.h`

### Build intention

- this layer should not block the earliest “portable core” success criteria
- if needed for Windows, it can remain intact behind a platform gate while the macOS path avoids or replaces it

## Immediate implications for the port

### What this means for the current `CMake` scaffold

The current `alchemy_kernel` target should no longer be treated as a guaranteed portable foundation target in the strict sense.

Instead:

- the current concrete target is useful as a discovery target
- its fallout should guide the split
- success for milestone-1 foundation bring-up may require either:
  - a bounded portable subset inside `alchemy_kernel`, or
  - a temporary separation of Win32-service-facing files and headers from the portable-core path

### What this means for coding strategy

- stop broadening compatibility shims in `Kernel.h` beyond clearly tiny primitive cases
- prefer moving Win32 service surface behind narrower boundaries
- treat registry, event handles, key-state helpers, and `winhttp` as service features, not kernel-core features

## Practical split candidates

## Candidate group 1 - Keep in the portable-core path first

These are likely the best early keepers once direct Win32 leakage is reduced:

- basic string/container infrastructure after small compiler fixes
- math/geometry helpers
- non-OS-specific utility code
- archive/data structures after 64-bit cleanup

## Candidate group 2 - Move behind a Win32 service boundary first

These are the strongest candidates to stop exposing through the “portable” path:

- heap/process-handle helpers
- `CCriticalSection` and event/wait wrappers
- registry wrappers
- file enumeration wrappers using `WIN32_FIND_DATA`
- Win32 key-state and keycode conversion helpers
- internet helpers that force `winhttp.h`

## Candidate group 3 - Separate compiler-correctness work from platform split

These are real issues, but they should not drive the platform split shape:

- pointer-to-`int` casts in `Alchemy/Kernel/CArchiver.cpp`
- lvalue-reference default-argument issue in `Alchemy/Include/TLinkedList.h`
- assignment-in-condition warnings

These should be fixed, but as a separate correctness pass after the layer boundary is clearer.

## Suggested implementation sequence

1. identify a minimum non-Win32 include surface that `Kernel.h` should expose for the macOS path
2. classify the current Win32-heavy declarations in `Kernel.h` into a service header or gated section
3. make the bounded build target depend only on the portable include surface where possible
4. only then resume compile-driven fixes in `alchemy_kernel`

## Recommended first split slice inside `Kernel.h`

The safest first slice to gate or move out is the part of `Kernel.h` that most clearly represents Win32 service surface instead of portable kernel-core behavior.

### First slice candidates

1. synchronization and wait wrappers
   - `CCriticalSection`
   - `CSmartLock`
   - `COSObject`
   - `CManualEvent`

2. Win32 file and registry service surface
   - `CResourceReadBlock`
   - `CFileDirectory`
   - `CRegKey`

3. Win32-flavored UI/input helpers and declarations near the bottom of `Kernel.h`
   - `uiCopyTextToClipboard`
   - `uiIsNumLockOn`
   - `uiGetCharFromKeyCode`
   - `sysGetProcessorsInMask`

### Why this slice first

- these declarations are obviously service-like, not container/string/math core
- they are the strongest source of current compile fallout after the initial shim passes
- moving or gating them reduces the amount of fake Win32 surface the macOS path must pretend to support

## What to leave in place for the first split

For the first boundary refactor, do not try to move everything.

Keep in place initially:

- core object model declarations
- basic containers and string declarations
- archive/data structures, even if they still need later 64-bit cleanup
- small primitive typedefs that are genuinely needed for broad parsing

## Practical next code step after this strategy

The best next code step is:

1. gate or move the synchronization/event classes and Win32 helper declarations out of the portable `Kernel.h` surface first
2. rerun the build
3. then re-evaluate whether the remaining fallout is mostly portable-core correctness or still leaking Win32 services

This is safer than continuing to add broader fake Win32 definitions into the same header.

## What not to do

- do not continue adding broad fake Win32 definitions indefinitely
- do not assume every symbol in `Kernel.h` belongs in the portable core
- do not push `mammoth_tsui_core` or higher targets forward until this boundary becomes clearer

## Recommended next step

The best next step after this document is:

1. review `Alchemy/Include/Kernel.h` as a boundary surface, not just as a missing-symbol source
2. identify which declarations can be gated or moved out of the portable path first
3. then make one bounded refactor that reduces the public Win32 service surface before resuming compile-driven fixes

## Success criteria

This split strategy is successful when:

- the port stops depending on an ever-growing fake Win32 shim inside `Kernel.h`
- the early macOS build path can target a clearer portable-core subset
- later compile fallout becomes attributable either to true portable-core issues or to explicitly deferred Win32 service surfaces
