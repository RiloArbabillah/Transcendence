# File-Based Resource Loader Plan

## Document Status

- Version: v1.1
- Last Updated: 2026-04-15
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Purpose: define the smallest file-based resource loader needed to unblock milestone-1 loading screen and intro menu bring-up

## Purpose

This document translates the source audit into a concrete design for replacing Win32 resource loading on the milestone-1 path.

The goal is not to redesign the full asset pipeline. The goal is to introduce the minimum file-based loader needed for:

- `.dxfn` title/menu fonts
- JPEG loading for loading screen and UI atlases
- BMP mask loading for alpha/mask workflows

## Why this exists

The current menu-boot path still depends on Win32 resource APIs in several places:

- `Mammoth/TSUI/CVisualPalette.cpp:433` loads `DXFN_*` fonts via `CreateFromResource`
- `Mammoth/TSUI/CVisualPalette.cpp:506` loads JPEG UI atlases via `JPEGLoadFromResource`
- `Mammoth/TSUI/CVisualPalette.cpp:517` loads BMP masks via `dibLoadFromResource`
- `Transcendence/Transcendence/CLoadingSession.cpp:29` loads title/loading images from resources
- `Transcendence/Transcendence/CButtonBarData.cpp:125` loads menu button art from resources
- `Transcendence/Transcendence/CTranscendenceWnd.cpp:754` loads title/menu fonts via `CreateFromResource`

Milestone 1 cannot progress on macOS until this path is replaced.

## Existing code we can reuse

### Stream infrastructure

- `Alchemy/Include/Kernel.h:923` defines `CMemoryReadStream`
- `Alchemy/Include/Kernel.h:975` defines `CFileReadStream`

These are enough to support a file- or memory-based font loader without inventing a new stream abstraction.

### Serialized `.dxfn` reader already exists

- `Alchemy/DirectXUtil/CG16bitFont.cpp:1143` defines `CG16bitFont::ReadFromStream`

This is the most valuable existing seam in the codebase. It means `.dxfn` assets are already portable at the data-format level. The current problem is only how those bytes are obtained.

### File-based image entry points already exist, but are not portable yet

- `Alchemy/DirectXUtil/CG32bitImage.cpp:644` defines `CG32bitImage::CreateFromFile`
- `Alchemy/DirectXUtil/CG16BitImage.cpp:1947` defines `CG16bitImage::CreateFromFile`
- `Alchemy/DirectXUtil/Files.cpp:8` defines `dxLoadImageFile`

Important caveat:

- these file-based image APIs still flow through `JPEGLoadFromFile` and `dibLoadFromFile`
- those functions still return `HBITMAP` and therefore remain Windows-specific internally

So they are useful as API shape references, but they are not yet sufficient for the macOS path.

## Scope for the first loader slice

### In scope

- load `.dxfn` fonts from files under `Transcendence/Transcendence/Resources/`
- load JPEG assets from files under `Transcendence/Transcendence/Resources/`
- load BMP mask assets from files under `Transcendence/Transcendence/Resources/`
- provide a deterministic mapping from current resource names/IDs to file paths
- update the milestone-1 call sites to use file-based loading on macOS

### Out of scope

- replacing all asset loading in the entire game
- redesigning TDB/XML resource lookup
- solving general-purpose runtime asset packaging
- rewriting the full image decode stack for all platforms at once
- refactoring all Win32 graphics internals in one pass

## Minimum required assets

These are the assets that should drive the first implementation.

### Fonts

- `Transcendence/Transcendence/Resources/Header.dxfn`
- `Transcendence/Transcendence/Resources/HeaderBold.dxfn`
- `Transcendence/Transcendence/Resources/SubTitle.dxfn`
- `Transcendence/Transcendence/Resources/SubTitleBold.dxfn`
- `Transcendence/Transcendence/Resources/SubTitleHeavyBold.dxfn`
- `Transcendence/Transcendence/Resources/Title.dxfn`
- `Transcendence/Transcendence/Resources/LogoTitle.dxfn`

### Loading screen

- `Transcendence/Transcendence/Resources/Title.JPG`
- `Transcendence/Transcendence/Resources/Stargate.JPG`
- `Transcendence/Transcendence/Resources/StargateMask.BMP`

### Intro/title menu UI

- `Transcendence/Transcendence/Resources/GameButtonIcons.jpg`
- `Transcendence/Transcendence/Resources/UIIcons.jpg`
- `Transcendence/Transcendence/Resources/UIIconsMask.bmp`

## Proposed design

## 1. Add a small resource path resolver

Introduce a small helper dedicated to milestone-1 assets.

Proposed responsibility:

- resolve logical resource names such as `DXFN_HEADER` or `IDR_TITLE_IMAGE` to concrete files in `Resources/`
- avoid any Win32 `FindResource` dependency on macOS
- keep the mapping explicit and small for the first slice

Proposed logical API shape:

```cpp
class CResourcePathResolver
    {
    public:
        static CString GetResourceRoot();
        static bool FindFontResource(const CString &sName, CString *retsFilespec);
        static bool FindJPEGResource(const CString &sName, CString *retsFilespec);
        static bool FindBitmapResource(const CString &sName, CString *retsFilespec);
    };
```

Notes:

- for milestone 1, this can be table-driven and hardcoded
- later it can be replaced with bundle-aware or platform-layer lookup without changing higher-level callers

## 2. Add stream/file-based font loading

`CG16bitFont` already knows how to deserialize itself from a stream. It needs a file entry point.

Proposed addition:

- add `CG16bitFont::CreateFromFile(const CString &sFilespec)`

Expected implementation shape:

1. open `CFileReadStream`
2. call `Open()`
3. call `ReadFromStream(&Stream)`
4. close the stream

This should be the preferred macOS path for all `DXFN_*` fonts.

Possible extension:

- optionally add `CreateFromStream(IReadStream *pStream)` if a direct stream-based API feels cleaner

## 3. Introduce a file-based image loader seam for milestone-1 callers

For images, the short-term need is not a perfect new abstraction. The short-term need is a call surface that does not mention Win32 resources.

Proposed helper surface:

```cpp
namespace ResourceLoader
    {
    ALERROR LoadJPEGResource(const CString &sName, HBITMAP *rethBitmap);
    ALERROR LoadBitmapResource(const CString &sName, HBITMAP *rethBitmap, EBitmapTypes *retiType = NULL);
    }
```

Initial behavior:

- resolve resource name to file path via `CResourcePathResolver`
- call existing file-based loaders such as `JPEGLoadFromFile` or `dibLoadFromFile`

Important limitation:

- this is only a transitional seam if `JPEGLoadFromFile` and `dibLoadFromFile` still produce `HBITMAP`
- it may still fail to compile on macOS until the lower image pipeline is ported

Even so, it is still a useful milestone because it separates:

- resource name resolution
- from actual image decode and bitmap creation

That split reduces the next implementation step substantially.

## 4. Prefer a platform gate instead of cross-platform churn on day one

The smallest change is likely:

- keep current Win32 resource code on Windows
- add file-based resource loading only for the macOS path

This keeps the existing Windows build stable while making the new path explicit.

Possible shape:

```cpp
#ifdef PLATFORM_MAC
    // use file-based resource path
#else
    // existing Win32 resource path
#endif
```

The exact macro can match whatever the new CMake/macOS build introduces.

## Call sites to change first

### Highest priority

1. `Mammoth/TSUI/CVisualPalette.cpp`
   - replace font resource loading for `DXFN_*`
   - replace UI atlas JPEG and mask bitmap loading

2. `Transcendence/Transcendence/CLoadingSession.cpp`
   - replace title/stargate image and mask loading

3. `Transcendence/Transcendence/CTranscendenceWnd.cpp`
   - replace direct `CreateFromResource` for title/menu fonts

### Second wave

4. `Transcendence/Transcendence/CButtonBarData.cpp`
   - replace menu button atlas loading

5. nearby non-critical sessions
   - `CHelpSession.cpp`
   - `CStatsSession.cpp`
   - `CModExchangeSession.cpp`

## Recommended implementation order

### Phase A: Separate naming from loading

1. add `CResourcePathResolver`
2. encode the known mappings for milestone-1 assets
3. verify all required files exist on disk

Verification:

- title/menu resource names resolve to actual files under `Transcendence/Transcendence/Resources/`

### Phase B: Unblock fonts first

1. add `CG16bitFont::CreateFromFile`
2. update `CVisualPalette::Init` to use file-based `.dxfn` loading on macOS
3. update `CTranscendenceWnd::WMCreate` to use file-based `.dxfn` loading on macOS

Verification:

- `CVisualPalette::Init` can populate `Header`, `SubTitle`, `Title`, and `LogoTitle` without `FindResource`
- host `FindFont` works through initialized visuals

### Phase C: Move loading-screen and UI images off `.rc`

1. add file-based wrappers for JPEG/BMP name resolution
2. update `CLoadingSession.cpp`
3. update `CButtonBarData.cpp`
4. update `CVisualPalette.cpp` UI image atlas loading

Verification:

- loading screen assets resolve through files
- intro button bar and title icons can resolve through files

### Phase D: Replace lower-level Windows image internals as needed

If the code still depends on `HBITMAP` creation and therefore does not compile on macOS, the next step is to replace or isolate the lower image decode path.

