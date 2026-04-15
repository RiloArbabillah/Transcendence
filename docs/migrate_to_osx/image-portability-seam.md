# Image Portability Seam

## Document Status

- Version: v1.1
- Last Updated: 2026-04-15
- Project: Native macOS Apple Silicon port of `kronosaur/TranscendenceDev`
- Purpose: define the next seam after file-based resource lookup, focused on isolating or removing `HBITMAP` from the milestone-1 image path

## Purpose

The previous implementation slice moved milestone-1 callers away from Win32 resource lookup.

That solved the question of how loading screen and intro/menu assets are found.

It did not solve the next portability blocker: image decode and image object creation still depend on `HBITMAP`.

This document captures the smallest credible next seam to address that blocker.

## What is already done

The following resource-lookup work is already in place:

- `Transcendence/Transcendence/CResourcePathResolver.cpp` resolves milestone-1 resource names to files
- `Alchemy/DirectXUtil/CG16bitFont.cpp` supports `CG16bitFont::CreateFromFile`
- `Mammoth/TSUI/CVisualPalette.cpp` uses file-based lookup for milestone-1 fonts and UI image resources
- `Transcendence/Transcendence/CTranscendenceWnd.cpp` uses file-based lookup for milestone-1 fonts
- `Transcendence/Transcendence/CLoadingSession.cpp` uses file-based lookup for loading screen images
- `Transcendence/Transcendence/CButtonBarData.cpp` uses file-based lookup for menu button art

The remaining blocker is lower in the stack.

## Current blocker

### Resource lookup is no longer the main problem

The code now mostly knows which file to load for milestone-1 image assets.

### The main problem is image decode output type

The image path still looks like this:

1. resolve logical resource name to filespec
2. call `JPEGLoadFromFile(...)` or `dibLoadFromFile(...)`
3. receive `HBITMAP`
4. call `CG32bitImage::CreateFromBitmap(...)` or `CG16bitImage::CreateFromBitmap(...)`

That means the code still depends on Windows bitmap objects after lookup has already become portable.

## Evidence from the code

### `CG32bitImage`

- `Alchemy/DirectXUtil/CG32bitImage.cpp:644`
  - `CreateFromFile(...)` loads a file into `HBITMAP`
- `Alchemy/DirectXUtil/CG32bitImage.cpp:676`
  - calls `CreateFromBitmap(hImage, hMask, iMaskType, dwFlags)`
- `Alchemy/DirectXUtil/CG32bitImage.cpp:307`
  - `CreateFromBitmap(...)` is the real image ingestion seam today

### `CG16bitImage`

- `Alchemy/DirectXUtil/CG16BitImage.cpp:1947`
  - `CreateFromFile(...)` also loads into `HBITMAP`
- `Alchemy/DirectXUtil/CG16BitImage.cpp:2002`
  - calls `CreateFromBitmap(hImage, hImageMask, ...)`
- `Alchemy/DirectXUtil/CG16BitImage.cpp:1584`
  - `CreateFromBitmap(...)` is the real ingestion seam for the 16-bit image path

### JPEG decode path

- `Alchemy/IntelJPEGUtil/Load.cpp:8`
  - `JPEGLoadFromFile(...)`
- `Alchemy/IntelJPEGUtil/Load.cpp:71`
  - decode path allocates a DIB via `dibCreate24bitDIB(...)`

### BMP decode path

- `Alchemy/Graphics/DIB.cpp:519`
  - `dibLoadFromFile(...)`
- `Alchemy/Graphics/DIB.cpp:483`
  - DIB load path creates an `HBITMAP`

### Underlying DIB helpers are Windows-bound

- `Alchemy/Graphics/DIB.cpp:160`
  - `dibCreate24bitDIB(...)`
- `Alchemy/Graphics/DIB.cpp:127`
  - uses `CreateDIBSection`
- `Alchemy/Graphics/DIB.cpp:51`
  - uses `GetObject`

## Important observation

