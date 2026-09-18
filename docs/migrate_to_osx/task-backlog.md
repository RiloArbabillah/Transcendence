# Task Backlog

## Document Status

- Version: v1.5
- Last Updated: 2026-09-19
- Derived From: `PRD.md`
- Companion Document: `roadmap.md`

## Purpose

This backlog converts the roadmap into actionable engineering work. Tasks are grouped by epic, ordered by dependency, and written to support phased execution.

## Status Legend

- `todo` - not started
- `in_progress` - actively being worked
- `blocked` - waiting on another task or decision
- `done` - completed
- `deferred` - intentionally postponed

## Priority Legend

- `P0` - critical path
- `P1` - high value, next after critical path
- `P2` - important but not blocking early milestones
- `P3` - later optimization or polish

## Epic A - Audit and Architecture

### A-001 Inventory Windows-only dependencies

- Priority: `P0`
- Status: `todo`
- Goal: identify all Win32, DirectX, GDI, MCI, and platform-specific assumptions in the current source tree
- Scope:
  - startup and app shell
  - rendering and presentation
  - graphics/font/resource loading
  - audio
  - save/settings paths
  - Steam/cloud integration
- Likely source areas:
  - `Transcendence/Transcendence/Main.cpp`
  - `Transcendence/Transcendence/CTranscendenceWnd.cpp`
  - `Mammoth/TSUI/Run.cpp`
  - `Mammoth/TSUI/CHumanInterface.cpp`
  - `Alchemy/DirectXUtil/*`
  - `Alchemy/Graphics/DIB.cpp`
  - `Alchemy/Graphics/GDI.cpp`
  - `Mammoth/TSUI/CMCIMixer.cpp`
- Acceptance criteria:
  - each dependency is labeled as `replace`, `wrap`, `stub`, or `defer`

### A-002 Define subsystem boundaries

- Priority: `P0`
- Status: `todo`
- Goal: define clean interfaces for platform, input, renderer, audio, and filesystem boundaries
- Depends on:
  - A-001
- Acceptance criteria:
  - interface ownership is documented
  - SDL and Metal do not leak into shared engine headers

### A-003 Confirm v1 feature scope

- Priority: `P0`
- Status: `todo`
- Goal: lock the first playable target as native menu plus first gameplay slice
- Decisions required:
  - Steam disabled for v1
  - cloud/Hexarc contributor-safe path only
  - compatibility-first renderer retained
- Depends on:
  - A-001
  - A-002
- Acceptance criteria:
  - v1 scope is documented and no longer ambiguous

## Epic B - Build System and Toolchain

### B-001 Add root `CMakeLists.txt`

- Priority: `P0`
- Status: `done`
- Goal: create a parallel build system for macOS without replacing Visual Studio projects
- Depends on:
  - A-002
- Acceptance criteria:
  - project configures on macOS with `cmake`
- Current validation: `cmake --preset macos-debug` succeeds locally

### B-002 Add `CMakePresets.json`

- Priority: `P1`
- Status: `done`
- Goal: provide repeatable debug/release/sanitizer presets for macOS arm64
- Depends on:
  - B-001
- Acceptance criteria:
  - at least debug and release-oriented presets exist
- Current validation: `macos-debug`, `macos-relwithdebinfo`, and `macos-release` presets exist

### B-003 Define core library targets

- Priority: `P0`
- Status: `in_progress`
- Goal: mirror the existing logical project graph in CMake
- Initial targets:
  - `Alchemy/Kernel`
  - `Alchemy/CodeChain`
  - `Alchemy/XMLUtil`
  - `Alchemy/Graphics`
  - `Mammoth/TSE`
  - `Mammoth/TSUI`
- Depends on:
  - B-001
- Acceptance criteria:
  - CMake target graph builds in dependency order
- Current status: bounded target graph configures, but first concrete target `alchemy_kernel` does not compile yet

### B-004 Fix Apple Clang compatibility issues

- Priority: `P0`
- Status: `done`
- Goal: resolve compile blockers due to compiler differences, case-sensitive includes, and old platform assumptions
- Depends on:
  - B-003
