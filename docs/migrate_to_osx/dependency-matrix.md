# Dependency Matrix

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Derived From: `PRD.md`
- Companion Docs: `roadmap.md`, `task-backlog.md`
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`

## Purpose

This document maps the major code and build dependencies that affect a native macOS Apple Silicon port of `TranscendenceDev`. It is intended to answer four questions:

1. which subsystems are already likely portable
2. which subsystems are strongly Windows-specific
3. which dependencies should be replaced, wrapped, stubbed, or deferred
4. which files and modules are likely on the critical path for the first macOS milestones

## Classification Legend

- `Portable` - likely to compile cross-platform with minimal or moderate compiler fixes
- `Mostly Portable` - core logic is portable, but the module may have some platform assumptions or adjacent Windows-bound dependencies
- `Windows-Specific` - directly tied to Win32, DirectX, GDI, MCI, or other Windows-only behavior
- `Optional/Deferred` - not required for the first native playable build

## Action Legend

- `Keep` - preserve as-is except for normal compiler portability fixes
- `Wrap` - keep logic, but isolate behind a platform-neutral interface
- `Replace` - new implementation required for macOS
- `Stub` - provide reduced behavior for early milestones
- `Defer` - postpone until after first native playable build

## Milestone Relevance Legend

- `M0` - audit and architecture baseline
- `M1` - core build on macOS arm64
- `M2` - SDL native shell
- `M3` - Metal compatibility presenter
- `M4` - main menu usable
- `M5` - first playable gameplay
- `M6` - native runtime features
- `M7` - packaging and performance

## Executive Summary

- The core engine and scripting layers are likely the most portable part of the codebase.
- The main blockers are the Win32 app shell, DirectX presentation path, GDI/DIB graphics helpers, and Windows-specific audio systems.
- The recommended port path is not a full renderer rewrite. It is a shell-and-backend replacement strategy:
  - `SDL2` replaces the Win32 shell
  - `Metal` replaces DirectX presentation
  - existing software rendering stays alive as long as possible
- Steam and production cloud integrations should not block the first native playable build.

## Matrix by Dependency Area

| Area | Primary Paths / Files | Classification | Action | Milestones | Notes |
|---|---|---|---|---|---|
| Build system | `Transcendence/Transcendence.sln`, `*.vcxproj` across repo | Windows-Specific | Wrap | M0, M1 | Keep existing VS build for Windows; add parallel `CMake` for macOS |
| Core foundation | `Alchemy/Kernel/*` | Portable | Keep | M1 | Highest-value early target for Apple Clang bring-up |
| Scripting/runtime | `Alchemy/CodeChain/*` | Portable | Keep | M1 | Likely portable after compiler fixes |
| XML/data parsing | `Alchemy/XMLUtil/*` | Portable | Keep | M1 | Important for game data and toolchain |
| Graphics primitives | `Alchemy/Graphics/*` | Mostly Portable | Keep / Wrap | M1, M3, M4 | Some helpers likely portable, but `DIB.cpp` and `GDI.cpp` are Windows-bound |
| DirectX presentation | `Alchemy/DirectXUtil/CDXScreen.cpp`, `Alchemy/Include/DirectXUtil.h`, `Alchemy/Include/DXScreenMgr3D.h` | Windows-Specific | Replace | M0, M3 | Critical presentation seam for `Metal` backend |
| Software image pipeline | `Alchemy/DirectXUtil/CG16BitImage.cpp`, `Alchemy/DirectXUtil/CG32bitImage.cpp`, blit/draw files in `Alchemy/DirectXUtil/` | Mostly Portable | Keep / Wrap | M1, M3, M5 | Likely best reused for compatibility-first renderer |
| Windows app shell | `Transcendence/Transcendence/Main.cpp`, `Transcendence/Transcendence/CTranscendenceWnd.cpp`, `Mammoth/TSUI/Run.cpp` | Windows-Specific | Replace | M0, M2 | Core startup and message-pump blocker |
| Human interface/session shell | `Mammoth/TSUI/CHumanInterface.cpp`, `Mammoth/TSUI/IHISession.cpp` | Mostly Portable | Wrap | M2, M4 | Logic likely reusable once event source changes |
| Game input model | `Transcendence/Transcendence/CGameKeys.cpp`, `DefaultKeyMappings.h`, `KeyboardMapData.h`, `GameSessionInput.cpp` | Mostly Portable | Wrap | M2, M4, M5 | Needs SDL event translation layer |
| Resource file loading | `Alchemy/Graphics/CGResourceFile.cpp`, `Transcendence/Transcendence/Resources/*` | Mostly Portable | Wrap | M4, M6 | Must move away from Windows resource assumptions |
| Fonts and text | `Alchemy/Graphics/CGFont.cpp`, `Alchemy/DirectXFont/DirectXFont.cpp`, `.dxfn` resources | Mostly Portable | Wrap / Replace | M4, M5, M6 | Preserve `.dxfn` first to reduce layout drift |
| GDI / bitmap helpers | `Alchemy/Graphics/DIB.cpp`, `Alchemy/Graphics/GDI.cpp` | Windows-Specific | Replace | M0, M4, M6 | Likely affects fonts, images, and Win32 bitmap handling |
| Core engine/gameplay | `Mammoth/TSE/*` | Portable | Keep | M1, M5 | Expected to stay mostly intact |
| TSUI gameplay/UI systems | `Mammoth/TSUI/*` | Mostly Portable | Keep / Wrap | M1, M4, M5 | Some files portable, some tied to shell/audio |
| Main game application | `Transcendence/Transcendence/*` | Mostly Portable | Wrap / Replace selectively | M2, M4, M5 | Mix of gameplay logic and shell integration |
| Tooling | `Transcendence/TransData/*` | Mostly Portable | Keep | M1 | Good candidate for early macOS smoke target |
| Sound effects | `Alchemy/DirectXUtil/Sound.cpp` | Windows-Specific | Replace | M0, M6 | DirectSound-based SFX backend |
| Music / soundtrack | `Mammoth/TSUI/CMCIMixer.cpp`, `CSoundtrackManager.cpp`, `Mammoth/Include/Soundtrack.h` | Windows-Specific | Replace / Wrap | M0, M6 | MCI-based playback is a major porting blocker |
| Save/settings paths | `Transcendence/Transcendence/CGameSettings.cpp`, `GameSettings.h`, `Mammoth/TSUI/CUserSettings.cpp`, `CListSaveFilesTask.cpp`, `Mammoth/TSE/CGameFile.cpp` | Mostly Portable | Wrap | M5, M6 | Needs macOS Application Support pathing |
| Steam integration | `Mammoth/SteamUtil/*` | Optional/Deferred | Defer | M6+ | Out of scope for first native playable build |
| Cloud / Hexarc integration | `Mammoth/TSUI/CHexarcService.cpp`, `CHexarcServiceStub.cpp` | Optional/Deferred | Stub / Defer | M4, M6+ | Use contributor-safe stub for early milestones |

## Detailed Breakdown by Subsystem

## 1. Build and Toolchain

### Current Dependency

- Visual Studio solution and project files
- Windows-first build graph
- legacy DirectX SDK expectations in current Windows projects

### Classification

- `Windows-Specific`

### Porting Action

- `Wrap`

### Decision

- Do not replace the existing Windows build flow.
- Add a parallel `CMake` build for macOS.

### Why It Matters

- This is the foundation for every later milestone.
- It is also the least disruptive way to preserve trust in the existing repo structure.

### First Files / Targets to Mirror

- `Alchemy/Kernel`
- `Alchemy/CodeChain`
- `Alchemy/XMLUtil`
- `Alchemy/Graphics`
- `Mammoth/TSE`
- `Mammoth/TSUI`
- later `Transcendence/TransData`
- later `Transcendence/Transcendence`

## 2. Core Foundation and Scripting

### Primary Paths

- `Alchemy/Kernel/*`
- `Alchemy/CodeChain/*`
- `Alchemy/XMLUtil/*`

### Classification

- `Portable`

### Porting Action

- `Keep`

### Notes

- These modules are likely the cleanest early compilation targets.
- Expect compiler and portability fixes, but not architectural replacement.

### Milestone Relevance

- `M1` critical path

## 3. Core Engine and Gameplay Logic

### Primary Paths

- `Mammoth/TSE/*`
- large portions of `Transcendence/Transcendence/*`

### Classification

- `Portable` to `Mostly Portable`

### Porting Action

- `Keep`
- `Wrap` where shell or renderer assumptions leak in

### Notes

- The key strategy is to avoid modifying gameplay logic unless platform assumptions make it unavoidable.
- This subsystem should benefit from boundary replacement rather than heavy rewrites.

### Milestone Relevance

- `M1` for compile
- `M5` for first playable gameplay

## 4. Win32 Application Shell

### Primary Paths

- `Transcendence/Transcendence/Main.cpp`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp`
- `Mammoth/TSUI/Run.cpp`
- parts of `Mammoth/TSUI/CHumanInterface.cpp`

### Classification

- `Windows-Specific`

### Porting Action

- `Replace`

### Why It Matters

- This is the first hard blocker for a native executable.
- The current app lifecycle appears tied to Win32 message semantics.

### Replacement Direction

- `SDL2` app startup
- `SDL2` event loop
- SDL-backed window lifecycle and focus behavior

### Milestone Relevance

- `M0` architecture audit
- `M2` native shell

## 5. Input System

### Primary Paths

- `Transcendence/Transcendence/CGameKeys.cpp`
- `Transcendence/Transcendence/DefaultKeyMappings.h`
- `Transcendence/Transcendence/KeyboardMapData.h`
- `Transcendence/Transcendence/GameSessionInput.cpp`
- text input and control files in TSUI and game sessions

### Classification

- `Mostly Portable`

### Porting Action

- `Wrap`

### Why It Matters

- The logic is reusable, but the source event model must change from Win32 to SDL.
- The input model should stay behaviorally stable for gameplay.

### Replacement Direction

- map `SDL_KEYDOWN` and `SDL_KEYUP` to command input
- map `SDL_TEXTINPUT` to text-entry flows
- map mouse and wheel input to existing control semantics

### Milestone Relevance

- `M2`, `M4`, `M5`

## 6. DirectX Presentation and Screen Management

### Primary Paths

- `Alchemy/DirectXUtil/CDXScreen.cpp`
- `Alchemy/Include/DirectXUtil.h`
- `Alchemy/Include/DXScreenMgr3D.h`
- `Alchemy/DirectXUtil/CScreenMgr3D.cpp`

### Classification

- `Windows-Specific`

### Porting Action

- `Replace`

### Why It Matters

- This is the rendering backend seam for the macOS path.
- It should be replaced with a `Metal` presenter while preserving existing software framebuffer generation where possible.

### Replacement Direction

- define a narrow renderer boundary
- initialize a `Metal` device from the SDL window
- upload CPU-generated frames to a Metal texture
- present through a simple full-screen draw pass

### Milestone Relevance

- `M0`, `M3`

## 7. Software Rendering and Image Pipeline

### Primary Paths

- `Alchemy/DirectXUtil/CG16BitImage.cpp`
- `Alchemy/DirectXUtil/CG32bitImage.cpp`
- `Alchemy/DirectXUtil/16bit*.cpp`
- `Alchemy/DirectXUtil/32bit*.cpp`
- `Alchemy/DirectXUtil/Draw*.cpp`

### Classification

- `Mostly Portable`

### Porting Action

- `Keep`
- `Wrap`

### Why It Matters

- This is the strongest candidate for reuse in the compatibility-first renderer.
- Keeping this code alive avoids a risky full Metal renderer rewrite at the start.

### Risks

- hidden x86-era assumptions
- alignment or cast issues on arm64
- pixel format mismatch at the presentation layer

### Milestone Relevance

- `M1`, `M3`, `M5`

## 8. Graphics Helpers, DIB, and GDI

### Primary Paths

- `Alchemy/Graphics/DIB.cpp`
- `Alchemy/Graphics/GDI.cpp`

### Classification

- `Windows-Specific`

### Porting Action

- `Replace`

### Why It Matters

- These are highly likely to rely on Win32 bitmap APIs and GDI behaviors.
- They may affect font rendering, image conversion, and resource handling.

### Replacement Direction

- move resource and bitmap handling toward platform-neutral file/buffer paths
- use the preserved software image pipeline where possible
- keep Windows-specific helpers isolated from the macOS path

### Milestone Relevance

- `M0`, `M4`, `M6`

## 9. Fonts and Text Rendering

### Primary Paths

- `Alchemy/Graphics/CGFont.cpp`
- `Alchemy/DirectXFont/DirectXFont.cpp`
- `.dxfn` resources in `Transcendence/Transcendence/Resources/*`

### Classification

- `Mostly Portable`

### Porting Action

- `Wrap`
- `Replace` only where system-font creation is inherently Windows-specific

### Why It Matters

- UI fidelity depends heavily on text metrics.
- Preserving `.dxfn` support is the least risky path for early parity.

### Strategy

- keep shipped `.dxfn` resources working first
- avoid introducing new text layout drift unless necessary
- validate text-heavy screens early in `M4`

### Milestone Relevance

- `M4`, `M5`, `M6`

## 10. Resource Loading and Filesystem Access

### Primary Paths

- `Alchemy/Graphics/CGResourceFile.cpp`
- `Transcendence/Transcendence/Resources/*`
- save and settings code in:
  - `Transcendence/Transcendence/CGameSettings.cpp`
  - `Transcendence/Transcendence/GameSettings.h`
  - `Mammoth/TSUI/CUserSettings.cpp`
  - `Mammoth/TSUI/CListSaveFilesTask.cpp`
  - `Mammoth/TSE/CGameFile.cpp`

### Classification

- `Mostly Portable`

### Porting Action

- `Wrap`

### Why It Matters

- The app must work when launched from Finder, not just from a terminal.
- Runtime writes must move to a proper macOS user-writable location.

### Strategy

- app resources resolved from the bundle
- save and config paths resolved to Application Support or equivalent user paths
- no logic should depend on the current working directory

### Milestone Relevance

- `M5`, `M6`, `M7`

## 11. UI and Session Layer

### Primary Paths

- `Mammoth/TSUI/*`
- session code in `Transcendence/Transcendence/*Session*.cpp`

### Classification

- `Mostly Portable`

### Porting Action

- `Keep`
- `Wrap` where shell, input, or audio assumptions leak in

### Why It Matters

- Menu usability depends on this layer integrating cleanly with the new SDL shell and Metal presenter.

### Milestone Relevance

- `M1` compile
- `M4` menu usable
- `M5` gameplay usability

## 12. Tools and CLI-like Targets

### Primary Paths

- `Transcendence/TransData/*`

### Classification

- `Mostly Portable`

### Porting Action

- `Keep`

### Why It Matters

- This is a good early smoke target before the full app shell is ready.

### Milestone Relevance

- `M1`

## 13. Sound Effects Backend

### Primary Paths

- `Alchemy/DirectXUtil/Sound.cpp`

### Classification

- `Windows-Specific`

### Porting Action

- `Replace`

### Why It Matters

- Native gameplay cannot be considered complete without SFX.
- DirectSound is not portable to macOS.

### Strategy

- replace with an SDL- or macOS-compatible backend
- preserve expected overlap, loop, and volume semantics as much as practical

### Milestone Relevance

- `M0`, `M6`

## 14. Music and Soundtrack Backend

### Primary Paths

- `Mammoth/TSUI/CMCIMixer.cpp`
- `Mammoth/TSUI/CSoundtrackManager.cpp`
- `Mammoth/Include/Soundtrack.h`

### Classification

- `Windows-Specific`

### Porting Action

- `Replace` backend
- `Wrap` higher-level orchestration

### Why It Matters

- MCI semantics do not exist on macOS.
- Soundtrack behavior is a known high-risk area.

### Strategy

- keep `CSoundtrackManager` if possible
- replace only the underlying playback/mixer implementation

### Milestone Relevance

- `M0`, `M6`

## 15. Steam Integration

### Primary Paths

- `Mammoth/SteamUtil/*`

### Classification

- `Optional/Deferred`

### Porting Action

- `Defer`

### Why It Matters

- It is not required for the first native playable build.
- Treating it as blocking scope would slow the port significantly.

### Milestone Relevance

- after `M6`

## 16. Cloud / Hexarc Integration

### Primary Paths

- `Mammoth/TSUI/CHexarcService.cpp`
- `Mammoth/TSUI/CHexarcServiceStub.cpp`

### Classification

- `Optional/Deferred`

### Porting Action

- `Stub`
- `Defer`

### Why It Matters

- A contributor-safe stub already aligns well with the early porting strategy.

### Milestone Relevance

- `M4` if the stub must be wired in
- otherwise later

## Critical Path for First Native Main Menu

The most important dependencies for reaching a native main menu are:

1. `CMake` build graph for portable core
2. core compile of `Kernel`, `CodeChain`, `XMLUtil`, `Graphics`, `TSE`, `TSUI`
3. replacement of Win32 startup and window lifecycle with `SDL2`
4. input translation from SDL to existing command and text models
5. replacement of DirectX presentation with the Metal compatibility presenter
6. enough resource and font loading to draw title/menu screens correctly

## Critical Path for First Native Playable Build

After the main menu works, the critical path expands to:

1. gameplay session integration
2. HUD and dock rendering validation
3. save/settings path migration
4. audio backend replacement

## Recommended First Engineering Slice

- audit Win32/DirectX/GDI/MCI dependencies in known blocker files
- create a clean platform/render/audio/filesystem boundary map
- define the initial `CMake` target graph
- begin compiling the portable core on macOS arm64

## Decisions Locked by This Matrix

- use `SDL2` as the shell replacement layer
- use `Metal` as the native presentation backend
- preserve software rendering first
- defer Steam
- prefer stubs or contributor-safe paths for cloud integrations during early milestones

## Related Docs

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
