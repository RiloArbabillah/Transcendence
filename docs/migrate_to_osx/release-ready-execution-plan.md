# Release-Ready Execution Plan

## Document Status

- Version: v1.1
- Last Updated: 2026-05-04
- Scope: Native macOS Apple Silicon release-ready port
- Build Path: `CMake` (`macos-debug` / `macos-release`)

## Current Baseline

- `transcendence_app` builds and runs on macOS.
- SDL shell is active.
- The current renderer safety path uses SDL's software renderer plus vsync; the previous Metal-backed path hit a thread callback crash and should not be treated as the active release path until stabilized.
- Loading-screen color pipeline is corrected.
- SDL keyboard, mouse, wheel, and text input events are bridged into the `CHumanInterface` path, but M4 still needs complete manual validation.
- Current first-playable blocker is a background-thread crash during `CCodeChain::Boot()` / `CString::GetPointer()`.
- Audio backend is still compatibility/stub level and not release-ready.
- Packaging and Finder launch validation are not complete.

## Release-Ready Definition

Release-ready means all items below are true:

1. `.app` launches from Finder without terminal dependencies.
2. Menu and gameplay entry are stable (no critical crash in short play loop).
3. Save/load and settings paths use macOS user-writable locations.
4. Audio (SFX + music) works via native-compatible backend.
5. Required checks in `qa-test-matrix.md` for M2-M7 pass.

## Execution Stages

## Stage 0 - Baseline Lock

### Goals

- Freeze current status and blockers so implementation and QA use the same truth.

### Tasks

- Record current build commands and expected outputs.
- Record known blockers and classify as P0/P1.
- Keep this file updated as source of execution truth.

### Exit Gate

- A reproducible baseline is documented and shared.

## Stage 1 - Runtime Crash Triage

### Goals

- Resolve the background-thread `CString::GetPointer()` crash before treating first playable validation as meaningful.

### Tasks

- Instrument or inspect `CCodeChain::Boot()`, `CSymbolTable`, and `CDictionary` pointer-key handling as needed.
- Confirm whether background-thread `kernelInit()` is sufficient, unnecessary, or only a partial mitigation.
- Rebuild and reproduce under LLDB until the active crash is classified and fixed.

### Exit Gate

- New Game or the current first-playable startup path no longer crashes in CodeChain boot.

## Stage 2 - Input and Menu Operability

### Goals

- Ensure keyboard, mouse, and text input are fully usable in menu flows.

### Tasks

- Validate key navigation, confirm/cancel flows, repeat behavior.
- Validate mouse hover/click/wheel with Retina scaling.
- Validate at least one text-entry flow.

### Exit Gate

- Menu and settings flows are fully operable without input mapping issues.

## Stage 3 - First Playable Stability

### Goals

- Start game and sustain short gameplay loop safely.

### Tasks

- Validate New Game -> gameplay -> HUD/dock/map.
- Resolve runtime crashes in direct run (non-lldb) path.
- Run repeated launch/play/quit cycles.

### Exit Gate

- 20-30 minute basic play session completes without critical crash.

## Stage 4 - Native Runtime Parity

### Goals

- Replace remaining Windows-centric runtime behavior.

### Tasks

- Move save/settings to macOS Application Support/Preferences locations.
- Validate resource resolution in debug run and bundled app layout.
- Replace audio stub with native-compatible backend path.

### Exit Gate

- Save/load/settings/audio are functional in macOS runtime.

## Stage 5 - Packaging and Distribution Readiness

### Goals

- Produce and validate runnable `.app` bundle.

### Tasks

- Add/finish bundle target and resource packaging.
- Validate Finder launch, relaunch, focus, minimize, resize, fullscreen.
- Validate app behavior outside repo root and terminal shell.

### Exit Gate

- `.app` can be handed to tester and run without dev setup.

## Stage 6 - QA Gate and Release Candidate

### Goals

- Pass milestone-required QA and cut release candidate.

### Tasks

- Execute required checks from `qa-test-matrix.md` M2-M7.
- Record pass/fail evidence and known-issue triage.
- Build release candidate and publish release notes.

### Exit Gate

- All required gates pass or have approved non-blocking exceptions.

## Immediate Next Slice (In Progress)

1. Fix or conclusively classify the background CodeChain/CString crash.
2. Re-validate the M4 menu/input checklist with the SDL bridge changes.
3. Re-run first playable validation after the runtime crash is resolved.
