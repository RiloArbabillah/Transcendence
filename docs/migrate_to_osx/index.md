# Documentation Index

## Document Status

- Version: v1.3
- Last Updated: 2026-05-11
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Purpose: central navigation portal for planning and implementation documentation

## Overview

This folder contains the planning and execution documents for porting `TranscendenceDev` to macOS Apple Silicon using `SDL2 + Metal`.

Use this file as the primary entry point for the documentation set.

`PRD.md` is the source of truth for product scope, goals, non-goals, success criteria, and milestone intent. All other planning documents should stay aligned with it.

## Recommended Reading Order

1. `../macOS_port_status.md`
2. `execution-task-plan.md`
3. `minimax-m27-port-completion-plan.md`
4. `release-ready-execution-plan.md`
5. `qa-test-matrix.md`
6. `architecture.md`
7. `dependency-matrix.md`
8. `task-backlog.md`
9. `cmake-build-plan.md`
10. `decision-log.md`
11. `change-log.md`

## Core Documents

### Product and Scope

- `PRD.md`
  - source of truth for product goals, scope, non-goals, success criteria, and delivery phases

- `roadmap.md`
  - milestone and release sequencing document derived from the PRD

- `task-backlog.md`
  - actionable engineering backlog with epics, task IDs, dependencies, and acceptance criteria

### Technical Planning

- `dependency-matrix.md`
  - inventory of major platform dependencies with portability classification and action type

- `architecture.md`
  - target subsystem boundaries, interface ownership rules, and dependency flow for the macOS port

- `cmake-build-plan.md`
  - proposed `CMake` target graph, build order, presets, framework links, and fallback strategy

- `execution-task-plan.md`
  - living task plan and investigation log for the current runtime-debugging sequence

- `minimax-m27-port-completion-plan.md`
  - ordered execution handoff for taking the current runnable baseline to release-candidate quality

- `release-ready-execution-plan.md`
  - higher-level release path covering gameplay, runtime parity, packaging, and QA gates

- `resource-loader-plan.md`
  - retained reference for the file-based loader seam that enabled title/menu asset loading

- `image-portability-seam.md`
  - retained reference for image-ingestion portability boundaries after resource lookup moved off Win32 resources

### Validation and Governance

- `qa-test-matrix.md`
  - minimum validation required per milestone, including manual checks and exit gates

- `decision-log.md`
  - recorded architecture and build decisions with rationale and implementation impact

- `change-log.md`
  - chronological log of documentation changes and major planning updates

## Document Roles

| Document | Primary Role |
|---|---|
| `PRD.md` | what we are building and why |
| `roadmap.md` | when and in what order we deliver |
| `task-backlog.md` | what to do next in actionable form |
| `dependency-matrix.md` | what is portable vs blocked by platform dependencies |
| `architecture.md` | how the system should be structured |
| `cmake-build-plan.md` | how the macOS build graph should be introduced |
| `resource-loader-plan.md` | how to replace milestone-1 Win32 resource loading with a file-based path |
| `image-portability-seam.md` | how to remove the remaining `HBITMAP` dependency from milestone-1 image ingestion |
| `execution-task-plan.md` | what is actively being debugged and what the next verified slice is |
| `minimax-m27-port-completion-plan.md` | how to execute the release-ready path from the current runnable baseline |
| `release-ready-execution-plan.md` | how to close the remaining gaps to a tester-usable `.app` |
| `qa-test-matrix.md` | how milestone success is validated |
| `decision-log.md` | why key technical decisions were made |
| `change-log.md` | what changed in the documentation set over time |

## Recommended Usage by Phase

### Before Coding

- read `PRD.md`
- read `architecture.md`
- read `execution-task-plan.md`
- read `cmake-build-plan.md`

### During Build-System Bring-Up

- use `task-backlog.md`
- use `dependency-matrix.md`
- use `cmake-build-plan.md`
- use `execution-task-plan.md` to track the live blocker and next verified slice
- update `decision-log.md` when a build or architecture choice is locked

### During Source Audit or Early Bring-Up

- use `execution-task-plan.md`
- cross-check assumptions against `dependency-matrix.md`
- use `minimax-m27-port-completion-plan.md` to keep work aligned with the audited release path

### During Resource Loader Implementation

- use `resource-loader-plan.md` for the minimum loader shape and implementation order
- keep `execution-task-plan.md` in view to avoid expanding scope beyond the current validated blocker slice

### During Image Portability Work

- use `resource-loader-plan.md` for the current lookup-layer status
- use `image-portability-seam.md` for the next seam after file-based lookup
- keep `execution-task-plan.md` nearby to confirm that the current blocker slice is still aligned with the active runtime-debugging priorities

### After Asset Path Stabilization

- use `execution-task-plan.md`
- use `minimax-m27-port-completion-plan.md`
- use `release-ready-execution-plan.md`
- keep `../macOS_port_status.md` open so runtime findings stay anchored to the current audited status

### During Milestone Validation

- use `qa-test-matrix.md`
- cross-check milestone expectations in `roadmap.md`

### When Scope or Direction Changes

- update `PRD.md` if product scope changes
- update `roadmap.md` if milestone ordering changes
- update `task-backlog.md` if execution sequencing changes
- record meaningful decision changes in `decision-log.md`
- record document changes in `change-log.md`

## Maintenance Rules

- `PRD.md` remains the top-level source of truth for scope and goals
- `roadmap.md` and `task-backlog.md` should stay aligned with `PRD.md`
- `decision-log.md` should capture non-trivial decisions before or when implementation lands
- `change-log.md` should record document additions, restructures, and major content changes

## Source of Truth

- `PRD.md` is the authoritative product document
- if a roadmap, backlog, or architecture detail conflicts with `PRD.md`, update the derived document or explicitly revise `PRD.md`
- derived documents should refine execution, not silently redefine scope

## Current Status Snapshot

- planning documents for architecture, execution, build setup, validation, and decision tracking exist
- root `CMakeLists.txt` and `CMakePresets.json` exist
- `cmake --preset macos-debug` configures successfully locally
- core and engine static library targets now build through `mammoth_tsui`
- `transcendence_app` builds and launches from the active CMake macOS path
- the current critical path is runtime stabilization in background universe init and first visible frame, not app-link closure
- use `../macOS_port_status.md` as the current audited status and completion plan when deciding the next implementation slice

## Related Files in This Folder

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
- `architecture.md`
- `cmake-build-plan.md`
- `resource-loader-plan.md`
- `image-portability-seam.md`
- `execution-task-plan.md`
- `minimax-m27-port-completion-plan.md`
- `release-ready-execution-plan.md`
- `qa-test-matrix.md`
- `decision-log.md`
- `change-log.md`
