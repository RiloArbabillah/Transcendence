# Release-Ready Execution Plan

## Document Status

- Version: v1.0
- Last Updated: 2026-05-01
- Scope: Native macOS Apple Silicon release-ready port
- Build Path: `CMake` (`macos-debug` / `macos-release`)

## Current Baseline

- `transcendence_app` builds and runs on macOS.
- SDL shell and Metal-backed presentation path are active.
- Loading-screen color pipeline is corrected.
- Remaining known visual issue: stargate loading animation still shows residual shadow artifact in user validation.
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

## Stage 1 - Visual Correctness (Boot/Menu)

### Goals

- Eliminate remaining loading visual artifact and verify color/alpha integrity.

### Tasks

- Finalize deterministic stargate blit path (no blend residue).
- Verify title/background/loading visuals against expected reference.
- Confirm no regressions in other menu images.

### Exit Gate

- No visible blocking artifacts in loading/title/menu visuals.

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

1. Close Stage 1 loading animation shadow artifact.
2. Re-validate menu visual parity with latest build.
3. Promote Stage 2 input validation checklist to active execution.
