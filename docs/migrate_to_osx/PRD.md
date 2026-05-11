# Product Requirements Document

## Product

Native macOS Apple Silicon port of `kronosaur/TranscendenceDev` using `SDL2 + Metal`

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Status: Draft v1
- Owner: Akira
- Target platform: macOS on Apple Silicon (`arm64`)
- Source project: `kronosaur/TranscendenceDev`
- Port strategy: preserve engine and gameplay logic where possible, replace Windows-specific platform and rendering layers with `SDL2 + Metal`

## Related Docs

- `index.md` - documentation portal, reading order, and maintenance guide for the full planning set
- `roadmap.md` - phased delivery roadmap, milestones, validation gates, and release sequencing
- `task-backlog.md` - actionable engineering backlog organized by epic, priority, dependencies, and acceptance criteria
- `dependency-matrix.md` - platform dependency inventory with portability classification and replace/wrap/stub/defer decisions
- `architecture.md` - target subsystem boundaries, interface ownership, dependency flow, and macOS port architecture rules
- `execution-task-plan.md` - living execution plan for the active macOS runtime-debugging and release-ready slices
- `cmake-build-plan.md` - macOS `CMake` build target graph, dependency order, presets, framework linkage, and fallback strategy
- `decision-log.md` - recorded architecture and build decisions with rationale, alternatives, and implementation impact
- `qa-test-matrix.md` - minimum validation gates, manual checks, and milestone QA requirements for the macOS port
- `change-log.md` - chronological log of documentation additions, restructures, and major planning updates

## 1. Executive Summary

This PRD defines the requirements for porting `TranscendenceDev` from its current Windows-centric implementation to a native macOS Apple Silicon build.

The project is not a simple build migration. The current codebase depends heavily on Windows technologies such as Win32 app lifecycle APIs, DirectX 9 presentation, GDI/DIB graphics paths, and Windows-specific audio backends. The safest and fastest path to a working native build is to preserve the existing software rendering and core game logic while replacing the platform shell and presentation backend.

The selected technical direction is:

- `SDL2` for application lifecycle, window management, input, timer, and platform services
- `Metal` for frame presentation and rendering backend on macOS
- `CMake` as a parallel build system for macOS, while preserving existing Visual Studio projects for Windows
- minimal Objective-C++ only where required for Apple APIs and Metal integration

The first meaningful delivery target is a native macOS build that can boot to the main menu, render correctly, and accept user input. Later milestones expand that to gameplay, save/load, audio, packaging, and performance optimization.

## 2. Problem Statement

`TranscendenceDev` currently builds and runs primarily as a Windows desktop game. The project structure and source code assume:

- Visual Studio and `.sln`/`.vcxproj` build flow
- Win32 entry point and message loop
- DirectX SDK-era rendering and presentation
- GDI/DIB support for graphics and fonts
- DirectSound and MCI-based audio paths

This prevents native execution on macOS Apple Silicon and blocks contributors who want to build, debug, and run the game on modern Mac hardware.

The project needs a maintainable native port strategy that:

- does not destroy the existing Windows workflow
- avoids a risky full-engine rewrite
- reaches a working macOS build incrementally
- keeps core gameplay and data systems intact

## 3. Goals

### 3.1 Primary Goals

- Produce a native `arm64` macOS build of `TranscendenceDev`
- Preserve existing gameplay logic and engine behavior as much as possible
- Replace the Windows shell with `SDL2`
- Replace the DirectX presentation path with a `Metal` backend
- Introduce a parallel `CMake` build system for macOS without replacing the existing Windows build system
- Reach a playable native macOS build in phased milestones

### 3.2 Secondary Goals

- Keep new macOS-specific code isolated behind narrow interfaces
- Support future optimization beyond the initial compatibility renderer
- Align save/config/resource behavior with standard macOS filesystem conventions
- Keep nonessential integrations optional or stubbed during the initial port

## 4. Non-Goals

The following are explicitly out of scope for the first implementation phases:

- replacing the Windows `.sln` / `.vcxproj` workflow
- full renderer rewrite to pure GPU-native Metal draw calls on day one
- Steam support on macOS in the first milestone set
- production cloud/Hexarc integration in the first milestone set
- notarization, App Store packaging, or distribution polish in the first milestone set
- universal binary support (`x86_64 + arm64`) in the first milestone set
- major gameplay/system redesigns unrelated to portability

## 5. Product Vision

The product is a native macOS Apple Silicon port of the existing Windows game and supporting engine layers, delivered incrementally. The port should feel like the same game, not a reimplementation. Core rendering, UI logic, scripting, game data, and session flow should remain behaviorally close to the original codebase.

The long-term architecture should support:

- Windows continuing to use the current path
- macOS using a dedicated `SDL2 + Metal` path
- future optional renderer optimizations without destabilizing gameplay logic

## 6. Success Criteria

### 6.1 Milestone Success Criteria

- S1: core engine libraries compile on macOS `arm64`
- S2: the native macOS executable opens a window and runs an event loop
- S3: a valid frame can be presented through `Metal`
- S4: title screen and menu render correctly and accept input
- S5: gameplay can start and basic game loop functions are usable
- S6: save/load and audio work natively on macOS
- S7: the game can launch as a `.app` bundle with correct resource and save paths

### 6.2 Quality Success Criteria

- no active runtime dependency on DirectX in the macOS build path
- no requirement for Win32 message loop or Windows resource loading in the macOS build path
- no writes to application bundle at runtime
- behaviorally correct keyboard, mouse, and text input in major screens
- stable runtime under common macOS lifecycle actions such as focus changes, minimize, resize, and fullscreen toggles

## 7. User Personas

### 7.1 Contributor on macOS

A developer using Apple Silicon hardware who wants to compile, run, and debug the game natively without a Windows VM.

Needs:

- local native build
- repeatable setup
- debuggable symbols
- project structure that does not require Visual Studio

### 7.2 Maintainer of the Original Codebase

A contributor or maintainer who wants the macOS port to coexist with the Windows build without destabilizing the primary code path.

Needs:

- minimal disruption to Windows build system
- clear layering and ownership of new code
- phased rollout with measurable milestones

### 7.3 Power User / Tester

A user who wants to run the game natively on macOS and expects menu, gameplay, saves, and performance to be functionally correct.

Needs:

- stable main menu and gameplay
- proper fullscreen/window behavior
- working input, sound, and saves

## 8. Technical Strategy

### 8.1 Selected Stack

- Platform shell: `SDL2`
- Rendering backend: `Metal`
- Build system: `CMake`
- Language split:
  - existing engine/gameplay code remains primarily C++
  - Objective-C++ limited to macOS backend/platform files

### 8.2 Why `SDL2 + Metal`

`SDL2` is the fastest low-risk replacement for the Win32 shell responsibilities currently embedded in the codebase, including window creation, event pump, keyboard, mouse, text input, cursor handling, and timer services.

`Metal` is the correct native rendering backend for Apple Silicon. However, the recommended first step is not to rewrite all rendering primitives into Metal. Instead, the engine should keep generating software-rendered frames where possible, and the macOS backend should upload and present those frames through Metal.

This approach reduces risk, preserves behavior, and reaches the first usable macOS build much faster.

### 8.3 Compatibility-First Renderer Strategy

Phase-one rendering should follow this sequence:

1. engine produces a CPU framebuffer using existing rendering logic
2. the macOS backend uploads that framebuffer into a Metal texture
3. the backend presents the texture to the screen

Only after the game is stable and profile data exists should the team consider migrating expensive draw paths to more GPU-native implementations.

## 9. Current-Code Constraints

The current codebase contains major Windows-specific dependencies, including but not limited to:

- Win32 shell and message loop:
  - `Transcendence/Transcendence/Main.cpp`
  - `Transcendence/Transcendence/CTranscendenceWnd.cpp`
  - `Mammoth/TSUI/Run.cpp`
  - `Mammoth/TSUI/CHumanInterface.cpp`
