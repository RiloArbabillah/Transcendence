# Roadmap

## Document Status

- Version: v1.1
- Last Updated: 2026-04-30
- Derived From: `PRD.md`
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Technical Direction: `SDL2 + Metal`

## Purpose

This roadmap translates the product requirements in `PRD.md` into a phased delivery plan. It focuses on outcome-based milestones, implementation order, dependencies, and exit criteria.

## Product Direction

- Preserve existing engine and gameplay logic where possible
- Replace Win32 app shell responsibilities with `SDL2`
- Replace DirectX presentation path with a `Metal` backend
- Keep software rendering alive first, optimize later
- Add `CMake` for macOS without disrupting the existing Windows Visual Studio workflow

## Roadmap Principles

- Compatibility first, optimization second
- Build the portable core before touching gameplay integration
- Replace boundaries, not entire subsystems
- Keep macOS-specific code isolated
- Validate each milestone before moving upward in the stack

## Release Strategy

### Release 0 - Architecture and Build Foundation

Goal:

- Establish a safe macOS porting base without changing the Windows release path

Success looks like:

- core libraries compile on macOS arm64
- project has a parallel `CMake` build system
- known platform blockers are cataloged

### Release 1 - Native Main Menu

Goal:

- A native macOS build opens a window, presents frames through Metal, and supports basic menu input

Success looks like:

- SDL window and event loop work
- Metal presenter shows the title or menu screen
- keyboard, mouse, and text entry work in core menus

### Release 2 - First Playable Build

Goal:

- A native macOS build can start a game and run the basic gameplay loop

Success looks like:

- player can enter gameplay
- HUD and dock UI render correctly enough to play
- core interactions are stable

### Release 3 - Native Runtime Parity

Goal:

- Save/load, resource lookup, and audio behave like a real native app

Success looks like:

- saves and settings go to the right macOS locations
- music and SFX work without Windows-only APIs
- app launches correctly from a macOS bundle

### Release 4 - Stabilization and Optimization

Goal:

- Deliver a stable `.app` with acceptable performance on Apple Silicon

Success looks like:

- fullscreen, resize, focus, and long-session stability are solid
- frame pacing is acceptable
- packaging is clean and repeatable

## Milestones

### M0 - Architecture Baseline

Objectives:

- inventory Windows-only dependencies
- confirm subsystem boundaries
- lock first-pass technical decisions

Primary outputs:

- dependency matrix
- platform/render/audio/filesystem boundary definitions
- phased implementation sequence

Depends on:

- none

Exit criteria:

- every major Win32/DirectX dependency has a replacement, wrapper, stub, or defer decision

### M1 - Core Build on macOS arm64

Objectives:

- build portable core libraries under Apple Clang
- establish `CMake` target graph

Primary outputs:

- root `CMakeLists.txt`
- build presets
- successful static library builds for core targets

Priority targets:

- `Alchemy/Kernel`
- `Alchemy/CodeChain`
- `Alchemy/XMLUtil`
- `Alchemy/Graphics`
- `Mammoth/TSE`
- `Mammoth/TSUI`

Depends on:

- M0

Exit criteria:

- portable core builds on macOS arm64
- no mandatory dependency on DirectX for core build success

Current status:

- static library targets now build through `mammoth_tsui`
- M1 is not fully closed until app-link smoke coverage is added, because the current final executable still exposes omitted implementation files and software drawing gaps

### M1.5 - Executable Link Closure

Objectives:

- turn static-library success into a linkable macOS executable target
- add implementation files that already exist but are missing from CMake source lists
- restore software drawing coverage without pulling in DirectX presentation
- classify remaining unresolved symbols as platform shell, presenter, audio, or deferred gameplay seams

Primary outputs:

- `transcendence_app` link succeeds, or fails only on a small documented seam list
- CMake source lists include existing support files needed by the active app path
- no unresolved-symbol noise from already-present implementation files

Depends on:

- M1

Exit criteria:

- `cmake --build --preset macos-debug --target transcendence_app` links, or every remaining unresolved symbol maps to an explicit implementation seam with an owner

### M2 - SDL Native Shell

Objectives:

- replace Win32 startup and event loop in the macOS path
- create a native SDL window with clean lifecycle behavior

Primary outputs:

- SDL-backed executable
- event pump
- window creation, resize, focus, and shutdown behavior

Depends on:

- M1
- M1.5

Exit criteria:

- app launches and quits cleanly with a visible native window

### M3 - Metal Compatibility Presenter

Objectives:

- present engine-generated frames through Metal
- preserve software framebuffer generation as the first rendering strategy

Primary outputs:

- Metal device initialization
- framebuffer upload path
- validated color and alpha output
- high-DPI-aware presentation

Depends on:

- M1
- M2

Exit criteria:

- title screen or equivalent frame displays correctly through Metal

### M4 - Main Menu Usable

Objectives:

- connect SDL input translation to the UI/session layer
- make intro and menu screens usable end to end

Primary outputs:

- keyboard-driven navigation
- text entry support
- mouse support in major menus

