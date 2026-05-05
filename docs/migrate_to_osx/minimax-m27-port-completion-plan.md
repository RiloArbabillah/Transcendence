# Minimax M2.7 macOS Port Completion Plan

## Document Status

- Version: v1.1
- Last Updated: 2026-05-04
- Owner: Akira
- Intended Executor: Minimax M2.7
- Scope: finish the native macOS Apple Silicon port from the current runnable baseline to a release-candidate-quality `.app`
- Build Path: `CMake` (`macos-debug`, `macos-release`)

## Purpose

This document is the execution handoff for Minimax M2.7.

The goal is to give the agent a concrete, ordered, verification-driven plan that starts from the current live port status, not from older bring-up assumptions. Earlier build/link and loading visual issues are no longer the active critical path.

## Current Source Of Truth

Use these documents in this order when context is needed:

1. `../macOS_port_status.md`
2. `minimax-m27-port-completion-plan.md`
3. `release-ready-execution-plan.md`
4. `qa-test-matrix.md`
5. `architecture.md`
6. `dependency-matrix.md`
7. `resource-loader-plan.md`
8. `image-portability-seam.md`

If another document conflicts with this plan or `../macOS_port_status.md`, prefer this plan and the newest user instruction. In particular, do not treat the previous stargate visual issue as an active blocker.

## Current Baseline

Assume the following are complete unless local validation proves otherwise:

- `transcendence_app` builds and runs on macOS from the CMake build tree.
- SDL shell is active.
- Compatibility presentation is active. The current safety path explicitly forces SDL's software renderer to avoid the unstable SDL/Metal texture upload path observed in runtime testing.
- `OnBoot` and `OnInit` are active in `GameUIBridge.cpp`.
- Resource path fixes are in place for loading/title/menu-critical assets.
- JPEG color channel issues for loading/title/background are corrected.
- Loading stargate shadow/trail visual artifact is accepted as done and is not a blocker.
- Source-list/link closure work and CPU draw/filter/fractal coverage are complete enough for the current runnable app baseline.
- `CMCIMixer` has a no-op macOS-compatible stub, so the build runs silently until real audio is implemented.
- SDL keyboard, mouse, wheel, and text input events are bridged into the `CHumanInterface` handler path.
- Mouse message packing includes both X and Y coordinates.
- Background task threads call `kernelInit()` before task execution.
- The earlier background `CCodeChain::Boot()` / `CString::GetPointer()` crash is fixed.
- The app now reaches the main loop, but the current user-visible state is still a black window and background universe/base-file initialization is not yet stable.

## Active Release Readiness Gaps

Treat these as the active blockers:

| Priority | Gap | Why it matters | Primary docs |
|---|---|---|---|
| P0 | Menu and input operability validation | A visible menu is not release-ready unless keyboard, mouse, wheel, text input, repeat behavior, and Retina mapping work | `qa-test-matrix.md` M4 |
| P0 | First playable stability | Release candidate needs New Game -> gameplay -> short stable loop | `qa-test-matrix.md` M5 |
| P0 | Black screen / no visible first frame | A runnable binary is not release-ready if title/menu UI never becomes visible | `qa-test-matrix.md` M3-M4, `../macOS_port_status.md` |
| P0 | Background base/embedded extension load failure | Universe initialization still fails while loading the base file and embedded extensions from `Transcendence.xml` | `../macOS_port_status.md` |
| P0 | macOS save/settings/resource path parity | The app must not depend on repo CWD or write into the bundle | `architecture.md`, `dependency-matrix.md` |
| P0 | Native audio backend | Current audio is stub-level; release readiness needs SFX and music | `release-ready-execution-plan.md`, `dependency-matrix.md` |
| P0 | `.app` packaging and Finder launch | The build must behave like a native app outside the terminal | `qa-test-matrix.md` M7 |
| P1 | Lifecycle and long-session QA | Resize, focus, minimize, fullscreen, repeated relaunch, and longer play need validation | `qa-test-matrix.md` M7 |
| P2 | Performance profiling | Optimize only after functional gates pass | `roadmap.md`, `qa-test-matrix.md` |

