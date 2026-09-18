# Utilitas Platform/Event & Performa macOS

Dokumen ini menjelaskan utilitas publik dan keputusan yang dihasilkan oleh Fase 3 register
temuan port macOS (`PDR-016`..`PDR-027`): antrean pesan, packing payload pesan, skema
timer, jalur crash handler, dan dua workaround render yang kini dapat dikonfigurasi.

## Document Status

- Version: v1.0
- Last Updated: 2026-09-19
- Branch: `osx`
- Terkait: `port-defect-register.md` (`PDR-016`..`PDR-027`),
  `platform-input-utilities.md`, `functional-fs-utilities.md`, `compat-layer-bugfix-plan.md`

## Ruang Lingkup

Fase 3 menyelaraskan semantik message/event platform macOS dengan Win32 dan menghapus
pemaksaan performa yang tidak lagi diperlukan:

- `Transcendence/Transcendence/Platform/PlatformMessage.{h,cpp}` — **berkas baru**: antrean
  pesan dan helper payload.
- `Transcendence/Transcendence/Platform/AppCore.{h,cpp}` — pemompaan event SDL, registry
  timer, dan crash handler.
- `Transcendence/Transcendence/GameUIBridge.cpp` — konsumen antrean pesan.
- `Alchemy/Include/Kernel.h` — `VirtualAlloc`, `posixResolvePathCase`, `ShellExecute`, dan
  tipe/entry point pesan.
- `Alchemy/Graphics/DIBSDL.cpp`, `Alchemy/Include/SDLBitmap.h`,
  `Alchemy/DirectXUtil/SDLBitmap.cpp` — deteksi monokrom terbatas.
- `Mammoth/TSE/CSFXOptions.cpp`, `Mammoth/TSE/CSystem.cpp` — workaround render.

Semua perubahan bersifat platform-netral atau berada di dalam guard macOS
(`TARGET_PLATFORM_MACOS`, `__APPLE__`, `#ifndef _WIN32`), sehingga build Windows tidak
berubah.

## Berkas

| Berkas | Peran |
|---|---|
| `Transcendence/Transcendence/Platform/PlatformMessage.h` | deklarasi helper payload `PlatformPackPoint`/`PlatformUnpackPoint`/`PlatformPackMouseWheel`/`PlatformUnpackMouseWheelDelta`/`PlatformUnpackMouseWheelFlags` |
| `Transcendence/Transcendence/Platform/PlatformMessage.cpp` | pemilik antrean pesan (`std::deque` + mutex), timestamp, dan dispatch sinkron |
| `Alchemy/Include/Kernel.h` | `struct SPlatformMessage`, `tagMSG`/`MSG`, entry point `PlatformPostMessage`/`PlatformPeekMessage`/`PlatformSendMessage`, shim `PostMessage`/`PeekMessage`/`SendMessage`, `PlatformEnvFlagIsOff`/`PlatformRenderWorkaroundEnabled` |
| `Transcendence/Transcendence/Platform/AppCore.cpp` | `App_PumpEvents` (produsen pesan), `PlatformSetTimerCompat`/`PlatformKillTimerCompat`, crash handler |
| `Transcendence/Transcendence/GameUIBridge.cpp` | `DispatchGameUIMessage` + `UpdateGameUI` (konsumen antrean) |

`PlatformMessage.cpp` sengaja dipisah dari layer window SDL agar antrean dan helper payload
dapat diuji tanpa window, tanpa event loop, dan tanpa inisialisasi subsistem video. Berkas
ini dikompilasi pada target `platform_sdl`, `mac_portability_tests`, `asset_validation_tool`,
dan `transcompiler_tool`; proyek Visual Studio tidak merujuknya.

## Antrean Pesan

### `struct SPlatformMessage`

Satu pesan Win32 lengkap, dengan tipe lebar penuh:

```cpp
struct SPlatformMessage
	{
	void* hwnd;
	unsigned int message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
	};
```

`tagMSG`/`MSG` adalah struktur dengan field yang sama, sehingga `PeekMessage` dapat menyalin
seluruh isi pesan ke `MSG` milik pemanggil.

### `bool PlatformPostMessage(int msg, WPARAM wParam, LPARAM lParam)`

Produsen (event pump, thread timer SDL, dan callback penyelesaian task) memanggil ini.
Sebelum masuk antrean, setiap field diisi (`FillMessage`):