- Acceptance criteria:
  - core targets compile under Apple Clang on arm64
- Current validation: core and engine static library targets build through `mammoth_tsui`; remaining warnings are not treated as milestone blockers unless they become runtime bugs

### B-005 Audit x86-only and asm-sensitive paths

- Priority: `P1`
- Status: `todo`
- Goal: find and neutralize paths that assume x86-specific optimizations or assembly
- Depends on:
  - B-003
- Acceptance criteria:
  - no arm64 build is blocked by x86-only code

## Epic C - Portable Core Bring-Up

### C-001 Build `Alchemy/Kernel`

- Priority: `P0`
- Status: `done`
- Depends on:
  - B-003

- Current validation: `cmake --build --preset macos-debug --target alchemy_kernel` builds locally

### C-002 Build `Alchemy/CodeChain`

- Priority: `P0`
- Status: `done`
- Depends on:
  - C-001

### C-003 Build `Alchemy/XMLUtil`

- Priority: `P0`
- Status: `done`
- Depends on:
  - C-001

### C-004 Build `Alchemy/Graphics`

- Priority: `P0`
- Status: `in_progress`
- Depends on:
  - C-001
- Current validation: target builds, but app link shows missing `CGDraw`, `CGFilter`, `CGFractal`, `CGRunList`, `AGArea`, `AGScreen`, `CIconLabelBlock`, and `CNoiseGenerator` coverage

### C-005 Build `Mammoth/TSE`

- Priority: `P0`
- Status: `done`
- Depends on:
  - C-002
  - C-003
  - C-004

### C-006 Build `Mammoth/TSUI`

- Priority: `P0`
- Status: `done`
- Depends on:
  - C-005
- Current validation: `cmake --build --preset macos-debug --target mammoth_tsui` builds locally; app link still needs omitted TSUI support files such as `CExtensionListMap.cpp`

### C-008 Close `transcendence_app` link gaps from omitted source files

- Priority: `P0`
- Status: `todo`
- Goal: add existing implementation files that currently satisfy unresolved final-link symbols before writing new stubs
- Depends on:
  - C-006
- First source candidates:
  - `Alchemy/Kernel/CDictionary.cpp`
  - `Alchemy/Kernel/CAtomizer.cpp`
  - `Alchemy/Kernel/CException.cpp`
  - `Alchemy/Kernel/CFileDirectory.cpp`
  - `Alchemy/Kernel/quickhull/QuickHull.cpp`
  - `Alchemy/DirectXUtil/CIconLabelBlock.cpp`
  - `Alchemy/DirectXUtil/CNoiseGenerator.cpp`
  - `Alchemy/DirectXUtil/AGArea.cpp`
  - `Alchemy/DirectXUtil/AGScreen.cpp`
  - `Mammoth/TSUI/CExtensionListMap.cpp`
- Acceptance criteria:
  - app link no longer reports unresolved symbols for implementation files that already exist and compile cleanly
- Verification:
  - `cmake --build --preset macos-debug --target transcendence_app`

### C-009 Restore CPU software drawing implementation coverage

- Priority: `P0`
- Status: `todo`
- Goal: compile enough existing software draw/filter/fractal code for menu and first gameplay rendering without using DirectX presentation
- Depends on:
  - C-008
- Likely source areas:
  - `Alchemy/DirectXUtil/DrawLine.cpp`
  - `Alchemy/DirectXUtil/DrawRect.cpp`
  - `Alchemy/DirectXUtil/DrawCircle.cpp`
  - `Alchemy/DirectXUtil/DrawFill.cpp`
  - `Alchemy/DirectXUtil/DrawRegion.cpp`
  - `Alchemy/DirectXUtil/BlendModes.cpp`
  - `Alchemy/DirectXUtil/FilterBlur.cpp`
  - `Alchemy/DirectXUtil/FilterThreshold.cpp`
  - `Alchemy/DirectXUtil/DrawClouds.cpp`
- Acceptance criteria:
  - final app link is not dominated by `CGDraw`, `CGFilter`, `CGFractal`, or `CGRunList` unresolved symbols