- DirectX rendering/presentation:
  - `Alchemy/DirectXUtil/CDXScreen.cpp`
  - `Alchemy/Include/DirectXUtil.h`
  - `Alchemy/Include/DXScreenMgr3D.h`
- Win32 graphics support:
  - `Alchemy/Graphics/DIB.cpp`
  - `Alchemy/Graphics/GDI.cpp`
- Audio:
  - `Alchemy/DirectXUtil/Sound.cpp`
  - `Mammoth/TSUI/CMCIMixer.cpp`
  - `Mammoth/TSUI/CSoundtrackManager.cpp`

The port must address these constraints without destabilizing higher-level engine and gameplay modules.

## 10. Scope by Area

### 10.1 In Scope

- macOS Apple Silicon build system via `CMake`
- SDL-based platform app shell
- SDL-based input translation layer
- Metal presentation backend
- migration away from Win32 resource assumptions for macOS runtime
- save/settings path adaptation to macOS conventions
- replacement of Windows-only audio backends needed for a usable native build

### 10.2 Out of Scope for Initial Milestones

- Steam support on macOS
- App Store distribution
- notarization and full packaging workflow
- fully GPU-native rewrite of software drawing routines
- redesign of scripting, data, or gameplay systems

## 11. Requirements

### 11.1 Functional Requirements

#### FR-1 Build and Toolchain

- The project must support a macOS `arm64` build via `CMake`
- The existing Visual Studio project structure must remain intact for Windows contributors
- The macOS build must be able to compile core engine libraries independently of Windows-only projects

#### FR-2 Native Application Shell

- The macOS executable must initialize, open a window, process events, and shut down cleanly
- The application shell must not depend on Win32 `WinMain` or a Win32 window procedure in the macOS path

#### FR-3 Input Handling

- Keyboard input must support command-based gameplay controls
- Text input must support UI text entry without double-processing command keys
- Mouse input must support menu interactions, in-game interactions, and wheel input
- Input behavior should remain as close as possible to the existing Windows gameplay semantics

#### FR-4 Rendering

- The macOS build must present engine-generated frames using a `Metal` backend
- The renderer must correctly handle pixel format, alpha, and scaling semantics
- The renderer must support resize and high-DPI behavior on modern macOS displays

#### FR-5 UI and Session Flow

- The intro screen, main menu, settings, and load/new game sessions must render and accept input
- Existing UI session logic should remain intact unless platform constraints require targeted adaptation

#### FR-6 Gameplay Bring-Up

- A user must be able to start a game and reach playable in-game state
- Core screens such as HUD, dock screens, and map-related UI must render without major corruption

#### FR-7 Resources and Filesystem

- Runtime resources must be loaded from bundle/filesystem locations compatible with macOS
- Save data, settings, and writable files must be stored in a user-writable macOS location
- The application must not rely on the current working directory for correctness

#### FR-8 Audio

- The macOS build must support sound effects and music playback without Windows-only APIs
- The soundtrack management behavior should remain close to the existing logic, including transitions and pause behavior where practical

#### FR-9 Packaging

- The application must support running as a macOS `.app` bundle
- Required resources, shaders, and data files must be discoverable when launched from Finder

### 11.2 Non-Functional Requirements

#### NFR-1 Stability

- The game must remain stable under minimize, focus loss/regain, fullscreen toggles, and window resizes

#### NFR-2 Maintainability

- macOS-specific code should remain isolated behind a clear platform/backend interface
- Objective-C++ must not leak into shared engine headers

#### NFR-3 Compatibility

- Windows build behavior should remain unchanged unless explicitly modified for cross-platform compatibility

#### NFR-4 Performance

- The first release does not require a fully optimized GPU-native renderer
- The compatibility renderer must still be performant enough to achieve acceptable interactivity on Apple Silicon hardware

#### NFR-5 Debuggability

- The port must support debug builds with symbol information
- The macOS build should support sanitizers in dedicated debug presets

## 12. Architectural Requirements

