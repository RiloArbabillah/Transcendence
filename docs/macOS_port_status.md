# Transcendence macOS Port - Current Status

## Overview

This document records the current state of the native macOS Apple Silicon port and the audited path needed to finish it. The port strategy remains compatibility-first: keep existing engine/gameplay logic, keep software frame generation, replace Win32 shell/presentation/resource/audio boundaries with native macOS equivalents.

## Build Status

**Current State:** the macOS app target builds and launches from the CMake-generated build tree. The active runtime path uses the SDL shell, software frame generation, SDL event forwarding, and a compatibility SDL software renderer while the Metal callback crash is avoided.

**Validated Locally:** 2026-05-04

```sh
xcodebuild -project build/TranscendenceMacOS.xcodeproj -scheme transcendence_app -configuration Debug build
```

Result:

- generated Xcode project builds successfully
- executable is produced at `build/Debug/Transcendence`
- app launches far enough to exercise loading/menu/input paths
- current crash investigation is runtime-only, not a link/build blocker

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
- **2026-04-30**: Phase A complete - 10 present-but-omitted source files added to CMakeLists.txt:
  - `CDictionary.cpp`, `CAtomizer.cpp`, `CException.cpp`, `CFileDirectory.cpp`, `quickhull/QuickHull.cpp` → `alchemy_kernel`
  - `CIconLabelBlock.cpp`, `CNoiseGenerator.cpp`, `AGArea.cpp`, `AGScreen.cpp` → `alchemy_graphics`
  - `CExtensionListMap.cpp` → `mammoth_tsui` (also fixed missing `TSUISettings.h` include)

## Current Status

**Build/runtime baseline (2026-05-04):**

- `transcendence_app` builds and runs on macOS from CMake build tree.
- SDL shell is active.
- Software frame generation remains the compatibility renderer.
- SDL software renderer + vsync is active to avoid the prior Metal thread callback crash.
- OnBoot/OnInit path is active in `GameUIBridge.cpp` (no longer skipped).
- Resource path and JPEG color channel issues are corrected for loading background/title.
- SDL keyboard, mouse, wheel, and text input events are now bridged into the `CHumanInterface` Win32-style handlers.
- Mouse coordinate packing now sends both X and Y through the message bridge instead of losing Y.
- macOS `CRITICAL_SECTION` compatibility now uses recursive `pthread_mutex_t` instead of a no-op/std::mutex mismatch.
- Background task threads now call `kernelInit()` before executing `IHITask` work.

**Known active blocker for release readiness:**

- First playable stability is blocked by a runtime crash on a background task thread during `CCodeChain::Boot()` / `CString::GetPointer()`.
- The added background-thread `kernelInit()` call is a mitigation attempt, but does not yet prove the crash is fixed.
- The older loading stargate shadow/trail artifact is no longer treated as an active blocker unless user validation reopens it.
- Menu/input bridge code exists, but M4 still needs a complete manual validation pass for keyboard, mouse, wheel, text input, and Retina behavior.

**Release readiness gaps still open:**

- Native audio parity (current implementation is still stub-level behavior).
- Save/settings/resource path parity validation for packaged `.app` runtime.
- Finder-launch packaging and full milestone QA gate execution.

## Linker Blocker Clusters

### 1. RESOLVED - Source Files Present but Not Linked ✅

These have been added to CMakeLists.txt:

