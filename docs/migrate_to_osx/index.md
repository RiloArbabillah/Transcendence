# Documentation Index

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Purpose: central navigation portal for planning and implementation documentation

## Overview

This folder contains the planning and execution documents for porting `TranscendenceDev` to macOS Apple Silicon using `SDL2 + Metal`.

Use this file as the primary entry point for the documentation set.

`PRD.md` is the source of truth for product scope, goals, non-goals, success criteria, and milestone intent. All other planning documents should stay aligned with it.

## Recommended Reading Order

1. `PRD.md`
2. `roadmap.md`
3. `task-backlog.md`
4. `dependency-matrix.md`
5. `architecture.md`
6. `milestone-1-plan.md`
7. `cmake-build-plan.md`
8. `qa-test-matrix.md`
9. `decision-log.md`
10. `change-log.md`

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

- `milestone-1-plan.md`
  - implementation plan for the first executable goal: native title or main menu bring-up

- `cmake-build-plan.md`
  - proposed `CMake` target graph, build order, presets, framework links, and fallback strategy

- `source-audit-handoff.md`
  - source-level handoff capturing actual boot path, runtime blockers, host contract findings, and font/resource pipeline risks discovered during code audit

- `resource-loader-plan.md`
  - implementation plan for replacing Win32 resource loading with a minimal file-based loader for milestone-1 fonts and UI assets

- `image-portability-seam.md`
  - next-phase design for removing `HBITMAP` from the milestone-1 image path after resource lookup has been made file-based

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
| `milestone-1-plan.md` | how to execute the first milestone without scope creep |
| `cmake-build-plan.md` | how the macOS build graph should be introduced |
| `source-audit-handoff.md` | what the current source tree actually does and where milestone-1 blockers are |
| `resource-loader-plan.md` | how to replace milestone-1 Win32 resource loading with a file-based path |
| `image-portability-seam.md` | how to remove the remaining `HBITMAP` dependency from milestone-1 image ingestion |
| `qa-test-matrix.md` | how milestone success is validated |
| `decision-log.md` | why key technical decisions were made |
| `change-log.md` | what changed in the documentation set over time |

## Recommended Usage by Phase

### Before Coding

- read `PRD.md`
- read `architecture.md`
- read `milestone-1-plan.md`
- read `cmake-build-plan.md`

### During Build-System Bring-Up

- use `task-backlog.md`
- use `dependency-matrix.md`
- use `cmake-build-plan.md`
- use `source-audit-handoff.md` to cross-check the real code path and blocker files
- update `decision-log.md` when a build or architecture choice is locked

### During Source Audit or Early Bring-Up

- use `source-audit-handoff.md`
- cross-check assumptions against `dependency-matrix.md`
- use `milestone-1-plan.md` to keep early work focused on menu bring-up

### During Resource Loader Implementation

- use `source-audit-handoff.md` for the real asset and call-site inventory
- use `resource-loader-plan.md` for the minimum loader shape and implementation order
- keep `milestone-1-plan.md` in view to avoid expanding scope beyond loading screen and intro menu

### During Image Portability Work

- use `resource-loader-plan.md` for the current lookup-layer status
- use `image-portability-seam.md` for the next seam after file-based lookup
- keep `source-audit-handoff.md` nearby to confirm that the target callers are still aligned with milestone-1 priorities

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

- planning documents for architecture, milestone execution, build setup, validation, and decision tracking now exist
- documentation is sufficient to begin implementation of milestone 1
- the next practical step is implementation, starting with repo import and `CMake` scaffold work

## Related Files in This Folder

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
- `architecture.md`
- `milestone-1-plan.md`
- `cmake-build-plan.md`
- `source-audit-handoff.md`
- `resource-loader-plan.md`
- `image-portability-seam.md`
- `qa-test-matrix.md`
- `decision-log.md`
- `change-log.md`