The rendering surfaces themselves are not the first problem.

`CG32bitImage` already owns an internal RGBA buffer and alpha metadata:

- `Alchemy/DirectXUtil/CG32bitImage.cpp:44` `AllocRGBA(...)`
- `Alchemy/DirectXUtil/CG32bitImage.cpp:271` `Create(...)`
- `Alchemy/DirectXUtil/CG32bitImage.cpp:616` `CreateFromExternalBuffer(...)`

This suggests the best seam is not at presentation time.

The best seam is at image ingestion time.

## Proposed next seam

## 1. Introduce a decoded-image buffer type

Add a tiny intermediate representation for decoded image data that does not mention Win32 types.

Proposed shape:

```cpp
struct SDecodedImage
    {
    int cxWidth = 0;
    int cyHeight = 0;
    int iPitch = 0;
    CG32bitImage::EAlphaTypes AlphaType = CG32bitImage::alphaNone;
    TArray<BYTE> Pixels;
    };
```

Notes:

- exact storage type can vary; the key point is that the result must be platform-neutral
- for milestone 1, 32-bit RGBA is the most useful common representation

## 2. Add a non-Win32 image ingestion API to `CG32bitImage`

The cleanest existing seam is likely next to `CreateFromExternalBuffer(...)`.

Proposed addition:

- `bool CG32bitImage::CreateFromRaw(const void *pPixels, int cxWidth, int cyHeight, int iPitch, EAlphaTypes iAlphaType)`

This could be implemented by:

- allocating `m_pRGBA`
- copying row data into the internal buffer
- setting `m_cxWidth`, `m_cyHeight`, `m_iPitch`, and `m_AlphaType`

This avoids touching any presentation backend.

## 3. Add file decoders that return decoded buffers instead of `HBITMAP`

Introduce a new loader seam for milestone-1 images:

```cpp
ALERROR JPEGLoadToRGBA(const CString &sFilespec, SDecodedImage *retImage);
ALERROR BMPLoadToRGBA(const CString &sFilespec, SDecodedImage *retImage, EBitmapTypes *retiType = NULL);
```

Short-term goal:

- load bytes from file
- decode into a neutral RGBA buffer
- avoid creating DIB sections or any `HBITMAP`

## 4. Compose image + mask at the `CG32bitImage` layer instead of the Win32 bitmap layer

Current code often loads:

- an RGB JPEG image
- a BMP mask

Instead of combining them through `HBITMAP`, do this in platform-neutral memory:

1. decode JPEG into `SDecodedImage`
2. decode BMP mask into `SDecodedImage`
3. apply the mask in code
4. create `CG32bitImage` from final RGBA pixels

This matches the actual data model the engine wants and removes the Windows object dependency.

## Why `CG32bitImage` should go first

The milestone-1 callers we care about are all image/UI/title related and naturally fit the 32-bit path:

- loading screen image
- title/menu icon atlas
- UI button art

Also, `CG32bitImage` already stores RGBA internally, which makes it a better first landing spot than `CG16bitImage`.

## Recommended implementation order

### Phase 1: define the neutral decode seam

1. add `SDecodedImage`
2. add `CG32bitImage::CreateFromRaw(...)` or equivalent
3. do not remove any existing `CreateFromBitmap(...)` code yet

Verification:

- a unit-sized synthetic RGBA buffer can be copied into a `CG32bitImage`

### Phase 2: make JPEG file decode produce neutral memory

1. add `JPEGLoadToRGBA(...)`
2. keep existing `JPEGLoadFromFile(...)` intact for Windows callers
3. route one milestone-1 caller through the new path for proof of viability

Verification:

- `CLoadingSession` title image can reach `CG32bitImage` without creating `HBITMAP`

### Phase 3: make BMP mask decode produce neutral memory

1. add `BMPLoadToRGBA(...)` or a grayscale/alpha variant
2. apply masks in code instead of via `CreateFromBitmap(hImage, hMask, ...)`