| Missing symbols | Source file | Target owner | Status |
|---|---|---|---|
| `CIconLabelBlock::*` | `Alchemy/DirectXUtil/CIconLabelBlock.cpp` | `alchemy_graphics` | ✅ Added |
| `CNoiseGenerator::*` | `Alchemy/DirectXUtil/CNoiseGenerator.cpp` | `alchemy_graphics` | ✅ Added |
| `AGArea::*` | `Alchemy/DirectXUtil/AGArea.cpp` | `alchemy_graphics` | ✅ Added |
| `AGScreen::*` | `Alchemy/DirectXUtil/AGScreen.cpp` | `alchemy_graphics` | ✅ Added |
| `Kernel::CDictionary::*` | `Alchemy/Kernel/CDictionary.cpp` | `alchemy_kernel` | ✅ Added |
| `Kernel::CAtomizer::*` | `Alchemy/Kernel/CAtomizer.cpp` | `alchemy_kernel` | ✅ Added |
| `Kernel::CException::GetErrorMessage` | `Alchemy/Kernel/CException.cpp` | `alchemy_kernel` | ✅ Added |
| `Kernel::CFileDirectory::*` | `Alchemy/Kernel/CFileDirectory.cpp` | `alchemy_kernel` | ✅ Added |
| `CExtensionListMap::*` | `Mammoth/TSUI/CExtensionListMap.cpp` | `mammoth_tsui` | ✅ Added |
| `quickhull::QuickHull<double>::*` | `Alchemy/Kernel/quickhull/QuickHull.cpp` | `alchemy_kernel` | ✅ Added |

### 2. RESOLVED - CGDraw / Filter / Fractal Implementation Gap ✅

All drawing files now compile and link successfully:
- `DrawLine.cpp`, `DrawRect.cpp`, `DrawCircle.cpp`, `DrawFill.cpp`
- `DrawRegion.cpp`, `BlendModes.cpp`, `FilterBlur.cpp`, `FilterThreshold.cpp`
- `DrawClouds.cpp`, `8bitNoise.cpp`, `16bitDrawGradient.cpp`

### 3. HUD and Gameplay UI Classes

Missing constructors include:

- `CShieldHUDDefault::CShieldHUDDefault()`
- `CWeaponHUDCircular::CWeaponHUDCircular()`
- `CReactorHUDCircular::CReactorHUDCircular()`

Plan:

- Locate and add the corresponding source files if present.
- If they are not present or depend on excluded Windows-only draw code, add minimal milestone stubs only after CGDraw coverage is improved.
- Treat these as first-playable blockers, not menu-only blockers, unless `IHUDPainter::Create` is linked into the menu path.

### 4. RESOLVED - Audio Backend Windows-Coupled ✅

Created stub implementation:
- `Mammoth/TSUI/CMCIMixerStub.cpp` and `CMCIMixerStub.h`
- All CMCIMixer methods have no-op implementations
- Build succeeds; audio silent until proper backend added

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

### Phase B - Restore Software Drawing Coverage ✅

Goal: compile enough CPU draw primitives for menu and first gameplay rendering.

1. Fix Clang template/friend declaration issues in excluded draw files.
2. Add draw/filter/fractal files incrementally to `alchemy_graphics`.
3. Keep DirectX presentation files excluded; include CPU raster/draw utilities only.

**Status: COMPLETE (2026-04-30)**

Fixed:
- `TRegionPainter.h:58` - Changed `private: friend TRegionPainter32` → `public:` for `GetPixelAt` method
- `TLinePainter.h:55` - Changed `private:` → `public:` for `GetPixel` in `TLinePainterSolid`

Added to `alchemy_graphics`:
- `DrawLine.cpp`, `DrawRect.cpp`, `DrawCircle.cpp`, `DrawFill.cpp`
- `DrawRegion.cpp`, `BlendModes.cpp`, `FilterBlur.cpp`, `FilterThreshold.cpp`
- `DrawClouds.cpp`, `8bitNoise.cpp`, `16bitDrawGradient.cpp`

Exit gate: ✅ app link is no longer dominated by `CGDraw`, `CGFilter`, `CGFractal`, or `CGRunList` unresolved symbols.

### Phase G - Audio Backend Stub ✅

Goal: provide macOS-compatible audio stub for CSoundtrackManager.

**Status: COMPLETE (stub only - 2026-04-30)**

- Created `Mammoth/TSUI/CMCIMixerStub.cpp` and `CMCIMixerStub.h`
- Stub provides minimal no-op implementations of all CMCIMixer methods
- Audio playback deferred to Phase F+ when real backend is needed
- Build succeeds; runtime audio will be silent until proper backend added

### Phase C - Ship a Real SDL Shell ✅

Goal: replace the Win32 message-loop path in the active macOS app.

