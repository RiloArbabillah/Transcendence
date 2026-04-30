# Transcendence macOS Port - Current Status

## Overview

This document records the current state of the native macOS Apple Silicon port and the audited path needed to finish it. The port strategy remains compatibility-first: keep existing engine/gameplay logic, keep software frame generation, replace Win32 shell/presentation/resource/audio boundaries with native macOS equivalents.

## Build Status

**Current State:** core and engine static libraries build on macOS; the final app target compiles but fails at link due to missing implementation files and unfinished platform/backend seams.

**Validated Locally:** 2026-04-30

```sh
cmake --preset macos-debug
cmake --build --preset macos-debug --target alchemy_kernel
cmake --build --preset macos-debug --target alchemy_codechain alchemy_xmlutil alchemy_jpeg alchemy_graphics mammoth_tse
cmake --build --preset macos-debug --target mammoth_tsui
cmake --build --preset macos-debug --target transcendence_app
```

Result:

- configure succeeds
- `alchemy_kernel` builds
- `alchemy_codechain`, `alchemy_xmlutil`, `alchemy_jpeg`, `alchemy_graphics`, and `mammoth_tse` build
- `mammoth_tsui` builds
- `transcendence_app` reaches final link and fails on unresolved symbols

## What Works

### Build Graph

- Root `CMakeLists.txt` and `CMakePresets.json` exist.
- The macOS target graph configures under `macos-debug`.
- The following static library targets build:
  - `libalchemy_kernel.a`
  - `libalchemy_codechain.a`
  - `libalchemy_xmlutil.a`
  - `libalchemy_jpeg.a`
  - `libalchemy_graphics.a`
  - `libmammoth_tse.a`
  - `libmammoth_tsui.a`
  - `libplatform_sdl.a`

### Portability Progress

- The previous `Alchemy/Include/Kernel.h` arm64 pointer-cast blocker no longer prevents `alchemy_kernel` from building.
- The previous Win32 file-mapping blockers in `CFileReadBlock.cpp` / `CFileReadStream.cpp` no longer block `alchemy_kernel` compilation.
- File-based resource lookup and neutral image-loading work exists for title/menu-critical callers.
- `CMake` failure quality is now useful: failures are mostly at final link instead of early global compile blockers.

## Current Blocker

The active blocker is final app linking, not core compilation.

`transcendence_app` currently fails with unresolved symbols in several clusters. Most are not truly missing from the repository; they are implementation files that are absent from the CMake source lists or platform-specific backends that still need stubs/replacements.

## Linker Blocker Clusters

### 1. Source Files Present but Not Linked

These should be added to the appropriate CMake targets first. They are the cheapest wins and should reduce linker noise before any new code is written.

| Missing symbols | Source file found | Target owner |
|---|---|---|
| `CIconLabelBlock::*` | `Alchemy/DirectXUtil/CIconLabelBlock.cpp` | `alchemy_graphics` |
| `CNoiseGenerator::*` | `Alchemy/DirectXUtil/CNoiseGenerator.cpp` | `alchemy_graphics` |
| `AGArea::*` | `Alchemy/DirectXUtil/AGArea.cpp` | `alchemy_graphics` |
| `AGScreen::*` | `Alchemy/DirectXUtil/AGScreen.cpp` | `alchemy_graphics` |
| `Kernel::CDictionary::*` | `Alchemy/Kernel/CDictionary.cpp` | `alchemy_kernel` |
| `Kernel::CAtomizer::*` | `Alchemy/Kernel/CAtomizer.cpp` | `alchemy_kernel` |
| `Kernel::CException::GetErrorMessage` | `Alchemy/Kernel/CException.cpp` | `alchemy_kernel` |
| `Kernel::CFileDirectory::*` | `Alchemy/Kernel/CFileDirectory.cpp` | `alchemy_kernel` |
| `CExtensionListMap::*` | `Mammoth/TSUI/CExtensionListMap.cpp` | `mammoth_tsui` |
| `quickhull::QuickHull<double>::*` | `Alchemy/Kernel/quickhull/QuickHull.cpp` | `alchemy_kernel` |

