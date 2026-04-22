# Milestone-1 Source Subset

## Document Status

- Version: v1.2
- Last Updated: 2026-04-15
- Derived From: `source-audit-handoff.md`, `milestone-1-plan.md`, `cmake-build-plan.md`, `next-implementation-focus.md`
- Purpose: define a concrete, target-oriented source subset for the first macOS `CMake` scaffold and menu-boot implementation slice

## Purpose

This document turns the audited menu-boot path into a more concrete milestone-1 source subset.

It is not a final source-of-truth build manifest. It is a bounded planning document meant to keep the first `CMake` and shell/presenter work focused on the menu-boot path.

## Guiding rule

If a file or module does not materially help reach:

- loading screen
- intro/title menu
- minimum controller/model boot path
- first presented frame

then it should stay out of the initial macOS target graph unless required by compilation dependencies.

## Target-oriented subset

## Group A - Foundation targets

These remain the same high-value early build targets recommended by `cmake-build-plan.md`.

### `alchemy_kernel`

- include as a full target basis from `Alchemy/Kernel/*`
- rationale: foundational dependency for nearly everything else
- milestone status: required now

### `alchemy_codechain`

- include as a full target basis from `Alchemy/CodeChain/*`
- rationale: required by `Mammoth/TSE`
- milestone status: required now

### `alchemy_xmlutil`

- include as a full target basis from `Alchemy/XMLUtil/*`
- rationale: required for game data parsing and universe init
- milestone status: required now

### `alchemy_zlib`

- include as a full or minimally sufficient target basis
- rationale: engine/runtime dependency expected by existing code paths
- milestone status: required now

### `alchemy_jpeg`

- include enough of the current JPEG path to support milestone-1 image decode
- current relevant files now include:
  - `Alchemy/IntelJPEGUtil/Load.cpp`
  - public declarations in `Alchemy/Include/JPEGUtil.h`
- milestone status: required now

### `alchemy_graphics`

- include a milestone-1 subset, not a blind full target on day one
- likely include:
  - portable/basic graphics structures needed by `CG32bitImage`, fonts, and UI code
  - the newer neutral BMP buffer path in `Alchemy/Graphics/DIB.cpp`
- treat as bounded subset because `GDI.cpp` and older bitmap helpers are still Windows-heavy
- milestone status: required now, but subset carefully

## Group B - Engine and UI targets

### `mammoth_tse`

- include enough of `Mammoth/TSE/*` to support:
  - `CUniverse::Boot()`
  - design/adventure loading
  - intro system creation
  - host callbacks used by menu boot
- rationale: title/menu path still requires real universe init
- milestone status: required now

### `mammoth_tsui_core`

- include enough of `Mammoth/TSUI/*` for:
  - `CHumanInterface`
  - session base types
  - visual palette
  - background processors
  - timer/task plumbing still needed by the current boot path
- avoid treating all TSUI files as equally critical
- milestone status: required now

### `mammoth_tsui_shell_legacy`

- carve out the Win32-heavy shell path conceptually:
  - `Mammoth/TSUI/Run.cpp`
  - Win32-facing parts of `Mammoth/TSUI/CHumanInterface.cpp`
- rationale: this is still required for understanding and replacement, but should not define the long-term macOS boundary
- milestone status: replace in milestone shell work

## Group C - App/menu path target

### `transcendence_menu_path`

This is the most useful way to think about the app-layer subset for milestone 1.

Required app bootstrap and host files:

