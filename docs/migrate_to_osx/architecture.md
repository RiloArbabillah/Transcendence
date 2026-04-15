# Architecture

## Document Status

- Version: v1.0
- Last Updated: 2026-04-14
- Derived From: `PRD.md`
- Companion Docs: `roadmap.md`, `task-backlog.md`, `dependency-matrix.md`
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Technical Direction: `SDL2 + Metal`

## Purpose

This document defines the target architecture for the macOS Apple Silicon port of `TranscendenceDev`. It focuses on subsystem boundaries, ownership rules, integration flow, and the minimum interface design needed to replace Windows-specific behavior without rewriting core gameplay logic.

The design goal is to preserve existing engine and game code wherever possible while introducing a narrow portability layer for macOS.

## Architectural Principles

- Preserve existing engine and gameplay logic first
- Replace platform boundaries, not entire systems
- Keep macOS-specific code isolated
- Do not leak `SDL2`, `Metal`, or Objective-C++ into shared engine headers
- Start with a compatibility-first renderer based on software framebuffer upload
- Treat build portability and runtime portability as separate problems

## High-Level Architecture

The macOS port is built around five layers:

1. Core Engine Layer
2. Application and Session Layer
3. Platform Abstraction Layer
4. Rendering Backend Layer
5. Audio and Runtime Services Layer

The intended dependency direction is:

`Game / UI / Engine -> Platform Abstractions -> SDL2 / Metal / macOS APIs`

Not:

`Game / UI / Engine -> SDL2 / Metal / AppKit directly`

## Layer Overview

### 1. Core Engine Layer

Primary code areas:

- `Alchemy/Kernel/*`
- `Alchemy/CodeChain/*`
- `Alchemy/XMLUtil/*`
- large parts of `Alchemy/Graphics/*`
- `Mammoth/TSE/*`

Responsibilities:

- scripting and runtime primitives
- XML/data parsing
- core engine systems
- core game data handling
- software image and drawing logic that is already engine-owned

Constraints:

- should remain unaware of `SDL2`, `Metal`, and AppKit
- should compile on macOS with only portability fixes, not architecture changes

### 2. Application and Session Layer

Primary code areas:

- `Mammoth/TSUI/*`
- `Transcendence/Transcendence/*`

Responsibilities:

- intro and menu sessions
- game session control
- UI orchestration
- key binding behavior
- user-facing application flow

Constraints:

- should depend on abstract platform services, not directly on Win32 or SDL
- should remain behaviorally close to the original codebase

### 3. Platform Abstraction Layer

New macOS portability boundary.

Responsibilities:

- app startup/shutdown
- event pump
- window lifecycle
- input translation
- timer access
- clipboard and message box services
- cursor visibility and capture
- resource and writable path discovery

Implementation direction:

- `SDL2` owns the actual app shell and event source
- wrapper interfaces expose only engine-friendly C++ types

### 4. Rendering Backend Layer

Responsibilities:

- own the screen-present path for macOS
- accept engine-generated frame/image data
- upload CPU framebuffer into a `Metal` texture
- present to the native window
- handle resize and drawable-size changes

Implementation direction:

- first version is a compatibility presenter, not a full GPU-native renderer
- software rendering stays in the engine
- `Metal` is initially used to display the final frame

### 5. Audio and Runtime Services Layer

Responsibilities:

- sound effects playback
- soundtrack/music playback
- save/settings path translation
- resource lookup translation for the bundle/runtime environment

Implementation direction:

- replace DirectSound and MCI backends behind stable higher-level logic where possible

## Target Dependency Graph

### Stable Core Dependency Flow

- `Alchemy/Kernel` -> foundational utilities
- `Alchemy/CodeChain` -> scripting on top of core utilities
- `Alchemy/XMLUtil` -> data parsing on top of core utilities
- `Alchemy/Graphics` -> graphics data structures and helpers
- `Mammoth/TSE` -> game engine systems
- `Mammoth/TSUI` -> human interface and UI systems
- `Transcendence/Transcendence` -> game application and sessions

### macOS Runtime Dependency Flow

- app/session code calls platform abstractions
- platform abstractions are implemented using `SDL2`
- renderer abstraction is implemented using `Metal`
- macOS-specific low-level code is confined to backend or platform adapter files

