# Alchemy Kernel Service Surface Next Slice

## Document Status

- Version: v1.1
- Last Updated: 2026-04-15
- Derived From: `alchemy-kernel-split-strategy.md`, `alchemy-kernel-portability-fallout.md`, current `cmake --build --preset macos-debug` output
- Purpose: identify the safest next Win32-service boundary slice inside `alchemy_kernel` after the initial synchronization/event and header-helper reductions

## Purpose

This document narrows the next boundary step inside the `alchemy_kernel` Win32 service surface.

The goal is to avoid a broad refactor and instead choose one bounded slice that reduces compile fallout without breaking active dependencies blindly.

## What was evaluated

The following Win32-service-facing areas were reviewed:

- `CResourceReadBlock`
- `CFileDirectory`
- `CRegKey`
- the surrounding file/internet/helper declarations that still leak through `Kernel.h`

## Key findings

### `CResourceReadBlock` is an active dependency

- implementation: `Alchemy/Kernel/CResourceReadBlock.cpp`
- active use on the current compile path: `Alchemy/Kernel/CDataFile.cpp:741`

Interpretation:

- do not move or gate this first without a more deliberate replacement or path-aware fallback

### `CFileDirectory` is an active dependency

- implementation: `Alchemy/Kernel/CFileDirectory.cpp`
- active use on the current compile path: `Alchemy/Kernel/Path.cpp:424`

Interpretation:

- do not move or gate this first without handling the immediate file-enumeration impact on `Path.cpp`

### `CRegKey` is lower-risk than the file classes

- implementation: `Alchemy/Kernel/CRegKey.cpp`
- no evidence that it is part of the active compile blockers beyond header exposure

Interpretation:

- this remains a good service-surface candidate to keep out of the portable path, but it is not the highest-leverage blocker by itself

### `CDataFile.cpp` is the first true implementation-heavy Win32 file boundary

Observed compile fallout shows `Alchemy/Kernel/CDataFile.cpp` relying directly on Win32 file APIs such as:

- `CreateFile`
- `WriteFile`
- `DeleteFile`
- `SetFilePointer`
- file access and creation flags like `GENERIC_READ`, `CREATE_ALWAYS`, and `FILE_BEGIN`

Interpretation:

- this is a stronger signal than the header-only declarations
- `CDataFile.cpp` is a better candidate for the next boundary-focused decision than trying to blindly gate the file/resource classes from `Kernel.h`

## Recommended next slice

The safest next slice is:

1. keep `CResourceReadBlock` and `CFileDirectory` in place for now
2. keep `CRegKey` as gated/stubbed service surface where possible
3. treat `CDataFile.cpp` as the next true Win32 file-service boundary candidate

## Why this slice is safer

- it respects active dependencies already visible in the compile path
- it avoids breaking `Path.cpp` and resource-backed read paths prematurely
- it targets a concrete implementation file whose Win32 coupling is explicit and concentrated

## What this means for the next code step

The next code step should not be “move all file service classes out of `Kernel.h`.”

It should be one of these bounded decisions:

- define a portable-vs-Win32 handling strategy specifically for `CDataFile.cpp`
- or, if we want to defer deeper file work, pivot temporarily to the separate 64-bit correctness track (`CArchiver.cpp`, pointer-as-int storage)

## Recommended choice

Recommended next choice:

1. do not widen the header-level split further yet
2. choose between:
   - `CDataFile.cpp` file-service boundary work, or
   - the 64-bit correctness cleanup track

Given the current build output, the more bounded and lower-risk next coding track is likely the 64-bit correctness cleanup.

## Follow-up conclusion after deeper 64-bit inspection

After inspecting the remaining pointer-as-`int` fallout more closely, the 64-bit track is no longer a purely local cast-fix problem.

The problem reaches into the storage model used by:

- `CDictionary`
- `CIntArray`
- `CIDTable`
- `CArchiver` / `CUnarchiver`

This means the remaining 64-bit failures should now be treated as a design-level archive/reference-storage issue, not as a short sequence of safe local edits.

## Updated recommendation

Because the 64-bit issue is deeper than first expected, the safest next step is now:

1. avoid further local cast cleanup in archive/reference code for the moment
2. treat archive/reference-heavy kernel paths as candidates for deferral from the earliest portable-core success criteria
3. if continued coding is needed immediately, prefer a bounded `CDataFile.cpp` service-boundary slice over deeper pointer-storage redesign

## Bounded `CDataFile` slice for build bring-up

The safest first implementation slice for `CDataFile.cpp` is not a full portable rewrite.

It is a bounded non-Windows behavior split:

1. keep `OpenFromResource(...)` and the `m_pFile` / `IReadBlock` read path intact as much as possible
2. treat direct Win32 file-handle operations as unsupported on non-Windows for now
3. make the non-Windows path fail explicitly with `ERR_FAIL` instead of forcing broad Win32 shims

### Smallest candidate methods for this slice

- `CDataFile::Create(...)`
- `CDataFile::Open(...)`
- `CDataFile::WriteBlockChain(...)`
- the file-handle branch inside `CDataFile::ReadBuffer(...)`

### Why this slice is useful

- it isolates the implementation hotspot with the clearest Win32 coupling
- it avoids touching `CFileDirectory` and `CResourceReadBlock`, which are active dependencies elsewhere
- it can reduce compile fallout from file-service constants and Win32 write/seek APIs without forcing a full file-I/O port immediately

## Minimum header-boundary work needed before `CDataFile` becomes verifiable again

Before this `CDataFile` slice can be verified cleanly by the current build, the include surface still needs one more bounded reduction in `Kernel.h`:

- avoid forcing `WIN32_FIND_DATA` into the portable path while `CFileDirectory` remains present
- keep `CRegKey` and other clearly non-core service declarations out of the portable surface where possible

This means the practical order is:

1. one more bounded header-boundary reduction
2. then apply the `CDataFile.cpp` non-Windows service split
3. then rerun the build

## Success criteria

This document is successful when:

- we stop guessing which service-surface class to move next
- the next coding step is chosen from actual active dependencies
- file-service refactoring proceeds from concentrated implementation hotspots instead of broad header surgery