### C-010 Add macOS app-link smoke gate

- Priority: `P0`
- Status: `todo`
- Goal: make final link success a first-class milestone gate before runtime bring-up work expands
- Depends on:
  - C-008
  - C-009
- Acceptance criteria:
  - `transcendence_app` links or remaining unresolved symbols are explicitly classified as platform shell, presenter, audio, or deferred gameplay seams

### C-007 Add core smoke-test target

- Priority: `P1`
- Status: `todo`
- Goal: verify engine bootstrap or data parsing without needing the full app shell
- Depends on:
  - C-005
- Acceptance criteria:
  - one minimal executable or test target validates core startup paths

## Epic D - SDL Platform Shell

### D-001 Create macOS SDL app entry

- Priority: `P0`
- Status: `todo`
- Goal: replace Win32 startup in the macOS path with SDL initialization and clean shutdown
- Likely touch points:
  - `Transcendence/Transcendence/Main.cpp`
  - platform-specific wrapper code
- Depends on:
  - C-006
- Acceptance criteria:
  - app launches and exits cleanly

### D-002 Add SDL window abstraction

- Priority: `P0`
- Status: `todo`
- Goal: handle create/destroy, resize, focus, fullscreen, and drawable size queries
- Depends on:
  - D-001
- Acceptance criteria:
  - SDL window behavior is stable across open, resize, and close operations

### D-003 Add timer and lifecycle hooks

- Priority: `P1`
- Status: `todo`
- Goal: expose timers, frame pacing hooks, and activation/deactivation events to the engine
- Depends on:
  - D-001
- Acceptance criteria:
  - main loop timing is observable and controllable

### D-004 Add platform services wrapper

- Priority: `P1`
- Status: `todo`
- Goal: provide message boxes, clipboard access, cursor visibility, mouse capture, and path helpers behind one boundary
- Depends on:
  - D-002
- Acceptance criteria:
  - app shell no longer depends directly on Win32 utility calls in the macOS path

## Epic E - Input Translation Layer

### E-001 Map SDL keyboard events to engine command input

- Priority: `P0`
- Status: `todo`
- Likely source areas:
  - `Transcendence/Transcendence/CGameKeys.cpp`
  - `Transcendence/Transcendence/DefaultKeyMappings.h`
  - `Transcendence/Transcendence/KeyboardMapData.h`
- Depends on:
  - D-001
- Acceptance criteria:
  - gameplay and menu commands respond correctly to keyboard input

### E-002 Map SDL text input to UI text entry

- Priority: `P0`
- Status: `todo`
- Goal: separate command keys from text input to avoid duplicate character handling
- Depends on:
  - E-001
- Acceptance criteria:
  - text fields accept characters, deletion, and cursor movement correctly

### E-003 Map mouse buttons, movement, and wheel input

- Priority: `P0`
- Status: `todo`
- Goal: support menu interactions, game mouse handling, and wheel-driven controls
- Depends on:
  - D-002
- Acceptance criteria:
  - mouse behavior matches expected menu and gameplay semantics closely enough for testing

### E-004 Validate input behavior under Retina scaling

- Priority: `P1`
- Status: `todo`
- Goal: ensure coordinate mapping stays correct between logical and drawable sizes
- Depends on:
  - E-003
- Acceptance criteria:
  - hit testing and mouse positions are correct on high-DPI displays

## Epic F - Metal Presenter and Render Boundary

### F-001 Define renderer abstraction for the macOS path

- Priority: `P0`
- Status: `todo`
- Goal: isolate engine-facing rendering from backend-specific presentation
- Depends on:
  - A-002
  - D-002
- Acceptance criteria:
  - engine code does not directly depend on Metal types

### F-002 Initialize Metal device via SDL window

- Priority: `P0`
- Status: `todo`
- Goal: create the `Metal` device and present surface from the SDL window
- Depends on:
  - F-001
- Acceptance criteria:
  - Metal initializes successfully on target Apple Silicon hardware

### F-003 Implement CPU framebuffer upload path

