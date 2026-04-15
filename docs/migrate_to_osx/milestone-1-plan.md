# Milestone 1 Plan

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Derived From: `PRD.md`
- Companion Docs: `roadmap.md`, `task-backlog.md`, `dependency-matrix.md`, `architecture.md`
- Milestone Target: Native macOS main menu bring-up

## Purpose

This document defines the implementation plan for the first meaningful port milestone: a native macOS Apple Silicon build that opens a window, presents the title or main menu through `Metal`, and accepts basic keyboard, mouse, and text input through `SDL2`.

This is the first slice that proves the selected architecture works in practice.

## Milestone Definition

### Goal

Reach a native macOS executable that:

- builds on Apple Silicon with `CMake`
- opens a native window using `SDL2`
- presents engine-generated frames through a `Metal` backend
- reaches the title screen or main menu
- accepts keyboard and mouse input in core menu flows

### Explicit Non-Goals

- full gameplay parity
- Steam support
- production cloud integration
- full audio parity
- notarization and release packaging
- full GPU-native renderer rewrite

## Success Criteria

The milestone is successful when all of the following are true:

- core libraries required for the app path compile on macOS arm64
- the native executable launches and exits cleanly
- the SDL window appears and remains stable through resize/focus changes
- the first real frame is presented through the Metal compatibility presenter
- title screen or main menu is visible and readable
- keyboard navigation works in the main menu
- mouse interaction works in the main menu where applicable
- text entry works in at least one UI flow that needs it

## Scope

### In Scope

- architecture validation for the macOS path
- initial `CMake` build graph
- Apple Clang compilation fixes needed for the milestone path
- SDL app shell
- SDL input translation for menu flows
- Metal compatibility presenter
- enough resource and font loading for title/menu screens
- contributor-safe or stubbed handling for unsupported integrations

### Out of Scope

- complete gameplay stabilization
- sound effect and music parity
- save/load parity unless required for menu boot
- extensive performance optimization
- all tools and auxiliary applications

## Milestone Deliverables

- `CMake` build skeleton for macOS arm64
- successful build of portable core libraries used by the menu boot path
- macOS SDL app entry and platform shell
- macOS Metal presenter that can display a framebuffer
- menu input translation layer
- enough runtime path/resource support to reach title or menu
- validated log of remaining blockers for gameplay and audio phases

## Dependencies

### Hard Dependencies

- `PRD.md`
- `dependency-matrix.md`
- `architecture.md`
- Apple Clang and `CMake`
- `SDL2`
- `Metal` runtime and frameworks

### Internal Technical Dependencies

- `Alchemy/Kernel`
- `Alchemy/CodeChain`
- `Alchemy/XMLUtil`
- enough of `Alchemy/Graphics`
- enough of `Mammoth/TSE`
- enough of `Mammoth/TSUI`
- enough of `Transcendence/Transcendence`

## Assumptions

- the title or menu path can be reached without implementing every gameplay subsystem
- the software rendering path can remain intact long enough to support menu presentation
- Win32 shell code can be bypassed or replaced without rewriting session logic wholesale
- contributor-safe stubs are acceptable for unsupported integrations in this milestone

## Workstreams

## Workstream 1 - Audit the Real Menu-Boot Path

### Objective

Confirm the minimum set of libraries, resources, and platform services required to reach the title screen or main menu.

### Tasks

- identify the actual startup path from app entry to title/menu session
- identify which modules are touched before the first real frame is drawn
- identify which subsystems are required for menu-only boot versus gameplay boot
- identify any hard blockers in resource loading, fonts, or UI init

### Likely Source Areas