- `hwnd` ← `PlatformGetMessageWindow()` (pointer `SDL_Window` dipakai sebagai handle buram;
  `NULL` pada build headless);
- `message`, `wParam`, `lParam` ← argumen, **tanpa penyempitan ke `int`** (`PDR-016`);
- `time` ← `clock_gettime(CLOCK_MONOTONIC)` dalam milidetik;
- `pt` ← `PlatformGetCursorPos()` dalam koordinat layar, sesuai `MSG.pt` Win32 (`PDR-017`).

Selalu mengembalikan `true`; antrean tidak pernah menolak pesan.

### `int PlatformPeekMessage(SPlatformMessage* pMessage)`

Mengeluarkan satu pesan dari depan antrean (`PM_REMOVE`-style) dan menyalin **seluruh** field
bila `pMessage` tidak `NULL`. Mengembalikan `1` bila ada pesan, `0` bila antrean kosong.

### `bool PeekMessage(void* pMsg, HWND, UINT, UINT, UINT)` (shim `Kernel.h`)

Memanggil `PlatformPeekMessage` lalu menyalin keenam field ke `MSG` milik pemanggil. Sebelum
`PDR-017` ditutup, hanya `message`/`wParam`/`lParam` yang diisi sehingga `msg.time` dan
`msg.pt` berisi sampah.

### `void PlatformClearMessageQueue(void)`

Mengosongkan antrean. Dipakai oleh `App_Shutdown` dan oleh unit test agar satu tes tidak
mewarisi pesan dari tes sebelumnya.

### `LRESULT PlatformSendMessage(HWND, UINT Msg, WPARAM, LPARAM)`

Kontrak Win32 `SendMessage` bersifat **sinkron** (`PDR-018`). Urutan yang dipakai:

1. `WM_CLOSE`/`WM_DESTROY` → jalankan `PlatformCloseRequest` yang terdaftar (bila ada).
2. Dispatcher yang terdaftar (`PlatformSetMessageDispatch`) → jalankan dan kembalikan
   `LRESULT`-nya. Handler selesai **sebelum** fungsi kembali.
3. Tidak ada dispatcher (command-line tool dan unit test) → jatuh ke `PlatformPostMessage`,
   yaitu apa yang akan dilakukan message loop.

### Registrasi dari shell

| Entry point | Dipakai oleh |
|---|---|
| `PlatformSetMessageDispatch(PlatformMessageDispatch)` | `GameUIBridge.cpp` → `DispatchGameUIMessage` |
| `PlatformSetCloseRequest(PlatformCloseRequest)` | `GameUIBridge.cpp` → `RequestGameClose` |
| `PlatformSetMessageWindow(void*)` | `GameUIBridge.cpp` (pointer `SDL_Window`) |

Ketiganya di-reset ke `NULL` oleh `CleanUpGameUI` dan `App_Shutdown` agar tidak ada pointer
menggantung antar sesi.

## Helper Payload

Helper ini adalah **satu-satunya** tempat payload mouse dipaketkan dan dibongkar, sehingga
kedua sisi tidak dapat menyimpang. Semuanya fungsi murni dan diuji unit tanpa window.

### `DWORD PlatformPackPoint(int x, int y)` / `void PlatformUnpackPoint(DWORD, int*, int*)`

`Win32` memaketkan sepasang koordinat ke satu `LPARAM` sebagai dua nilai **signed 16-bit**
dan membongkarnya dengan `GET_X_LPARAM`/`GET_Y_LPARAM`, yang melakukan *sign-extension*.
`PlatformUnpackPoint` meniru hal itu.

**Divergensi yang disengaja:** nilai di luar `[-32768, 32767]` **di-clamp** ke batas
terdekat. Win32 membungkus (*wrap*), yang mengubah koordinat pada monitor di sebelah kiri
titik asal menjadi koordinat di sebelah kanan. Koordinat client selalu dalam rentang; hanya
koordinat **layar** yang dibawa `WM_MOUSEWHEEL` yang dapat keluar rentang. Clamping menjaga
nilai tetap di sisi desktop yang benar. Divergensi ini didokumentasikan di
`PlatformMessage.h`.

### `DWORD PlatformPackMouseWheel(WORD wKeyFlags, int iDelta)` + accessor

