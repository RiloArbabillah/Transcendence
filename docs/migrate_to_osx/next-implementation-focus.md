# Next Implementation Focus

## Document Status

- Version: v1.3
- Last Updated: 2026-04-25
- Purpose: define the fastest next implementation focus for turning the existing macOS CMake scaffold into a compiling target sequence

## Why this document exists

The recent implementation slices have substantially reduced asset-path risk for milestone 1.

At this point, continuing to chase every remaining image caller is lower leverage than moving to the next milestone-1 blockers.

This document identifies the next best focus area so the next agent can move directly into the highest-value work.

## Current build-scaffold status

The repository now contains:

- root `CMakeLists.txt`
- `CMakePresets.json`
- six bounded concrete library targets in the scaffold:
  - `alchemy_kernel`
  - `alchemy_codechain`
  - `alchemy_xmlutil`
  - `alchemy_jpeg`
  - bounded `alchemy_graphics`
  - bounded `mammoth_tse`

The current blocker is no longer planning ambiguity or missing tools.

As of 2026-04-25, local validation shows:

- `cmake --version` succeeds with CMake 4.3.2
- `ninja --version` succeeds with Ninja 1.13.2
- `cmake --preset macos-debug` configures successfully
- `cmake --build --preset macos-debug --target alchemy_kernel` reaches compilation and fails in `alchemy_kernel`

This means the fastest path is compile-driven kernel bring-up, not more source-audit work.

First observed blockers:

- `Alchemy/Include/Kernel.h:743` casts `CObject *` to `int`, which fails on arm64 before most translation units can compile
- `Alchemy/Kernel/CFileReadBlock.cpp` and `Alchemy/Kernel/CFileReadStream.cpp` still call Win32 file mapping APIs such as `UnmapViewOfFile`, `GENERIC_READ`, `CreateFile`-style constants, `PAGE_READONLY`, `FILE_MAP_READ`, and `GetFileSize`

## What is considered done enough

For milestone-1 title/menu bring-up, the following critical asset callers are now covered by the newer file-based and neutral-memory paths:

- `Transcendence/Transcendence/CLoadingSession.cpp`
- `Mammoth/TSUI/CVisualPalette.cpp`
- `Transcendence/Transcendence/CButtonBarData.cpp`

This means the remaining bottlenecks are no longer centered on title/menu asset lookup.

## What is explicitly deferred

These image callers remain on the older path and can stay deferred unless milestone-1 validation proves otherwise:

- `Transcendence/Transcendence/CHelpSession.cpp`
- `Transcendence/Transcendence/CStatsSession.cpp`
- `Transcendence/Transcendence/CModExchangeSession.cpp`

These are not the best next targets because they do not directly block the first native loading/title/menu flow.

## Recommended next implementation focus

## Immediate recommendation

Do not broaden the graph further. The current scaffold configures, so the next fastest path is to make the existing concrete targets compile in order.

In particular:

- do not make `mammoth_tsui_core` concrete yet
- do not pull in broader `Mammoth/TSE/*` files by guesswork
- do not start SDL or Metal implementation until `alchemy_kernel` compiles
- treat `alchemy_kernel` as the active critical path target

Fastest command loop:

```sh
cmake --preset macos-debug
cmake --build --preset macos-debug --target alchemy_kernel
```

Only after `alchemy_kernel` builds should the loop move to:

```sh
cmake --build --preset macos-debug --target alchemy_codechain
cmake --build --preset macos-debug --target alchemy_xmlutil
cmake --build --preset macos-debug --target alchemy_graphics
cmake --build --preset macos-debug --target alchemy_jpeg
cmake --build --preset macos-debug --target mammoth_tse
```

## 1. Define the milestone-1 source subset

### Objective

Turn the audit work into a concrete list of source files and targets required for:

- loading screen
- intro/title menu
- minimum host/runtime boot path

### Why this is next

- `docs/migrate_to_osx/milestone-1-plan.md` requires a reduced build graph
- `docs/migrate_to_osx/source-audit-handoff.md` already identified the real menu-boot path
- the asset side is now stable enough to support this scoping work

