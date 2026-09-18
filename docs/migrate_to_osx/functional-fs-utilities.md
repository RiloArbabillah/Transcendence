# Utilitas Fungsional & Filesystem macOS

Dokumen ini menjelaskan utilitas publik dan keputusan cakupan yang dihasilkan oleh Fase 2
register temuan port macOS (`PDR-007`..`PDR-015`): pembuat DIB, kursor dan konversi
koordinat, waktu file, penyalinan file, dan satu root app-data.

## Document Status

- Version: v1.0
- Last Updated: 2026-09-19
- Branch: `osx`
- Terkait: `port-defect-register.md` (`PDR-007`..`PDR-015`),
  `platform-input-utilities.md`, `compat-layer-bugfix-plan.md`

## Ruang Lingkup

Fase 2 menutup kegagalan senyap pada jalur fungsional macOS:

- `Alchemy/Graphics/DIBSDL.cpp` — pembuat DIB yang sebelumnya selalu `ERR_FAIL`.
- `Alchemy/Include/Kernel.h` — shim `ShowCursor`/`SetCapture`/`ReleaseCapture`/
  `ScreenToClient`/`ClientToScreen`/`GetCursorPos`/`SetCursorPos` dan makro `MCIWnd*`.
- `Alchemy/Kernel/Path.cpp` — `SHGetFolderPath`, `GetFileTime`,
  `FileTimeToSystemTime`, `CopyFile`, dan waktu tulis pada `WIN32_FIND_DATA`.
- `Transcendence/Transcendence/Platform/AppCore.cpp` dan
  `Transcendence/Transcendence/Platform/PlatformInput.cpp` — implementasi SDL di balik
  shim tersebut.

Semua perubahan bersifat platform-netral atau berada di dalam guard macOS
(`TARGET_PLATFORM_MACOS`, `__APPLE__`, `#ifndef _WIN32`), sehingga build Windows tidak
berubah.

## Berkas

| Berkas | Peran |
|---|---|
| `Alchemy/Include/PathCompat.h` | deklarasi publik permukaan path/waktu macOS (`SHGetFolderPath`, `GetFileTime`, `FileTimeToSystemTime`, `CopyFile`) |
| `Alchemy/Kernel/Path.cpp` | implementasi permukaan tersebut + `Kernel::pathGetAppDataRoot()` |
| `Transcendence/Transcendence/Platform/PlatformInput.h` | deklarasi utilitas origin window, kursor, dan konversi layar↔client |
| `Transcendence/Transcendence/Platform/PlatformInput.cpp` | state origin/kursor dan aritmetika konversi (dapat diuji tanpa window) |
| `Transcendence/Transcendence/Platform/AppCore.cpp` | bagian yang memerlukan SDL: posisi window, warp kursor, visibilitas kursor, capture mouse |
| `Transcendence/Tools/PlatformToolStubs.cpp` | jawaban no-op untuk tool command-line yang tidak pernah membuat window |
| `Alchemy/Graphics/DIBSDL.cpp` | pembuat DIB di atas `SDL_Surface` |

## API

### `Kernel::CString Kernel::pathGetAppDataRoot(void)`

Mengembalikan **satu** direktori data aplikasi. Pada macOS nilainya
`~/Library/Application Support/Kronosaur/Transcendence` dan direktori tersebut dibuat bila
belum ada (semantik `mkdir -p`). Pada Windows fungsi ini meneruskan ke
`pathGetSpecialFolder(folderAppData)`.

Utilitas ini adalah kontrak tunggal untuk lokasi data: shim `SHGetFolderPath`
(`CSIDL_APPDATA` dan `CSIDL_LOCAL_APPDATA`), fallback `pathGetSpecialFolder(folderAppData)`,
dan `GetAppLogPath()` pada shell SDL semuanya memakai root yang sama. Sebelum `PDR-015`
ditutup, shim mengembalikan `~/Library/Application Support` tanpa komponen
`Kronosaur/Transcendence`, sehingga save/settings dan log berada di dua pohon berbeda.

### Permukaan path/waktu macOS (`PathCompat.h`)