### 12.1 Platform Abstraction Boundary

The project must define a narrow platform abstraction that owns:

- app startup/shutdown
- event pumping
- window lifecycle
- timer access
- cursor and mouse capture behavior
- fullscreen behavior
- file path discovery for resources and saves

The game and engine layers should depend on this abstraction, not on SDL directly.

### 12.2 Input Abstraction Boundary

An input translation layer must convert `SDL_Event` data into the engine's existing command and text input model. The goal is to preserve current gameplay behavior while replacing the source event system.

### 12.3 Rendering Abstraction Boundary

The renderer boundary must separate:

- engine-generated image/frame data
- platform/window ownership
- actual screen presentation through `Metal`

The first implementation must avoid exposing Metal types throughout engine code.

### 12.4 Audio Abstraction Boundary

The project must provide macOS-compatible replacements for:

- sound effects playback
- music/soundtrack playback

Higher-level soundtrack orchestration should remain above the backend layer where possible.

## 13. Delivery Plan

### Phase 0 - Audit and Design

Objective:

- identify all Windows-only dependencies
- classify each as replace, wrap, stub, or defer
- define architecture seams before implementation

Deliverables:

- dependency matrix
- subsystem boundary definitions
- phased implementation map

Exit criteria:

- each major Win32/DirectX dependency has an explicit plan

### Phase 1 - macOS Build System Bring-Up

Objective:

- add a parallel `CMake` build system for macOS `arm64`
- compile core libraries in dependency order

Priority targets:

- `Alchemy/Kernel`
- `Alchemy/CodeChain`
- `Alchemy/XMLUtil`
- `Alchemy/Graphics`
- `Mammoth/TSE`
- `Mammoth/TSUI`

Deliverables:

- root `CMakeLists.txt`
- build presets
- successful arm64 static library build for the portable core

Exit criteria:

- core libraries build on Apple Clang

### Phase 2 - SDL2 Platform Shell

Objective:

- replace the Win32 shell with SDL-backed startup, event loop, and window creation

Deliverables:

- native macOS executable
- SDL window
- resize/focus/close behavior

Exit criteria:

- app launches, runs, and exits cleanly with an SDL window

### Phase 3 - Input Translation

Objective:

- route SDL keyboard, mouse, wheel, and text input into the engine's existing control systems

Deliverables:

- keyboard-driven menu navigation
- working text entry in UI fields
- mouse support for interactive screens

Exit criteria:

- menu and text entry screens are operable on macOS

### Phase 4 - Metal Presenter

Objective:

- present engine-generated frames through Metal while preserving software rendering where possible

Deliverables:

- Metal device initialization
- framebuffer upload path
- correct color/alpha output
- high-DPI-aware present path

Exit criteria:

- title screen or equivalent frame displays correctly

### Phase 5 - UI Session Bring-Up

Objective:

- enable intro/menu/settings/load/new game session flow on top of the new macOS shell and renderer

Deliverables:

- working intro/session flow
- stable transitions between major UI screens

Exit criteria:

- main menu and core menus are usable

### Phase 6 - Gameplay Bring-Up

Objective:

- reach playable in-game state with correct rendering and input

Deliverables:

- ship control
- HUD rendering
- dock UI
- basic transitions such as entering gameplay and opening key interfaces

Exit criteria:

- basic gameplay loop is playable

### Phase 7 - Resource, Save, and Audio Migration

Objective:

- finalize native macOS runtime behavior for resources, saves/settings, and audio backends

Deliverables:

- macOS save path
- bundle-based resource loading
- native sound/music path replacing Windows-only backends

Exit criteria:

- users can save/load and hear both music and SFX natively on macOS

### Phase 8 - Packaging and Optimization

Objective:

- produce a stable `.app` bundle and optimize where profiling shows real need

Deliverables:

- launchable macOS app bundle
- packaged resources and shaders
- improved frame pacing and performance where needed

Exit criteria:

- stable native `.app` build with acceptable performance on Apple Silicon hardware

## 14. Prioritized Milestones

