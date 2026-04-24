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

### 2026-04-15

- Document: `image-portability-seam.md`
- Change Type: updated
- Summary: recorded that `CLoadingSession.cpp` now uses the neutral image path for title and stargate loading-screen assets, and updated the next recommended callers to `CVisualPalette.cpp` and `CButtonBarData.cpp`

### 2026-04-15

- Document: `resource-loader-plan.md`
- Change Type: updated
- Summary: noted that the loading-screen caller now has a no-`HBITMAP` proof-of-concept path and shifted the next implementation target to `CVisualPalette.cpp` and `CButtonBarData.cpp`

### 2026-04-15

- Document: `source-audit-handoff.md`
- Change Type: updated
- Summary: added the current milestone-1 status showing that `CLoadingSession.cpp` now bypasses `HBITMAP` for its loading-screen assets while the remaining title/menu callers still need that neutral path propagated

### 2026-04-15

- Document: `image-portability-seam.md`
- Change Type: updated
- Summary: recorded that `CVisualPalette.cpp` and `CButtonBarData.cpp` now also use the neutral image path and shifted the recommended next step away from asset callers and toward shell/presenter work

### 2026-04-15

- Document: `resource-loader-plan.md`
- Change Type: updated
- Summary: noted that the title/menu-critical callers are now largely covered by the neutral image path and that remaining low-priority image callers can be deferred while milestone-1 focus moves to shell and presentation seams

### 2026-04-15

- Document: `source-audit-handoff.md`
- Change Type: updated
- Summary: updated the milestone-1 status to reflect that the title/menu-critical callers now have a neutral image path and added the next focus on source subset, shell, and presentation integration

### 2026-04-15

- Document: `next-implementation-focus.md`
- Change Type: created
- Summary: added a post-asset implementation guide recommending source-subset definition plus shell and presentation seam mapping before broader `CMake` work

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `next-implementation-focus.md` so the next agent can move cleanly from asset work to shell/presenter work

### 2026-04-15

- Document: `next-implementation-focus.md`
- Change Type: updated
- Summary: added a concrete milestone-1 source subset, shell seam map, and presentation seam map so the next implementation phase can move directly into bounded shell/presenter work

### 2026-04-15

- Document: `milestone-1-source-subset.md`
- Change Type: created
- Summary: added a target-oriented source subset for the first bounded macOS `CMake` scaffold, including required menu-path groups, deferred files, and a practical target order

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `milestone-1-source-subset.md` so the next build-system step can start from a bounded source subset

### 2026-04-15

- Document: `cmake-build-plan.md`
- Change Type: updated
- Summary: recorded that the initial root `CMakeLists.txt` and `CMakePresets.json` scaffold now exist, that three core targets are concrete static libraries, and that validation is currently blocked by missing `cmake` in the environment

### 2026-04-15

- Document: `milestone-1-source-subset.md`
- Change Type: updated
- Summary: recorded that `alchemy_kernel`, `alchemy_codechain`, and `alchemy_xmlutil` are now concrete scaffold targets and clarified that the next subset extension should proceed after `cmake` becomes available

### 2026-04-15

- Document: `cmake-build-plan.md`
- Change Type: updated
- Summary: recorded that `alchemy_jpeg` and bounded `alchemy_graphics` are now also concrete static targets in the bounded scaffold

### 2026-04-15

- Document: `milestone-1-source-subset.md`
- Change Type: updated
- Summary: recorded that the bounded scaffold now includes five concrete foundation targets and shifted the next target-expansion recommendation toward bounded `mammoth_tse`

### 2026-04-15

- Document: `mammoth-tse-bounded-candidate-set.md`
- Change Type: created
- Summary: added a bounded candidate-set plan for `mammoth_tse`, including first-tier menu-boot engine files, second-tier expansion areas, and explicit cautions about `DirectXUtil.h` header coupling

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `mammoth-tse-bounded-candidate-set.md` so engine-target expansion can proceed more safely than a blind full-source import

### 2026-04-15

