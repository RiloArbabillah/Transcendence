# QA Test Matrix

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Derived From: `PRD.md`
- Companion Docs: `roadmap.md`, `task-backlog.md`, `dependency-matrix.md`, `architecture.md`, `milestone-1-plan.md`, `cmake-build-plan.md`, `decision-log.md`
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`

## Purpose

This document defines the minimum validation required for each milestone of the macOS port. It is meant to keep validation proportional to the active delivery target and to prevent implementation from drifting ahead of proof.

The matrix is intentionally milestone-driven, not feature-complete. Each milestone should only be considered complete when its minimum QA bar is met.

## Validation Principles

- Validate the current milestone, not the entire future product
- Prefer small repeatable checks over vague “seems fine” verification
- Separate build validation from runtime validation
- Validate both correctness and isolation of platform boundaries
- Add deeper regression scenarios only when a milestone introduces those features

## Status Legend

- `required` - must pass before the milestone is considered complete
- `recommended` - strongly preferred, but not a hard gate for the milestone
- `deferred` - intentionally postponed to a later milestone

## Test Categories

- Build and configure
- Launch and lifecycle
- Rendering and presentation
- Input and UI interaction
- Resources and filesystem
- Audio
- Gameplay
- Performance and stability
- Architecture and boundary integrity

## Milestone QA Overview

| Milestone | Main Goal | Minimum QA Focus |
|---|---|---|
| M0 | Architecture baseline | planning correctness, dependency classification, boundary clarity |
| M1 | Core build on macOS | configure/build success, target graph correctness, portability blockers identified |
| M2 | SDL native shell | app launch, window lifecycle, event pump, resize/focus behavior |
| M3 | Metal compatibility presenter | first frame correctness, color/alpha validation, resize and DPI correctness |
| M4 | Main menu usable | menu visibility, keyboard/mouse input, text-entry viability, required resources |
| M5 | First playable gameplay | gameplay entry, HUD, dock/map interactions, basic play-loop stability |
| M6 | Native runtime parity | save/load, bundle/resource pathing, sound/music functionality |
| M7 | Packaging and performance | `.app` launch, lifecycle polish, frame pacing, long-session stability |

## Milestone-by-Milestone Minimum Validation

## M0 - Architecture Baseline

### Objective

Validate that the planning artifacts are specific enough to begin implementation safely.

### Required Checks

- dependency areas are classified as `keep`, `wrap`, `replace`, `stub`, or `defer`
- build strategy is explicit and does not conflict with preserving the Windows path
- platform, renderer, audio, and filesystem boundaries are documented
- milestone 1 success criteria are defined and testable

### Recommended Checks

- major risks are logged with mitigations
- key build and runtime decisions are recorded in the decision log

### Exit Rule

- do not begin coding without a clear milestone-1 target and subsystem boundary agreement

## M1 - Core Build on macOS arm64

### Objective

Validate that the portable core can be configured and built in a macOS `CMake` flow.

### Required Checks

#### Build and Configure

- `cmake` configure succeeds on Apple Silicon
- selected preset resolves expected variables and toolchain settings
- build graph includes the intended core targets in dependency order

#### Core Target Validation

- `alchemy_kernel` builds
- `alchemy_codechain` builds
- `alchemy_xmlutil` builds
- `alchemy_graphics` builds or is reduced cleanly to the subset needed for the milestone path
- `mammoth_tse` builds or reaches a known blocker with a documented cause
- `mammoth_tsui` builds or reaches a known blocker with a documented cause

#### Failure Quality

- compile failures are localized to specific targets
- active blockers are recorded with file/module ownership

### Recommended Checks

- one early smoke target or tool target compiles
- sanitizer preset configures successfully even if not fully run yet

### Exit Rule

- milestone passes when the macOS build path is real and the blocking compile issues are localized and understood

## M2 - SDL Native Shell

### Objective

Validate that the app can launch and own a real native window without Win32 lifecycle dependencies.

### Required Checks

#### Launch and Lifecycle

- app launches from the build directory
- app opens a native window
- app closes cleanly from the window close action
- repeated relaunch works

#### Window Behavior

- window resize does not immediately corrupt or crash the app
- focus loss and regain do not immediately crash the app
- minimize and restore do not immediately crash the app

#### Event Loop

- SDL event pump is active
- no hard dependency on Win32 message pumping remains in the active macOS path

#### Architecture Integrity

- SDL is not exposed through shared engine headers introduced for this milestone

### Recommended Checks

- window title updates correctly
- fullscreen toggle path is either working or explicitly stubbed with a known limitation

### Exit Rule

- milestone passes when the app shell behaves as a usable native windowed process on macOS

## M3 - Metal Compatibility Presenter

### Objective

Validate that the macOS path can show a correct frame through `Metal` using the compatibility-first presenter.

### Required Checks

#### Renderer Bring-Up

- Metal initialization succeeds on the target machine
- renderer can present a deterministic test frame
- renderer can present the first real engine frame or a menu-adjacent frame

#### Pixel Correctness

- red, green, and blue channels are correct
- alpha and transparency behavior are acceptable for milestone visuals
- there is no obvious byte-order corruption

#### Resize and DPI

- output remains visible after resize
- logical size and drawable size are both recorded and handled
- pointer/input coordinate system is not obviously broken by scaling assumptions

#### Architecture Integrity

- Metal implementation details remain isolated to renderer/platform backend code

### Recommended Checks

- compare screenshots or visual output against the Windows build for a simple screen
- test a simple letterbox/pillarbox scenario if aspect handling is already implemented

### Exit Rule

- milestone passes when the app can reliably present a correct frame in a native macOS window

## M4 - Main Menu Usable

### Objective

Validate that the title or main menu is not just visible, but actually operable.

### Required Checks

#### Menu Rendering

- title screen or main menu is visible
- required menu text is readable
- no critical menu assets are missing

#### Keyboard Input

- main menu navigation works from keyboard
- enter/escape/back behavior is correct enough for menu flow
- key repeat does not break navigation

#### Mouse Input

- pointer hover works where expected
- pointer click works where expected
- pointer position is accurate enough for menu interaction

#### Text Entry

- at least one UI screen requiring text entry accepts typed input
- text input does not double-insert characters from mixed key/text handling

#### Resource Loading

- menu boot path finds the resources it needs without relying on Windows resource APIs

### Recommended Checks

- settings and load/new game related menus open without obvious layout corruption
- text-heavy menu screens are sampled for clipping or font regressions

### Exit Rule

- milestone passes when a user can launch the app and use the main menu normally on macOS

## M5 - First Playable Gameplay

### Objective

Validate the first end-to-end playable slice after menu bring-up.

### Required Checks

#### Gameplay Entry

- new game starts successfully
- the game reaches an in-game state without immediate fatal errors

#### Core Interaction

- basic movement or ship control works
- pause/help/menu interactions used in gameplay work at a basic level

#### UI and Rendering

- HUD is visible and readable
- dock screens open and respond to input
- at least one map-related screen or overlay opens correctly if required by the flow

#### Stability

- short gameplay session is possible without critical crashes

### Recommended Checks

- test at least one combat or effects-heavy scene
- test stargate or scene transition if reachable early

### Exit Rule

- milestone passes when a user can start and sustain a short gameplay loop on macOS

## M6 - Native Runtime Parity

### Objective

Validate the runtime features that make the app behave like a real native macOS build.

### Required Checks

#### Saves and Settings

- save files can be created
- save files can be loaded
- settings persist across relaunch
- runtime writes do not target the app bundle

#### Resource and Path Handling

- launching from Finder resolves resources correctly
- launch from terminal and launch from Finder behave consistently enough for runtime paths

#### Audio

- menu UI sounds work if applicable
- gameplay sound effects work if applicable
- music/soundtrack playback works in the supported milestone path
- pause/resume behavior for music is acceptable

### Recommended Checks

- test a transition where music state changes
- test repeated save/load cycles

### Exit Rule

- milestone passes when runtime paths, save behavior, and audio all work as native features rather than temporary hacks

## M7 - Packaging and Performance

### Objective

Validate that the product feels stable and launchable as a native macOS application.

### Required Checks

#### Packaging

- `.app` bundle launches from Finder
- required assets and shaders are present in the bundle
- application no longer depends on a terminal working directory

#### Lifecycle and Stability

- resize works repeatedly
- minimize and restore behave correctly
- focus loss and regain behave correctly
- fullscreen behavior is either stable or clearly documented as limited

#### Performance

- frame pacing is acceptable on target Apple Silicon hardware
- there are no obvious catastrophic stalls in normal play/menu flows

#### Stability Duration

- app survives a longer representative session without critical instability

### Recommended Checks

- run under a sanitizer build where practical
- capture at least one profiling session for renderer upload cost and frame timing

### Exit Rule

- milestone passes when the app is stable enough to be treated as a real macOS build, not just a debug-only porting artifact

## Cross-Cutting Validation Matrix

| Category | M0 | M1 | M2 | M3 | M4 | M5 | M6 | M7 |
|---|---|---|---|---|---|---|---|---|
| Planning completeness | required | recommended | deferred | deferred | deferred | deferred | deferred | deferred |
| Configure/build success | recommended | required | required | required | required | required | required | required |
| Window launch/lifecycle | deferred | recommended | required | required | required | required | required | required |
| Renderer correctness | deferred | deferred | recommended | required | required | required | required | required |
| Keyboard input | deferred | deferred | recommended | recommended | required | required | required | required |
| Mouse input | deferred | deferred | recommended | recommended | required | required | required | required |
| Text entry | deferred | deferred | deferred | deferred | required | recommended | recommended | recommended |
| Resource loading | deferred | recommended | recommended | required | required | required | required | required |
| Save/load | deferred | deferred | deferred | deferred | deferred | recommended | required | required |
| Audio | deferred | deferred | deferred | deferred | deferred | recommended | required | required |
| Gameplay loop | deferred | deferred | deferred | deferred | deferred | required | required | required |
| Packaging | deferred | deferred | deferred | deferred | deferred | deferred | recommended | required |
| Performance profiling | deferred | deferred | deferred | recommended | recommended | recommended | recommended | required |

## Minimum Manual Test Passes Per Milestone

### M1 Manual Pass

- configure build
- compile core targets
- review blocker list

### M2 Manual Pass

- launch app
- close app
- resize window
- alt-tab or switch focus away and back

### M3 Manual Pass

- show test frame
- resize while frame is visible
- validate color correctness visually

### M4 Manual Pass

- launch to menu
- navigate menu via keyboard
- navigate menu via mouse
- test one text-entry field

### M5 Manual Pass

- start new game
- play briefly
- open a core UI surface such as HUD or dock

### M6 Manual Pass

- save game
- reload game
- verify audio playback
- launch from Finder if packaging is available

### M7 Manual Pass

- launch bundled app
- play longer session
- exercise resize/minimize/fullscreen/focus transitions

## Recommended Automated Checks Over Time

These are not all required immediately, but should be added when practical.

### Early Candidates

- `cmake` configure in CI
- core target build in CI
- app target build in CI

### Later Candidates

- smoke executable or startup test
- simple frame generation / renderer smoke test
- save/load smoke test
- sanitizer build lane

## Failure Reporting Template

When a milestone validation fails, record:

- milestone
- category
- exact step that failed
- file/module suspected
- severity
- whether it blocks milestone exit

Suggested structure:

```text
Milestone:
Category:
Step:
Observed Result:
Expected Result:
Likely Area:
Blocker: yes/no
Notes:
```

## Exit Gate Summary

- M0 gate: planning is specific and implementation-safe
- M1 gate: build graph is real and core portability blockers are localized
- M2 gate: native SDL app shell is stable
- M3 gate: Metal presenter shows correct frames
- M4 gate: main menu is usable
- M5 gate: first playable gameplay loop works
- M6 gate: save/load, paths, and audio behave natively
- M7 gate: bundled app is stable and performs acceptably

## Related Docs

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
- `architecture.md`
- `milestone-1-plan.md`
- `cmake-build-plan.md`
- `decision-log.md`
