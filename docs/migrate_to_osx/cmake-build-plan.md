# CMake Build Plan

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Derived From: `PRD.md`
- Companion Docs: `roadmap.md`, `task-backlog.md`, `dependency-matrix.md`, `architecture.md`, `milestone-1-plan.md`
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Build Strategy: Parallel `CMake` build for macOS, preserve existing Visual Studio workflow for Windows

## Purpose

This document defines the build-system plan for bringing `TranscendenceDev` to macOS Apple Silicon using `CMake`. It focuses on:

- target graph design
- dependency order
- build presets
- framework and library linkage
- fallback options when individual modules are not immediately portable

The goal is to create a practical, low-risk path to a working macOS build without disrupting the existing Windows solution.

## Build Strategy Summary

- Keep the current Windows `.sln` and `.vcxproj` files unchanged
- Add a parallel top-level `CMake` build for macOS
- Mirror the current logical project graph rather than inventing a new build structure too early
- Build portable core libraries first
- Add macOS-specific platform and rendering targets only where needed
- Keep unsupported integrations optional or disabled in early milestones

## Build Goals

### Primary Goals

- configure and build on macOS Apple Silicon (`arm64`)
- preserve the current Windows build path untouched
- compile the portable core before the app shell
- support a native `SDL2 + Metal` app path
- make the first milestone build independently testable

### Non-Goals

- replacing Windows MSBuild with `CMake`
- achieving full platform parity in one build step
- universal binary support in the initial phase
- Steam-enabled macOS builds in the initial phase

## Build Topology

The macOS build should be organized in four groups:

1. foundation libraries
2. engine and UI libraries
3. macOS platform/backend libraries
4. app and tool executables

## Proposed Target Graph

### Group A - Foundation Libraries

These targets should compile first and should avoid macOS-specific frameworks unless strictly necessary.

- `alchemy_kernel`
  - source basis: `Alchemy/Kernel/*`
  - role: foundational utilities and shared low-level code

- `alchemy_codechain`
  - source basis: `Alchemy/CodeChain/*`
  - depends on: `alchemy_kernel`
  - role: scripting and expression runtime

- `alchemy_xmlutil`
  - source basis: `Alchemy/XMLUtil/*`
  - depends on: `alchemy_kernel`
  - role: XML and structured data parsing

- `alchemy_graphics`
  - source basis: portable subset of `Alchemy/Graphics/*`
  - depends on: `alchemy_kernel`
  - role: graphics data structures and helpers

- `alchemy_zlib`
  - source basis: vendored zlib or fallback source selection
  - role: compression dependency for engine/runtime features

- `alchemy_jpeg`
  - source basis: existing JPEG path or a selected portable subset
  - depends on: `alchemy_kernel`
  - role: image decoding support

### Group B - Engine and UI Libraries

- `mammoth_tse`
  - source basis: `Mammoth/TSE/*`
  - depends on:
    - `alchemy_kernel`
    - `alchemy_codechain`
    - `alchemy_xmlutil`
    - `alchemy_graphics`
    - `alchemy_zlib`
    - `alchemy_jpeg`
  - role: core engine/game systems

- `mammoth_tsui`
  - source basis: `Mammoth/TSUI/*`
  - depends on:
    - `mammoth_tse`
    - `alchemy_kernel`
    - `alchemy_graphics`
  - role: human interface and UI framework

### Group C - macOS Platform and Runtime Libraries

- `platform_common`
  - role: shared platform-neutral wrappers and interfaces
  - contains: C++ interfaces only

- `platform_sdl`
  - role: SDL-backed app shell, window lifecycle, event pump, input translation support
  - depends on:
    - `platform_common`
    - SDL2

- `platform_macos`
  - role: macOS-specific adapters that are awkward or unsafe to express purely via SDL
  - depends on:
    - `platform_common`
  - implementation language: Objective-C++ where needed

- `render_common`
  - role: renderer-facing abstractions and framebuffer view types
  - contains: C++ interfaces only

- `render_metal`
  - role: Metal compatibility presenter for macOS
  - depends on:
    - `render_common`
    - `platform_common`
    - `platform_sdl`
  - implementation language: Objective-C++ and/or C++ depending on SDL/Metal bridge design

