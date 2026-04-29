# Transcendence macOS Port - Documentation

## Overview

This document describes the current state and remaining work required to build and run Transcendence on macOS (Apple Silicon).

## Build Status

**Current State:** Partial compilation - links fail at final link stage with ~50 missing symbol groups.

**Last Working Commit:** `e7e576e1` - fix: continue macOS port - add more source files and stubs

## What Works

### Compilation Phase
- Alchemy/Kernel library compiles with extensive Windows API stubs
- Alchemy/CodeChain library compiles
- Alchemy/XMLUtil library compiles (partially)
- Alchemy/Graphics library compiles (partially - Windows GDI files excluded)
- Mammoth/TSE library compiles (partially)
- Mammoth/TSUI library compiles (partially)
- Transcendence app compiles (partially)

### Libraries Successfully Built
- `libalchemy_kernel.a`
- `libalchemy_codechain.a`
- `libalchemy_xmlutil.a`
- `libalchemy_jpeg.a`
- `libalchemy_graphics.a` (partial)
- `libmammoth_tse.a` (partial)
- `libmammoth_tsui.a` (partial)

### Key Fixes Applied
1. Windows API stubs in `Kernel.h` (socket, registry, file I/O, memory, threading)
2. CString operator== for const char* comparisons
3. CCItemPool template explicit instantiations
4. VoronoiGenerator.h LOG macros fixed
5. CHTML.h header created
6. DIBSDL.cpp stub for dibLoadFromBlock
7. CGDrawStub.cpp partial stub implementations
8. lodepng library integration

## What Doesn't Work

### Linking Errors
The final link step fails with ~50 missing symbol groups:

#### 1. GUI/Area Classes (Critical)
```
AGArea::SignalAction(unsigned int)              - MISSING
AGArea::AddShadowEffect()                      - MISSING
AGArea::Init(AGScreen*, IAreaContainer*, ...)  - MISSING
AGArea::SetRect(RECT const&)                  - MISSING
AGArea::ShowHide(bool)                         - MISSING
AGArea::AGArea()                               - MISSING
```
**Status:** AGArea files exist but excluded from build due to compilation issues.

#### 2. CGDraw Functions (Critical)
```
CGDraw::LineBroken(...)          - MISSING
CGDraw::LineDotted(...)          - MISSING
CGDraw::CircleImage(...)        - MISSING
CGDraw::RectOutline(...)        - MISSING
CGDraw::RingGlowing(...)        - MISSING
CGDraw::RoundedRect(...)        - MISSING
CGDraw::LineGradient(...)      - MISSING
CGDraw::CircleOutline(...)      - MISSING
```
**Status:** DrawLine.cpp excluded due to template friend declaration bugs.

#### 3. HUD Classes (High Priority)
```
CShieldHUDDefault::CShieldHUDDefault()        - MISSING
CWeaponHUDCircular::CWeaponHUDCircular()      - MISSING
CReactorHUDCircular::CReactorHUDCircular()    - MISSING
```
**Status:** These are game-specific UI components.

#### 4. CIconLabelBlock (Medium Priority)
```
CIconLabelBlock::Add(SLabelDesc const&)        - MISSING
CIconLabelBlock::Format(int)                   - MISSING
```
**Status:** File not found in codebase.

#### 5. CNoiseGenerator (Medium Priority)
```
CNoiseGenerator::CNoiseGenerator(int)         - MISSING
CNoiseGenerator::~CNoiseGenerator()            - MISSING
```
**Status:** File not found in codebase.

#### 6. CExtensionListMap (Low Priority)
```
CExtensionListMap::WriteAsXML(...)            - MISSING
CExtensionListMap::ReadFromXML(...)           - MISSING
```
**Status:** File not found in codebase.

## Root Causes

### 1. Windows GDI/DirectX Dependencies
Many graphics files have deep dependencies on:
- DirectDraw/Direct3D interfaces
- GDI (Graphics Device Interface) functions
- Win32k.sys calls

### 2. C++ Template Friend Declaration Bugs
Files like `DrawLine.cpp`, `DrawRegion.cpp`, `BlendModes.cpp` have template friend declarations that fail on modern Clang:

```cpp
// BROKEN - compiler error
friend TRegionPainter32;  // Should be friend TRegionPainter32<TFillRegionSolid<BLENDER>>;

// ACTUAL CODE IN DrawRegionImpl.h:20
friend TRegionPainter32;  // ERROR: use of class template requires template arguments
```

### 3. Missing Source Files
Some referenced classes don't exist in the codebase at all:
- `CIconLabelBlock`
- `CNoiseGenerator`
- `CExtensionListMap`
- Various HUD classes

### 4. Incomplete SDL Port
The SDL-based graphics backend is incomplete. Files like `CScreenMgrSDL.cpp` exist but many drawing operations aren't implemented.

## What's Needed to Complete

### Option 1: Extensive Stub Implementation (Easier but Limited)
1. Create stub implementations for all 50+ missing symbols
2. Disable or stub out complex graphics features
3. Focus on getting a minimal game that can launch

**Effort:** ~1-2 weeks of work
**Outcome:** Limited functionality - game UI won't render properly

### Option 2: Complete SDL Graphics Port (Recommended)
1. Implement SDL2-based equivalents for all graphics functions
2. Fix template friend declaration bugs in source files
3. Create proper CGDraw implementations using SDL2 rendering
4. Implement AGArea/GUI system using SDL2

**Effort:** ~2-3 months of work
**Outcome:** Fully functional game UI on macOS

### Option 3: Hybrid Approach
1. Add missing files incrementally
2. Focus on core game functionality first
3. Defer advanced graphics until later

**Effort:** ~1 month initial, ongoing
**Outcome:** Playable game with basic graphics

## File Exclusions (Windows GDI)

The following files are currently excluded from the build due to Windows GDI dependencies:

```
Alchemy/Graphics/CGBitmap.cpp      - GDI dependent
Alchemy/Graphics/CGFont.cpp        - GDI dependent
Alchemy/Graphics/CGResourceFile.cpp - GDI dependent
Alchemy/Graphics/DIB.cpp           - GDI dependent (replaced with DIBSDL.cpp stub)
Alchemy/Graphics/GDI.cpp           - GDI dependent
Alchemy/Graphics/Regions.cpp       - GDI dependent
Alchemy/DirectXUtil/DrawLine.cpp  - Template bugs
Alchemy/DirectXUtil/DrawRegion.cpp - Template bugs
Alchemy/DirectXUtil/BlendModes.cpp - Template bugs
Alchemy/DirectXUtil/DrawClouds.cpp  - Template bugs
Alchemy/DirectXUtil/DrawFill.cpp    - Template bugs
Alchemy/DirectXUtil/DrawRect.cpp    - Template bugs
Alchemy/DirectXUtil/DrawCircle.cpp  - Template bugs
Alchemy/DirectXUtil/CG16bitFont.cpp - GDI dependent
```

## Build Commands

```bash
# Configure and build
cd build
cmake ..
make -j4

# Check specific errors
make 2>&1 | grep "error:"

# Check linker errors
make 2>&1 | grep "referenced from"
```

## Key Files Modified

| File | Change |
|------|--------|
| `Alchemy/Include/Kernel.h` | Windows API stubs for sockets, registry, file I/O, memory, threading |
| `Alchemy/Include/KernelString.h` | CString operator== for const char* |
| `Alchemy/CodeChain/CCItemPool.cpp` | Template explicit instantiation order |
| `Alchemy/Include/CHTML.h` | New header for HTML entity translation |
| `Alchemy/Graphics/DIBSDL.cpp` | Stub for dibLoadFromBlock |
| `Alchemy/DirectXUtil/CGDrawStub.cpp` | Stub implementations for CGDraw |
| `CMakeLists.txt` | Added many source files |

## Next Steps

1. **Immediate:** Fix CGDraw functions by uncommenting and fixing DrawLine.cpp
2. **Short-term:** Add missing AGArea implementation
3. **Medium-term:** Implement SDL-based CGDraw functions
4. **Long-term:** Complete GUI system port

## References

- Original Windows build: Visual Studio 2022, `Transcendence.sln`
- SDL2 port guide: See project documentation
- Graphics subsystem: Alchemy/DirectXUtil and Alchemy/Graphics directories
