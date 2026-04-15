# macOS Port Source Audit Handoff

This document captures the source-level audit completed after reading the migration docs in `docs/migrate_to_osx/`.

It is intended as a handoff for the next agent so they do not need to rediscover the same boot path, blockers, and boundary assumptions.

## Scope of this audit

- Audit the live source tree against `docs/migrate_to_osx/task-backlog.md` and `docs/migrate_to_osx/dependency-matrix.md`
- Identify concrete Windows-specific blockers for the first macOS bring-up
- Trace the real boot path to title/main menu
- Inspect `CInitModelTask`, `CreateIntroSystem`, `CUniverse::IHost`, and the font pipeline because these appear on the critical path for milestone 1

## High-level conclusion

The migration docs are directionally correct.

The current codebase still has hard runtime coupling to:

- Win32 shell and message loop
- DirectX/GDI presentation and font generation
- Win32 resource loading for images and `.dxfn` fonts
- Win32 audio backends (DirectSound/MCI)
- Windows-specific filesystem and current-directory assumptions

However, the menu boot path is still a realistic first milestone because it does not require full gameplay initialization. It does require a real universe load, a real intro system, and a working font/resource path.

## Source tree areas relevant to the port

- `Alchemy/` - low-level libraries; some code is portable, some is strongly Windows-bound
- `Mammoth/TSUI/` - human interface layer; contains Win32 run loop and UI runtime
- `Mammoth/TSE/` - game engine; more portable overall, but depends on host callbacks and some font assumptions
- `Transcendence/Transcendence/` - application/controller/session code; title/menu and app boot live here
- `docs/migrate_to_osx/` - planning docs for the port

## Concrete blocker audit by subsystem

### Platform shell / app lifecycle

These are direct Win32 blockers and cannot be used unchanged for macOS:

- `Transcendence/Transcendence/Main.cpp:28` uses `WinMain`, `HINSTANCE`, `MessageBox`
- `Mammoth/TSUI/Run.cpp:240`-`Run.cpp:357` routes the entire app through `WM_*` messages
- `Mammoth/TSUI/Run.cpp:414` `CHumanInterface::WMCreate` initializes the runtime from a Win32 `HWND`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp:694` `WMCreate` performs app-specific init tied to the Windows window lifecycle
- `Transcendence/Transcendence/Transcendence.h:532` exposes `HWND` and message-style methods in `CTranscendenceWnd`

Implication:

- The macOS path needs a new shell, likely SDL-based, that feeds equivalent lifecycle, input, timer, and frame callbacks into existing HI/session code

### Rendering / presentation

These are still tightly bound to DirectX and old screen abstractions:

- `Alchemy/Include/DirectXUtil.h:13` includes `d3d9.h`, `ddraw.h`, `dsound.h`, and related Windows headers
- `Alchemy/DirectXUtil/CDXScreen.cpp:8` links against `d3d9.lib`
- `Mammoth/TSUI/CHumanInterface.cpp:449` depends on `m_ScreenMgr.CheckIsReady()`
- `Mammoth/TSUI/CHumanInterface.cpp:486` and `:487` call `BltScreen()` and `FlipScreen()`
- `Transcendence/Transcendence/PreComp.h:5` includes `DirectXUtil.h`, which drags Windows rendering dependencies into the app path

Implication:

- A new presentation backend is required for macOS
- The first workable approach should preserve software rendering and upload the framebuffer to Metal, consistent with the docs

### Graphics / GDI / bitmap path

These remain strongly Windows-based:

- `Alchemy/Graphics/DIB.cpp:14` uses `BITMAPINFOHEADER`, `HBITMAP`, `HPALETTE`, `HDC`, and DIB APIs
- `Alchemy/Graphics/GDI.cpp:8` uses `HDC`, `RECT`, `COLORREF`, `ExtTextOut`
- `Alchemy/DirectXUtil/CG16bitFont.cpp:325` rasterizes fonts via GDI

Implication:

- Some graphics code can be reused, but DIB/GDI-backed code paths must be replaced or bypassed for macOS

### Audio

These are Windows-specific and not required for the first visible menu if stubbed carefully:

- `Alchemy/DirectXUtil/Sound.cpp:7` includes `dsound.h`
- `Alchemy/DirectXUtil/Sound.cpp:111` uses MCI window APIs
- `Mammoth/TSUI/CMCIMixer.cpp:35` and later uses Win32 hidden windows, events, and MCI playback

Implication:

- Audio parity can be deferred
- The first menu bring-up only needs non-crashing sound manager and soundtrack stubs

### Filesystem / save / resource path

Current code still assumes Windows-style app data and current directory behavior:

- `Transcendence/Transcendence/CGameSettings.cpp:163` uses current-directory-writable logic before falling back to app data
- `Transcendence/Transcendence/CGameSettings.cpp:174` uses `pathGetSpecialFolder(folderAppData)` and `Kronosaur\Transcendence`
- `Transcendence/Transcendence/CTranscendenceController.cpp:509` and `:515` still assume Windows app data / `SetCurrentDirectory`
- `Mammoth/TSUI/CUserSettings.cpp:76` follows similar logic

Implication:

- The macOS platform layer needs explicit app support, save, and bundle resource paths
- The port should not depend on process working directory

### Win32 resource usage

Title/menu and other UI sessions still load assets from `.rc` resources:

- `Transcendence/Transcendence/CLoadingSession.cpp:29` loads `IDR_TITLE_IMAGE` via `JPEGLoadFromResource`
- `Transcendence/Transcendence/CLoadingSession.cpp:43` loads `IDR_STARGATE_IMAGE`
- `Transcendence/Transcendence/CLoadingSession.cpp:51` loads `IDR_STARGATE_MASK` via `dibLoadFromResource`
- `Transcendence/Transcendence/CStatsSession.cpp:99` loads background JPEG from resource
- `Transcendence/Transcendence/CHelpSession.cpp:81` loads help background from resource
- `Transcendence/Transcendence/CButtonBarData.cpp:125` loads game button images from resource

Implication:

- For macOS, the resource loader must become file- or bundle-based instead of `.rc`-based

## Real boot path to title/main menu

The boot path is heavier than a simple session switch.

### Boot sequence

1. `Transcendence/Transcendence/Main.cpp:28`
   - Creates `CTranscendenceController`
   - Calls `CHumanInterface::Run(...)`

2. `Mammoth/TSUI/Run.cpp:35`
   - Calls controller boot path (`HIBoot` / `OnBoot`)

3. `Transcendence/Transcendence/CTranscendenceController.cpp:496` `OnBoot`
   - Parses command line
   - Loads settings
   - Sets working directory and logging behavior
   - Calls `CUniverse::Boot()`

4. `Mammoth/TSUI/Run.cpp:414` `CHumanInterface::WMCreate`
   - Initializes sound manager
   - Initializes screen manager
   - Initializes background processors
   - Initializes visual palette (`m_Visuals.Init(NULL, retsError)`)

5. `Transcendence/Transcendence/CTranscendenceController.cpp:2062` `OnInit`
   - Creates `g_pTrans = new CTranscendenceWnd(...)`
   - Initializes model state
   - Builds extension/save folder list
   - Starts `CInitModelTask`
   - Adds timers
   - Shows `CLoadingSession`
   - Initializes soundtrack manager
   - Calls `g_pTrans->WMCreate(...)`

6. `Transcendence/Transcendence/CTranscendenceController.cpp:743`
   - Receives `modelInitDone`
   - On success, shows `CIntroSession(..., isOpeningTitles)`

7. `Transcendence/Transcendence/CIntroSession.cpp:1153` `OnInit`
   - Calls `CreateIntroSystem()`
   - Calls `g_pTrans->StartIntro(this)`

8. `Transcendence/Transcendence/IntroScreen.cpp:1395` `StartIntro`
   - Builds title/menu UI
   - Builds animations and button bar
   - Enters intro state

### Important consequence

The first visible title/menu frame depends on all of the following being functional:

- shell creation
- visual palette init
- loading session resources
- background universe init
- legacy window/app host init
- intro system creation
- font lookup and UI assets

## `CInitModelTask` and what it really requires

### `CInitModelTask`

- `Transcendence/Transcendence/BackgroundTasks.h:49`
  - `CInitModelTask` calls `m_Model.InitBackground(...)`

### `InitBackground`

- `Transcendence/Transcendence/CTranscendenceModel.cpp:1105`
  - Applies graphics settings
  - Chooses adventure
  - Computes enabled extensions
  - Calls `LoadUniverse(...)`
  - Calls `LoadHighScoreList(...)`

### `LoadUniverse`

- `Transcendence/Transcendence/CTranscendenceModel.cpp:1471`
  - Sets debug mode on the universe
- `Transcendence/Transcendence/CTranscendenceModel.cpp:1472`
  - Injects HI sound manager pointer into the universe
- `Transcendence/Transcendence/CTranscendenceModel.cpp:1479`
  - Sets `Ctx.pHost = g_pTrans`
- `Transcendence/Transcendence/CTranscendenceModel.cpp:1503`
  - Calls `m_Universe.Init(Ctx, retsError)`

### Consequence

Title/menu boot requires a real universe load. It is not enough to fake a loading screen and then show a static intro UI.

Minimum engine requirements before intro:

- working `CUniverse::Boot()`
- working host object (`g_pTrans`) implementing required `IHost` behavior
- enough collection/extension discovery to bind the default adventure
- enough sound manager surface to avoid crashes

## `CreateIntroSystem` and what intro actually needs

- `Transcendence/Transcendence/CIntroSession.cpp:291` `CreateIntroSystem`
- `:303` calls `g_pUniverse->CreateEmptyStarSystem(...)`
- `:309` sets current system
- `:313` fires `OnGlobalIntroStarted`
- If nothing sets a POV during that event:
  - `:319` finds player sovereign
  - `:320` finds enemy sovereign
  - `:326` and `:329` create ships
  - `:347` and `:349` order ships to attack each other
  - `:356` sets POV
  - `:367` marks library bitmaps

### Consequence

The intro screen needs:

- a loaded design collection
- current adventure and event hooks
- sovereign definitions
- ship class definitions
- object/system creation that actually works
- image library bookkeeping

This is still less than full gameplay boot, but it is much more than static menu layout.

## `CUniverse::IHost` audit for menu boot

### Interface definition

- `Mammoth/Include/TSEUniverse.h:287`

Host methods:

- `ConsoleClear`
- `ConsoleOutput`
- `CreatePlayerController`
- `CreateShipController`
- `DebugOutput`
- `FindCommandKey`
- `FindFont`
- `GameOutput`
- `GetColor`
- `GetFont`
- `LogOutput`
- `OnSaveGame`
- `PostAchievement`

### Host methods actually seen on the critical path

- `Mammoth/TSE/CUniverse.cpp:414` uses `m_pHost->CreateShipController(sAI)`
- `Mammoth/TSE/CUniverse.cpp:1439` uses `m_pHost->FindFont(...)` in `InitFonts`
- `Mammoth/TSE/CUniverse.cpp:564` uses `m_pHost->DebugOutput(...)` in debug logging path only
- `Mammoth/TSE/CUniverse.cpp:2110` uses `m_pHost->CreatePlayerController()` when loading a save game, not needed for intro boot
- `Mammoth/TSE/CUniverse.cpp:2758` uses `m_pHost->PostAchievement(...)`, not needed for title/menu bring-up

### Practical classification for milestone 1

Required or likely required:

- `FindFont`
- `CreateShipController` (at least enough for any controller names used during intro path; built-in TSE fallbacks already exist for many AI controllers)

Safe to no-op for first menu boot:

- `ConsoleClear`
- `ConsoleOutput`
- `DebugOutput`
- `GameOutput`
- `PostAchievement`
- `OnSaveGame`

Not required for intro boot, but needed later:

- `CreatePlayerController`
- `FindCommandKey`

### Consequence

The minimum macOS host bridge can be much smaller than the current `CTranscendenceWnd`, but it must have a real font path and enough controller creation for intro objects.

## Font pipeline audit

This is one of the most important blockers for the menu bring-up.

### Current visual font flow

- `Mammoth/TSUI/Run.cpp:483`
  - `m_Visuals.Init(NULL, retsError)`
- `Mammoth/TSUI/CVisualPalette.cpp:409`
  - initializes colors, fonts, and UI images
- `Mammoth/TSUI/CVisualPalette.cpp:321`
  - `GetFont` looks up by name from `m_Font[]`
- `Transcendence/Transcendence/GameOutput.cpp:63`
  - `CTranscendenceWnd::FindFont` delegates to `g_pHI->GetVisuals().GetFont(...)`
- `Mammoth/TSE/CUniverse.cpp:1439`
  - universe asks host for fonts during `InitFonts`

### Font table contents

`Mammoth/TSUI/CVisualPalette.cpp:92` defines two kinds of fonts:

System-created fonts:

- `Small`
- `SmallBold`
- `Medium`
- `MediumBold`
- `MediumHeavyBold`
- `Large`
- `LargeBold`
- `ConsoleMediumHeavy`

Resource-backed fonts:

- `Header`
- `HeaderBold`
- `SubTitle`
- `SubTitleBold`
- `SubTitleHeavyBold`
- `Title`
- `LogoTitle`

The title/menu path uses the resource-backed fonts extensively.

### Win32-specific font implementation details

#### System font creation

- `Alchemy/DirectXUtil/CG16bitFont.cpp:276` `CG16bitFont::Create`
  - calls `::CreateFont(...)`
  - then `CreateFromFont(hFont)`

#### GDI rasterization path

- `Alchemy/DirectXUtil/CG16bitFont.cpp:325` `CreateFromFont`
  - uses `CreateCompatibleDC`
  - `SelectObject`
  - `GetTextMetrics`
  - `TextOut`
  - `GetCharABCWidths`
  - DIB-backed image extraction

#### `.dxfn` resource loading

- `Alchemy/DirectXUtil/CG16bitFont.cpp:435` `CreateFromResource`
  - uses `FindResource`
  - `LoadResource`
  - `LockResource`
  - then reads the serialized font stream via `ReadFromStream`

#### Universe fallback font path

- `Mammoth/TSE/CUniverse.cpp:1445`
  - if host `FindFont` fails, the universe creates a default Windows font via `CreateFont`

### Good news

The `.dxfn` format itself is not the problem.

- `Alchemy/DirectXUtil/CG16bitFont.cpp:1143` `ReadFromStream`
  - deserializes a font from a stream into internal metrics and image data

This means the port can avoid GDI font generation if it loads `.dxfn` assets directly from files or a bundle instead of Win32 resources.

### Asset/resource mapping discovered

`Transcendence/Transcendence/Transcendence.rc` contains the actual asset mapping, including:

- `DXFN_HEADER` -> `Resources\Header.dxfn`
- `DXFN_HEADER_BOLD` -> `Resources\HeaderBold.dxfn`
- `DXFN_SUBTITLE` -> `Resources\SubTitle.dxfn`
- `DXFN_SUBTITLE_BOLD` -> `Resources\SubTitleBold.dxfn`
- `DXFN_SUBTITLE_HEAVY_BOLD` -> `Resources\SubTitleHeavyBold.dxfn`
- `DXFN_TITLE` -> `Resources\Title.dxfn`
- `JPEG_UI_ICONS` -> `Resources\UIIcons.jpg`
- `BMP_UI_ICONS_MASK` -> `Resources\UIIconsMask.bmp`
- `JPEG_DAMAGE_TYPE_ICONS` -> `Resources\DamageTypes.jpg`

### Consequence

The most promising first porting step is:

- keep using `.dxfn` fonts
- stop loading them through Win32 resources
- load them from files or app bundle resources instead

This avoids building a brand new font rasterizer for milestone 1.

## UI image pipeline note

`CVisualPalette` also loads UI images through Win32 resources:

- `Mammoth/TSUI/CVisualPalette.cpp:505` `JPEGLoadFromResource(...)`
- `Mammoth/TSUI/CVisualPalette.cpp:517` `dibLoadFromResource(...)`

This is relevant because the font problem and UI image problem likely need the same solution shape: a file- or bundle-based resource loader.

## Asset inventory for loading screen and intro menu

The live asset files needed for milestone-1 menu bring-up are present on disk under:

- `Transcendence/Transcendence/Resources/`

### Resource ID to file mapping

The following mappings come from `Transcendence/Transcendence/Transcendence.rc`.

#### Loading screen and title assets

- `IDR_TITLE_IMAGE` -> `Transcendence/Transcendence/Resources/Title.JPG`
- `IDR_STARGATE_IMAGE` -> `Transcendence/Transcendence/Resources/Stargate.JPG`
- `IDR_STARGATE_MASK` -> `Transcendence/Transcendence/Resources/StargateMask.BMP`

Used by:

- `Transcendence/Transcendence/CLoadingSession.cpp:29`
- `Transcendence/Transcendence/CLoadingSession.cpp:43`
- `Transcendence/Transcendence/CLoadingSession.cpp:51`

#### Intro/title menu button bar assets

- `IDR_GAME_BUTTONS_IMAGE` -> `Transcendence/Transcendence/Resources/GameButtonIcons.jpg`

Used by:

- `Transcendence/Transcendence/CButtonBarData.cpp:125`
- consumed by intro button-bar display via `CButtonBarDisplay`

Note:

- No `IDR_GAME_BUTTONS_MASK` mapping appears in `Transcendence/Transcendence/Transcendence.rc`, even though `resource.h` defines the ID
- This should be verified before implementing a file-based loader, but it does not currently block the title/menu audit because only the JPEG mapping is obvious in the current resource file

#### Visual palette UI icon atlas assets

- `JPEG_UI_ICONS` -> `Transcendence/Transcendence/Resources/UIIcons.jpg`
- `BMP_UI_ICONS_MASK` -> `Transcendence/Transcendence/Resources/UIIconsMask.bmp`

Used by `Mammoth/TSUI/CVisualPalette.cpp` for:

- profile icon
- mod exchange icon
- music on/off icons
- cancel/OK arrows
- settings icon
- play/debug icons
- small directional icons
- difficulty icons
- small human genome icons

These are referenced by title/menu and nearby menu flows, including:

- `Transcendence/Transcendence/IntroScreen.cpp:631`
- `Transcendence/Transcendence/IntroScreen.cpp:639`
- `Transcendence/Transcendence/IntroScreen.cpp:646`
- `Transcendence/Transcendence/IntroScreen.cpp:655`
- `Transcendence/Transcendence/IntroScreen.cpp:665`
- `Transcendence/Transcendence/CChooseAdventureSession.cpp:217`
- `Transcendence/Transcendence/CNewGameSession.cpp:325`

#### Visual palette damage icon atlas assets

- `JPEG_DAMAGE_TYPE_ICONS` -> `Transcendence/Transcendence/Resources/DamageTypes.jpg`
- `BMP_DAMAGE_TYPE_ICONS_MASK` -> `Transcendence/Transcendence/Resources/DamageTypesMask.bmp`

These are not required for the very first title frame, but they are needed by help and related UI flows.

Referenced by:

- `Transcendence/Transcendence/CHelpSession.cpp:182`
- `Transcendence/Transcendence/CHelpSession.cpp:217`

#### Resource-backed menu/title fonts

- `DXFN_HEADER` -> `Transcendence/Transcendence/Resources/Header.dxfn`
- `DXFN_HEADER_BOLD` -> `Transcendence/Transcendence/Resources/HeaderBold.dxfn`
- `DXFN_SUBTITLE` -> `Transcendence/Transcendence/Resources/SubTitle.dxfn`
- `DXFN_SUBTITLE_BOLD` -> `Transcendence/Transcendence/Resources/SubTitleBold.dxfn`
- `DXFN_SUBTITLE_HEAVY_BOLD` -> `Transcendence/Transcendence/Resources/SubTitleHeavyBold.dxfn`
- `DXFN_TITLE` -> `Transcendence/Transcendence/Resources/Title.dxfn`
- `DXFN_LOGO_TITLE` -> `Transcendence/Transcendence/Resources/LogoTitle.dxfn`

Loaded by:

- `Mammoth/TSUI/CVisualPalette.cpp:433`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp:754`
- `Transcendence/Transcendence/CTranscendenceWnd.cpp:760`