- `audio_backend`
  - role: native audio backends for future milestones
  - depends on:
    - `platform_common`
  - note: may remain stubbed or excluded in milestone 1 if not required to reach menu boot

### Group D - Executables

- `transdata_cli`
  - source basis: `Transcendence/TransData/*`
  - depends on:
    - `mammoth_tse`
    - relevant Alchemy libraries
  - role: early smoke-test utility target

- `transcendence_app`
  - source basis: `Transcendence/Transcendence/*`
  - depends on:
    - `mammoth_tsui`
    - `mammoth_tse`
    - `platform_common`
    - `platform_sdl`
    - `render_common`
    - `render_metal`
    - `platform_macos` where needed
  - role: native macOS game application

## Target Graph Summary

Recommended dependency flow:

1. `alchemy_kernel`
2. `alchemy_codechain`
3. `alchemy_xmlutil`
4. `alchemy_graphics`
5. `alchemy_zlib`
6. `alchemy_jpeg`
7. `mammoth_tse`
8. `mammoth_tsui`
9. `platform_common`
10. `platform_sdl`
11. `platform_macos`
12. `render_common`
13. `render_metal`
14. `transdata_cli`
15. `transcendence_app`

## Why This Graph

- It mirrors the existing logical layering in the Windows solution as much as possible.
- It lets the team build portable code first before dealing with SDL, Objective-C++, or Metal.
- It creates narrow seams for platform-specific code instead of letting those dependencies spread upward.

## Dependency Order by Milestone

### Milestone 1 - Native Main Menu

Required build order:

1. `alchemy_kernel`
2. `alchemy_codechain`
3. `alchemy_xmlutil`
4. `alchemy_graphics`
5. `alchemy_zlib`
6. `alchemy_jpeg`
7. `mammoth_tse`
8. `mammoth_tsui`
9. `platform_common`
10. `platform_sdl`
11. `render_common`
12. `render_metal`
13. `transcendence_app`

Optional in milestone 1:

- `platform_macos`
- `audio_backend`
- `transdata_cli`

### Milestone 2 - First Playable Build

Adds stronger dependence on:

- more `Transcendence/Transcendence/*` gameplay files
- runtime path services
- save/settings path support

### Milestone 3 - Native Runtime Parity

Adds stronger dependence on:

- `audio_backend`
- bundle resource path handling
- save/load infrastructure

## Recommended CMake Layout

Suggested top-level structure:

- `CMakeLists.txt`
- `CMakePresets.json`
- `cmake/`
  - `CompilerOptions.cmake`
  - `PlatformOptions.cmake`
  - `ThirdParty.cmake`
  - `AppleFrameworks.cmake`
  - `TranscendenceTargets.cmake`
- optional future directories:
  - `Platform/`
  - `Render/`
  - `Audio/`

## Recommended Presets

### `macos-debug`

Purpose:

- main local development preset

Configuration:

- `CMAKE_BUILD_TYPE=Debug`
- `CMAKE_OSX_ARCHITECTURES=arm64`
- assertions enabled
- symbols enabled
- contributor-safe mode enabled
- Steam disabled

Use cases:

- day-to-day compile and debug
- first menu bring-up

### `macos-relwithdebinfo`

Purpose:

- stable profiling and integration testing build

Configuration:

- `CMAKE_BUILD_TYPE=RelWithDebInfo`
- `CMAKE_OSX_ARCHITECTURES=arm64`
- symbols enabled
- moderate optimization enabled

Use cases:

- frame pacing checks
- general QA

### `macos-release`

Purpose:

- candidate build for later packaging tests

Configuration:

- `CMAKE_BUILD_TYPE=Release`
- `CMAKE_OSX_ARCHITECTURES=arm64`
- Steam disabled in early milestones

Use cases:

- bundle and launch verification

### `macos-asan`

Purpose:

- detect memory and undefined behavior issues early in the macOS path

Configuration:

- `CMAKE_BUILD_TYPE=Debug` or `RelWithDebInfo`
- `-fsanitize=address,undefined`
- frame pointers preserved

Use cases:

- milestone integration verification
- debugging startup and renderer issues

## Recommended Toolchain Settings

- `CMAKE_OSX_ARCHITECTURES=arm64`
- `CMAKE_OSX_DEPLOYMENT_TARGET=13.0` or lower if broader support becomes necessary
- C++ standard: `C++20`
- exceptions: enabled
- RTTI: enabled unless proven unnecessary
- debug symbols: enabled in debug and relwithdebinfo

### Warning Policy

Initial recommendation:

- do not enable global `-Werror` immediately for the entire inherited codebase
- allow normal warnings during early bring-up
- tighten warning policy later per target or for new macOS-specific files first

Why:

- Apple Clang will expose a different warning profile from MSVC
- strict zero-warning policy too early will turn the port into a cleanup project instead of a build project

## Framework and Library Link Plan

### Frameworks Likely Needed for macOS App Targets

Link only to targets that need them.

- `Foundation`
  - path and runtime helpers
- `AppKit`
  - app and window integration not covered cleanly by SDL
- `QuartzCore`
  - `CAMetalLayer`
- `Metal`
  - renderer backend
- `MetalKit`
  - optional; only if using helper classes or a simpler presentation integration path
- `CoreGraphics`
  - optional path utilities, image or display helpers if needed later
- `CoreText`
  - optional later fallback for font work if required
- audio frameworks later if not purely SDL-backed:
  - `AudioToolbox`
  - `AVFoundation`

### SDL Link Strategy

For milestone 1:

- `platform_sdl` links against SDL2
- `transcendence_app` should not need to link SDL directly if platform code owns that dependency cleanly

### zlib and JPEG Strategy

For early milestones:

- prefer vendored or source-based build integration under `CMake`
- avoid introducing package-manager dependence unless needed to unblock progress fast

## Compile Definitions and Feature Flags

Recommended early options:

- `TRANSCENDENCE_PLATFORM_MACOS=ON`
- `TRANSCENDENCE_PLATFORM_SDL=ON`
- `TRANSCENDENCE_RENDER_METAL=ON`
- `TRANSCENDENCE_ENABLE_STEAM=OFF`
- `TRANSCENDENCE_ENABLE_HEXARC=OFF` or contributor-stub path enabled
- `TRANSCENDENCE_CONTRIBUTOR_BUILD=ON`

Use these through `target_compile_definitions` instead of global preprocessor leakage where possible.

## Build Order Execution Plan

### Stage 1 - Configure Foundation

- add root `CMakeLists.txt`
- add preset file
- add common include directories and compile definitions

Exit criteria:

- `cmake` configure succeeds or fails with clear missing-target issues only

### Current implementation status

The repository now contains an initial bounded scaffold:

- root `CMakeLists.txt`
- `CMakePresets.json`

The current scaffold already models the milestone-1 target graph shape and includes concrete `STATIC` targets for:

- `alchemy_kernel`
- `alchemy_codechain`
- `alchemy_xmlutil`
- `alchemy_jpeg`
- bounded `alchemy_graphics`

Current caution:

- `alchemy_kernel` is now known to include both a Win32 service surface and archive/reference paths with 32-bit pointer storage assumptions
- `alchemy_kernel` also still exposes internet-heavy paths that are not good candidates for the earliest portable-core attempt
- early build success may require narrowing the effective portable subset further instead of assuming the current concrete target can be made arm64-clean purely through local shim work

The remaining milestone-1 targets are still placeholders so the target graph can stay bounded while source lists are narrowed further.

### Current validation blocker

`cmake` itself is not available in the current environment.

Observed checks:

- `which -a cmake` -> not found
- `xcrun --find cmake` -> not found
- `/Applications/CMake.app/Contents/bin` -> not present
- `/opt/homebrew/bin` -> present, but no `cmake` binary available there

Practical implication:

- the next agent should not assume configure failure means a build-graph error yet
- the first required step is to provide `cmake` in `PATH` or through an installed app/toolchain
- once `cmake` is available, the first meaningful validation command remains `cmake --preset macos-debug`

### Stage 2 - Bring Up Foundation Libraries