## Non-Blockers And Deferred Scope

Do not spend time on these unless the user explicitly changes scope:

- Loading stargate shadow/trail artifact.
- Old app-link closure tasks such as adding omitted implementation files already covered by the current runnable baseline.
- Full GPU-native Metal renderer rewrite.
- Restoring the Metal-present path before the current software-renderer safety path and background crash are stable.
- Steam support on macOS.
- Production cloud/Hexarc integration.
- Universal binary support.
- Notarization/App Store/distribution polish.
- Global warning cleanup.
- Mass migration of non-critical old image callers unless QA proves they are on the required path.

Deferred image callers unless proven necessary:

- `Transcendence/Transcendence/CHelpSession.cpp`
- `Transcendence/Transcendence/CStatsSession.cpp`
- `Transcendence/Transcendence/CModExchangeSession.cpp`

## Operating Rules For Minimax M2.7

- Make surgical changes; each changed line must map to an active release readiness gap.
- Preserve Windows build behavior and Visual Studio project ownership.
- Keep SDL, Metal, AppKit, and Objective-C++ out of shared engine headers.
- Keep software rendering as the frame-generation path; Metal presents the final framebuffer.
- Keep the SDL software renderer fallback acceptable while the Metal callback crash is triaged; do not re-enable an unstable Metal path only to satisfy stale wording.
- Prefer one small verified slice over broad refactors.
- For every runtime fix, run the smallest relevant build/run loop before moving on.
- If a failure appears, classify it before coding: input, renderer/presenter, resource path, save/settings, audio, gameplay, package, or lifecycle.
- Do not hide assumptions. If behavior cannot be validated automatically, record a manual observation and the exact flow used.

## Baseline Commands

Start every work session with the smallest command set that confirms the current state:

```sh
cmake --preset macos-debug
cmake --build "build" -j8
./build/Transcendence
```

If target presets are used locally:

```sh
cmake --build --preset macos-debug --target transcendence_app
```

For release/package validation later:

```sh
cmake --preset macos-release
cmake --build --preset macos-release --target transcendence_app
```

## Required Session Report Format

At the end of each implementation slice, record:

```text
Slice:
Files changed:
Build command:
Build result:
Runtime/manual test:
Observed result:
Remaining blockers:
Next recommended slice:
```

## Execution Overview

Follow this order exactly unless a preceding validation reveals a critical blocker:

1. Baseline lock.
2. Menu/input operability validation and fixes.
3. First playable gameplay validation and fixes.
4. macOS runtime path parity for resources, saves, and settings.
5. Native audio backend implementation.
6. `.app` bundle packaging and resource packaging.
7. Lifecycle, Retina, fullscreen, and long-session QA.
8. Release QA gate execution.
9. Performance profiling and targeted optimization only if needed.

## Stage 0 - Baseline Lock

### Goal

Confirm the current runnable baseline and prevent Minimax from chasing stale bring-up tasks.

### Tasks

1. Run the debug configure/build/run commands.
2. Confirm app opens a native macOS window.
3. Confirm loading/title/menu render enough to proceed.
4. Confirm the stargate visual issue is not tracked as a blocker.
5. Record current top blocker based on observed behavior.

### Do Not Fix In This Stage

- Visual polish already accepted as done.
- Audio silence.
- Packaging gaps.

### Exit Gate

- The agent has a current build/run result and a concrete next blocker in menu/input, gameplay, paths, audio, or packaging.

## Stage 1 - Menu And Input Operability

### Goal

Make the menu fully operable, not just visible.

### Relevant Areas

- SDL event bridge and platform shell files.
- `GameUIBridge.cpp` if it owns HI event delivery.
- `Transcendence/Transcendence/CGameKeys.cpp`
- `Transcendence/Transcendence/GameSessionInput.cpp`
- `Alchemy/DirectXUtil/CAniTextInput.cpp`
- Any platform window code that maps logical size to drawable size.