### Milestone M1

- `CMake` exists and builds the portable core on macOS arm64

### Milestone M2

- SDL shell creates a native macOS window and event loop

### Milestone M3

- Metal presenter displays the first engine frame

### Milestone M4

- title screen and menu are usable

### Milestone M5

- gameplay can start and basic in-game flow works

### Milestone M6

- audio, save/load, and resource paths behave correctly

### Milestone M7

- `.app` bundle launches reliably and performs acceptably

## 15. Risks

### R1 Win32 Assumptions Spread Beyond the Obvious Shell

The codebase may contain Win32 dependencies outside the known shell files, including hidden assumptions in UI, file handling, and graphics helpers.

Mitigation:

- complete dependency audit before large-scale refactors

### R2 Pixel Format and Alpha Mismatch

The software rendering pipeline and the Metal upload path may disagree on channel order or alpha semantics.

Mitigation:

- validate with test patterns and screenshots before integrating gameplay screens

### R3 Font Metrics and UI Layout Drift

Replacing Windows font behavior may alter UI text flow and clipping.

Mitigation:

- preserve bundled `.dxfn` fonts where possible
- validate text-heavy screens early

### R4 Music Backend Replacement Complexity

The existing music system is coupled to MCI semantics that do not exist on macOS.

Mitigation:

- keep high-level soundtrack logic intact while replacing only the playback backend

### R5 Retina / Drawable Size Bugs

The logical game coordinate system may diverge from actual drawable dimensions on modern macOS displays.

Mitigation:

- explicitly separate logical size from drawable size in the renderer and input mapping layers

### R6 Performance of CPU Frame Uploads

The compatibility renderer may incur significant frame upload costs, especially on high-DPI displays.

Mitigation:

- start with fixed internal render size where appropriate
- profile before rewriting draw paths

## 16. Validation and QA Requirements

### 16.1 Renderer Validation

- title screen
- help screen
- dock screen
- HUD in gameplay
- map-related UI
- fullscreen and resize behavior

### 16.2 Input Validation

- menu navigation
- gameplay controls
- remapped controls
- text entry screens
- mouse wheel and pointer behavior

### 16.3 Audio Validation

- menu clicks
- overlapping combat sounds
- soundtrack transitions
- pause/unpause behavior

### 16.4 Filesystem Validation

- save game
- load game
- relaunch and recover save/settings state

### 16.5 Stability Validation

- minimize/restore
- focus loss/regain
- repeated window resize
- fullscreen transitions
- longer play sessions

## 17. Dependencies

The port depends on:

- Apple Clang toolchain with macOS `arm64` support
- `CMake`
- `SDL2`
- `Metal` and required macOS frameworks
- retained access to original game resources required by the runtime

Optional or deferred dependencies:

- Steamworks for macOS
- production cloud integrations

## 18. Open Questions

- Which JPEG/PNG path is most stable and portable for Apple Silicon in this codebase?
- Which parts of `Alchemy/Graphics` are already portable versus tightly coupled to GDI/DIB?
- Which soundtrack formats must be supported by the first native audio backend?
- Are there hidden assumptions in the resource lookup path that will affect bundle packaging?
- How much of the current renderer layering model is required for visual parity in v1?

## 19. Launch Recommendation

The recommended first launch target is not a fully polished release. The recommended sequence is:

1. core build on macOS
2. SDL shell
3. Metal presenter
4. native main menu
5. gameplay bring-up
6. audio and save/load
7. bundle packaging and optimization

This reduces scope, preserves confidence, and creates measurable checkpoints that can be validated independently.

## 20. Final Recommendation

Proceed with a compatibility-first macOS port built around `SDL2 + Metal`.

Do not begin with a full renderer rewrite. Preserve existing software rendering and core gameplay behavior, replace the platform shell and presentation backend first, and expand from there.

The immediate implementation target should be: a native macOS build that opens a window, renders the title screen or main menu, and accepts input reliably. That milestone proves the architecture and unlocks the rest of the port with manageable risk.