### 2. CGDraw / Filter / Fractal Implementation Gap

Representative missing symbols:

- `CGDraw::LineBroken`, `LineDotted`, `LineGradient`, `LineHD`, `LineBresenham`, `LineBresenhamTrans`
- `CGDraw::Circle`, `CircleImage`, `CircleGradient`, `CircleOutline`
- `CGDraw::RoundedRect`, `RoundedRectOutline`, `RoundedRectBottom`, `RectOutline`, `RectGradient`, `RectOutlineDotted`
- `CGDraw::Arc`, `ArcQuadrilateral`, `TriangleCorner`, `MaskRoundedRect`, `Region`, `Fill`, `ParseBlendMode`
- `CGFilter::Blur`, `CGFilter::Threshold`
- `CGFractal::*`
- `CGRunList::*`

Plan:

- Prefer compiling existing implementation files such as `DrawLine.cpp`, `DrawRect.cpp`, `DrawCircle.cpp`, `DrawFill.cpp`, `DrawRegion.cpp`, `BlendModes.cpp`, `FilterBlur.cpp`, `FilterThreshold.cpp`, `DrawClouds.cpp`, and related rasterizer/run-list files after fixing Clang template issues.
- Only create stubs when a function is not required for the current milestone path or when a full implementation would pull in Windows-only dependencies.
- Do not move this work to Metal yet; these are CPU/software drawing primitives used before frame presentation.

### 3. HUD and Gameplay UI Classes

Missing constructors include:

- `CShieldHUDDefault::CShieldHUDDefault()`
- `CWeaponHUDCircular::CWeaponHUDCircular()`
- `CReactorHUDCircular::CReactorHUDCircular()`

Plan:

- Locate and add the corresponding source files if present.
- If they are not present or depend on excluded Windows-only draw code, add minimal milestone stubs only after CGDraw coverage is improved.
- Treat these as first-playable blockers, not menu-only blockers, unless `IHUDPainter::Create` is linked into the menu path.

### 4. Audio Backend Still Windows-Coupled

Missing symbols are currently from `CMCIMixer::*`, referenced by `CSoundtrackManager`.

Plan:

- For the main-menu milestone, provide a macOS no-audio or SDL/AVFoundation-backed stub behind the same high-level `CSoundtrackManager` contract.
- For first playable/runtime parity, replace MCI behavior with a native backend that supports play, fade, pause/resume, current-track position, volume, and shutdown semantics.

### 5. Geometry / Utility Gaps

Missing symbols include several `CGeometry::*` helpers and `CAniSolidLine` vtable coverage.

Plan:

- Add the existing implementation files if present.
- If unavailable, classify each as menu, gameplay, or effects-only before writing code.

## Audited Path to Completion

### Phase A - Collapse Linker Noise

Goal: make final link failures small and meaningful.

1. Add present-but-omitted implementation files to CMake targets.
2. Build each owner target before relinking the app.
3. Re-run `transcendence_app` link and record the remaining unresolved groups.

Exit gate:

- no unresolved symbols remain from source files that already exist and compile cleanly.

### Phase B - Restore Software Drawing Coverage

Goal: compile enough CPU draw primitives for menu and first gameplay rendering.

1. Fix Clang template/friend declaration issues in excluded draw files.
2. Add draw/filter/fractal files incrementally to `alchemy_graphics`.
3. Keep DirectX presentation files excluded; include CPU raster/draw utilities only.

Exit gate:

- app link is no longer dominated by `CGDraw`, `CGFilter`, `CGFractal`, or `CGRunList` unresolved symbols.

### Phase C - Ship a Real SDL Shell

Goal: replace the Win32 message-loop path in the active macOS app.

