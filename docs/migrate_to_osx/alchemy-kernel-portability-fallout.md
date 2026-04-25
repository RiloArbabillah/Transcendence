# Alchemy Kernel Portability Fallout

## Document Status

- Version: v1.1
- Last Updated: 2026-04-15
- Derived From: current `cmake --build --preset macos-debug` fallout after the bounded `alchemy_*` and `mammoth_tse` scaffold work
- Purpose: cluster the first real `alchemy_kernel` compile blockers into a small number of actionable portability groups so we can avoid unfocused patching

## Why this document exists

The first real macOS build attempt moved past environment setup and revealed concrete `alchemy_kernel` compile failures.

The initial `windows.h` blocker has already been reduced.

The next errors are now more localized, but they show that `alchemy_kernel` still has deeper Windows assumptions in its foundational headers.

This document groups those failures into actionable clusters before further code changes are made.

## Compile-fallout snapshot

The current build reaches `alchemy_kernel` and then fails in shared headers included from `Alchemy/Include/Kernel.h`.

Representative failure areas:

- `Alchemy/Include/TArray.h`
  - `::GetProcessHeap`
- `Alchemy/Include/Kernel.h`
  - `SYSTEMTIME`
  - `LONGLONG`
  - `HeapAlloc` / `HeapFree`
  - `CRITICAL_SECTION`
  - `InitializeCriticalSection` / `DeleteCriticalSection` / `EnterCriticalSection`
  - `WaitForSingleObject` / `WAIT_OBJECT_0`
  - `SetEvent` / `ResetEvent`
  - `CloseHandle`
  - `WIN32_FIND_DATA`
  - `HKEY`
  - `GetCurrentThreadId`
  - `ULONG64`
  - `KAFFINITY`
  - `HWND`
  - `GetKeyState` / `VK_NUMLOCK`
  - `MapVirtualKey` / `MAPVK_VK_TO_CHAR`
- `Alchemy/Include/TLinkedList.h`
  - non-Windows-independent C++ issue involving non-const lvalue references bound to temporaries
- `Alchemy/Include/Internets.h`
  - `winhttp.h`
- `Alchemy/Include/KernelString.h`
  - partially addressed by minimal non-Windows fallbacks, but this area remains part of the portability surface
- `Alchemy/Include/KernelExceptions.h`
  - Win32 SEH-specific constructor now guarded, but remains a Windows-only concern conceptually
- implementation files such as `Alchemy/Kernel/CArchiver.cpp`
  - pointer-to-`int` casts that break on 64-bit Apple Clang
  - assignment-in-condition warnings that are noisy but secondary

## Cluster 1 - Win32 heap and handle allocation model

### Symptoms

- `TArray.h` uses `::GetProcessHeap()`
- `Kernel.h` uses `HeapAlloc` and `HeapFree`

### Why it matters

This is not a one-off API call. It reflects a design assumption that the foundation layer owns memory through Win32 heap primitives.

### Suggested action

- introduce a minimal non-Windows heap path for the foundation layer
- keep the change narrow and local to allocation wrappers if possible
- avoid editing every caller individually

### Priority

- highest

This is likely the smallest high-leverage cluster because it affects multiple headers and many compile units.

## Cluster 2 - Win32 synchronization primitives

### Symptoms

- `CRITICAL_SECTION`
- `InitializeCriticalSection`
- `DeleteCriticalSection`
- `EnterCriticalSection`
- `LeaveCriticalSection`
- `WaitForSingleObject`
- `SetEvent`
- `ResetEvent`
- `CloseHandle`

### Why it matters

The foundation layer currently exposes a Win32-backed critical-section abstraction directly from `Kernel.h`.

### Suggested action

- provide a small non-Windows implementation for `CCriticalSection`
- likely back it with `std::mutex` or a similarly narrow portable primitive
- preserve the existing class surface as much as possible

### Priority

- high, immediately after heap/allocation model

## Cluster 3 - Win32 time, counter, and primitive type leakage

### Symptoms

- `SYSTEMTIME`
- `LONGLONG`
- `WORD`
- `DWORDLONG`
- `ULONG64`

### Why it matters

These types appear in public foundation headers, which means they block even basic parsing of the kernel layer.

### Suggested action

- determine whether they need:
  - portable type aliases only, or
  - a real abstraction boundary for time/date APIs
- do not guess broader behavior before heap/sync fallout is under control

### Priority

- medium-high

## Cluster 4 - Win32 OS services still exposed from `Kernel.h`

### Symptoms