### Validation Checklist

Keyboard:

- Arrow or expected navigation keys move menu selection.
- Confirm/enter activates selected item.
- Escape/back/cancel works where expected.
- Key repeat does not skip unpredictably or flood the UI.
- Modifiers do not trigger text input by mistake.

Mouse:

- Hover state updates correctly.
- Click activates the intended menu item.
- Wheel works in list/settings flows if present.
- Mouse coordinates remain accurate after resize.
- Mouse coordinates remain accurate on Retina/high-DPI displays.

Text input:

- At least one text-entry flow accepts typed characters.
- Backspace/delete works.
- Text is not double-inserted from both key and text events.
- Command/navigation keys do not inject text.

### Implementation Strategy

1. First validate and document failures without editing.
2. If keyboard command mapping fails, inspect SDL key-to-engine key translation before touching gameplay code.
3. If text input double-inserts, separate SDL text input from command-key handling.
4. If mouse hit testing is wrong, fix logical-to-drawable coordinate conversion in the platform/input boundary.
5. Re-run the same manual flow after each small fix.

### Current Audit Fix Plan

Apply these fixes before marking Stage 1 complete. They are based on the current SDL bridge audit after the Minimax input/runtime changes.

1. Restore Win32-style message packing in the macOS SDL bridge.
   - In `AppCore.cpp`, post mouse messages with `wParam = button/key flags` and `lParam = MAKELONG(x, y)`.
   - In `GameUIBridge.cpp`, decode mouse coordinates from `lParam` the same way `Run.cpp` does.
   - Do not keep a macOS-only convention where coordinates are stored in `wParam`; it makes `PeekMessage`/Win32-compat paths inconsistent.
2. Route each mouse message to the matching `CHumanInterface` handler.
   - `WM_LBUTTONDOWN` / `WM_LBUTTONUP` -> `WMLButtonDown` / `WMLButtonUp`.
   - `WM_RBUTTONDOWN` / `WM_RBUTTONUP` -> `WMRButtonDown` / `WMRButtonUp`.
   - `WM_MBUTTONDOWN` / `WM_MBUTTONUP` -> `WMMButtonDown` / `WMMButtonUp`.
   - `WM_MOUSEWHEEL` -> `WMMouseWheel` with the wheel delta and coordinates decoded like the Windows path.
   - Expose only the needed handlers in `TSUI.h`; avoid broad public surface changes.
3. Fix SDL mouse button mapping.
   - Use `SDL_BUTTON_RIGHT` and `SDL_BUTTON_MIDDLE` instead of hardcoded `2`/`3` assumptions.
   - SDL uses middle as `2` and right as `3`; the current hardcoded mapping swaps them.
4. Keep coordinate conversion consistent with Windows behavior.
   - In `CHumanInterfaceMac.cpp`, call `m_ScreenMgr.ClientToLocal` before `HIMouseMove`, just as `Run.cpp` does.
   - Verify hover and click after resize and on Retina/high-DPI displays.
5. Handle resize and move messages.
   - If `AppCore.cpp` posts `WM_SIZE` and `WM_MOVE`, `GameUIBridge.cpp` must dispatch them to `WMSize` and `WMMove`.
   - If the macOS path does not need these yet, do not post them until the dispatch path is ready.
6. Tighten keyboard mapping before relying on gameplay/menu hotkeys.
   - Map `A-Z`, `0-9`, keypad, plus/minus, and other game-used keys to Win32 virtual-key values.
   - Do not use raw SDL scancode as a generic virtual-key fallback unless it is known to match the engine expectation.
7. Treat text input as ASCII-only unless the engine path is widened.
   - SDL text events are UTF-8; posting one `WM_CHAR` per byte is only safe for ASCII.
   - If non-ASCII input is out of scope, document the limitation and filter to supported characters.

Verification after these fixes:

