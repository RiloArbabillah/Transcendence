# Mammoth TSE Bounded Candidate Set

## Document Status

- Version: v1.1
- Last Updated: 2026-04-15
- Derived From: `source-audit-handoff.md`, `milestone-1-source-subset.md`, `milestone-1-plan.md`
- Purpose: identify a safer bounded candidate set for turning `mammoth_tse` into a concrete milestone-1 build target without blindly pulling in all of `Mammoth/TSE/*`

## Purpose

This document exists because `mammoth_tse` is not yet safe to convert into a concrete `CMake` target by guesswork alone.

Unlike the earlier `alchemy_*` targets, the menu-boot path in `Mammoth/TSE` quickly fans out into a large number of engine, topology, system-creation, design-binding, object, and image-related files.

The goal here is not to define the final source list.

The goal is to identify a bounded candidate set and a safer expansion order.

## Why `mammoth_tse` needs a separate planning step

Two facts make this target riskier than the earlier foundation targets:

- `Mammoth/TSE/PreComp.h:4` includes `DirectXUtil.h`
- `Mammoth/Include/TSE.h:16` also includes `DirectXUtil.h`

This means a naive “include everything used by intro/menu” approach is likely to drag in broad Windows-specific surface area immediately.

## Menu-boot responsibilities that definitely touch `Mammoth/TSE`

From the earlier audit, the menu-boot path requires TSE support for:

- universe boot and init
- design collection loading and event dispatch
- default adventure binding
- intro-system creation
- empty star-system creation
- sovereign lookup
- intro-time ship creation
- image-marking lifecycle used by intro/menu assets

## Files and clusters that look likely required

## Cluster A - Universe bootstrap and host/font bridge

These appear directly on the menu-boot path and should be treated as first-tier candidates.

- `Mammoth/TSE/CUniverse.cpp`
  - needed for `Boot`, `Init`, `InitFonts`, `CreateEmptyStarSystem`, image marking, and host callbacks

Key relevant evidence:

- `Mammoth/TSE/CUniverse.cpp:1428` `InitFonts`
- `Mammoth/TSE/CUniverse.cpp:270` `CreateEmptyStarSystem`
- `Mammoth/TSE/CUniverse.cpp:414` host-driven `CreateShipController`
- `Mammoth/TSE/CUniverse.cpp:774` and `:775` image marking/sweep

## Cluster B - Design collection and intro events

These are likely required because intro startup fires global intro events through the design collection.

- `Mammoth/TSE/CDesignCollection.cpp`

Key evidence:

- `Mammoth/TSE/CDesignCollection.cpp:893` `FireOnGlobalIntroStarted`
- `Mammoth/TSE/CDesignCollection.cpp:912` `FireOnGlobalMarkImages`

## Cluster C - System creation path

These are likely required because intro creates an empty system and then populates ships and related objects.

- `Mammoth/TSE/CreateSystem.cpp`
- `Mammoth/TSE/CSystem.cpp`

Key evidence:

- `Mammoth/TSE/CUniverse.cpp:280` delegates empty-system creation to `CSystem::CreateEmpty`
- `Mammoth/TSE/CreateSystem.cpp:3929` `CSystem::CreateEmpty`
- `Mammoth/TSE/CreateSystem.cpp:3965` `CSystem::CreateFromXML`

## Cluster D - Sovereigns and ship/object creation

These are likely needed because intro fallback logic uses sovereign lookup and creates ships in the generated intro system.

Strong candidate areas:

- `Mammoth/TSE/CSovereign.cpp`
- ship/object creation support reached through `CUniverse` and `CSystem`
- object/image support that intro-time ships rely on

This cluster is likely larger and should be added incrementally, not all at once.

## Cluster E - Image marking and object/image arrays

These are likely required because the intro/menu path marks library bitmaps during intro setup.

Strong candidate areas:

- image array and image library support files under `Mammoth/TSE/`
- any object-image support directly referenced by `CUniverse` or intro-system setup

This cluster should also be added incrementally.

## Likely required first-tier candidate files

These are the safest first candidates for a bounded `mammoth_tse` build attempt:

- `Mammoth/TSE/CUniverse.cpp`
- `Mammoth/TSE/CDesignCollection.cpp`
- `Mammoth/TSE/CreateSystem.cpp`
- `Mammoth/TSE/CSystem.cpp`

These four files should not be assumed sufficient by themselves, but they represent the narrowest obvious entry into the menu-boot-critical engine path.

## Likely second-tier expansion areas

These should be added only after the first-tier compile blockers are known.

- topology support files (`CTopology*`, supporting generators)
- sovereign files (`CSovereign.cpp` and nearby dependencies)
- ship/object creation files
- object-image and resource-marking support files
- extension/adventure binding support beyond what is already pulled by `CUniverse.cpp` and `CDesignCollection.cpp`

## Areas likely deferrable from the first `mammoth_tse` build slice

These do not look like the best first target for bounded bring-up unless compile dependencies force them in.

- broad gameplay update files such as:
  - `ShipUpdate.cpp`
  - large combat/effects clusters
- trading/economy files
- mission-heavy files
- gameplay-only UI/property helpers
- audio-adjacent engine integrations

## Practical strategy for converting `mammoth_tse` to a concrete target

Do not jump directly from placeholder to “all `Mammoth/TSE/*.cpp`”.

Instead:

1. start with first-tier candidates:
   - `CUniverse.cpp`
   - `CDesignCollection.cpp`
   - `CreateSystem.cpp`
   - `CSystem.cpp`
2. attempt to identify the first compile errors once `cmake` is available
3. add second-tier files only in response to concrete unresolved symbols or missing-type errors
4. keep notes on which additions are true menu-boot requirements versus legacy include spillover

## Immediate risk to watch

Because `TSE.h` and `PreComp.h` still pull `DirectXUtil.h`, some compile blockers may come from inherited include structure rather than true runtime need.

This is important because it means:

- some files will appear required only due to legacy header coupling
- the first bounded `mammoth_tse` attempt may reveal boundary-cleanup work instead of straightforward missing source files

## Recommended next step

When it is time to make `mammoth_tse` concrete, the next agent should:

1. use this document to add a first-tier source list only
2. keep `mammoth_tsui_core` as a placeholder until the `mammoth_tse` expansion is understood
3. treat compile fallout from `DirectXUtil.h` as a boundary problem to classify, not a signal to broaden the target blindly

## Current scaffold status

The current root `CMakeLists.txt` now reflects this first-tier candidate set directly.

`mammoth_tse` has been converted into a bounded `STATIC` target with:

- `Mammoth/TSE/CUniverse.cpp`
- `Mammoth/TSE/CDesignCollection.cpp`
- `Mammoth/TSE/CreateSystem.cpp`
- `Mammoth/TSE/CSystem.cpp`

This does not mean the target is expected to compile cleanly yet.

It means the next compile fallout, once `cmake` is available, should be attributable to a bounded first-tier engine slice instead of an unbounded engine import.

## Success criteria

This candidate-set plan is successful when:

- `mammoth_tse` can be expanded in a bounded way without immediately collapsing into “all engine files”
- compile blockers are attributable to specific clusters
- the next expansion step remains aligned with menu boot instead of gameplay breadth