## Boundary Rules

### Rule 1 - No Direct SDL Includes in Shared Engine Headers

Allowed:

- SDL includes in platform implementation files
- SDL includes in platform-specific wrapper translation units

Not allowed:

- SDL types in shared headers under `Alchemy`, `Mammoth`, or shared game headers

### Rule 2 - No Metal or AppKit Types in Shared Engine Headers

Allowed:

- `Metal` and AppKit usage in `.mm` files
- private implementation objects hidden behind C++ interfaces

Not allowed:

- `MTLDevice`, `CAMetalLayer`, `NSWindow`, or Objective-C declarations in shared engine headers

### Rule 3 - Input Semantics Stay Engine-Owned

The platform layer provides raw input events, but the engine/application layer keeps ownership of:

- command mapping
- keybinding logic
- gameplay-specific input semantics
- text entry handling rules at the session level

### Rule 4 - Rendering Semantics Stay Engine-Owned First

The first macOS renderer should not redefine how the game draws.

The engine continues to produce a frame using existing drawing logic. The renderer backend only presents that frame.

### Rule 5 - Filesystem Policy Is Platform-Owned

The platform/runtime layer owns:

- bundle resource base path
- writable settings path
- writable saves path
- temporary/cache path if needed

The engine should request these through abstraction, not infer paths from working directory.

## Subsystem Design

## A. Platform Application Host

### Purpose

Provide a platform-neutral application host interface that replaces Win32 startup and message-loop assumptions.

### Responsibilities

- initialize SDL video/event subsystems
- create and destroy the application window
- pump events
- expose application lifecycle events
- provide timing hooks to the main loop
- mediate fullscreen and focus changes

### Suggested Interface Shape

```cpp
class IPlatformAppHost {
public:
    virtual ~IPlatformAppHost() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void PumpEvents() = 0;
    virtual bool IsRunning() const = 0;
    virtual IPlatformWindow &GetWindow() = 0;
};
```

### Implementation Notes

- SDL owns the real app/event loop behavior
- shared engine code only sees C++ interfaces
- lifecycle callbacks should be explicit, not hidden behind platform globals

### Likely Source Mapping

- replaces responsibilities currently centered in:
  - `Transcendence/Transcendence/Main.cpp`
  - `Transcendence/Transcendence/CTranscendenceWnd.cpp`
  - `Mammoth/TSUI/Run.cpp`

## B. Platform Window

### Purpose

Provide a stable abstraction for window operations and geometry state.

### Responsibilities

- create/destroy native window
- report logical size and drawable size
- set title
- toggle fullscreen
- manage focus/visibility state

### Suggested Interface Shape

```cpp
struct WindowSize {
    int width;
    int height;
};

class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;

    virtual WindowSize GetLogicalSize() const = 0;
    virtual WindowSize GetDrawableSize() const = 0;
    virtual void SetTitle(const char *title) = 0;
    virtual void SetFullscreen(bool enabled) = 0;
    virtual bool HasFocus() const = 0;
};
```

### Critical Requirement

The architecture must explicitly distinguish:

- logical UI/game coordinates
- drawable pixel size

This is necessary for Retina correctness and for mapping mouse input accurately.

## C. Input Adapter

### Purpose

Translate `SDL_Event` data into the engine's existing input and command model.

### Responsibilities

- map SDL key events to engine command events
- map SDL text input to UI text-entry flows
- map mouse movement, button, and wheel events
- preserve repeat behavior rules where possible

### Suggested Interface Shape

```cpp
class IInputAdapter {
public:
    virtual ~IInputAdapter() = default;

    virtual void HandlePlatformEvents() = 0;
    virtual void OnKeyDown(int keyCode, int modifiers, bool repeat) = 0;
    virtual void OnKeyUp(int keyCode, int modifiers) = 0;
    virtual void OnTextInput(const char *text) = 0;
    virtual void OnMouseMove(int x, int y) = 0;
    virtual void OnMouseButtonDown(int button, int x, int y) = 0;
    virtual void OnMouseButtonUp(int button, int x, int y) = 0;
    virtual void OnMouseWheel(int deltaX, int deltaY) = 0;
};
```

### Design Notes