- Priority: `P0`
- Status: `todo`
- Goal: upload an engine-generated software frame to a Metal texture and present it
- Depends on:
  - F-002
- Acceptance criteria:
  - a test frame renders correctly through Metal

### F-004 Validate color, alpha, and pixel format correctness

- Priority: `P0`
- Status: `todo`
- Goal: catch channel ordering and blending issues before gameplay integration
- Depends on:
  - F-003
- Acceptance criteria:
  - known test frames display without channel swaps or alpha corruption

### F-005 Add resize and high-DPI presentation support

- Priority: `P1`
- Status: `todo`
- Goal: make the presenter stable across window size changes and Retina scaling
- Depends on:
  - F-003
- Acceptance criteria:
  - resize and fullscreen transitions do not corrupt output or input mapping

## Epic G - UI and Session Integration

### G-001 Bring up intro and main menu sessions

- Priority: `P0`
- Status: `todo`
- Depends on:
  - E-002
  - E-003
  - F-004
- Acceptance criteria:
  - title screen and main menu are usable

### G-002 Validate settings, new game, and load game flows

- Priority: `P1`
- Status: `todo`
- Depends on:
  - G-001
- Acceptance criteria:
  - key menu screens are navigable and stable

### G-003 Validate text-heavy and list-heavy UI screens

- Priority: `P1`
- Status: `todo`
- Goal: catch font/layout issues early
- Depends on:
  - G-001
- Acceptance criteria:
  - no major clipping or unreadable layouts in critical UI flows

## Epic H - Gameplay Bring-Up

### H-001 Enter gameplay from menu

- Priority: `P0`
- Status: `todo`
- Depends on:
  - G-002
- Acceptance criteria:
  - user can start a game and reach in-game state

### H-002 Validate HUD rendering

- Priority: `P0`
- Status: `todo`
- Depends on:
  - H-001
- Acceptance criteria:
  - HUD remains readable and functionally correct

### H-003 Validate dock UI and map-related screens

- Priority: `P1`
- Status: `todo`
- Depends on:
  - H-001
- Acceptance criteria:
  - dock and map interfaces work without major corruption or input bugs

### H-004 Run first playable gameplay smoke test

- Priority: `P0`
- Status: `todo`
- Goal: validate flight, interaction, and basic transitions
- Depends on:
  - H-002
  - H-003
- Acceptance criteria:
  - basic play loop is possible without critical blockers

## Epic I - Resources, Fonts, and Filesystem

### I-001 Replace Win32 resource assumptions in the macOS path

- Priority: `P0`
- Status: `todo`
- Goal: load runtime resources from bundle/filesystem instead of Windows resources
- Depends on:
  - G-001
- Acceptance criteria:
  - title/menu/gameplay resources resolve from macOS-friendly locations

### I-002 Migrate bundled font/resource access

- Priority: `P1`
- Status: `todo`
- Goal: preserve `.dxfn`-based fidelity where possible and reduce layout drift
- Depends on:
  - I-001
- Acceptance criteria:
  - critical UI fonts load correctly without Windows resource APIs

### I-003 Implement macOS save/settings paths

- Priority: `P0`
- Status: `todo`
- Goal: move writable runtime data into standard macOS user directories
- Depends on:
  - H-001
- Acceptance criteria:
  - app no longer writes runtime state into the bundle or relies on current working directory

## Epic J - Audio Replacement

### J-000 Add milestone no-audio backend seam

- Priority: `P0`
- Status: `todo`
- Goal: stop Windows MCI symbols from blocking menu bring-up while preserving the high-level soundtrack manager contract
- Depends on:
  - C-010
- Acceptance criteria:
  - `CMCIMixer` unresolved symbols no longer block `transcendence_app` for the menu milestone
- Note: this is allowed to be silent/no-op for M4; full playback remains J-002 through J-004

### J-001 Audit actual audio format and playback requirements

- Priority: `P1`
- Status: `todo`
- Goal: determine what formats and behaviors must be preserved for SFX and music
- Depends on:
  - A-001