| Fungsi | Semantik pada macOS |
|---|---|
| `HRESULT SHGetFolderPath(void*, int iCSIDL, void*, DWORD, char *pDest)` | memetakan `CSIDL_*` ke direktori macOS, membuat hasilnya, dan mengembalikan `S_OK`/`E_FAIL` |
| `BOOL GetFileTime(HANDLE, FILETIME*, FILETIME*, FILETIME*)` | mengisi creation/access/write dari `fstat()` atas deskriptor; output boleh `NULL` |
| `BOOL FileTimeToSystemTime(FILETIME*, SYSTEMTIME*)` | konversi 100-ns sejak 1601-01-01 UTC ke kalender UTC; gagal untuk nilai sebelum 1601 |
| `BOOL CopyFile(const char*, const char*, BOOL bFailIfExists)` | `copyfile()` dengan `COPYFILE_EXCL` bila `bFailIfExists` |

`FILETIME` pada port ini adalah `unsigned long long` (jumlah interval 100 nanodetik sejak
1601-01-01 UTC). Konversi epoch Unix ↔ FILETIME memakai selisih 11.644.473.600 detik dan
disimpan pada satu helper, sehingga `GetFileTime`, `FileTimeToSystemTime`, dan
`WIN32_FIND_DATA::ftLastWriteTime` tidak dapat menyimpang satu sama lain.

### Kursor dan konversi koordinat

Win32 berbicara dalam **koordinat layar** pada `GetCursorPos`/`SetCursorPos` dan pada
`ScreenToClient`/`ClientToScreen`, sementara pesan mouse (`WM_MOUSEMOVE`,
`WM_LBUTTONDOWN`, …) membawa **koordinat client**. Layer platform menyimpan posisi client
kursor dan posisi client area window di desktop, lalu melakukan konversi di satu tempat.

| Fungsi | Peran |
|---|---|
| `void PlatformSetWindowOrigin(int x, int y)` | mencatat posisi client area window di desktop (dipanggil shell dari `SDL_GetWindowPosition`) |
| `BOOL PlatformGetWindowOrigin(int *retx, int *rety)` | mengembalikan origin; default `(0,0)` selama belum ada window |
| `void PlatformSetMouseClientPos(int x, int y)` | mencatat posisi kursor dalam koordinat client (dipanggil dari event mouse SDL) |
| `BOOL PlatformGetCursorPos(POINT *pPoint)` | koordinat **layar** (client + origin) |
| `void PlatformSetCursorPos(int x, int y)` | menerima koordinat layar, menerjemahkan ke client, lalu `PlatformWarpMouseInWindow` |
| `BOOL PlatformScreenToClient(HWND, LPPOINT)` / `BOOL PlatformClientToScreen(HWND, LPPOINT)` | konversi shim Win32; mengembalikan `FALSE` bila `lpPoint == NULL` |
| `void PlatformTranslateScreenToClient(POINT*, int xOrigin, int yOrigin)` / `void PlatformTranslateClientToScreen(...)` | helper murni tanpa state; aman terhadap `NULL`; dipakai untuk pengujian unit |
| `void PlatformWarpMouseInWindow(int xClient, int yClient)` | memindahkan kursor OS; no-op pada build tanpa window (tool) |
| `int PlatformShowCursor(BOOL bShow)` | `SDL_ShowCursor(SDL_ENABLE/SDL_DISABLE)`; mengembalikan counter tampilan |
| `BOOL PlatformSetCapture(HWND)` / `BOOL PlatformReleaseCapture(void)` | `SDL_CaptureMouse(SDL_TRUE/SDL_FALSE)` |

Aritmetika konversi sengaja berada di `PlatformInput.cpp` (bukan di shell SDL) agar dapat
diuji tanpa membuat window, dan agar `CScreenMgrSDL::GlobalToLocal`/`LocalToGlobal` memakai
jalur yang sama dengan shim Win32.

### `bool PlatformReportUnsupportedFeature(const char *pszFeature)`

Mencatat fitur Win32 yang sengaja tidak didukung port macOS ke `stderr` **satu kali** per
nama fitur (state `static`), lalu mengembalikan `false` pada pemanggilan berikutnya.
Dipakai agar jalur yang tidak diimplementasikan tetap terlihat di log alih-alih gagal
diam-diam, tanpa membanjiri log dari polling loop. Dua pemakai saat ini:

- `MCIWndCreate`/`MCIWndOpen` (lihat keputusan non-goal video di bawah);
- `dibLoadFromResource` (resource Win32 tidak ada di build macOS; pemanggilnya nol).