- `Transcendence/Transcendence/Main.cpp`
- `Transcendence/Transcendence/CTranscendenceController.cpp`
- `Transcendence/Transcendence/CTranscendenceModel.cpp`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp`
- `Transcendence/Transcendence/GameOutput.cpp`

Required menu/loading sessions and UI support:

- `Transcendence/Transcendence/CLoadingSession.cpp`
- `Transcendence/Transcendence/CIntroSession.cpp`
- `Transcendence/Transcendence/IntroScreen.cpp`
- `Transcendence/Transcendence/CButtonBarData.cpp`
- `Transcendence/Transcendence/CButtonBarDisplay.cpp`
- `Transcendence/Transcendence/CUIResources.cpp`

Required supporting resource/port files introduced during the port work:

- `Transcendence/Transcendence/CResourcePathResolver.cpp`

Likely supporting files that may be pulled in by compile dependencies or session references:

- `Transcendence/Transcendence/Utilities.cpp`
- lightweight display helpers referenced by intro/button-bar flows

## Group D - First macOS-only replacement targets

These should exist conceptually in the first `CMake` scaffold even if initially stubbed.

### `platform_common`

- platform-neutral interfaces for:
  - app lifecycle hooks
  - timer and command delivery
  - path/resource root resolution
  - basic utility services currently sourced from Win32 shell code

### `platform_sdl`

- SDL-backed shell replacement target for:
  - app entry
  - window lifecycle
  - event pump
  - frame loop ownership
  - input translation source

### `render_common`

- interface-level target for framebuffer presentation semantics

### `render_metal`

- Metal compatibility presenter target
- should attach at the `m_ScreenMgr` boundary rather than inside session/UI drawing code

## Explicitly deferred from the first target slice

Keep these out unless the build proves they are unexpectedly required.

### Non-critical sessions

- `Transcendence/Transcendence/CHelpSession.cpp`
- `Transcendence/Transcendence/CStatsSession.cpp`
- `Transcendence/Transcendence/CModExchangeSession.cpp`

### Runtime features beyond menu boot

- full gameplay session graph
- audio parity work
- Steam/cloud production paths
- save/load parity beyond what menu boot absolutely requires

### Windows-specific infrastructure to avoid in the first graph if possible

- broad DirectX presentation files under `Alchemy/DirectXUtil/`
- MCI/DirectSound-specific code paths
- Windows shell-only support code not needed for compile-time references

## Practical grouping for first `CMake` work

The first bounded `CMake` scaffold should aim at this build order:

1. `alchemy_kernel`
2. `alchemy_codechain`
3. `alchemy_xmlutil`
4. `alchemy_zlib`
5. `alchemy_jpeg`
6. bounded `alchemy_graphics`
7. bounded `mammoth_tse`
8. bounded `mammoth_tsui_core`
9. `transcendence_menu_path`
10. placeholder `platform_common`
11. placeholder `platform_sdl`
12. placeholder `render_common`
13. placeholder `render_metal`

### Current scaffold status

The current root `CMakeLists.txt` already reflects this plan partially:

- `alchemy_kernel` is now a concrete `STATIC` target with an initial source list
- `alchemy_codechain` is now a concrete `STATIC` target with an initial source list
- `alchemy_xmlutil` is now a concrete `STATIC` target with an initial source list
- `alchemy_jpeg` is now a concrete `STATIC` target with a bounded milestone-1 source list
- `alchemy_graphics` is now a concrete `STATIC` target with a bounded milestone-1 source list centered on `DIB.cpp`, `Misc.cpp`, and `Raw.cpp`
- the remaining milestone-1 groups still exist as placeholders so the graph stays bounded

This means the next source-subset step is no longer “start from nothing.”

It is now:

- validate those three concrete targets once `cmake` is available
- validate the five concrete foundation targets once `cmake` is available
- then extend the same approach to bounded `mammoth_tse`

This order intentionally separates:

- portable core compilation
- menu-path app compilation
- macOS shell/presenter replacement work

## Scope guard for the first scaffold

The first `CMake` scaffold does not need to build the whole app.

It only needs to:

- make the target graph concrete
- compile portable code in dependency order
- isolate remaining compile blockers into shell/presenter-specific groups

If the graph starts pulling in broad gameplay, audio, or tool dependencies, it is too wide for milestone 1.

## Success criteria

This source-subset plan is successful when:

- the next implementation step can create bounded `CMake` targets without re-auditing the whole repo
- the menu-boot path remains the clear center of gravity
- compile blockers become attributable to specific target groups instead of the entire application