- Build `transcendence_app` successfully.
- Confirm keyboard navigation, enter/escape, and at least one hotkey using letter or number input.
- Confirm hover, left click, right click if a flow uses it, middle click if a flow uses it, and wheel scrolling.
- Confirm pointer hit testing still works after resize and on Retina/high-DPI display.
- Confirm one text-entry flow does not double-insert characters and does not inject command/navigation keys.

### Exit Gate

- Main menu and related menu/settings flows are operable by keyboard and mouse.
- One representative text-entry flow works.
- M4 required checks in `qa-test-matrix.md` pass or have explicit non-blocking exceptions.

## Stage 2 - First Playable Stability

### Goal

Start a new game and sustain a short gameplay loop.

### Relevant Areas

- `Transcendence/Transcendence/CIntroSession.cpp`
- `Transcendence/Transcendence/IntroScreen.cpp`
- `Transcendence/Transcendence/CTranscendenceController.cpp`
- `Transcendence/Transcendence/CTranscendenceModel.cpp`
- gameplay input bridge
- HUD/dock/map rendering files pulled into the current app target

### Validation Checklist

- Start New Game from the menu.
- Reach in-game state without fatal startup errors.
- Basic movement or ship control works.
- HUD is visible and readable.
- Dock UI opens and responds if reachable.
- Map or a representative overlay opens if reachable.
- Pause/help/menu interaction used during gameplay does not crash.
- Repeat launch -> new game -> play briefly -> quit at least three times.
- Run one 20-30 minute basic play session if practical.

### Implementation Strategy

1. Treat first crash as the active blocker.
2. Classify the crash before editing:
   - missing resource,
   - input mapping,
   - HUD/dock/map missing source or draw primitive,
   - save/settings path,
   - audio stub assumption,
   - arm64 pointer/int issue,
   - renderer/presenter issue.
3. Fix the narrowest active path.
4. Do not broaden gameplay systems beyond the flow needed for first playable.

### Exit Gate

- User can start a new game and complete a short stable gameplay loop.
- M5 required checks in `qa-test-matrix.md` pass or have explicit non-blocking exceptions.

## Stage 3 - macOS Runtime Paths: Resources, Saves, Settings

### Goal

Make runtime path behavior native and packaging-safe.

### Required Policy

- Bundle/read-only resources resolve from `.app/Contents/Resources` when bundled.
- Debug builds may fall back to repo paths, but must not require a specific current working directory.
- Saves are written to a user-writable macOS location such as `~/Library/Application Support/Transcendence/`.
- Settings are written to `~/Library/Application Support/Transcendence/` or a consistent preferences location.
- Runtime writes must never target the `.app` bundle.

### Relevant Areas

- `Transcendence/Transcendence/CResourcePathResolver.cpp`
- `Transcendence/Transcendence/CGameSettings.cpp`
- `Transcendence/Transcendence/GameSettings.h`
- `Mammoth/TSUI/CUserSettings.cpp`
- `Mammoth/TSUI/CListSaveFilesTask.cpp`
- `Mammoth/TSE/CGameFile.cpp`
- macOS/platform path helper code

### Validation Checklist

- Launch from repo root.
- Launch from a different working directory.
- Launch from bundled layout once Stage 5 exists.
- Resources resolve in all supported launch modes.
- Create a save file.
- Load the save file after relaunch.
- Change a setting and verify it persists after relaunch.
- Confirm no runtime-created files appear inside the `.app` bundle.

### Implementation Strategy

1. Audit current path resolution before editing.
2. Add or tighten a platform-owned path service if path logic is scattered.
3. Keep resource root and writable roots separate.
4. Preserve debug fallback for developer convenience.
5. Make failures diagnosable with concise logging of selected roots.

### Exit Gate

- Save/load/settings/resource behavior works in debug run and is ready for bundled app validation.
- M6 path-related checks in `qa-test-matrix.md` pass or have explicit non-blocking exceptions.

## Stage 4 - Native Audio Backend

