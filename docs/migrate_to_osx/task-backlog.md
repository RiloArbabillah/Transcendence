# Task Backlog

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
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
- Status: `todo`
- Goal: create a parallel build system for macOS without replacing Visual Studio projects
- Depends on:
  - A-002
- Acceptance criteria:
  - project configures on macOS with `cmake`

### B-002 Add `CMakePresets.json`

- Priority: `P1`
- Status: `todo`
- Goal: provide repeatable debug/release/sanitizer presets for macOS arm64
- Depends on:
  - B-001
- Acceptance criteria:
  - at least debug, release, and sanitizer presets exist

### B-003 Define core library targets

- Priority: `P0`
- Status: `todo`
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

### B-004 Fix Apple Clang compatibility issues

- Priority: `P0`
- Status: `todo`
- Goal: resolve compile blockers due to compiler differences, case-sensitive includes, and old platform assumptions
- Depends on:
  - B-003
- Acceptance criteria:
  - core targets compile under Apple Clang on arm64

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
- Status: `todo`
- Depends on:
  - B-003

### C-002 Build `Alchemy/CodeChain`

- Priority: `P0`
- Status: `todo`
- Depends on:
  - C-001

### C-003 Build `Alchemy/XMLUtil`

- Priority: `P0`
- Status: `todo`
- Depends on:
  - C-001

### C-004 Build `Alchemy/Graphics`

- Priority: `P0`
- Status: `todo`
- Depends on:
  - C-001

### C-005 Build `Mammoth/TSE`

- Priority: `P0`
- Status: `todo`
- Depends on:
  - C-002
  - C-003
  - C-004

### C-006 Build `Mammoth/TSUI`

- Priority: `P0`
- Status: `todo`
- Depends on:
  - C-005

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

## Critical Path Summary

The shortest path to a native main menu is:

1. A-001
2. A-002
3. B-001
4. B-003
5. B-004
6. C-001 through C-006
7. D-001
8. D-002
9. E-001 through E-003
10. F-001 through F-004
11. G-001

The shortest path to a first playable build is:

1. complete native main menu path
2. G-002
3. H-001
4. H-002
5. H-003
6. H-004
7. I-003

## Recommended Next Execution Slice

- A-001 Inventory Windows-only dependencies
- A-002 Define subsystem boundaries
- B-001 Add root `CMakeLists.txt`
- B-003 Define core library targets

These tasks unlock the rest of the roadmap and should be completed before touching renderer optimization or packaging work.