### Pembuat DIB (`Alchemy/Graphics/DIBSDL.cpp`)

Pada port ini `HBITMAP` dibacking oleh `SDLBitmap` (lihat `SDLBitmap.h`). Fungsi yang
sebelumnya selalu `ERR_FAIL` sekarang benar-benar membuat bitmap:

| Fungsi | Hasil |
|---|---|
| `dibCreate16bitDIB(cx, cy, &hBitmap, &pPixel)` | `SDL_PIXELFORMAT_RGB565` (5-6-5) |
| `dibCreate24bitDIB(cx, cy, dwFlags, &hBitmap, &pPixel)` | `SDL_PIXELFORMAT_BGR24` |
| `dibCreate32bitDIB(cx, cy, dwFlags, &hBitmap, &pPixel)` | `SDL_PIXELFORMAT_BGRA32` (`bitmapAlpha`) |
| `dibConvertToDDB(hDIB, hPalette, &hBitmap)` | memvalidasi handle lalu mengembalikan handle yang sama (port ini tidak punya DDB terpisah) |
| `dibCrop(hDIB, x, y, cx, cy, &hBitmap)` | hasil 24-bit baru; menolak area di luar sumber |
| `dibLoadFromResource(...)` | tetap `ERR_FAIL` (non-goal) tetapi melaporkan dirinya sekali lewat `PlatformReportUnsupportedFeature` |

`SDLBitmapLookup(void *hBitmap)` ditambahkan sebagai API publik `SDLBitmap.h` agar
`dibCrop`/`dibConvertToDDB` dapat memvalidasi handle tanpa duplikasi tabel pencarian.

**Catatan layout:** `SDL_Surface` selalu *top-down* (baris 0 adalah scanline atas, pitch
positif) dan `SDLBitmapGetInfo` melaporkan itu sebagai `biHeight` negatif. Bitmap yang
dibuat di sini mengikuti layout tersebut, bukan layout *bottom-up* DIB section Win32.
Pointer piksel yang dikembalikan menunjuk baris pertama; pemanggil yang menelusuri
scanline harus memakai stride dari `dibGetInfo`, karena SDL dapat memberi padding di ujung
baris seperti DIB section Win32.

## Keputusan Cakupan (Non-Goal)

### Video/intro MCI (`PDR-008`)

Port macOS tidak memiliki backend MCI. Semua makro `MCIWnd*` tetap mengembalikan
`0`/`nullptr`, tetapi `MCIWndCreate` dan `MCIWndOpen` kini melaporkan dirinya sekali lewat
`PlatformReportUnsupportedFeature`. Getter yang dipanggil dalam polling loop
(`MCIWndGetLength`, `MCIWndGetPosition`, `MCIWndGetMode`, …) sengaja tetap senyap agar log
tidak dibanjiri. **Pemutaran video/intro tetap non-goal** untuk port ini; yang diperbaiki
adalah visibilitas kegagalannya, bukan fiturnya.

### Inventaris paritas audio `CMCIMixer` (`PDR-009`)

`CMakeLists.txt` mengompilasi `Mammoth/TSUI/CMCIMixerStub.cpp` (462 baris, dibungkus
`#ifdef TARGET_PLATFORM_MACOS`); sumber asli `Mammoth/TSUI/CMCIMixer.cpp` (1178 baris) tidak
dikompilasi pada build macOS. Stub **bukan** no-op: ia adalah implementasi `SDL_mixer` yang
mengisi seluruh daftar method `CMCIMixer` (ctor/dtor, `Boot`, `Play`, `PlayFadeIn`, `Stop`,
`SetVolume`, `FadeNow`, `FadeAtPos`, `SetPlayPaused`, `TogglePausePlay`,
`GetCurrentPlayLength`, `GetCurrentPlayPos`, `GetDebugInfo`, `Shutdown`,
`AbortAllRequests`). Perbedaan yang tercatat terhadap versi MCI asli:

| Aspek | `CMCIMixer.cpp` (Windows, tidak dikompilasi) | `CMCIMixerStub.cpp` (macOS) | Dampak |
|---|---|---|---|
| `Boot()` | memulai processing thread MCI | hanya mengembalikan status `g_bInitialized` | tidak ada thread/queue; tidak ada kegagalan yang dapat dilaporkan |
| `AbortAllRequests()` | mengosongkan antrean request | menghentikan dan membebaskan musik saat ini | antrean tidak ada, jadi tidak ada request yang tertinggal |
| `FadeAtPos(iPos)` | menunggu posisi `iPos - FADE_LENGTH` (`FADE_LENGTH = 2000` ms) lalu fade-out | fade-out tetap 500 ms, `iPos` diabaikan | transisi soundtrack berakhir lebih cepat dari yang diminta pemanggil |
| `FadeNow()` | fade-out selesai pada `GetCurrentPlayPos() + FADE_LENGTH` | fade-out tetap 1000 ms | durasi fade berbeda dari Windows |
| `GetCurrentPlayPos(dwTimeout)` | menunggu hasil dari processing thread dengan timeout | membaca posisi SDL langsung, `dwTimeout` diabaikan | tidak ada timeout karena tidak ada thread |
| `GetDebugInfo()` | hanya mengisi baris di bawah `DEBUG_SOUNDTRACK_STATE`, memakai `retLines->Insert(...)` | selalu mengisi dua baris | kebocoran `retLines->Insert(*new CString(buf))` **sudah diperbaiki** pada Fase 2 (lihat di bawah) |
| Notifikasi posisi | memposting `cmdSoundtrackUpdatePlayPos` (baris 478) | tidak pernah memposting | UI yang menunggu update posisi tidak menerima notifikasi |

`SetPlayPaused(bool bPlay)` stub sudah **sesuai** semantik asli (pause saat `bPlay == false`,
resume saat `bPlay == true`), jadi tidak diubah.

Perbaikan yang sudah dilakukan pada Fase 2 adalah yang berisiko langsung dan dapat
diverifikasi tanpa menjalankan game: `GetDebugInfo` sekarang memakai
`retLines->Insert(CString(buf))`, karena `Insert()` menyalin nilai ke dalam array sehingga
objek hasil `new` akan bocor selamanya. Gap paritas lainnya (durasi fade, `iPos` yang
diabaikan, notifikasi `cmdSoundtrackUpdatePlayPos`) bersifat kualitas transisi audio dan
memerlukan validasi runtime, sehingga tetap dicatat sebagai temuan lanjutan di dalam entri
`PDR-009` pada register, bukan diklaim selesai.

## Verifikasi

Gate untuk perubahan pada berkas-berkas ini:

1. `cmake --preset macos-debug`
2. `cmake --build --preset macos-debug`
3. `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure`
4. `git diff --check`

Tes di `Transcendence/Tests/MacPortabilityTests.cpp` memverifikasi:

- `PlatformTranslateScreenToClient`/`PlatformTranslateClientToScreen` (termasuk `NULL`) dan
  konsistensi `PlatformSetWindowOrigin` + `PlatformSetMouseClientPos` →
  `PlatformGetCursorPos`/`PlatformSetCursorPos`/`PlatformScreenToClient`/
  `PlatformClientToScreen`;
- `GetFileTime` mengisi output non-nol dan `FileTimeToSystemTime` menghasilkan UTC yang
  sesuai timestamp file uji, serta menolak timestamp Unix mentah;
- `CopyFile` gagal saat `bFailIfExists` dan tujuan sudah ada, menimpa saat `FALSE`, dan tetap
  membuat tujuan yang belum ada;
- `SHGetFolderPath(CSIDL_APPDATA)` dan `CSIDL_LOCAL_APPDATA` mengembalikan root yang sama
  dengan `Kernel::pathGetAppDataRoot()` dan berada di bawah
  `Library/Application Support/Kronosaur/Transcendence`;
- `dibCreate24bitDIB`/`dibCrop`/`dibConvertToDDB` mengembalikan `NOERROR` dengan geometri
  yang benar, dan `dibCrop` menolak area di luar sumber.

## Catatan

- Perubahan pada berkas-berkas ini hanya menyentuh jalur macOS; build Visual Studio tidak
  memakai `PlatformInput.cpp`, `PathCompat.h`, atau `DIBSDL.cpp` versi port.
- Temuan yang tidak dapat diverifikasi tanpa menjalankan game (transisi fade audio,
  pemutaran video, warp kursor di window nyata) tetap tercatat sebagai gate runtime terpisah
  dan tidak diklaim selesai oleh dokumen ini.