- build `alchemy_kernel`
- build `alchemy_codechain`
- build `alchemy_xmlutil`
- build `alchemy_graphics`
- build `alchemy_zlib`
- build `alchemy_jpeg`

Exit criteria:

- foundation libraries compile on macOS arm64

### Stage 3 - Bring Up Engine Libraries

- build `mammoth_tse`
- build `mammoth_tsui`

Exit criteria:

- engine and UI libs compile far enough to support app-shell integration

### Stage 4 - Bring Up Platform and Renderer Targets

- add `platform_common`
- add `platform_sdl`
- add `render_common`
- add `render_metal`

Exit criteria:

- native window and Metal test-frame path can be compiled

### Stage 5 - Bring Up the App Target

- define `transcendence_app`
- connect milestone path sources only as needed
- link platform and renderer targets

Exit criteria:

- app target links and can be debugged even if not yet fully functional

### Stage 6 - Add Tool and Smoke Targets

- define `transdata_cli` when practical
- add basic smoke tests or CTest registration if possible

Exit criteria:

- build graph supports at least one non-app validation path

## Fallback Strategy

### Fallback 1 - If `Alchemy/Graphics` Is Not Immediately Portable

Action:

- isolate only the subset required for menu boot
- temporarily exclude deeply Windows-bound helpers from the macOS path

Why:

- prevents one graphics helper from blocking the whole foundation build

### Fallback 2 - If JPEG Integration Blocks Core Build

Action:

- stub or replace the JPEG target temporarily for milestone 1
- focus on the minimal resource set needed to reach title/menu

Why:

- not every image path must be solved before the first menu frame

### Fallback 3 - If `TSUI` Pulls in Too Many Windows-Specific Files

Action:

- split portable session/UI logic from shell/audio-specific files inside the CMake target graph
- create a reduced `mammoth_tsui` source list for macOS if needed

Why:

- avoids blocking the menu milestone on unrelated Windows shell code

### Fallback 4 - If the App Target Is Too Large Too Early

Action:

- prioritize `transdata_cli` or a dedicated smoke executable first
- use a minimal boot harness to validate the engine and presenter independently

Why:

- preserves progress even if the full app integration slips

### Fallback 5 - If `Metal` Integration Slows the Build Bring-Up

Action:

- first compile the platform and app shell without a real renderer
- then add a test-frame presenter once the executable lifecycle is stable

Why:

- separates app-shell failures from renderer failures

### Fallback 6 - If Objective-C++ Linkage Creates Early Friction

Action:

- isolate all `.mm` files to `platform_macos` and `render_metal`
- keep the rest of the graph pure C++

Why:

- keeps language-mode problems contained to the smallest possible set of targets

## What Not To Do

- do not replace the Windows solution with `CMake`
- do not make `transcendence_app` the first target to compile
- do not expose SDL or Metal headers in shared engine interfaces
- do not block the first milestone on Steam, production cloud integration, or full audio support
- do not overdesign the renderer abstraction before a working frame path exists

## Recommended First Implementation Slice

Use this exact build slice first:

1. root `CMakeLists.txt`
2. `CMakePresets.json`
3. `alchemy_kernel`
4. `alchemy_codechain`
5. `alchemy_xmlutil`
6. `alchemy_graphics`
7. `mammoth_tse`
8. `mammoth_tsui`
9. `platform_common`
10. `platform_sdl`
11. `render_common`
12. `render_metal`
13. `transcendence_app`

This sequence aligns with milestone 1 and avoids dragging in deferred runtime concerns too early.

## Validation Checklist

### Configure Validation

- `cmake` configure succeeds on macOS arm64
- presets resolve correctly

### Build Validation

- targets compile in dependency order
- per-target compile failures are understandable and localized

### Link Validation

- app links without Windows SDK or DirectX requirements in the macOS path
- frameworks only link to the targets that need them

### Architecture Validation

- shared engine headers remain free of SDL, Metal, and Objective-C declarations
- platform-specific code remains isolated to platform/backend targets

## Recommended Follow-Up Docs After This Plan

- `decision-log.md`
- optional future `cmake-target-map.md` with actual file lists once the repo is cloned locally

## Related Docs

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
- `architecture.md`
- `milestone-1-plan.md`