Memaketkan `WM_MOUSEWHEEL` `wParam` seperti Win32: word rendah berisi flag `MK_*`, word
tinggi berisi delta roda (dikali 120 per notch oleh pemanggil).

| Accessor | Mengembalikan |
|---|---|
| `PlatformUnpackMouseWheelDelta(DWORD)` | delta roda sebagai `int` (sign-extended) |
| `PlatformUnpackMouseWheelFlags(DWORD)` | word rendah flag `MK_*` |

Sebelum `PDR-019` ditutup, `WM_MOUSEWHEEL` memaketkan flag dan delta ke `lParam` (semantik
Win32 menaruh keduanya di `wParam` dan posisi kursor layar di `lParam`), dan
`WM_MOUSEMOVE`/tombol mouse membongkar koordinat dengan cast tak bertanda.

`WM_SIZE`/`WM_MOVE` **tidak** memakai `PlatformPackPoint`: keduanya membawa dua nilai
**tak bertanda** 16-bit (ukuran client dan posisi window), sehingga dipaketkan oleh
`PackUnsignedPair` lokal di `AppCore.cpp`.

## Timer

`PlatformSetTimerCompat`/`PlatformKillTimerCompat` (`PDR-021`) memakai registry berbasis
**slot + generasi**, bukan pointer callback:

- `g_TimerSlots[64]` — setiap slot menyimpan `bInUse`, `dwTimerID`, dan `iGeneration`
  (atomik).
- `g_Timers` — `timerID` → `{ SDL_TimerID, iSlot, iGeneration }`.
- `TimerThunk` (dipanggil dari thread timer SDL) memvalidasi **slot** dan **generasi**
  sebelum mem-post `WM_TIMER`. Tick yang tidak dapat dihentikan oleh `SDL_RemoveTimer`
  (sudah mengantre di thread timer) membuang dirinya sendiri, bukan mengirim `WM_TIMER`
  untuk timer yang sudah tidak ada.
- Slot dipublikasikan (`dwTimerID`, lalu `bInUse`) hanya **setelah** `SDL_AddTimer`
  berhasil, sehingga tick yang datang segera masih menemukan slot yang lengkap.
- `PlatformSetTimerCompat` pada `timerID` yang sudah ada me-retire slot lama lebih dulu.
- Bila tabel penuh, fungsi mengembalikan `0` dan mencatat `log_va` alih-alih diam.

## Crash Handler

`PDR-020`: handler sinyal fatal sebelumnya memanggil `open()` (bukan async-signal-safe) dan
`siglongjmp` dari signal context. Sekarang:

- `installCrashHandler` mengalokasikan `sigaltstack` 64 KB dan memasang handler dengan
  `SA_ONSTACK`, sehingga crash *stack overflow* tetap dapat dilaporkan.
- Deskriptor log (`Crash.log`) dibuka **sekali** sebelum crash (`g_CrashLogFD`), bukan di
  dalam handler. Seluruh penulisan memakai `write()` mentah lewat `sigWrite`/`sigWriteHex`/
  `sigWriteDec`.
- `crashHandler` menulis header, nomor sinyal, nama sinyal, dan backtrace, lalu **memulihkan
  `SIG_DFL` dan kembali** — sinyal yang masih tertunda dikirim ulang dengan disposisi default
  sehingga core dump tetap dihasilkan. (Sebelumnya `_exit` dipakai, yang menelan crash tanpa
  jejak.)
- Jalur recovery (`crashHandlerWithRecovery`) hanya menambahkan satu baris pendek
  `=== RECOVERED CRASH ===` lewat fd yang sudah terbuka sebelum `siglongjmp`: tidak ada
  `open()`, backtrace, atau alokasi dari signal context.

`siglongjmp` dari signal handler **tetap** perilaku tak terdefinisi menurut POSIX. Ini adalah
trade-off yang disengaja dan terbatas: jalur ini hanya di-arm oleh
`CTranscendenceModel::StartGame` di sekitar satu callback script, agar save file yang korup
dapat dilaporkan alih-alih mematikan sesi.

## Workaround Render yang Dapat Dikonfigurasi

Dua workaround performa tetap **aktif secara default** karena crash intro-render belum
tertutup, tetapi sekarang dapat dimatikan dari environment agar jalur multithreaded tetap
dapat divalidasi (`PDR-025`, `PDR-026`).