### Goal

Replace silent stub behavior with native-compatible SFX and music playback.

### Current State

- `CMCIMixer` has a stub implementation for macOS build/run safety.
- Audio parity is not release-ready.
- Windows MCI/DirectSound must not return to the active macOS path.

### Relevant Areas

- `Mammoth/TSUI/CMCIMixerStub.cpp`
- `Mammoth/TSUI/CMCIMixerStub.h`
- `Mammoth/TSUI/CSoundtrackManager.cpp`
- `Mammoth/Include/Soundtrack.h`
- `Alchemy/DirectXUtil/Sound.cpp`

### Backend Recommendation

Choose the smallest backend that satisfies release-readiness requirements:

- Prefer SDL audio if it can support the shipped formats and expected SFX overlap quickly.
- Use AVFoundation/AudioToolbox only if SDL audio is unsuitable for the actual asset formats.
- Keep the high-level soundtrack manager behavior intact where possible.

### Audit Checklist

- Identify music file formats actually shipped/loaded.
- Identify SFX file formats actually shipped/loaded.
- Determine whether SFX overlap is required.
- Determine whether looped effects are required.
- Determine required music operations: play, stop, pause, resume, transition/fade if already expected.
- Determine volume/mute settings integration.

### Implementation Strategy

1. Preserve the stub as a fallback or development option.
2. Implement minimal SFX playback first if smaller.
3. Implement music playback second.
4. Wire pause/resume/stop into existing soundtrack manager calls.
5. Validate menu and gameplay audio separately.

### Validation Checklist

- Menu UI sounds play if present.
- Gameplay SFX play.
- Overlapping SFX do not crash or cut off incorrectly.
- Music starts and stops.
- Pause/resume behavior is acceptable.
- Volume/mute setting behavior is acceptable.
- No MCI/DirectSound dependency is active in macOS build.

### Exit Gate

- SFX and music work natively enough for release candidate QA.
- M6 audio checks in `qa-test-matrix.md` pass or have explicit non-blocking exceptions.

## Stage 5 - `.app` Bundle And Resource Packaging

### Goal

Produce a `.app` that launches from Finder and does not depend on the repo or terminal working directory.

### Relevant Areas

- root `CMakeLists.txt`
- `CMakePresets.json`
- CMake resource-copy/package rules
- `CResourcePathResolver`
- platform runtime path helpers
- SDL dylib/framework bundling if SDL is dynamically linked

### Minimum Bundle Metadata

- `CFBundleName`
- `CFBundleIdentifier`
- `CFBundleExecutable`
- `CFBundlePackageType`
- arm64 architecture setting
- macOS deployment target consistent with CMake presets

### Packaging Checklist

- Executable is inside `.app/Contents/MacOS/`.
- Required game resources are inside `.app/Contents/Resources/`.
- SDL runtime dependency is resolved for Finder launch.
- Any Metal shader or renderer support file needed at runtime is packaged.
- Save/settings paths still point to user-writable locations.

### Minimum Menu Resource Subset

Package at least:

- `Transcendence/Transcendence/Resources/Title.JPG`
- `Transcendence/Transcendence/Resources/Stargate.JPG`
- `Transcendence/Transcendence/Resources/StargateMask.BMP`
- `Transcendence/Transcendence/Resources/GameButtonIcons.jpg`
- `Transcendence/Transcendence/Resources/UIIcons.jpg`
- `Transcendence/Transcendence/Resources/UIIconsMask.bmp`
- `Transcendence/Transcendence/Resources/Header.dxfn`
- `Transcendence/Transcendence/Resources/HeaderBold.dxfn`
- `Transcendence/Transcendence/Resources/SubTitle.dxfn`
- `Transcendence/Transcendence/Resources/SubTitleBold.dxfn`
- `Transcendence/Transcendence/Resources/SubTitleHeavyBold.dxfn`
- `Transcendence/Transcendence/Resources/Title.dxfn`
- `Transcendence/Transcendence/Resources/LogoTitle.dxfn`