1. Keep shared engine/session code free of SDL headers.
2. Replace or bypass `WinMain`, Win32 window creation, `WM_*` dispatch, and `PostMessage` command delivery.
3. Route SDL close, focus, resize, keyboard, mouse, wheel, text, and timer events into existing HI/session entry points.

**Status: COMPLETE (2026-04-30)**

- `Main.cpp` → `App_Run()` → SDL main loop
- `App_PumpEvents()` - SDL event pumping
- `App_PresentFrameBuffer()` - SDL_RenderPresent with texture upload
- `App_Init()` - SDL_Window + SDL_Renderer + texture creation
- `InitGameUI()` / `UpdateGameUI()` connected to main loop

Exit gate: ✅ the app opens a native SDL window, pumps events, and exits cleanly without the Win32 message loop.

Note: Runtime crash in HIBoot due to resource/path loading is a Phase E issue (menu usability), not Phase C.

### Phase D - Add Metal Compatibility Presenter 🚧

Goal: present the existing `CG32bitImage` software framebuffer in a native macOS window.

1. Implement the screen-manager/presenter seam used by `CHumanInterface`.
2. Present a deterministic test frame first.
3. Present the real loading/title/menu framebuffer next.
4. Validate pixel order, alpha, resize, and Retina coordinate mapping.

**Status: INFRASTRUCTURE COMPLETE (2026-04-30)**

- Added `MetalRenderer.cpp/h` with C API for Metal layer access
- App_Init() now uses `SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal")`
- SDL_CreateRenderer configured with `SDL_RENDERER_METAL` hint
- Added `SDL_RenderGetMetalLayer()` verification after renderer creation
- `App_PresentFrameBuffer()` uses SDL_RenderPresent which routes through CAMetalLayer

Exit gate: a real title/menu frame appears through Metal.

Note: Frame rendering is blocked by HIBoot crash (Phase E territory). SDL-Metal bridge is in place.

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

Use these as current execution baseline:

```sh
cmake --build "build" -j8
./build/Transcendence
```

### Next Focus (Release-Ready Path)

1. Investigate and fix the background-thread `CString::GetPointer()` crash in the `CCodeChain::Boot()` path.
2. Execute the M4 menu/input validation checklist against the new SDL event bridge.
3. Validate first playable flow after the runtime crash is resolved.
4. Implement native audio backend parity and verify soundtrack/SFX behavior.
5. Validate save/settings/resource paths in both repo-run and bundled `.app` run.
6. Complete packaging + Finder launch and run M2-M7 required QA gates.

## Current Risks

- The CMake source lists have grown beyond the original bounded menu-only scope; this helps expose real blockers but can pull gameplay/audio/effects dependencies into the menu link.
- `Kernel.h` still carries duplicated Win32 compatibility definitions (`INADDR_NONE`, `INVALID_HANDLE_VALUE`, `WINAPI`) that generate warnings and should eventually be cleaned behind a single portability boundary.
- Some `DWORD`/`int` pointer-storage assumptions still appear in warnings and may become runtime correctness bugs on arm64 even when they do not block compilation.
- `CSymbolTable`/`CDictionary` paths still store and compare `CString *` keys through integer slots; this is a current suspect for the background `CString::GetPointer()` crash on arm64.
- Hardcoded Homebrew paths in `CMakeLists.txt` should be replaced with proper package discovery before the build is considered reproducible.
- **2026-04-30**: Phase A resolved. Remaining blockers: Phase B (CGDraw/CGFilter/CGFractal), Phase G (CMCIMixer/audio), and CGeometry/CGRunList gaps.

## Completion Definition

The macOS port is complete enough for the first native release when:

- `cmake --preset macos-debug` and release-oriented presets configure reproducibly
- app target builds and links without Windows SDK, DirectX, GDI, MCI, or Win32 message-loop dependencies in the active macOS path
- menu and first gameplay loop are usable
- resources, saves/settings, and audio behave natively
- `.app` bundle launches from Finder with packaged assets
- resize, focus, minimize, fullscreen, and a representative longer play session are stable