- this adapter should not reinterpret gameplay meaning beyond necessary translation
- the adapter is responsible for converting SDL semantics into the engine's expected event shape
- menu text input must be kept separate from command key processing

### Likely Source Mapping

- integrates with:
  - `Transcendence/Transcendence/CGameKeys.cpp`
  - `Transcendence/Transcendence/GameSessionInput.cpp`
  - `Alchemy/DirectXUtil/CAniTextInput.cpp`

## D. Renderer Abstraction

### Purpose

Provide a backend-neutral rendering boundary between the engine's frame generation and the macOS presentation path.

### Responsibilities

- initialize renderer backend
- react to resize events
- upload engine-generated frame data
- present frame to screen
- expose diagnostic info if needed

### Suggested Interface Shape

```cpp
struct FrameBufferView {
    const void *pixels;
    int width;
    int height;
    int stride;
};

class IRenderPresenter {
public:
    virtual ~IRenderPresenter() = default;

    virtual bool Initialize(IPlatformWindow &window) = 0;
    virtual void Shutdown() = 0;
    virtual void Resize(int width, int height, int drawableWidth, int drawableHeight) = 0;
    virtual bool Present(const FrameBufferView &frame) = 0;
};
```

### First-Version Strategy

- keep the engine's software framebuffer path
- upload the final CPU buffer to a Metal texture each frame
- draw a simple fullscreen pass

### Why This Matters

- avoids rewriting custom software blitters and image primitives too early
- isolates the platform risk to one presenter backend

### Likely Source Mapping

- replaces the screen-present responsibilities centered in:
  - `Alchemy/DirectXUtil/CDXScreen.cpp`
  - `Alchemy/Include/DXScreenMgr3D.h`
  - `Alchemy/Include/DirectXUtil.h`

## E. Audio Backend Boundary

### Purpose

Replace Windows-only sound backends while preserving higher-level sound and soundtrack logic.

### Responsibilities

- sound effect playback
- music playback
- pause/stop/fade support where required
- backend initialization and shutdown

### Suggested Interface Shape

```cpp
class IAudioEffectsBackend {
public:
    virtual ~IAudioEffectsBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual int PlayEffect(int effectId, int volume, int pan, bool loop) = 0;
    virtual void StopEffect(int handle) = 0;
};

class IMusicBackend {
public:
    virtual ~IMusicBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual bool PlayTrack(const char *trackId) = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Stop() = 0;
};
```

### Design Notes

- keep `CSoundtrackManager` if possible
- replace only the lower-level playback implementation first
- exact interface names can vary, but the separation between SFX and music should remain explicit

### Likely Source Mapping

- replaces responsibilities in:
  - `Alchemy/DirectXUtil/Sound.cpp`
  - `Mammoth/TSUI/CMCIMixer.cpp`
- integrates with:
  - `Mammoth/TSUI/CSoundtrackManager.cpp`

## F. Runtime Paths and Resource Resolution

### Purpose

Provide an explicit runtime path service for bundle resources and writable user data.

### Responsibilities

- resolve app bundle resource path
- resolve writable save/config path
- resolve temporary/cache path if needed

### Suggested Interface Shape

```cpp
class IRuntimePaths {
public:
    virtual ~IRuntimePaths() = default;

    virtual const char *GetResourceRoot() const = 0;
    virtual const char *GetSaveRoot() const = 0;
    virtual const char *GetSettingsRoot() const = 0;
};
```

### Design Notes

- this should be platform-owned
- the game should not infer save/resource paths from the working directory
- bundle and writable roots must be separate concepts

## Integration Flow

### Startup Flow

1. macOS executable enters the platform host
2. platform host initializes SDL and creates window
3. platform host initializes renderer presenter using the window
4. runtime paths service resolves bundle and writable locations
5. engine and app layers initialize using abstract services
6. application enters event pump and frame loop

### Frame Flow

1. platform host pumps SDL events
2. input adapter translates events into engine-facing callbacks
3. engine updates session and game state
4. engine renders into software image/framebuffer
5. renderer presenter uploads and presents the resulting frame through Metal

### Shutdown Flow

1. app requests shutdown
2. audio backend stops playback
3. renderer presenter shuts down
4. platform host destroys window and SDL state
5. application exits cleanly