For first-playable packaging, package all runtime game data and assets required by New Game and gameplay, not only the menu subset.

### Validation Checklist

- Launch from Finder.
- Launch from terminal using the app path while current directory is outside the repo.
- Relaunch after quit.
- Resource loading works in bundled layout.
- Save/load/settings still work.
- No terminal-only environment variable is required.

### Exit Gate

- `.app` can be handed to a tester and launched without a dev setup.
- M7 packaging checks in `qa-test-matrix.md` pass or have explicit non-blocking exceptions.

## Stage 6 - Lifecycle, Retina, Fullscreen, And Stability

### Goal

Validate native desktop behavior and close common macOS lifecycle gaps.

### Validation Checklist

- Resize repeatedly while menu is visible.
- Resize repeatedly during gameplay.
- Minimize and restore.
- Focus loss and regain.
- Fullscreen toggle if implemented.
- Mouse hit testing after resize/fullscreen.
- Repeated launch/play/quit cycles.
- One longer representative session.

### Implementation Strategy

1. If resize breaks rendering, inspect presenter drawable/logical size handling.
2. If mouse hit testing breaks after resize, inspect coordinate mapping before UI code.
3. If focus/minimize crashes, inspect event delivery and paused/update state transitions.
4. If fullscreen is unstable and not required, document limitation rather than destabilizing the port late.

### Exit Gate

- Lifecycle behavior is stable enough for release candidate testing.
- Any known limitation is explicit and non-critical.

## Stage 7 - QA Gate And Release Candidate

### Goal

Run milestone-required QA and produce a release-candidate status.

### Required Gates

Run required checks from `qa-test-matrix.md`:

- M2: SDL native shell.
- M3: Metal compatibility presenter.
- M4: main menu usable.
- M5: first playable gameplay.
- M6: native runtime parity.
- M7: packaging and performance.

### Failure Report Format

Use this exact structure for failures:

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

### Exit Gate

- All required gates pass or have explicit non-blocking exceptions.
- Remaining known issues are triaged by severity and area.

## Stage 8 - Performance Profiling And Targeted Optimization

### Goal

Improve performance only where measured data shows a real bottleneck.

### Timing

Do not start this stage until menu, first playable, paths, audio, packaging, and lifecycle gates are functionally acceptable.

### Profiling Targets

- CPU software rendering cost.
- Framebuffer upload cost.
- Frame pacing and timer cadence.
- Hot blit/draw paths during menu and gameplay.
- Resource loading stalls during startup or gameplay transition.

### Rules

- Record before/after measurement.
- Keep compatibility renderer unless data proves it is the bottleneck.
- Do not rewrite broad draw systems without a measured reason.

### Exit Gate

- Performance is acceptable on Apple Silicon, or remaining performance limitations are documented as non-blocking.

## Final Definition Of Done

The macOS Apple Silicon port is release-candidate ready when:

- `macos-debug` and `macos-release` CMake builds are reproducible.
- App launches from Finder as a `.app`.
- App also launches from a terminal outside the repo root.
- Menu is usable with keyboard, mouse, wheel where applicable, and at least one text-entry flow.
- New Game reaches gameplay and a short gameplay loop is stable.
- Save/load/settings use macOS user-writable paths.
- Resources resolve from the bundle and from debug fallback paths.
- SFX and music work without MCI/DirectSound.
- Resize, focus, minimize, and fullscreen behavior are stable or explicitly documented as limited.
- M2-M7 required QA gates pass or have approved non-blocking exceptions.
- SDL, Metal, AppKit, and Objective-C++ remain isolated behind platform/backend implementation boundaries.

## First Task For Minimax M2.7

Start with Stage 0 and Stage 1:

1. Confirm debug build/run baseline.
2. Skip stargate as a blocker.
3. Execute the M4 menu/input checklist.
4. Fix only the first observed input/menu blocker.
5. Report the slice using the required session report format.