| Variabel | Default | Efek saat di-set `off` |
|---|---|---|
| `TRANSCENDENCE_MT_BKRND_PAINT` | workaround aktif | `m_iMaxBkrndPaintWorkers`/`m_bUseMTBkrndPaint` memakai nilai hasil kalkulasi normal (background painting multithreaded) |
| `TRANSCENDENCE_FORCE_ST_PAINT` | workaround aktif | `Ctx.bForceSTPaint` memakai `IsForceSTPaintEnabled()`, bukan dipaksa `true` |

### `bool PlatformRenderWorkaroundEnabled(const char* pszEnvVar)`

Mengembalikan `true` bila variabel belum di-set atau kosong (workaround aktif). Bila di-set,
mengembalikan `!PlatformEnvFlagIsOff(pszValue)`. `PlatformEnvFlagIsOff` cocok
case-insensitively dengan `"0"`, `"off"`, `"no"`, `"false"`. Nilai lain (termasuk salah tulis)
**tetap mengaktifkan** workaround, sehingga salah tulis gagal ke sisi aman.

**Kriteria keluar:** hapus kedua workaround (beserta variabel environment-nya) setelah sesi
intro merender bersih dengan keduanya di-set `off` pada validasi interaktif.

## Perbaikan Lain di Fase Ini

| Temuan | Perbaikan |
|---|---|
| `PDR-022` | `VirtualAlloc` menghormati `MEM_COMMIT`: region baru dan alamat eksplisit yang di-commit sama-sama di-zero (`memset`) |
| `PDR-023` | `ShellExecute` memakai *double fork* sehingga proses launcher di-reparent dan tidak ada `waitpid` yang memblokir thread UI |
| `PDR-024` | `posixResolvePathCase` memakai cache per direktori (`namespace posixPathCase`) yang divalidasi dengan `st_mtime`/`st_dev`/`st_ino` dan dibersihkan pada 64 entri; peringatan `fprintf(stderr, ...)` per komponen dihapus |
| `PDR-027` | Deteksi monokrom dibatasi (`kMaxSamples = 4096`, grid maksimum 64×64) dan dibaca lewat buffer 4 byte yang di-zero, sehingga piksel 3 byte di ujung baris tidak dibaca melewati buffer; `SDLBitmapSurfaceIsMonochrome` dipakai bersama oleh `DIBSDL.cpp` dan `SDLBitmap.cpp` |

## Verifikasi

Gate untuk perubahan pada berkas-berkas ini:

1. `cmake --preset macos-debug`
2. `cmake --build --preset macos-debug`
3. `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure`
4. `git diff --check`

Tes di `Transcendence/Tests/MacPortabilityTests.cpp` memverifikasi:

- `VirtualAlloc(NULL, N, MEM_COMMIT, ...)` mengembalikan region berisi nol, dan
  `MEM_COMMIT` pada alamat eksplisit membersihkan range yang sudah terisi;
- `PostMessage`/`PeekMessage` mempertahankan `WPARAM`/`LPARAM` lebar penuh, dan
  `PeekMessage` mengisi `hwnd`, `time`, serta `pt` (dibandingkan dengan `PlatformGetCursorPos`);
- `SendMessage` menjalankan handler sebelum kembali, menerima `wParam` lebar penuh, dan
  tidak sekaligus mengantre pesan;
- `PlatformPackPoint`/`PlatformUnpackPoint` bolak-balik untuk nilai dalam rentang, negatif,
  dan di luar rentang (clamp), serta `PlatformPackMouseWheel` beserta kedua accessornya;
- `SDLBitmapSurfaceIsMonochrome` benar untuk permukaan hitam, putih, dan berwarna, baik pada
  jalur pemindaian penuh maupun jalur sampling grid.

## Catatan

- Perubahan pada berkas-berkas ini hanya menyentuh jalur macOS; build Visual Studio tidak
  memakai `PlatformMessage.cpp`, `AppCore.cpp` versi port, atau `DIBSDL.cpp` versi port.
- Gate berikut hanya dapat diverifikasi dengan menjalankan game dan **tidak** diklaim selesai
  oleh dokumen ini: koordinat mouse nyata pada konfigurasi multi-monitor, pemulihan crash
  nyata, perilaku timer di bawah beban, peluncuran `ShellExecute` sungguhan, dan render
  intro dengan kedua workaround dimatikan.