### Additional nearby assets not strictly required for first title frame

These assets are in the same folder and are used by adjacent sessions that likely appear soon after intro/menu bring-up:

- `Transcendence/Transcendence/Resources/Help Screen.jpg`
- `Transcendence/Transcendence/Resources/SelectShipIcons.jpg`
- `Transcendence/Transcendence/Resources/SelectShipIconsMask.bmp`
- `Transcendence/Transcendence/Resources/Game Stats Screen.jpg`
- `Transcendence/Transcendence/Resources/GenericExtensionSmall.jpg`
- `Transcendence/Transcendence/Resources/IconDisplay.JPG`

Used by:

- `Transcendence/Transcendence/CHelpSession.cpp:81`
- `Transcendence/Transcendence/CStatsSession.cpp:99`
- `Transcendence/Transcendence/CModExchangeSession.cpp:445`

### Minimum asset subset for milestone-1 menu boot

Strict minimum to show loading screen and intro/title menu with current logic:

- `Transcendence/Transcendence/Resources/Title.JPG`
- `Transcendence/Transcendence/Resources/Stargate.JPG`
- `Transcendence/Transcendence/Resources/StargateMask.BMP`
- `Transcendence/Transcendence/Resources/GameButtonIcons.jpg`
- `Transcendence/Transcendence/Resources/UIIcons.jpg`
- `Transcendence/Transcendence/Resources/UIIconsMask.bmp`
- `Transcendence/Transcendence/Resources/Header.dxfn`
- `Transcendence/Transcendence/Resources/HeaderBold.dxfn`
- `Transcendence/Transcendence/Resources/SubTitle.dxfn`
- `Transcendence/Transcendence/Resources/SubTitleBold.dxfn`
- `Transcendence/Transcendence/Resources/SubTitleHeavyBold.dxfn`
- `Transcendence/Transcendence/Resources/Title.dxfn`
- `Transcendence/Transcendence/Resources/LogoTitle.dxfn`