This step is complete enough for the first bounded scaffold. Do not spend more time re-auditing before fixing the current `alchemy_kernel` blockers.

### Output

- a milestone-1 source subset list
- a first-pass target grouping for `Alchemy`, `Mammoth`, and `Transcendence`
- a list of files that must stay out because they still hard-block compilation

### Current recommended subset

Required entry/runtime path:

- `Transcendence/Transcendence/Main.cpp`
- `Mammoth/TSUI/Run.cpp`
- `Mammoth/TSUI/CHumanInterface.cpp`
- `Transcendence/Transcendence/CTranscendenceController.cpp`
- `Transcendence/Transcendence/CTranscendenceModel.cpp`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp`

Required milestone-1 sessions and UI support:

- `Transcendence/Transcendence/CLoadingSession.cpp`
- `Transcendence/Transcendence/CIntroSession.cpp`
- `Transcendence/Transcendence/IntroScreen.cpp`
- `Transcendence/Transcendence/CButtonBarData.cpp`
- `Transcendence/Transcendence/CButtonBarDisplay.cpp`
- `Transcendence/Transcendence/CUIResources.cpp`
- enough font/output support from `Transcendence/Transcendence/GameOutput.cpp`

Required shared libraries and services:

- `Alchemy/Kernel`
- `Alchemy/CodeChain`
- `Alchemy/XMLUtil`
- enough of `Alchemy/Graphics`
- enough of `Alchemy/IntelJPEGUtil`
- enough of `Mammoth/TSE`
- enough of `Mammoth/TSUI`

Deferred unless milestone-1 validation proves otherwise:

- `Transcendence/Transcendence/CHelpSession.cpp`
- `Transcendence/Transcendence/CStatsSession.cpp`
- `Transcendence/Transcendence/CModExchangeSession.cpp`
- gameplay-heavy session flows
- full audio parity and Steam/cloud integrations

Files still likely to stay out of the first macOS build subset unless explicitly wrapped:

- DirectX presentation-heavy files under `Alchemy/DirectXUtil/`
- Win32-only shell responsibilities that remain embedded in `Run.cpp`
- MCI/DirectSound-specific code paths

## 2. Prepare the shell replacement seam

### Objective

Translate the earlier audit into a concrete plan for replacing the Win32 app loop and lifecycle path.

### Focus files

- `Transcendence/Transcendence/Main.cpp`
- `Mammoth/TSUI/Run.cpp`
- `Mammoth/TSUI/CHumanInterface.cpp`
- `Transcendence/Transcendence/CTranscendenceController.cpp`

### Questions to answer

- what is the minimum API surface the SDL shell must provide?
- what parts of `Run.cpp` can be preserved as flow control versus replaced entirely?
- what timing, session, and event callbacks must survive intact for menu boot?

### Output

- a shell seam outline for milestone 1
- a list of Win32 responsibilities that must be replaced first

### Current seam map

Responsibilities currently bundled into the Win32 shell:

- app entry and kernel init in `Transcendence/Transcendence/Main.cpp`
- controller boot and fatal startup dialogs in `Mammoth/TSUI/Run.cpp:16`
- window creation and show/maximize logic in `Mammoth/TSUI/Run.cpp:74`
- event pump and frame pacing in `Mammoth/TSUI/Run.cpp:147`
- timer resolution, sleep cadence, and main-loop ownership in `Mammoth/TSUI/Run.cpp:157`
- message-to-HI dispatch in `Mammoth/TSUI/Run.cpp:226`
- session event entry points in `Mammoth/TSUI/Run.cpp:363` onward

Minimum shell responsibilities a replacement must provide:

- initialize kernel and construct controller
- call `HIBoot` before HI/session init
- create a native window and associate it with the HI runtime
- drive a frame loop at 30/60 fps equivalent cadence
- translate keyboard, mouse button, mouse move, wheel, focus, resize, close, and timer events into existing HI entry points
- provide a replacement for `PostMessage`-style command delivery used by `HIPostCommand`
- provide a replacement for background-task completion notifications used by `OnTaskComplete`

Practical replacement boundary:

- preserve `HIBoot`, `HIInit`, `HIUpdate`, session `HI*` methods, and most controller flow
- replace `WinMain`, `CreateMainWindow`, `MainLoop`, `MainWndProc`, and all `WM_*` message plumbing with an SDL-driven shell adapter

First replacement targets:

1. `Transcendence/Transcendence/Main.cpp`
2. `Mammoth/TSUI/Run.cpp` entry loop and main window creation
3. `HIPostCommand` / task completion delivery path currently bound to `WM_HI_COMMAND` and `WM_HI_TASK_COMPLETE`

## 3. Prepare the presentation seam

### Objective

Define the minimum compatibility presenter needed to put the first real frame on screen.

### Focus files

- `Mammoth/TSUI/CHumanInterface.cpp`
- current screen manager / DirectX presentation path
- milestone-1 presenter expectations from `docs/migrate_to_osx/milestone-1-plan.md`

### Questions to answer

- what exact surface format does the engine/session path produce today?
- where is the narrowest seam to intercept the finished framebuffer?
- what state does the existing UI/session code assume about the screen manager?

### Output

- a presenter seam description
- a list of minimum methods or behaviors a Metal compatibility presenter must satisfy

### Current seam map

The first-frame path currently depends on `CScreenMgr3D` through `CHumanInterface`:

- `Mammoth/Include/TSUI.h:747` `GetScreen()` returns `m_ScreenMgr.GetScreen()`
- `Mammoth/TSUI/CHumanInterface.cpp:449` requires `m_ScreenMgr.CheckIsReady()`
- `Mammoth/TSUI/CHumanInterface.cpp:454` obtains the finished framebuffer as `CG32bitImage &Screen`
- sessions paint into that framebuffer through `HIAnimate(...)`
- `Mammoth/Include/TSUI.h:801` `BltScreen()` calls `m_ScreenMgr.Render()`
- `Mammoth/Include/TSUI.h:805` `FlipScreen()` calls `m_ScreenMgr.Flip()`

This means the narrowest useful presenter seam is not inside session code. It is at the screen-manager boundary used by `CHumanInterface`.

Minimum behaviors the compatibility presenter must satisfy:

- expose a mutable `CG32bitImage` framebuffer for sessions to draw into
- report width/height consistently to HI and sessions
- answer readiness checks equivalent to `CheckIsReady()`
- accept resize/focus/display-change notifications needed by HI
- present the completed framebuffer to a native window surface
- preserve enough semantics for `GetScreen()`, `Render()`, and `Flip()` to keep `CHumanInterface::OnAnimate()` stable

Practical interpretation:

- keep session/UI drawing code untouched
- intercept at the existing `m_ScreenMgr` contract
- implement a compatibility presenter that uploads the software-rendered `CG32bitImage` buffer to Metal
- avoid rewriting session rendering or introducing GPU-native UI drawing in milestone 1

## Recommended order

1. keep the current source subset frozen -> verify: `cmake --preset macos-debug` still configures
2. fix the header-level arm64 blocker in `Alchemy/Include/Kernel.h` -> verify: the pointer-to-`int` error no longer stops every `alchemy_kernel` compile unit
3. add the smallest non-Windows path for `CFileReadBlock.cpp` and `CFileReadStream.cpp` or remove those files from the first portable target if they are not needed yet -> verify: `alchemy_kernel` reaches the next distinct blocker
4. continue compile-driven fixes only inside `alchemy_kernel` -> verify: `alchemy_kernel` builds before touching higher targets
5. then build the remaining concrete targets one at a time in dependency order

## Why not broaden the graph right now

The graph already configures and the first target already exposes actionable compile fallout. Broadening now would mix kernel portability, engine dependencies, shell replacement, and presenter work into one noisy failure set.

The fastest path is to keep one active target, clear its global blockers, and only advance when the target builds.

## Success criteria for this focus

This focus is successful when:

- the next agent can name the exact milestone-1 source subset
- the shell replacement responsibilities are concrete enough to implement
- the presentation seam is concrete enough to support a first framebuffer path
- the existing bounded `CMake` target set advances from configure-success to `alchemy_kernel` compile-success
