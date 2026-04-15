# Documentation Change Log

## Document Status

- Version: v1.2
- Last Updated: 2026-04-15
- Scope: documentation-only changes for the macOS port planning set

## Purpose

This file tracks meaningful changes to the documentation set in `transcendece`.

Log entries should record:

- date
- document affected
- type of change
- short summary

This file is for documentation changes only, not source-code implementation changes.

## Log Format

Suggested entry structure:

```text
Date:
Document:
Change Type:
Summary:
```

## Entries

### 2026-04-14

- Document: `PRD.md`
- Change Type: created
- Summary: initial product requirements document for the macOS Apple Silicon port using `SDL2 + Metal`

### 2026-04-14

- Document: `PRD.md`
- Change Type: updated
- Summary: added explicit version and last updated metadata

### 2026-04-14

- Document: `PRD.md`
- Change Type: updated
- Summary: added related document references for derived planning documents

### 2026-04-14

- Document: `roadmap.md`
- Change Type: created
- Summary: added phased delivery roadmap, milestone sequencing, and validation gates

### 2026-04-14

- Document: `task-backlog.md`
- Change Type: created
- Summary: added actionable engineering backlog with epics, priorities, dependencies, and acceptance criteria

### 2026-04-14

- Document: `dependency-matrix.md`
- Change Type: created
- Summary: added portability classification and replace/wrap/stub/defer mapping for major dependency areas

### 2026-04-14

- Document: `architecture.md`
- Change Type: created
- Summary: added target subsystem boundaries, interface direction, and integration rules for the macOS port

### 2026-04-14

- Document: `milestone-1-plan.md`
- Change Type: created
- Summary: added detailed execution plan for the native title or main menu milestone

### 2026-04-14

- Document: `cmake-build-plan.md`
- Change Type: created
- Summary: added macOS `CMake` target graph, preset strategy, framework linkage plan, and fallback strategy

### 2026-04-14

- Document: `decision-log.md`
- Change Type: created
- Summary: added recorded architecture and build decisions with rationale and implementation impact

### 2026-04-14

- Document: `qa-test-matrix.md`
- Change Type: created
- Summary: added minimum validation requirements and exit gates for each milestone

### 2026-04-14

- Document: `index.md`
- Change Type: created
- Summary: added documentation portal and reading guide for the full planning set

### 2026-04-14

- Document: `change-log.md`
- Change Type: created
- Summary: added documentation-only change tracking for the project planning set

### 2026-04-14

- Document: `PRD.md`
- Change Type: updated
- Summary: added references to `index.md` and `change-log.md` in the related docs section

### 2026-04-14

- Document: `index.md`
- Change Type: updated
- Summary: clarified that `PRD.md` is the source of truth and added an explicit source-of-truth section

### 2026-04-15

- Document: `source-audit-handoff.md`
- Change Type: created
- Summary: added source-level audit handoff covering real menu boot path, milestone-1 blockers, host contract findings, and font/resource pipeline risks from the live codebase

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage references for `source-audit-handoff.md` so future implementation work can cross-check planning docs against the actual source audit

### 2026-04-15

- Document: `source-audit-handoff.md`
- Change Type: updated
- Summary: added milestone-1 asset inventory, Win32 resource ID to file mappings, and minimum file subset required for loading screen and intro menu bring-up

### 2026-04-15

- Document: `resource-loader-plan.md`
- Change Type: created
- Summary: added a minimal file-based loader design for milestone-1 `.dxfn`, JPEG, and BMP-mask assets, including implementation order and first-call-site targets

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `resource-loader-plan.md` so implementation can move from audit into a concrete loader plan

### 2026-04-15

- Document: `resource-loader-plan.md`
- Change Type: updated
- Summary: recorded implementation progress for the resolver, file-based font loading, and early menu image callers, and clarified that the next blocker is `HBITMAP`-based image portability

### 2026-04-15

- Document: `source-audit-handoff.md`
- Change Type: updated
- Summary: added current implementation status showing that milestone-1 font and early image lookup now use file-based resource resolution, plus noted the remaining non-critical callers and the unresolved `HBITMAP` dependency

### 2026-04-15

- Document: `image-portability-seam.md`
- Change Type: created
- Summary: added the next-phase design for removing `HBITMAP` from milestone-1 image ingestion, including the proposed neutral decoded-image seam and caller migration order

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `image-portability-seam.md` so future work can move from file-based lookup into true image portability work

## Maintenance Notes

- Add a new entry when a document is created, significantly restructured, or materially changes scope or execution direction.
- Small typo fixes do not need a change-log entry unless they affect interpretation.
- If a document is superseded or renamed, add a dedicated entry noting the transition.