Practical near-minimum if moving slightly beyond first title/menu and into surrounding UI:

- all of the above
- `Transcendence/Transcendence/Resources/DamageTypes.jpg`
- `Transcendence/Transcendence/Resources/DamageTypesMask.bmp`
- `Transcendence/Transcendence/Resources/Help Screen.jpg`
- `Transcendence/Transcendence/Resources/SelectShipIcons.jpg`
- `Transcendence/Transcendence/Resources/SelectShipIconsMask.bmp`

### Recommended loader work based on the inventory

The first file-based resource layer for macOS should be able to resolve at least three categories:

- `.dxfn` font files
- JPEG image files
- BMP mask files

The simplest migration path appears to be:

1. add a deterministic mapping from current resource IDs/names to files under `Transcendence/Transcendence/Resources/`
2. add stream/file-based font loading for `.dxfn`
3. add file-based image loading to replace `JPEGLoadFromResource` and `dibLoadFromResource`
4. make `CVisualPalette::Init` and `CLoadingSession` consume the new loader first on macOS

## Recommended strategy based on the audit

### What should happen first

1. Replace resource loading assumptions for fonts and UI images with file/bundle-based lookup
2. Preserve `.dxfn` usage for menu/title fonts
3. Avoid touching full gameplay systems until title/menu boot works
4. Keep the initial macOS rendering path compatibility-first: software rendering plus framebuffer upload to Metal

### Smallest credible technical slice

1. Introduce file-based loading for `.dxfn` fonts
2. Introduce file-based loading for loading-screen and UI images
3. Ensure `CVisualPalette::Init` succeeds without Win32 resources
4. Ensure host `FindFont` works via initialized visuals
5. Ensure `LoadUniverse()` can initialize fonts without touching GDI fallback
6. Replace the Win32 shell with an SDL-driven shell that can feed the same session/update loop

## Suggested next investigation items

If another agent picks this up, the most useful next steps are:

1. inventory the actual `Resources/` asset files required for loading screen, title/menu, and visual palette
2. design a file- or bundle-based loader to replace `CreateFromResource`, `JPEGLoadFromResource`, and `dibLoadFromResource`
3. determine the minimum set of `CTranscendenceWnd` behavior that must survive as a host/game bridge on macOS
4. only after that, scaffold the first macOS `CMake` target graph around the actual milestone-1 subset

## Summary for the next agent

The first milestone is not blocked by gameplay complexity; it is blocked by runtime infrastructure:

- shell creation
- presentation backend
- resource loading
- font loading
- enough universe initialization to create the intro system

The most leverage appears to be in converting the font and UI resource pipeline away from Win32 resources while keeping existing `.dxfn` assets and session logic intact.
