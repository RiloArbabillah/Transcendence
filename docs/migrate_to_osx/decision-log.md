# Decision Log

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Derived From: `PRD.md`
- Companion Docs: `roadmap.md`, `task-backlog.md`, `dependency-matrix.md`, `architecture.md`, `milestone-1-plan.md`, `cmake-build-plan.md`
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`

## Purpose

This document records key technical and product decisions for the macOS port. It exists to prevent hidden assumptions, reduce repeated debates, and preserve rationale as implementation begins.

Each entry should capture:

- what was decided
- why it was decided
- alternatives considered
- impact on implementation

## Decision Status Legend

- `accepted` - chosen and active
- `superseded` - replaced by a newer decision
- `deferred` - intentionally postponed
- `proposed` - under consideration but not yet locked

## D-001 - Target platform is macOS Apple Silicon only for v1

- Status: `accepted`
- Date: 2026-04-14

### Decision

The first native port target is macOS on Apple Silicon (`arm64`) only.

### Rationale

- reduces toolchain and packaging complexity
- aligns with the primary target environment for this project effort
- avoids expanding the scope to universal binaries before the native path is proven

### Alternatives Considered

- universal binary (`arm64 + x86_64`) from the start
- x86_64-first macOS compatibility path

### Impact

- `CMake` presets and compiler settings should target `arm64`
- no initial work should be blocked on Intel macOS support

## D-002 - Preserve the existing Windows build system

- Status: `accepted`
- Date: 2026-04-14

### Decision

The existing `.sln` and `.vcxproj` workflow remains the Windows build path. A parallel `CMake` build is added for macOS.

### Rationale

- lowers disruption to the upstream codebase
- avoids turning a platform port into a full build-system migration
- keeps existing Windows contributors unblocked

### Alternatives Considered

- replace the entire build with `CMake`
- generate Visual Studio projects from `CMake` as the main path immediately

### Impact

- `CMake` must be additive, not destructive
- documentation should treat `CMake` as the macOS path first

## D-003 - Use `SDL2` as the platform shell

- Status: `accepted`
- Date: 2026-04-14

### Decision

`SDL2` will replace Win32 responsibilities for windowing, event pumping, keyboard, mouse, text input, timers, and related app-shell behavior.

### Rationale

- fastest route to replacing Win32 lifecycle and input semantics
- significantly lower effort than starting with raw Cocoa/AppKit integration
- keeps the majority of the implementation in C++

### Alternatives Considered

- raw Cocoa + AppKit for the full shell
- a more custom platform layer without SDL

### Impact

- platform shell code should be organized around SDL-backed abstractions
- SDL should not leak into shared engine headers

## D-004 - Use `Metal` as the native renderer backend

- Status: `accepted`
- Date: 2026-04-14

### Decision

`Metal` will be the native renderer backend for the macOS path.

### Rationale

- it is the correct native graphics API for Apple Silicon
- it provides a long-term path to good performance and native compatibility

### Alternatives Considered

- OpenGL compatibility layer
- software-only present path as the only renderer
- Vulkan via external layers

### Impact

- app and renderer design must include a backend seam that can present via Metal
- Objective-C++ may be used in implementation files where necessary

## D-005 - Renderer strategy is compatibility-first

- Status: `accepted`
- Date: 2026-04-14

### Decision

The first renderer implementation will preserve existing software frame generation and use `Metal` only to upload and present the final frame.

### Rationale

- reduces risk dramatically compared to rewriting rendering primitives into Metal
- keeps game and UI rendering behavior closer to the Windows path
- enables faster arrival at the first visible native frame

### Alternatives Considered

- immediate GPU-native rewrite of rendering primitives
- full DirectX abstraction redesign before any frame is shown

### Impact

- renderer interfaces should focus on framebuffer presentation first
- optimization and GPU-native paths come later, only after profiling

## D-006 - Start with a native main menu milestone

- Status: `accepted`
- Date: 2026-04-14

### Decision

The first meaningful implementation milestone is a native macOS build that reaches the title or main menu and accepts input.

### Rationale

- proves shell, renderer, resource, and input architecture together
- avoids pulling full gameplay, audio, and packaging scope into the first sprint
- creates a concrete checkpoint that is visible and testable

### Alternatives Considered

- start with full gameplay as the first milestone
- start with tools-only porting before the app path

### Impact

- implementation work should prioritize menu boot path over full feature parity
- `milestone-1-plan.md` is the active execution reference for early coding

## D-007 - Preserve core gameplay and engine logic where possible

- Status: `accepted`
- Date: 2026-04-14

### Decision

The port should prefer replacing platform boundaries instead of rewriting engine and gameplay logic.

### Rationale

- reduces regression risk
- keeps behavior aligned with the original game
- avoids scope explosion

### Alternatives Considered

- broad refactoring of engine and gameplay layers while porting
- redesigning subsystems to be more generically cross-platform before reaching a working build

### Impact

- gameplay code should only be changed when a platform assumption blocks portability
- boundary interfaces must be kept narrow and practical

## D-008 - Objective-C++ is allowed only in backend and platform implementations

- Status: `accepted`
- Date: 2026-04-14

### Decision

Objective-C++ should be confined to `.mm` implementation files that need access to Apple APIs. Shared headers remain C++.

### Rationale

- keeps engine portability and readability intact
- prevents Apple APIs from leaking into shared layers
- contains language-mode complexity

### Alternatives Considered

- broader Objective-C++ usage across the project
- exposing Apple-specific types directly in headers

### Impact

- use private implementation or wrapper boundaries for `Metal`, `CAMetalLayer`, and AppKit interactions

## D-009 - Steam is out of scope for the first native playable build

- Status: `accepted`
- Date: 2026-04-14

### Decision

Steam integration is deferred until after the first native playable build.

### Rationale

- it is not necessary for milestone 1 or the first playable build
- it introduces integration complexity without helping early architecture validation

### Alternatives Considered

- keeping Steam in scope from the start

### Impact

- `TRANSCENDENCE_ENABLE_STEAM=OFF` for early `CMake` presets
- Steam code should not block macOS milestone progress

## D-010 - Use contributor-safe or stubbed cloud integration paths early

- Status: `accepted`
- Date: 2026-04-14

### Decision

Production cloud and service integrations are not required for the initial port. Contributor-safe stubs are acceptable in early milestones.

### Rationale

- reduces dependency on unavailable or private service code
- aligns with the existing contributor-oriented source distribution model

### Alternatives Considered

- attempting to support full service integration in the first milestones

### Impact

- service-dependent areas should use stubs where necessary for menu or gameplay bring-up

## D-011 - Save and resource paths must be platform-owned

- Status: `accepted`
- Date: 2026-04-14

### Decision

Bundle resources and writable runtime paths must be resolved by a platform/runtime service instead of depending on current working directory or Windows resource conventions.

### Rationale

- native macOS apps cannot rely on the same filesystem assumptions as Windows desktop binaries
- this is necessary for Finder-launched app bundles and proper save/config behavior

### Alternatives Considered

- preserving relative-path assumptions from the original Windows flow

### Impact

- resource and save path handling should be explicit from early milestones onward

## D-012 - Warning strictness should start conservative on Apple Clang

- Status: `accepted`
- Date: 2026-04-14

### Decision

Do not enforce global `-Werror` on the entire inherited codebase during early Apple Clang bring-up.

### Rationale

- reduces noise during initial portability work
- avoids turning the port into a warning cleanup project before the build graph exists

### Alternatives Considered

- strict zero-warning build from day one

### Impact

- warning policy can tighten later per target or for newly written macOS-specific code first

## D-013 - Use target-local framework linkage

- Status: `accepted`
- Date: 2026-04-14

### Decision

Apple frameworks must be linked only to the targets that require them, not globally.

### Rationale

- keeps CLI tools and core libraries free from unnecessary platform linkage
- maintains cleaner separation between core and platform-specific code

### Alternatives Considered

- global framework linkage for convenience

### Impact

- `Foundation`, `AppKit`, `QuartzCore`, and `Metal` should be attached only to platform/backend or app targets as needed

## D-014 - `transdata_cli` is a useful early smoke target, but not a blocker for milestone 1

- Status: `accepted`
- Date: 2026-04-14

### Decision

`TransData` is a good early macOS smoke target, but it should not delay the main menu milestone if the app path is already moving.

### Rationale

- it provides useful validation of the engine and data path
- but the main menu milestone is the primary architecture proof target

### Alternatives Considered

- making tools the mandatory first deliverable before any app shell work

### Impact

- `transdata_cli` can be added opportunistically during build bring-up

## D-015 - Record future significant implementation changes here

- Status: `accepted`
- Date: 2026-04-14

### Decision

All significant architecture and build choices made during implementation should be logged here rather than remaining implicit in code changes.

### Rationale

- prevents drift between planning documents and actual implementation
- makes future review and collaboration easier

### Alternatives Considered

- relying on commit history or chat history as the only rationale source

### Impact

- this file should evolve alongside implementation

## Next Expected Decisions

These are likely upcoming decisions that may need entries later:

- exact `CMake` target names used in code
- JPEG and PNG portability strategy for the macOS path
- exact SDL acquisition strategy for local builds and CI
- exact Metal view/layer integration approach
- whether menu boot requires temporary stubbing of audio or networking
- exact save/config directory naming on macOS

## Related Docs

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
- `architecture.md`
- `milestone-1-plan.md`
- `cmake-build-plan.md`