- Acceptance criteria:
  - requirements for SFX overlap, looping, fades, and soundtrack transitions are documented

### J-002 Implement SFX backend replacement

- Priority: `P1`
- Status: `todo`
- Goal: replace DirectSound-dependent SFX playback with a native macOS-compatible backend
- Depends on:
  - J-001
- Acceptance criteria:
  - gameplay and UI sound effects play without Windows-only APIs

### J-003 Implement music backend replacement

- Priority: `P1`
- Status: `todo`
- Goal: replace MCI-based music playback while preserving soundtrack behavior as closely as practical
- Depends on:
  - J-001
- Acceptance criteria:
  - music playback, pause, and transitions work in native builds

### J-004 Validate full audio behavior

- Priority: `P1`
- Status: `todo`
- Depends on:
  - J-002
  - J-003
- Acceptance criteria:
  - SFX overlap, music transitions, and volume behavior are acceptable for gameplay

## Epic K - Packaging and Native App Behavior

### K-001 Create `.app` bundle target

- Priority: `P1`
- Status: `todo`
- Goal: package the app as a macOS bundle
- Depends on:
  - I-001
  - I-003
- Acceptance criteria:
  - app launches from Finder with resources intact

### K-002 Package shaders and runtime assets

- Priority: `P1`
- Status: `todo`
- Goal: ensure required runtime data and Metal assets are shipped inside the bundle
- Depends on:
  - K-001
- Acceptance criteria:
  - bundled app can render and locate required assets outside the terminal

### K-003 Validate native lifecycle behavior

- Priority: `P1`
- Status: `todo`
- Goal: verify focus loss, minimize, resize, and fullscreen behavior on macOS
- Depends on:
  - K-001
- Acceptance criteria:
  - no major lifecycle bugs remain in common desktop usage

## Epic L - Performance and Stabilization

### L-001 Add debug logging for backend selection and frame sizing

- Priority: `P2`
- Status: `todo`
- Goal: improve diagnosability of SDL, Metal, and scaling issues
- Depends on:
  - F-003
- Acceptance criteria:
  - logs expose renderer backend, pixel format, logical size, and drawable size

### L-002 Add sanitizer build preset and smoke runs

- Priority: `P1`
- Status: `todo`
- Goal: catch memory and UB issues in the new macOS path
- Depends on:
  - B-002
  - H-004
- Acceptance criteria:
  - sanitizer build completes and can run the smoke path

### L-003 Profile frame pacing and CPU upload costs

- Priority: `P2`
- Status: `todo`
- Goal: identify whether the compatibility renderer needs targeted optimization
- Depends on:
  - H-004
- Acceptance criteria:
  - top frame-time hotspots are documented

### L-004 Optimize only measured bottlenecks

- Priority: `P3`
- Status: `todo`
- Goal: improve performance after profiling, not before
- Depends on:
  - L-003
- Acceptance criteria:
  - optimization work is tied to measured wins, not guesses

## Epic M - Deferred and Optional Work

### M-001 Steam support on macOS

- Priority: `P3`
- Status: `deferred`
- Reason: not required for first native playable build

### M-002 Production cloud integration

- Priority: `P3`
- Status: `deferred`
- Reason: contributor-safe path is sufficient for early milestones

### M-003 Universal binary support

- Priority: `P3`
- Status: `deferred`
- Reason: Apple Silicon-only target is sufficient for first release

### M-004 Notarization and distribution polish

- Priority: `P3`
- Status: `deferred`
- Reason: useful later, not a blocker for the development port

## Epic N - macOS Port Defect Register Execution

Source of truth: `port-defect-register.md`. Each task is one phase, delivered on its own
branch and pull request from `osx`; no direct commit to `main`/`master`.

### N-000 Publish the port defect register

- Priority: `P0`
- Status: `done`
- Goal: publish a single register of all static-audit port defects and wire it into the
  documentation set
- Deliverable:
  - `port-defect-register.md` with `PDR-001`..`PDR-038` grouped by fix phase
  - registration in `index.md`, `task-backlog.md`, `change-log.md`, `../bug_fix_plan.md`,
    `../macOS_port_status.md`