1. Keep shared engine/session code free of SDL headers.
2. Replace or bypass `WinMain`, Win32 window creation, `WM_*` dispatch, and `PostMessage` command delivery.
3. Route SDL close, focus, resize, keyboard, mouse, wheel, text, and timer events into existing HI/session entry points.

Exit gate:

- the app opens a native SDL window, pumps events, and exits cleanly without the Win32 message loop.

### Phase D - Add Metal Compatibility Presenter

Goal: present the existing `CG32bitImage` software framebuffer in a native macOS window.

1. Implement the screen-manager/presenter seam used by `CHumanInterface`.
2. Present a deterministic test frame first.
3. Present the real loading/title/menu framebuffer next.
4. Validate pixel order, alpha, resize, and Retina coordinate mapping.

Exit gate:

- a real title/menu frame appears through Metal.

### Phase E - Make Main Menu Usable

Goal: turn visible menu into an operable menu.

1. Complete keyboard command mapping.
2. Separate SDL text input from command-key handling.
3. Complete mouse movement, click, wheel, and high-DPI coordinate handling.
4. Verify required menu fonts/images load from filesystem or bundle paths.

Exit gate:

- title/main menu is visible, readable, and operable with keyboard and mouse; one text-entry flow works.

### Phase F - First Playable

Goal: start a game and sustain a short gameplay loop.

1. Add gameplay-heavy UI/HUD sources only after menu is stable.
2. Resolve HUD, dock, map, and effects draw gaps.
3. Implement macOS save/settings paths before treating gameplay as complete.

Exit gate:

- user can start a new game, move/interact, view HUD/dock/map surfaces, and play briefly without critical crashes.

### Phase G - Native Runtime Parity

Goal: remove remaining debug-port shortcuts.

1. Replace the music/SFX backend with native macOS-compatible playback.
2. Ensure resources resolve from a `.app` bundle and from local debug runs.
3. Ensure saves/settings write to user-writable macOS locations.

Exit gate:

- save/load, resource lookup, SFX, and music work without Windows-only APIs.

### Phase H - Packaging and Stabilization

Goal: produce a stable native `.app`.

1. Add `.app` bundle target and package resources/assets.
2. Validate Finder launch, terminal launch, focus, minimize, resize, fullscreen, and repeated relaunch.
3. Profile before optimizing; keep the compatibility renderer until data proves a bottleneck.

Exit gate:

- bundled app launches outside the terminal and performs acceptably on Apple Silicon.

## Immediate Next Commands

After adding each missing implementation group, use this loop:

```sh
cmake --build --preset macos-debug --target alchemy_kernel
cmake --build --preset macos-debug --target alchemy_graphics
cmake --build --preset macos-debug --target mammoth_tsui
cmake --build --preset macos-debug --target transcendence_app
```

Use the first failing command as the active blocker. Do not broaden into SDL/Metal runtime work until the app link is reduced to platform/presenter/audio seams rather than missing existing source files.

## Current Risks

- The CMake source lists have grown beyond the original bounded menu-only scope; this helps expose real blockers but can pull gameplay/audio/effects dependencies into the menu link.
- `Kernel.h` still carries duplicated Win32 compatibility definitions (`INADDR_NONE`, `INVALID_HANDLE_VALUE`, `WINAPI`) that generate warnings and should eventually be cleaned behind a single portability boundary.
- Some `DWORD`/`int` pointer-storage assumptions still appear in warnings and may become runtime correctness bugs on arm64 even when they do not block compilation.
- Hardcoded Homebrew paths in `CMakeLists.txt` should be replaced with proper package discovery before the build is considered reproducible.

## Completion Definition

The macOS port is complete enough for the first native release when:

- `cmake --preset macos-debug` and release-oriented presets configure reproducibly
- app target builds and links without Windows SDK, DirectX, GDI, MCI, or Win32 message-loop dependencies in the active macOS path
- menu and first gameplay loop are usable
- resources, saves/settings, and audio behave natively
- `.app` bundle launches from Finder with packaged assets
- resize, focus, minimize, fullscreen, and a representative longer play session are stable
