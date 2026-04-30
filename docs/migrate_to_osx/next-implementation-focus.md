# Next Implementation Focus

## Document Status

- Version: v1.4
- Last Updated: 2026-04-30
- Purpose: define the fastest next implementation focus for turning the existing macOS CMake scaffold into a compiling target sequence

## Why this document exists

The recent implementation slices have substantially reduced asset-path risk for milestone 1.

At this point, continuing to chase every remaining image caller is lower leverage than moving to the next milestone-1 blockers.

This document identifies the next best focus area so the next agent can move directly into the highest-value work.

## Current build-scaffold status

The repository now contains:

- root `CMakeLists.txt`
- `CMakePresets.json`
- concrete library targets for:
  - `alchemy_kernel`
  - `alchemy_codechain`
  - `alchemy_xmlutil`
  - `alchemy_jpeg`
  - `alchemy_graphics`
  - `mammoth_tse`
  - `mammoth_tsui`
  - `platform_sdl`
- app target `transcendence_app`

The current blocker is no longer planning ambiguity, missing tools, or `alchemy_kernel` compilation.

As of 2026-04-30, local validation shows:

- `cmake --preset macos-debug` configures successfully
- `cmake --build --preset macos-debug --target alchemy_kernel` builds
- `cmake --build --preset macos-debug --target alchemy_codechain alchemy_xmlutil alchemy_jpeg alchemy_graphics mammoth_tse` builds
- `cmake --build --preset macos-debug --target mammoth_tsui` builds
- `cmake --build --preset macos-debug --target transcendence_app` reaches final link and fails on unresolved symbols

This means the fastest path is app-link closure, not more kernel-only bring-up.

First app-link blocker classes:

- source files that exist but are not in CMake: `CDictionary.cpp`, `CAtomizer.cpp`, `CException.cpp`, `CFileDirectory.cpp`, `CIconLabelBlock.cpp`, `CNoiseGenerator.cpp`, `AGArea.cpp`, `AGScreen.cpp`, `CExtensionListMap.cpp`, `quickhull/QuickHull.cpp`
- software draw/filter/fractal primitives that are declared and referenced but not compiled into `alchemy_graphics`
- audio symbols from `CMCIMixer`, which should become a no-audio/native-audio backend seam rather than keeping Windows MCI in the macOS path

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

Do not start broad SDL or Metal runtime work yet. The current scaffold already builds the static libraries, so the fastest path is to reduce final app-link failures in order.

In particular:

- first add implementation files that already exist and match unresolved symbols
- then fix and include CPU software drawing files before inventing Metal replacements for draw primitives
- keep DirectX presentation files excluded; the goal is CPU draw coverage plus native presentation, not DirectX revival
- treat `transcendence_app` link closure as the active critical path target

Fastest command loop:

```sh
cmake --build --preset macos-debug --target alchemy_kernel
cmake --build --preset macos-debug --target alchemy_graphics
cmake --build --preset macos-debug --target mammoth_tsui
cmake --build --preset macos-debug --target transcendence_app
```

Use the first failing command as the active blocker and update `../macOS_port_status.md` if the blocker class changes.

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

1. add present-but-omitted source files to the owning CMake targets -> verify: `transcendence_app` link no longer reports those classes
2. restore CPU drawing coverage in `alchemy_graphics` by fixing Clang blockers in excluded draw/filter/fractal files -> verify: `CGDraw`/`CGFilter`/`CGFractal` unresolved groups shrink materially
3. decide the milestone audio seam -> verify: `CMCIMixer` unresolved symbols are either removed from the menu link path or satisfied by a native/no-audio macOS backend
4. keep the app link moving until remaining failures are platform/presenter semantics rather than missing existing implementation files -> verify: unresolved symbols are few enough to map to explicit seams
5. only then implement SDL shell and Metal compatibility presenter runtime behavior

## Why not broaden the runtime right now

The graph already configures and the static libraries build. The app link now exposes actionable missing implementation groups. Starting SDL/Metal runtime behavior before link closure would mix missing CPU draw code, omitted source files, audio backend decisions, and platform presentation design into one noisy failure set.

The fastest path is to close link gaps first, then implement the runtime seams against a build that links.

## Success criteria for this focus

This focus is successful when:

- present-but-omitted implementation files are included or explicitly deferred with reasons
- `transcendence_app` link failures are reduced to known platform, presenter, drawing, audio, or gameplay seams
- the shell replacement responsibilities remain concrete enough to implement
- the presentation seam remains concrete enough to support a first framebuffer path
- the build advances from static-library success to app-link success or a small documented final blocker list