## Implementation Boundaries by Repo Area

## Likely To Stay Mostly Intact

- `Alchemy/Kernel/*`
- `Alchemy/CodeChain/*`
- `Alchemy/XMLUtil/*`
- large portions of `Mammoth/TSE/*`
- much of menu and gameplay session logic in `Mammoth/TSUI/*` and `Transcendence/Transcendence/*`

## Likely To Need Adapter Integration

- `Mammoth/TSUI/CHumanInterface.cpp`
- `Transcendence/Transcendence/CGameKeys.cpp`
- `Transcendence/Transcendence/GameSessionInput.cpp`
- resource and settings path consumers

## Likely To Need Replacement in macOS Path

- `Transcendence/Transcendence/Main.cpp`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp`
- `Mammoth/TSUI/Run.cpp`
- `Alchemy/DirectXUtil/CDXScreen.cpp`
- `Alchemy/Graphics/DIB.cpp`
- `Alchemy/Graphics/GDI.cpp`
- `Alchemy/DirectXUtil/Sound.cpp`
- `Mammoth/TSUI/CMCIMixer.cpp`

## Objective-C++ Usage Policy

Objective-C++ should be used only in implementation files that need direct access to:

- AppKit windowing details not covered by SDL
- `Metal` device and layer setup
- macOS-specific bundle/path helpers that are awkward in pure C++

Preferred rule:

- public/shared headers stay C++
- `.mm` files hide Apple APIs behind private implementation details

## Build-System Architecture

### Windows Path

- existing `.sln` and `.vcxproj` remain authoritative for Windows contributors

### macOS Path

- `CMake` defines the macOS build graph
- targets should mirror the existing logical project structure
- macOS-specific platform and renderer targets are linked only into the macOS application path

### Suggested macOS Target Grouping

- core libraries:
  - `alchemy_kernel`
  - `alchemy_codechain`
  - `alchemy_xmlutil`
  - `alchemy_graphics`
  - `mammoth_tse`
  - `mammoth_tsui`
- platform/runtime targets:
  - `platform_sdl`
  - `platform_macos`
  - `render_metal`
  - `audio_backend`
- app targets:
  - `transdata_cli`
  - `transcendence_app`

## First-Version Architecture Decisions

- use `SDL2` as the app shell replacement
- use `Metal` as the native macOS presenter backend
- preserve engine software rendering first
- avoid a full GPU-native renderer rewrite at the beginning
- keep audio replacement behind backend boundaries
- keep Steam and production cloud integrations outside the critical path

## Architecture Risks

### Risk 1 - Boundary Leakage

If SDL or Metal types leak into shared engine headers, cross-platform maintenance cost rises immediately.

Mitigation:

- enforce interface-only boundaries
- confine backend details to implementation files

### Risk 2 - Platform Host Too Thin or Too Fat

If the host layer is too thin, Win32 assumptions leak upward. If it is too fat, it becomes a new mini-engine.

Mitigation:

- keep interfaces small and specific to real needs observed in the existing codebase

### Risk 3 - Renderer Boundary Starts Too Ambitious

Trying to define a full modern rendering abstraction before the first frame exists will slow the port.

Mitigation:

- keep the first renderer interface focused on presenting an existing framebuffer

### Risk 4 - Audio and Filesystem Are Treated as Afterthoughts

The game may appear to run while still failing as a usable native app.

Mitigation:

- keep audio and runtime paths as explicit architecture layers from the start, even if their full implementation lands later

## Architecture Validation Checklist

- can core engine headers compile without SDL and Metal includes
- can the platform host boot and shut down the app without Win32 code paths
- can the renderer presenter show a correct frame without rewriting game drawing logic
- can input translation preserve existing gameplay semantics
- can bundle resources and writable paths be resolved without relying on current working directory
- can audio backends be replaced without rewriting high-level soundtrack policy

## Near-Term Next Steps

- use this document to drive `A-002 Define subsystem boundaries` in `task-backlog.md`
- use it as the basis for the initial `CMake` target graph and scaffold
- use it to constrain future implementation so new code stays behind the defined boundaries

## Related Docs

- `PRD.md`
- `roadmap.md`
- `task-backlog.md`
- `dependency-matrix.md`