- Depends on:
  - none
- Acceptance criteria:
  - every entry records ID, title, finding status, priority, `file:line`, macOS impact,
    trigger condition, fix phase, verification gate, and work status
  - no source-code changes in this task
  - `git diff --check` is clean
- Delivery note: branch `docs/port-defect-register`, PR #1 open against `osx`; not merged

### N-001 Phase 1 - input and memory safety (P0)

- Priority: `P0`
- Status: `done`
- Goal: restore complete virtual-key mapping and correct memory-stream accounting/safety
- Scope:
  - `PDR-001` complete `PlatformGetAsyncKeyState` VK mapping, including a pure
    `PlatformVKToScancode` helper and its documentation
  - `PDR-002` `CMemoryWriteStream::Write` growth loop
  - `PDR-003` `m_iCommittedSize` accounting and zero-fill on forward `Seek`
  - `PDR-004` `CloseHandle` pointer-vs-fd disambiguation without heuristic dereference
  - `PDR-005` magic checks in event wait/set/reset paths
  - `PDR-006` `SDLBitmapDestroy` map cleanup
- Depends on:
  - N-000
- Acceptance criteria:
  - `PDR-001`..`PDR-006` marked `done` in the register in the same PR
  - `cmake --preset macos-debug`, `cmake --build --preset macos-debug`, and
    `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure` pass
  - new portability tests cover memory-stream growth/zero-fill/commit accounting, bitmap
    create/destroy lookup, and every VK used by `DefaultKeyMappings.h`
  - no Windows behavior change; changes are platform-neutral or guarded
- Delivery note: branch `fix/macos-port-p0-input-memory`, PR open against `osx`; not merged.
  Gate verified locally: `cmake --preset macos-debug`, `cmake --build --preset macos-debug`
  (including `Transcendence.app`), and `ctest -R mac-portability` passed

### N-002 Phase 2 - functional and filesystem gaps (P1)

- Priority: `P1`
- Status: `done`
- Goal: stop silent failures in image, video, audio, cursor, file-time, and app-data paths
- Scope:
  - `PDR-007` missing DIB creators (`dibCreate16/24/32bitDIB`, `dibCrop`,
    `dibConvertToDDB`, `dibLoadFromResource`)
  - `PDR-008` `MCIWnd*` no-op decision
  - `PDR-009` `CMCIMixerStub.cpp` vs `CMCIMixer.cpp` feature inventory
  - `PDR-010`/`PDR-011` cursor, capture, and screen/client coordinate conversion
  - `PDR-012`/`PDR-013` `GetFileTime`/`FileTimeToSystemTime` and `ftLastWriteTime`
  - `PDR-014` `CopyFile` `bFailIfExists`
  - `PDR-015` single app-data root consistent with `GetAppLogPath()`
- Depends on:
  - N-001
- Acceptance criteria:
  - `PDR-007`..`PDR-015` marked `done` (or explicitly `deferred` with reason) in the same PR
  - filesystem portability tests cover file time, `CopyFile` `bFailIfExists`, and
    `SHGetFolderPath` matching the log root
  - DIB tests return `NOERROR` with valid output, or assert that callers were redirected
  - build and `mac-portability` gate pass; `git diff --check` is clean
- Delivery note: branch `fix/macos-port-p1-functional-fs`, PR open against
  `fix/macos-port-p0-input-memory` (stacked); not merged. Gate verified locally:
  `cmake --preset macos-debug`, `cmake --build --preset macos-debug` (including
  `Transcendence.app` and the tools), and `ctest -R mac-portability` passed.
  `PDR-008` closed as an explicit video non-goal; `PDR-009` records the `CMCIMixer` parity
  inventory plus its remaining audio follow-ups in `functional-fs-utilities.md`.

### N-003 Phase 3 - platform/event semantics and performance (P2)