Depends on:

- M2
- M3

Exit criteria:

- main menu and related screens are operable without Win32 dependencies in the active path

### M5 - First Playable Gameplay

Objectives:

- enter gameplay from the menu
- keep in-game rendering and interaction stable enough for play testing

Primary outputs:

- basic flight control
- HUD rendering
- dock UI
- map and key in-game overlays

Depends on:

- M4

Exit criteria:

- user can start a game and complete a basic play loop

### M6 - Native Runtime Features

Objectives:

- move resources and writable state to macOS-friendly locations
- replace Windows-only audio paths

Primary outputs:

- bundle-based resource lookup
- save and settings path migration
- SFX and music backend replacements

Depends on:

- M4
- M5

Exit criteria:

- save/load works
- SFX and music play natively on macOS

### M7 - Packaging and Performance

Objectives:

- package the game as a stable `.app`
- optimize after profiling

Primary outputs:

- launchable `.app` bundle
- packaged resources and shaders
- targeted performance improvements

Depends on:

- M6

Exit criteria:

- app bundle launches outside the terminal and performs acceptably on Apple Silicon

## Phase Plan

### Phase 0 - Audit and Architecture

Focus:

- Windows dependency audit
- target boundary design
- implementation sequencing

Key risk retired:

- hidden Win32 assumptions spreading into later milestones

### Phase 1 - Toolchain and Core Portability

Focus:

- `CMake`
- Apple Clang compatibility
- arm64 compilation issues

Key risk retired:

- inability to build portable core without Windows tooling

### Phase 2 - Platform Shell and Input

Focus:

- SDL startup
- SDL window lifecycle
- keyboard, mouse, wheel, text input translation

Key risk retired:

- dependence on Win32 message semantics for basic app flow

### Phase 3 - Rendering Bring-Up

Focus:

- Metal present path
- framebuffer upload
- scaling and pixel correctness

Key risk retired:

- inability to render a correct frame in a native macOS window

### Phase 4 - UI and Gameplay Integration

Focus:

- menu/session flow
- gameplay entry
- HUD and dock interactions

Key risk retired:

- platform layers work individually, but not together in actual game flow

### Phase 5 - Runtime Completion

Focus:

- resources
- saves/settings
- audio
- bundle packaging

Key risk retired:

- technically running build that still does not behave like a native application

### Phase 6 - Stabilization and Optimization

Focus:

- lifecycle edge cases
- performance profiling
- frame pacing and polish

Key risk retired:

- unstable or unpleasant end-user runtime behavior

## Dependency Order

Recommended build and integration order:

1. dependency audit and architecture decisions
2. `CMake` build skeleton
3. `Alchemy/Kernel`
4. `Alchemy/CodeChain`
5. `Alchemy/XMLUtil`
6. `Alchemy/Graphics`
7. `Mammoth/TSE`
8. `Mammoth/TSUI`
9. executable link closure for omitted support files and CPU draw primitives
10. milestone no-audio/native-audio seam if MCI symbols block the link
11. SDL platform shell
12. input translation layer
13. Metal presenter
14. title/menu integration
15. gameplay integration
16. resource and save path migration
17. audio replacement
18. packaging and optimization

## Major Risks and Roadmap Responses

- Win32 dependencies beyond known shell files: handled in M0 before implementation scale-up
- pixel format and alpha mismatch in the Metal path: handled in M3 with validation before gameplay
- font and layout drift: addressed during menu and UI validation in M4 and M5
- music backend replacement complexity: deferred until native runtime features in M6, but tracked early
- Retina sizing and input coordinate mismatches: handled in M2 and M3 together
- CPU framebuffer upload performance: accepted for compatibility-first milestones, optimized only in M7

## Validation Gates

### Gate A - Build Gate

- core libraries compile on macOS arm64
- no critical unresolved toolchain blockers remain

### Gate B - Shell Gate

- SDL window opens and closes cleanly
- input events are observable in the app loop

### Gate C - Frame Gate

- a known frame renders correctly through Metal
- color, alpha, and scaling are validated

### Gate D - Menu Gate

- menu flow is usable with keyboard and mouse
- text entry works where needed

### Gate E - Gameplay Gate

- new game starts
- basic gameplay loop is interactive and stable

### Gate F - Native Runtime Gate

- save/load works
- resources resolve correctly from the bundle
- SFX and music work

### Gate G - Shipping Gate

- `.app` bundle launches cleanly
- fullscreen, resize, and focus behavior are stable
- performance is acceptable on target Apple Silicon hardware

## Immediate Next Steps

- close the `transcendence_app` link by adding present-but-omitted implementation files to CMake
- restore CPU draw/filter/fractal coverage before starting a GPU-native rewrite
- add a milestone no-audio/native-audio seam if `CMCIMixer` keeps the app from linking
- then proceed to SDL shell and Metal presenter runtime behavior

## Deferred Until Later

- Steam support on macOS
- production cloud integration
- universal binary support
- notarization and distribution polish
- broad renderer modernization beyond the compatibility presenter