- `WIN32_FIND_DATA`
- `HKEY`
- `GetCurrentThreadId`
- `KAFFINITY`
- `HWND`
- `GetKeyState`
- `MapVirtualKey`

### Why it matters

These are not generic foundation primitives. They indicate that registry, filesystem enumeration, thread identity, affinity, and UI/input helpers are still surfaced directly from the kernel foundation layer.

### Suggested action

- do not keep shimming these one by one blindly
- treat this cluster as evidence that `alchemy_kernel` mixes portable utilities with a Win32 service surface
- prefer a boundary split over continuing ad hoc fallback expansion

### Priority

- high, but as a design-split signal rather than a next micro-fix target

## Cluster 5 - Header-level Windows string/charset helpers

### Symptoms

- `CP_ACP`
- `CP_UTF8`
- `INT64`
- `IsCharAlpha`
- `IsCharAlphaNumeric`

### Current status

- this cluster has already been partially reduced with minimal non-Windows fallbacks

### Suggested action

- treat this as mostly contained unless new compile fallout points back into this area

### Priority

- lower than heap/sync clusters for now

## Cluster 6 - Windows internet stack leakage

### Symptoms

- `Alchemy/Include/Internets.h` includes `winhttp.h`

### Why it matters

This pulls platform-specific network stack assumptions into the foundation layer and is a strong signal that internet support should not live in the same “portable first” subset without extra boundary work.

### Suggested action

- keep this out of the earliest portable-core success criteria if possible
- classify it as a Win32 service surface, not a core kernel utility

### Priority

- medium-high

## Cluster 7 - Windows SEH-specific exception reporting

### Symptoms

- `EXCEPTION_POINTERS`
- `EXCEPTION_*` constants

### Current status

- the SEH-specific constructor in `KernelExceptions.h` is now guarded for `_WIN32`

### Suggested action

- treat this as contained for now
- revisit only if later compile fallout escapes the current guard

### Priority

- low for the immediate next step

## Cluster 8 - 64-bit and compiler correctness issues

### Symptoms

- `TLinkedList.h` binds non-const lvalue references to temporaries in default parameters
- multiple pointer-to-`int` casts in `Alchemy/Kernel/CArchiver.cpp` and related headers

### Why it matters

This is not a Windows portability issue. It is a compiler/language correctness issue that Apple Clang surfaces.

### Suggested action

- fix separately from platform shims
- keep it isolated so it does not get mixed into Windows abstraction work
- note that the remaining pointer-as-`int` fallout is no longer just a few local casts; it reflects storage design in `CDictionary`, `CIntArray`, `CIDTable`, and `CArchiver`

### Priority

- medium, but separate from platform shim work

## Recommended interpretation shift

The first few shim passes were useful because they moved the build past the most global blockers.

But the latest fallout shows something more structural:

- `alchemy_kernel` currently mixes portable utilities and Win32 service surface in the same foundational header layer
- `alchemy_kernel` also still contains 32-bit-era pointer storage assumptions in archive/reference-related code

That means continuing to shim one symbol at a time is becoming lower-leverage.

## Recommended next step

The best next step is now:

1. stop broadening the shim surface in `Kernel.h` unless a symbol is clearly part of a tiny portable primitive cluster
2. define a split strategy for `alchemy_kernel`:
 - portable kernel utilities
 - Win32 OS services (registry, file enumeration, event handles, input helpers, internet stack)
3. update the bounded build strategy so “portable core success” does not depend on the Win32 service surface compiling unchanged
4. treat pointer-size issues like those in `CArchiver.cpp` as a separate Apple Clang correctness track after the layer split is decided

Updated planning conclusion:

- for the earliest portable-core attempt, both the archive/reference-heavy path and the internet-heavy path should be treated as deferrable unless a later milestone slice proves they are required sooner

## Why this shift is recommended

- the build is now showing registry, window/input, thread, and internet APIs leaking from the same layer
- that is a better signal to split responsibilities than to keep adding ad hoc fallback definitions
- otherwise the port risks turning into a slow rewrite of Windows semantics inside one header

## What not to do next

- do not keep sprinkling ad hoc Win32 macros into `Kernel.h`
- do not expand `mammoth_tsui_core` or other higher targets yet
- do not try to port all of `alchemy_kernel` in one pass

## Success criteria

This fallout plan is successful when:

- the next step is guided by a boundary decision, not just by the next missing Win32 symbol
- portable-core work stops depending on increasingly broad Win32 shim expansion
- we avoid turning the port into a large, unfocused `Kernel.h` rewrite