- Document: `milestone-1-source-subset.md`
- Change Type: updated
- Summary: recorded that `mammoth_tse` is now a bounded concrete static target in the scaffold and clarified that the next step is to validate six concrete targets before broadening further

### 2026-04-15

- Document: `mammoth-tse-bounded-candidate-set.md`
- Change Type: updated
- Summary: recorded that the first-tier `mammoth_tse` candidate set has now been applied to the scaffold and that future expansion should follow compile fallout rather than broader guesswork

### 2026-04-15

- Document: `next-implementation-focus.md`
- Change Type: updated
- Summary: clarified that the bounded scaffold now includes six concrete targets and that the immediate next step should be validation once `cmake` is available, not further graph expansion

### 2026-04-15

- Document: `source-audit-handoff.md`
- Change Type: updated
- Summary: added a note that the scaffold has reached first-tier `mammoth_tse` and that further target expansion should wait for `cmake`-driven validation

### 2026-04-15

- Document: `alchemy-kernel-portability-fallout.md`
- Change Type: created
- Summary: added a cluster-based analysis of the first real `alchemy_kernel` compile blockers, including a bounded fix order for heap, synchronization, time, and non-Windows C++ issues

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `alchemy-kernel-portability-fallout.md` so the next portability fixes stay focused after the first macOS build attempt

### 2026-04-15

- Document: `alchemy-kernel-portability-fallout.md`
- Change Type: updated
- Summary: expanded the fallout analysis to include Win32 OS-service leakage, internet stack leakage, and 64-bit correctness issues, and shifted the recommendation from more shims toward a boundary split for `alchemy_kernel`

### 2026-04-15

- Document: `milestone-1-source-subset.md`
- Change Type: updated
- Summary: added an explicit caution that `alchemy_kernel` now appears to mix portable utilities with a broader Win32 service surface and may require a portable subset or boundary split

### 2026-04-15

- Document: `alchemy-kernel-split-strategy.md`
- Change Type: created
- Summary: added a strategy document recommending that `alchemy_kernel` be treated as portable core plus Win32 service surface, so the port can shift from incremental shims toward a clearer boundary split

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `alchemy-kernel-split-strategy.md` so the next step after kernel fallout is a boundary decision instead of more ad hoc Win32 shims

### 2026-04-15

- Document: `alchemy-kernel-split-strategy.md`
- Change Type: updated
- Summary: added a recommended first split slice inside `Kernel.h`, prioritizing synchronization/event wrappers, file and registry service surface, and bottom-of-header Win32 helper declarations for the first boundary refactor

### 2026-04-15

- Document: `alchemy-kernel-service-surface-next-slice.md`
- Change Type: created
- Summary: added a focused recommendation for the next Win32 service-surface boundary slice, explaining why `CDataFile.cpp` is the stronger implementation boundary while `CFileDirectory` and `CResourceReadBlock` should remain in place for now

### 2026-04-15

- Document: `index.md`
- Change Type: updated
- Summary: added navigation and usage guidance for `alchemy-kernel-service-surface-next-slice.md` so the next service-surface refactor can be chosen from active dependencies instead of guesswork

### 2026-04-15

- Document: `alchemy-kernel-portability-fallout.md`
- Change Type: updated
- Summary: clarified that the remaining 64-bit issues are not just local casts but reflect deeper 32-bit pointer-storage assumptions in archive/reference-related kernel code

### 2026-04-15

- Document: `milestone-1-source-subset.md`
- Change Type: updated
- Summary: added a caution that archive/reference-heavy `alchemy_kernel` paths may require deferral or redesign before the arm64 build can be considered clean

### 2026-04-15

- Document: `cmake-build-plan.md`
- Change Type: updated
- Summary: noted that `alchemy_kernel` now appears to contain both a Win32 service surface and 32-bit pointer-storage assumptions, which may require a narrower effective portable subset for early build success

## Maintenance Notes

- Add a new entry when a document is created, significantly restructured, or materially changes scope or execution direction.
- Small typo fixes do not need a change-log entry unless they affect interpretation.
- If a document is superseded or renamed, add a dedicated entry noting the transition.