- Priority: `P2`
- Status: `todo`
- Goal: align message/event semantics with Win32 and remove unnecessary performance caps
- Scope:
  - `PDR-016`/`PDR-017` message `WPARAM` width and `hwnd`/`time`/`pt` population
  - `PDR-018` synchronous `PlatformSendMessage`
  - `PDR-019` mouse coordinate packing for negative/multi-monitor values
  - `PDR-020` async-signal-safe crash handler
  - `PDR-021` timer race
  - `PDR-022` `VirtualAlloc` `MEM_COMMIT` semantics
  - `PDR-023` non-blocking `ShellExecute`
  - `PDR-024` `posixResolvePathCase` stderr noise and cost
  - `PDR-025`/`PDR-026` re-evaluate forced single-thread paint
  - `PDR-027` monochrome detection cost
- Depends on:
  - N-002
- Acceptance criteria:
  - `PDR-016`..`PDR-027` marked `done` (or explicitly `deferred` with reason) in the same PR
  - coordinate packing tests cover negative values and values above 32767
  - `PDR-025`/`PDR-026` keep an explicit, documented exit criterion if the cap remains
  - build and `mac-portability` gate pass; `git diff --check` is clean

### N-004 Phase 4 - hardening and minor defects (P3)

- Priority: `P3`
- Status: `todo`
- Goal: defensive correctness and cleanup for the remaining low-severity defects
- Scope:
  - `PDR-028` `_fcvt_s` bounded conversion
  - `PDR-029` `wsprintf` buffer-size safety
  - `PDR-030` `CreateFile` `dwShareMode`/`dwFlags`
  - `PDR-031` 64-bit file pointer/size
  - `PDR-032` `MoveFile`/`GetTempPath`/`FreePIDL`/`SHGetMalloc`
  - `PDR-033` SDL hint ordering, HIGHDPI hints, log mode, crash-log location
  - `PDR-034` CPU-info stubs
  - `PDR-035` varargs pragma removal after fixing call sites
  - `PDR-036` `DebugLog` on macOS debug builds
- Depends on:
  - N-003
- Acceptance criteria:
  - `PDR-028`..`PDR-036` marked `done` (or explicitly `deferred` with reason) in the same PR
  - `_fcvt_s` test proves no out-of-bounds read with a small buffer
  - build and `mac-portability` gate pass; `git diff --check` is clean

### N-005 Phase 5 - cross-platform regressions (deferred)

- Priority: `P3`
- Status: `deferred`
- Reason: `PDR-037` (`#define WINAPI` empty outside `_WIN32`) and `PDR-038`
  (`CMemoryStream.cpp` stub replacing the Windows implementation) only affect Windows
  builds; the current plan is macOS-only
- Acceptance criteria:
  - both entries remain recorded as `deferred` in `port-defect-register.md`
  - no source-code change for either entry

## Critical Path Summary

The shortest path to a native main menu from the current 2026-04-30 state is:

1. C-008 close app-link gaps from omitted source files
2. C-009 restore CPU software drawing coverage
3. C-010 make app link success or a small classified blocker list the gate
4. J-000 add a no-audio/native-audio seam if MCI symbols still block link
5. D-001 create macOS SDL app entry
6. D-002 add SDL window abstraction
7. F-001 through F-004 implement presenter boundary and Metal test/real frame path
8. E-001 through E-003 wire keyboard, text, and mouse input
9. G-001 bring up intro and main menu sessions

The shortest path to a first playable build is:

1. complete native main menu path
2. G-002
3. H-001
4. H-002
5. H-003
6. H-004
7. I-003

## Recommended Next Execution Slice

- C-008 Close `transcendence_app` link gaps from omitted source files
- C-009 Restore CPU software drawing implementation coverage
- C-010 Add macOS app-link smoke gate
- J-000 Add milestone no-audio backend seam if `CMCIMixer` remains in the link path

These tasks turn the current static-library build success into an executable link path and should be completed before expanding runtime SDL/Metal behavior or packaging work.

Defect-closure track added 2026-09-19: Epic N (`N-000`..`N-004`) executes the static-audit
findings from `port-defect-register.md` in four code phases plus the docs phase, each on its
own branch and pull request from `osx`. Phase 5 (`PDR-037`, `PDR-038`) stays `deferred`.