That should happen only after phases A-C make the dependency boundaries explicit.

## Design constraints

- keep changes additive to avoid destabilizing the Windows build
- do not broaden scope into all asset loading systems
- avoid rewriting the full renderer or font rasterizer in this step
- preserve the current `.dxfn` assets to keep title/menu fidelity
- keep resource mapping deterministic and easy to debug

## Risks and notes

### `HBITMAP` still leaks through image loaders

Even with file-based lookup, the current image decode path still ends in Windows bitmap objects.

This means:

- font loading is the cleaner first target because `.dxfn` deserialization already avoids GDI generation
- image loading may need one more layer of adaptation before it becomes truly macOS-safe

### `CUniverse::InitFonts` fallback remains Windows-specific

- `Mammoth/TSE/CUniverse.cpp:1445` still creates a fallback font via `CreateFont`

Mitigation:

- ensure the host font path succeeds for the milestone-1 font set so this fallback never runs on macOS

### Resource root cannot rely on current directory

The path resolver should be fed by the platform layer or a deterministic app-resource root, not by implicit working-directory assumptions.

## Success criteria for this loader design

This loader design is successful when:

- milestone-1 title/menu assets can be addressed by file path instead of Win32 resource ID lookup
- `.dxfn` fonts can be loaded without `FindResource`
- the remaining image portability problem is isolated to a small number of file-based loader functions instead of being spread through sessions and UI code

## Current implementation status

The following steps from this plan are now implemented in the source tree:

- `CResourcePathResolver` exists in `Transcendence/Transcendence/CResourcePathResolver.cpp`
- `CG16bitFont::CreateFromFile` exists in `Alchemy/DirectXUtil/CG16bitFont.cpp`
- `Mammoth/TSUI/CVisualPalette.cpp` now resolves `DXFN_*` fonts through `CResourcePathResolver` and `CreateFromFile`, with fallback to `CreateFromResource`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp` now resolves `DXFN_*` fonts through `CResourcePathResolver` and `CreateFromFile`, with fallback to `CreateFromResource`
- image wrappers now exist:
  - `LoadJPEGResourceAsDIB`
  - `LoadBMPResourceAsDIB`
- `Transcendence/Transcendence/CLoadingSession.cpp` now uses the image wrappers for loading screen assets
- `Mammoth/TSUI/CVisualPalette.cpp` now uses the image wrappers for UI atlas and mask lookup
- `Transcendence/Transcendence/CButtonBarData.cpp` now uses the image wrappers for title/menu button art lookup
- `Transcendence/Transcendence/CLoadingSession.cpp` now also has a proof-of-concept neutral image path for `Title.JPG`, `Stargate.JPG`, and `StargateMask.BMP`, bypassing `HBITMAP` for those loading-screen assets
- `Mammoth/TSUI/CVisualPalette.cpp` now uses the neutral image path for its title/menu-critical UI atlases and masks
- `Transcendence/Transcendence/CButtonBarData.cpp` now uses the neutral image path for title/menu button art

## Remaining work after the current implementation slice

Still not migrated to the wrapper path:

- `Transcendence/Transcendence/CHelpSession.cpp`
- `Transcendence/Transcendence/CStatsSession.cpp`
- `Transcendence/Transcendence/CModExchangeSession.cpp`

Still not fully portable enough for macOS compilation/runtime:

- `LoadJPEGResourceAsDIB` still ends in `JPEGLoadFromFile`, which returns `HBITMAP`
- `LoadBMPResourceAsDIB` still ends in `dibLoadFromFile`, which returns `HBITMAP`
- `CG32bitImage::CreateFromBitmap` and related image creation still depend on Windows bitmap objects

But note:

- `CLoadingSession.cpp` is no longer blocked by those legacy image paths for its main loading-screen assets, because it now uses the new neutral JPEG/BMP decode path and `CG32bitImage::CreateFromRaw(...)`
- `Mammoth/TSUI/CVisualPalette.cpp` and `Transcendence/Transcendence/CButtonBarData.cpp` are also no longer blocked by the legacy `HBITMAP` path for their main milestone-1 image assets

### Practical interpretation

The current implementation has largely solved resource lookup portability for milestone-1 callers.

It has not yet solved graphics-object portability.

That means the project is now in a better state to attack the next real blocker: replacing or isolating the `HBITMAP`-based image decode path.

## Recommended next implementation step

The best next code step is now:

1. treat the remaining image callers as deferred unless validation shows they are needed for milestone 1
2. shift implementation focus to shell replacement, presenter integration, and source-subset build work
3. preserve the current neutral image path as the pattern to reuse if another menu-adjacent caller becomes necessary

This keeps progress aligned with milestone 1 now that the title/menu-critical asset callers have largely been covered.