- `Transcendence/Transcendence/Main.cpp`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp`
- `Mammoth/TSUI/Run.cpp`
- `Mammoth/TSUI/CHumanInterface.cpp`
- `Transcendence/Transcendence/CIntroSession.cpp`
- `Transcendence/Transcendence/CTranscendenceModel.cpp`
- `Transcendence/Transcendence/CTranscendenceController.cpp`

### Output

- a reduced “menu boot dependency list”
- list of subsystems that can be stubbed for M1

### Acceptance Criteria

- we know exactly which code path to target for first menu bring-up

## Workstream 2 - CMake Build Skeleton for the Milestone Path

### Objective

Make the minimum build graph required to compile the menu boot path on macOS arm64.

### Tasks

- create root `CMakeLists.txt`
- add macOS presets or equivalent local configuration
- define initial targets for the core libraries needed by the app
- add framework and SDL linkage only where required
- keep the Windows solution untouched

### Initial Target Order

1. `Alchemy/Kernel`
2. `Alchemy/CodeChain`
3. `Alchemy/XMLUtil`
4. `Alchemy/Graphics`
5. `Mammoth/TSE`
6. `Mammoth/TSUI`
7. macOS platform target
8. Metal presenter target
9. `Transcendence` app target for the menu path

### Output

- configureable macOS build graph
- compile errors grouped by target rather than by entire repo

### Acceptance Criteria

- the project configures on macOS
- core libraries for the milestone path begin building in dependency order

## Workstream 3 - Apple Clang Bring-Up

### Objective

Resolve the minimum compile issues needed for the menu path.

### Tasks

- fix case-sensitive include issues
- fix compiler differences between MSVC and Apple Clang
- fix pointer-size or cast issues encountered in the milestone path
- isolate x86-only assumptions where they block compilation

### Scope Guard

- only fix issues on the active milestone path
- do not mass-refactor unrelated warnings or modules

### Acceptance Criteria

- required milestone targets compile cleanly enough to continue integration

## Workstream 4 - SDL Application Shell

### Objective

Replace Win32 startup and window lifecycle behavior in the macOS path.

### Tasks

- create SDL-backed app entry
- initialize SDL video and event subsystems
- create native macOS window with `SDL_WINDOW_METAL`
- handle close, focus, minimize, and resize events
- add app loop plumbing suitable for the existing engine/session flow

### Boundary Rules

- shared game code should not depend directly on SDL types
- SDL event handling should be funneled through platform wrappers

### Acceptance Criteria

- native app opens a window and shuts down cleanly
- resize and focus changes do not crash the app

## Workstream 5 - Metal Compatibility Presenter

### Objective

Display the engine-generated frame in the native SDL window.

### Tasks

- define a minimal presenter boundary
- initialize `Metal` using the SDL window
- create framebuffer upload path from CPU image memory to Metal texture
- present a known test frame first
- validate pixel format, alpha, and scaling
- wire the real engine menu frame into the presenter

### Important Constraints

- do not rewrite custom draw primitives into Metal yet
- do not overdesign the renderer abstraction for this milestone

### Acceptance Criteria

- first known test frame displays correctly
- title or menu frame displays correctly afterward

## Workstream 6 - Input Translation for Menu Flows

### Objective

Make menu interaction work through SDL input.

### Tasks

- map SDL key events to menu and command input paths
- map SDL text input for text-entry UI
- map mouse movement, button, and wheel input as needed by core menu flows
- validate repeat behavior and modifier handling

### Focus Areas

- keyboard navigation in title/menu
- any settings or text-entry UI needed to prove text input works
- pointer hit testing after drawable-size scaling

### Acceptance Criteria

- user can navigate the menu by keyboard
- mouse interactions work where expected
- text entry works in at least one required screen

## Workstream 7 - Resource and Font Bring-Up for the Menu Path

### Objective

Ensure title/menu assets load correctly in the macOS runtime path.

### Tasks

- determine which title/menu assets are required at startup
- resolve bundle or filesystem loading for those assets
- keep `.dxfn` fonts working if possible
- avoid dependence on Win32 resource APIs

### Acceptance Criteria

- title/menu screens load without missing critical resources
- key text and UI remain readable

## Workstream 8 - Integration Cleanup and Blocker Logging

### Objective

End the milestone with a clean understanding of what blocks gameplay and later runtime parity.

### Tasks

- log remaining blockers after the menu path works
- classify unresolved items into gameplay, audio, filesystem, and optimization buckets
- update backlog priorities for milestone 2 and beyond

### Acceptance Criteria

- next milestone work is obvious from the output of this milestone

## Execution Order

Recommended execution sequence:

1. audit the menu-boot path
2. create `CMake` skeleton and target graph
3. compile portable milestone dependencies
4. add SDL app shell
5. add Metal compatibility presenter with a test frame
6. connect menu frame rendering
7. wire SDL input into menu flows
8. fix resource/font path issues needed for title/menu
9. verify success criteria and log remaining blockers

## Detailed Task Breakdown

### Step 1 - Audit Menu Boot

- inspect current startup path
- determine first session shown on boot
- determine which services initialize before the first frame
- note platform-specific calls that must be bypassed early

Definition of done:

- we can name the exact minimum menu boot path and its blockers

### Step 2 - Build Graph Setup

- create root `CMakeLists.txt`
- add base include paths and definitions
- define initial library targets in dependency order
- create placeholder macOS platform and render targets if needed

Definition of done:

- project configures and produces target-by-target compile errors

### Step 3 - Core Compile Bring-Up

- compile the earliest required core targets
- fix only blockers in active path
- keep notes on modules postponed beyond the menu path

Definition of done:

- core targets for the menu milestone compile

### Step 4 - SDL Shell Bring-Up

- create window
- pump SDL events
- integrate minimal run loop
- confirm stable shutdown

Definition of done:

- a native blank window opens and closes cleanly

### Step 5 - Metal Test Frame

- initialize Metal device and present path
- show a deterministic test frame or color pattern
- validate scaling, color channels, and alpha expectations

Definition of done:

- test frame reliably presents in the SDL window

### Step 6 - Engine Frame Integration

- connect menu framebuffer output to the presenter
- verify title/menu visuals
- check layout and resource loading issues

Definition of done:

- title or main menu appears with acceptable correctness

### Step 7 - Input Integration

- keyboard support for main menu
- mouse support for main menu
- text-entry support for one representative screen

Definition of done:

- menu is actually usable, not just visible

### Step 8 - Stabilize Milestone Output

- verify launch and shutdown stability
- document unresolved blockers
- update backlog for the next milestone

Definition of done:

- milestone can be demonstrated and next steps are documented

## Risks Specific to This Milestone

### Risk 1 - Menu Boot Path Depends on More Systems Than Expected

Mitigation:

- do the startup-path audit first instead of assuming the menu path is lightweight

### Risk 2 - Fonts and Resources Break Visual Bring-Up

Mitigation:

- prioritize `.dxfn` and menu-specific resource loading early once the frame path works

### Risk 3 - SDL Input Semantics Diverge from Current Win32 Assumptions

Mitigation:

- treat keyboard, text, and mouse as separate validation steps

### Risk 4 - Metal Presenter Looks Correct with a Test Pattern but Not with Real UI Assets

Mitigation:

- validate test frame first, then validate title/menu assets immediately

### Risk 5 - Build Scope Expands Too Early

Mitigation:

- do not chase gameplay, audio, or Steam during this milestone unless they block menu boot directly

## QA Checklist for the Milestone

### Build

- project configures on macOS arm64
- milestone path builds successfully

### Window and Lifecycle

- app opens a native window
- app closes cleanly
- resize does not corrupt the window state
- focus loss/regain does not immediately break the app

### Rendering

- test frame renders correctly
- title/menu frame renders correctly
- colors are not swapped
- text is readable enough for menu use

### Input

- keyboard navigation works
- mouse click and hover work where expected
- text entry works in at least one UI screen

### Runtime Behavior

- app can be launched repeatedly without obvious startup corruption
- required startup resources are found reliably

## Exit Decision

The milestone is complete when the app demonstrates a working native main menu path. If the menu is visible but not usable, the milestone is not complete. If the app launches but only displays a blank or synthetic frame, the milestone is not complete.

## What This Unlocks Next

After this milestone succeeds, the next implementation focus should move to:

- first playable gameplay path
- save/settings path migration where needed
- SFX and music backend replacement
- bundle/runtime packaging work

## Related Docs

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
- `architecture.md`