Verification:

- `CLoadingSession` stargate image + mask can be constructed without `HBITMAP`

### Phase 4: migrate shared UI atlas callers

1. move `CResourceImageCache::GetImage(...)` to the neutral path
2. move `CButtonBarData.cpp`
3. then migrate remaining non-critical callers

## Minimal caller migration target

The best proof-of-concept caller is still:

- `Transcendence/Transcendence/CLoadingSession.cpp`

Why:

- small number of assets
- explicit image + mask workflow
- directly tied to milestone-1 success
- easier to validate than all of `CVisualPalette`

## What not to do yet

- do not rewrite all image code in `Alchemy/Graphics`
- do not refactor all of `CG16bitImage` first
- do not try to solve every legacy image path in one step
- do not combine this with renderer/presenter work

## Risks

### Alpha semantics may differ from current DIB path

Current `CreateFromBitmap(...)` likely bakes in assumptions about:

- pre-multiplied alpha
- monochrome mask behavior
- 8-bit mask behavior
- transparent-color fallback

These behaviors must be matched carefully for the loading screen and UI atlas path.

### JPEG decoder portability may become a separate issue

If the existing JPEG libraries are themselves Windows-assumptive in the current configuration, we may need one more seam around JPEG decode.

That is still preferable to keeping `HBITMAP` in the path.

## Success criteria for the next seam

The next seam is successful when:

- at least one milestone-1 image caller can create a `CG32bitImage` from file data without producing an `HBITMAP`
- `CLoadingSession` can load title and stargate assets through a platform-neutral path
- the `HBITMAP` dependency is pushed out of the milestone-1 boot path, even if legacy callers still use it elsewhere

## Current implementation status

The following parts of this seam are now implemented:

- `CG32bitImage::CreateFromRaw(...)` exists in `Alchemy/DirectXUtil/CG32bitImage.cpp`
- `SJPEGLoadInfo` exists in `Alchemy/Include/JPEGUtil.h`
- `JPEGLoadToRGBAFromFile(...)` and `JPEGLoadToRGBAFromMemory(...)` exist in `Alchemy/IntelJPEGUtil/Load.cpp`
- `SBMPImageLoad` exists in `Alchemy/Include/Graphics.h`
- `dibLoadToBuffer(...)` and `dibLoadToBufferFromFile(...)` exist in `Alchemy/Graphics/DIB.cpp`
- `Transcendence/Transcendence/CLoadingSession.cpp` now uses the neutral image path for:
  - `Title.JPG`
  - `Stargate.JPG`
  - `StargateMask.BMP`
- `CLoadingSession.cpp` now applies the stargate mask in memory instead of via `CreateFromBitmap(hImage, hMask)`

## What this proves

The milestone-1 loading-screen caller can now load and compose its key images without depending on `HBITMAP`.

That means the proposed seam is viable in the live codebase, at least for the first proof-of-concept caller.

## What still remains

The new neutral path is not yet used by:

- `Mammoth/TSUI/CVisualPalette.cpp`
- `Transcendence/Transcendence/CButtonBarData.cpp`
- `Transcendence/Transcendence/CHelpSession.cpp`
- `Transcendence/Transcendence/CStatsSession.cpp`
- `Transcendence/Transcendence/CModExchangeSession.cpp`

The current BMP-to-buffer implementation is also intentionally narrow:

- focused on 1-bit, 8-bit, and 24-bit BMP inputs
- designed to satisfy milestone-1 loading-screen and mask needs first

## Recommended next implementation step

The best next code step is now:

1. migrate `Mammoth/TSUI/CVisualPalette.cpp` image atlas loading to the neutral image path
2. migrate `Transcendence/Transcendence/CButtonBarData.cpp` to the same path for consistency
3. only then decide whether the remaining non-critical callers need to move immediately

This keeps the work focused on milestone-1 title/menu bring-up while building on the proof-of-concept that now exists in `CLoadingSession.cpp`.
