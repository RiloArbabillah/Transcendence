# Register Temuan Defect Port macOS

## Document Status

- Version: v1.0
- Last Updated: 2026-09-19
- Branch: `osx`
- Derived From: audit kode statis
- Cakupan: jalur build/runtime macOS (`osx`) pada `TranscendenceDev`
- Non-goal: perbaikan regresi Windows; temuan lintas platform hanya dicatat sebagai `deferred`

## Tujuan

Dokumen ini adalah **register tunggal** untuk semua temuan defect hasil audit kode statis
pada port Windows-ke-macOS. Setiap temuan mendapat ID `PDR-###`, dikelompokkan per fase
perbaikan, dan wajib diperbarui status kerjanya pada PR yang menutupnya.

Dokumen ini **melengkapi**, bukan menggantikan:

- `../bug_fix_plan.md` — status lama yang masih berlabel "All phases complete"; lihat catatan
  superseded di file tersebut.
- `compat-layer-bugfix-plan.md` — rencana perbaikan layer kompatibilitas yang sudah dieksekusi.

## Legenda

### Status Temuan

| Status | Arti |
|---|---|
| `confirmed` | Terbukti langsung dari kode pada branch `osx`; lokasi dan perilaku sudah diverifikasi |
| `likely` | Sangat mungkin defect, tetapi efek akhirnya bergantung pada pemanggil/runtime yang belum diverifikasi ulang |
| `latent` | Defect nyata tetapi jalur pemanggilnya saat ini tidak aktif (belum terpicu) |
| `dead-code` | Tidak dikompilasi atau tidak pernah dipanggil pada build macOS saat ini |

### Prioritas

| Prioritas | Arti |
|---|---|
| `P0` | Blocker: kontrol/keamanan memori rusak, game tidak dapat dimainkan atau korup |
| `P1` | Tinggi: fitur inti (filesystem, gambar, video, audio) gagal diam-diam |
| `P2` | Sedang: perilaku platform/event dan performa menyimpang dari Windows |
| `P3` | Rendah: hardening, pembersihan, dan noise diagnostik |

### Status Kerja

| Status | Arti |
|---|---|
| `todo` | Belum dikerjakan |
| `in_progress` | Sedang dikerjakan |
| `blocked` | Menunggu keputusan atau pekerjaan lain |
| `done` | Selesai dan sudah lolos gate verifikasi fase |
| `deferred` | Sengaja ditunda; tidak dikerjakan pada rencana ini |

### Skema ID dan Field Wajib

- ID berurutan: `PDR-001` … `PDR-038`.
- Setiap entri wajib memuat: **ID**, **judul**, **status temuan**, **prioritas**, **lokasi
  `file:line`**, **dampak di macOS**, **kondisi pemicu**, **fase perbaikan**, **gate
  verifikasi**, dan **status kerja**.

### Gate Verifikasi Standar

Semua fase memakai gate yang sama, tanpa menjalankan game:

1. `cmake --preset macos-debug`
2. `cmake --build --preset macos-debug`
3. `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure`
4. `git diff --check`

Gate tambahan per temuan dicantumkan pada field **Gate verifikasi** di entri terkait.

## Ringkasan Temuan

| ID | Judul singkat | Prioritas | Status temuan | Fase | Status kerja |
|---|---|---|---|---|---|
| PDR-001 | Pemetaan VK tidak lengkap di `PlatformGetAsyncKeyState` | P0 | `confirmed` | 1 | `todo` |
| PDR-002 | `CMemoryWriteStream::Write` tumbuh hanya sekali | P0 | `confirmed` | 1 | `todo` |
| PDR-003 | `m_iCommittedSize` tidak pernah di-update | P0 | `confirmed` | 1 | `todo` |
| PDR-004 | `CloseHandle` heuristik pointer-vs-fd | P0 | `confirmed` | 1 | `todo` |
| PDR-005 | Event handle tanpa magic check | P0 | `confirmed` | 1 | `todo` |
| PDR-006 | `SDLBitmapDestroy` tidak menghapus entri map | P0 | `confirmed` | 1 | `todo` |
| PDR-007 | `dibCreate*`/`dibCrop`/`dibConvertToDDB`/`dibLoadFromResource` selalu `ERR_FAIL` | P1 | `confirmed` | 2 | `todo` |
| PDR-008 | `MCIWnd*` no-op | P1 | `confirmed` | 2 | `todo` |
| PDR-009 | `CMCIMixerStub.cpp` menggantikan `CMCIMixer.cpp` | P1 | `likely` | 2 | `todo` |
| PDR-010 | `ShowCursor`/`SetCapture`/`ReleaseCapture`/`ScreenToClient`/`ClientToScreen` no-op | P1 | `confirmed` | 2 | `todo` |
| PDR-011 | `GetCursorPos`/`SetCursorPos` memakai koordinat window global | P1 | `confirmed` | 2 | `todo` |
| PDR-012 | `GetFileTime`/`FileTimeToSystemTime` return `TRUE` tanpa mengisi output | P1 | `confirmed` | 2 | `todo` |
| PDR-013 | `WIN32_FIND_DATA::ftLastWriteTime` menyimpan epoch Unix | P1 | `confirmed` | 2 | `todo` |
| PDR-014 | `CopyFile` mengabaikan `bFailIfExists` | P1 | `confirmed` | 2 | `todo` |
| PDR-015 | `SHGetFolderPath` tanpa komponen `Kronosaur` | P1 | `confirmed` | 2 | `todo` |
| PDR-016 | `PlatformPostMessage` memotong `WPARAM` ke `int` | P2 | `confirmed` | 3 | `todo` |
| PDR-017 | `PeekMessage` tidak mengisi `hwnd`/`time`/`pt` | P2 | `confirmed` | 3 | `todo` |
| PDR-018 | `PlatformSendMessage` asinkron | P2 | `confirmed` | 3 | `todo` |
| PDR-019 | `MAKELONG` untuk koordinat mouse | P2 | `confirmed` | 3 | `todo` |
| PDR-020 | Crash handler `siglongjmp` dari signal context | P2 | `confirmed` | 3 | `todo` |
| PDR-021 | `PlatformSetTimerCompat` race | P2 | `confirmed` | 3 | `todo` |
| PDR-022 | `VirtualAlloc` `MEM_COMMIT` tidak commit/zero | P2 | `confirmed` | 3 | `todo` |
| PDR-023 | `ShellExecute` blocking `waitpid` | P2 | `confirmed` | 3 | `todo` |
| PDR-024 | `posixResolvePathCase` noise stderr + O(n) syscall | P2 | `confirmed` | 3 | `todo` |
| PDR-025 | MT background paint dipaksa off | P2 | `confirmed` | 3 | `todo` |
| PDR-026 | `bForceSTPaint` dipaksa `true` | P2 | `confirmed` | 3 | `todo` |
| PDR-027 | Deteksi monokrom O(W·H) | P2 | `confirmed` | 3 | `todo` |
| PDR-028 | `_fcvt_s` over-read | P3 | `confirmed` | 4 | `todo` |
| PDR-029 | `wsprintf` asumsi buffer 4096 | P3 | `confirmed` | 4 | `todo` |
| PDR-030 | `CreateFile` abaikan `dwShareMode` | P3 | `confirmed` | 4 | `todo` |
| PDR-031 | `SetFilePointer`/`GetFileSize` 32-bit | P3 | `confirmed` | 4 | `todo` |
| PDR-032 | `MoveFile`/`GetTempPath`/`FreePIDL`/`SHGetMalloc` | P3 | `confirmed` | 4 | `todo` |
| PDR-033 | `SDL_SetHint` setelah `SDL_CreateWindow`, HIGHDPI, log `"w"`, `Crash.log` di CWD | P3 | `likely` | 4 | `todo` |
| PDR-034 | `GetSystemInfo`/`GetLogicalProcessorInformationEx` stub | P3 | `dead-code` | 4 | `todo` |
| PDR-035 | `#pragma clang diagnostic ignored "-Wnon-pod-varargs"` | P3 | `confirmed` | 4 | `todo` |
| PDR-036 | `DebugLog` mati di `CLanguageDataBlock.cpp` | P3 | `confirmed` | 4 | `todo` |
| PDR-037 | `#define WINAPI` kosong di luar guard `_WIN32` | P3 | `latent` | 5 | `deferred` |
| PDR-038 | `CMemoryStream.cpp` stub menggantikan implementasi Windows | P3 | `latent` | 5 | `deferred` |

Catatan: status kerja awal semua temuan adalah `todo`. Status berubah menjadi `done` pada PR
fase yang menutupnya, dan `done` hanya berarti temuan sudah ditutup pada branch fase terkait
serta lolos gate build + `mac-portability`; validasi runtime interaktif tetap merupakan gate
terpisah di luar cakupan rencana ini.

---

## Fase 1 — Input & Keamanan Memori (P0)

Fokus: pemetaan virtual-key yang lengkap dan koreksi akuntansi/keamanan memori pada
kompat layer. Fase ini adalah prasyarat agar kontrol default dan alokasi dinamis bekerja benar.

### PDR-001 — Pemetaan VK tidak lengkap di `PlatformGetAsyncKeyState`

- **Status temuan:** `confirmed`
- **Prioritas:** `P0`
- **Lokasi:** `Transcendence/Transcendence/Platform/AppCore.cpp:287-315`
- **Dampak di macOS:** `PlatformGetAsyncKeyState` hanya menangani `VK_SHIFT`, `VK_CONTROL`,
  `VK_MENU`, `VK_NUMLOCK`, huruf `A-Z`, digit `0-9`, dan `VK_DOWN/UP/NEXT/PRIOR/END`. Virtual
  key lain mengembalikan `0` permanen, sehingga seluruh kontrol default yang memakai VK
  tersebut tidak pernah aktif — termasuk `VK_LEFT`, `VK_RIGHT`, `VK_SPACE`, `VK_TAB`,
  `VK_PAUSE`, `VK_F1/F2/F6/F7/F8/F9`, `VK_LBUTTON`, `VK_RBUTTON`, dan `VK_MBUTTON`.
- **Kondisi pemicu:** Setiap pemanggilan `uiIsKeyDown`/`GetAsyncKeyState` dengan VK yang tidak
  ada di switch. Jalur pemanggil: `Transcendence/Transcendence/DefaultKeyMappings.h:43-63,97-118`
  (peta default `CGameKeys::DEFAULT_MAP`/`WASD_MAP`), `Transcendence/Transcendence/CGameKeys.cpp:392-405`
  (`CGameKeys::IsKeyDown` → `::uiIsKeyDown`), dan `Alchemy/Include/Kernel.h:2939`
  (`uiIsKeyDown` memakai `GetAsyncKeyState(iVirtKey) & 0x8000`). `GameSessionInput.cpp` juga
  memakai `VK_LBUTTON`/`VK_RBUTTON`/`VK_MBUTTON` secara langsung.
- **Fase perbaikan:** Fase 1 — PR `fix/macos-port-p0-input-memory`
- **Gate verifikasi:** Gate standar + unit test `PlatformVKToScancode` untuk setiap VK pada
  `DefaultKeyMappings.h`, plus assert `PlatformGetAsyncKeyState` mengembalikan `0x8000` saat
  scancode terkait aktif.
- **Status kerja:** `todo`

### PDR-002 — `CMemoryWriteStream::Write` tumbuh hanya sekali

- **Status temuan:** `confirmed`
- **Prioritas:** `P0`
- **Lokasi:** `Alchemy/Kernel/CMemoryStream.cpp:51-57`
- **Dampak di macOS:** Saat `m_iCurrentSize + iLength > m_iMaxSize`, buffer hanya digandakan
  **sekali** tanpa loop. Jika penulisan melampaui `2 × m_iMaxSize`, `realloc` menghasilkan
  blok yang masih terlalu kecil, lalu `memcpy` menulis melewati batas → heap overflow
  (korupsi memori / crash). Implementasi asli di `origin/master` menggandakan ukuran dalam
  loop sampai cukup.
- **Kondisi pemicu:** Penulisan tunggal atau kumulatif yang melampaui dua kali ukuran maksimum
  saat ini (mis. serialisasi save/`.tdb` besar, buffer teks panjang).
- **Fase perbaikan:** Fase 1 — PR `fix/macos-port-p0-input-memory`
- **Gate verifikasi:** Gate standar + unit test menulis melebihi `2 × m_iMaxSize` tanpa korupsi
  (build Debug + sanitizer-friendly assertion) dan memverifikasi data utuh.
- **Status kerja:** `todo`

### PDR-003 — `m_iCommittedSize` tidak pernah di-update

- **Status temuan:** `confirmed`
- **Prioritas:** `P0`
- **Lokasi:** `Alchemy/Kernel/CMemoryStream.cpp:33-45,51-62`
- **Dampak di macOS:** `m_iCommittedSize` hanya di-set `0` di `Create()` dan tidak pernah
  dinaikkan oleh `Write`. Akibatnya `Seek(iPos)` untuk `iPos > 0` selalu masuk cabang
  `Write(NULL, iPos - m_iCurrentSize)`; karena `pData == NULL`, `memcpy` dilewati sehingga
  region yang di-`Seek` berisi byte tak terinisialisasi (sampah heap) saat dibaca kembali.
- **Kondisi pemicu:** Setiap `Seek` maju melewati data yang sudah ditulis, lalu membaca kembali
  region tersebut (mis. padding/alignment pada serialisasi).
- **Fase perbaikan:** Fase 1 — PR `fix/macos-port-p0-input-memory`
- **Gate verifikasi:** Gate standar + unit test: `Seek` maju menghasilkan byte nol, dan
  `m_iCommittedSize` konsisten dengan jumlah byte yang pernah ditulis.
- **Status kerja:** `todo`

### PDR-004 — `CloseHandle` heuristik pointer-vs-fd

- **Status temuan:** `confirmed`
- **Prioritas:** `P0`
- **Lokasi:** `Alchemy/Include/Kernel.h:995-1030`
- **Dampak di macOS:** `CloseHandle` membedakan handle event/mapping dari file descriptor
  memakai heuristik nilai numerik (`if (iHandle > 1024)`) lalu melakukan dereference
  (`pMap->dwMagic`, `pEvent->dwMagic`). File descriptor nyata yang kebetulan `> 1024`
  (umum pada proses dengan banyak fd) akan di-dereference sebagai pointer → `EXC_BAD_ACCESS`.
  Sebaliknya, handle pointer dengan alamat rendah dapat salah dianggap fd.
- **Kondisi pemicu:** `CloseHandle(fd)` pada proses dengan file descriptor bernilai `> 1024`.
- **Fase perbaikan:** Fase 1 — PR `fix/macos-port-p0-input-memory`
- **Gate verifikasi:** Gate standar + unit test `CloseHandle` pada fd tinggi dan pada handle
  event/mapping (tanpa crash, tidak menutup fd yang salah).
- **Status kerja:** `todo`

### PDR-005 — Event handle tanpa magic check

- **Status temuan:** `confirmed`
- **Prioritas:** `P0`
- **Lokasi:** `Alchemy/Include/Kernel.h:583-598` (`SetEvent`/`ResetEvent`),
  `Alchemy/Include/Kernel.h:1289-1306` (`WaitForSingleObject`),
  `Alchemy/Include/Kernel.h:1307-1330` (`WaitForMultipleObjects`)
- **Dampak di macOS:** Keempat fungsi meng-cast `HANDLE` langsung ke `SEventHandle*` dan
  membaca `p->fd[...]` tanpa memverifikasi `dwMagic == EVENT_MAGIC`. Handle yang salah
  (fd file, mapping, atau pointer lain) menghasilkan `poll`/`read`/`write` pada fd sampah →
  perilaku tak terdefinisi, hang, atau crash.
- **Kondisi pemicu:** Pemanggilan fungsi sinkronisasi dengan handle yang bukan event, atau
  handle event yang sudah di-`CloseHandle`.
- **Fase perbaikan:** Fase 1 — PR `fix/macos-port-p0-input-memory`
- **Gate verifikasi:** Gate standar + unit test memastikan handle non-event ditolak dengan
  aman (`FALSE`/`WAIT_TIMEOUT`), bukan dereference liar.
- **Status kerja:** `todo`

### PDR-006 — `SDLBitmapDestroy` tidak menghapus entri map

- **Status temuan:** `confirmed`
- **Prioritas:** `P0`
- **Lokasi:** `Alchemy/DirectXUtil/SDLBitmap.cpp:86-93` (destroy) vs `:81` (insert) dan
  `:118-126` (`LookupBitmap`)
- **Dampak di macOS:** `SDLBitmapCreateFromSurface` mendaftarkan bitmap ke `GetBitmapMap()`
  (`GetBitmapMap()[hBitmap] = pBitmap`), tetapi `SDLBitmapDestroy` hanya `SDL_FreeSurface` +
  `delete` tanpa `erase`. Entri map tetap menunjuk ke memori bebas → **dangling pointer**
  ketika `dibGetInfo`/lookup lain dipanggil dengan handle lama, sekaligus **pertumbuhan map
  tanpa batas** pada siklus create/destroy berulang (mis. setiap frame efek).
- **Kondisi pemicu:** Bitmap dibuat lalu dihancurkan, kemudian handle lama dipakai lagi; atau
  pembuatan bitmap berulang dalam jumlah besar.
- **Fase perbaikan:** Fase 1 — PR `fix/macos-port-p0-input-memory`
- **Gate verifikasi:** Gate standar + unit test: create → destroy → lookup mengembalikan `null`,
  dan ukuran map tidak tumbuh setelah siklus berulang.
- **Status kerja:** `todo`

---

## Fase 2 — Fungsional & Filesystem (P1)

Fokus: mengembalikan fitur inti yang saat ini gagal diam-diam (gambar, video, audio, kursor,
waktu file, dan lokasi app-data).

### PDR-007 — Pembuat DIB yang hilang selalu `ERR_FAIL`

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Graphics/DIBSDL.cpp:125-165`
- **Dampak di macOS:** `dibCreate16bitDIB`, `dibCreate24bitDIB`, `dibCreate32bitDIB`,
  `dibConvertToDDB`, `dibCrop`, dan `dibLoadFromResource` semuanya mengembalikan `ERR_FAIL`
  dengan output `NULL`. Jalur apa pun yang membutuhkannya gagal tanpa gambar. Implementasi
  nyata masih ada di `Alchemy/Graphics/DIB.cpp` tetapi tidak dikompilasi pada target macOS.
- **Kondisi pemicu:** Pemanggil: `Alchemy/Graphics/Raw.cpp:36,69`,
  `Alchemy/IntelJPEGUtil/Load.cpp:66,115`, `Transcendence/TransWorkshop/CCmdUpload.cpp:270,284`.
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + unit test `dibCreate24bitDIB`/`dibCrop` mengembalikan
  `NOERROR` dengan output valid; atau test menegaskan seluruh pemanggil sudah dialihkan ke
  jalur `SDLBitmap`.
- **Status kerja:** `todo`

### PDR-008 — `MCIWnd*` no-op

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Include/Kernel.h:807-819`
- **Dampak di macOS:** Semua macro `MCIWnd*` (`MCIWndCreate`, `MCIWndOpen`, `MCIWndPlay`,
  `MCIWndGetLength`, `MCIWndGetPosition`, `MCIWndGetMode`, …) mengembalikan `0`/`nullptr`.
  Pemutaran video/intro MCI menjadi no-op senyap tanpa error yang terlihat.
- **Kondisi pemicu:** Jalur video/intro yang memakai `MCIWnd*` (juga dipakai
  `Alchemy/DirectXUtil/Sound.cpp`, yang tidak dikompilasi pada macOS).
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + keputusan eksplisit: implementasi backend video, atau
  tandai video sebagai non-goal dengan log/`ASSERT` yang terdeteksi saat debug.
- **Status kerja:** `todo`

### PDR-009 — `CMCIMixerStub.cpp` menggantikan `CMCIMixer.cpp`

- **Status temuan:** `likely`
- **Prioritas:** `P1`
- **Lokasi:** `CMakeLists.txt:265` (mengompilasi `Mammoth/TSUI/CMCIMixerStub.cpp`), sumber asli
  `Mammoth/TSUI/CMCIMixer.cpp` tidak dikompilasi
- **Dampak di macOS:** Implementasi MCI asli (Windows) digantikan oleh `CMCIMixerStub.cpp`.
  File stub **bukan** no-op: ia adalah implementasi berbasis `SDL_mixer` yang mengisi seluruh
  daftar method `CMCIMixer` (ctor/dtor, `Boot`, `Play`, `PlayFadeIn`, `Stop`, `SetVolume`,
  `FadeNow`, `FadeAtPos`, `SetPlayPaused`, `TogglePausePlay`, `GetCurrentPlayLength`,
  `GetCurrentPlayPos`, `GetDebugInfo`, `Shutdown`, `AbortAllRequests`). Risiko yang tersisa
  adalah paritas fitur terhadap versi MCI asli, bukan ketiadaan implementasi.
- **Kondisi pemicu:** Semua pemutaran soundtrack/musik pada build macOS.
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + inventaris fitur MCI asli vs stub (daftar fitur yang
  hilang/berbeda), dan keputusan eksplisit untuk setiap gap.
- **Status kerja:** `todo`

### PDR-010 — `ShowCursor`/`SetCapture`/`ReleaseCapture`/`ScreenToClient`/`ClientToScreen` no-op

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Include/Kernel.h:771-773` (`ShowCursor`, `SetCapture`, `ReleaseCapture`),
  `Alchemy/Include/Kernel.h:1100-1101` (`ScreenToClient`, `ClientToScreen`)
- **Dampak di macOS:** Kursor sistem tidak pernah disembunyikan/ditampilkan lewat jalur ini,
  capture mouse tidak pernah diambil/dilepas, dan konversi koordinat layar↔client tidak
  melakukan transformasi (langsung `return TRUE`). Pada mode fullscreen SDL hal ini sebagian
  dapat diterima, tetapi hit-testing dan posisi kursor UI menjadi tidak konsisten dengan
  Windows begitu window tidak memenuhi layar.
- **Kondisi pemicu:** Pemanggil: `Transcendence/Transcendence/CGameSession.cpp:60,343,468,486,668,750`,
  `Transcendence/Transcendence/IntroScreen.cpp:1101,1122,1138,1523`,
  `Transcendence/Transcendence/CIntroSession.cpp:1102,1232,1569`,
  `Transcendence/Transcendence/CTranscendenceWnd.cpp:173-174,974-975,1032-1033`.
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + verifikasi konversi koordinat benar untuk window
  non-fullscreen (offset origin window), dan capture/cursor dipetakan ke `SDL_CaptureMouse`
  / `SDL_ShowCursor`.
- **Status kerja:** `todo`

### PDR-011 — `GetCursorPos`/`SetCursorPos` memakai koordinat window global

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Include/Kernel.h:918-919`
- **Dampak di macOS:** `GetCursorPos`/`SetCursorPos` membaca/menulis `g_PlatformMouseX/Y` yang
  berisi **koordinat window**, bukan koordinat layar. Semantik Win32 mengharuskan koordinat
  layar, sehingga posisi kursor yang dilaporkan/di-set menyimpang ketika window tidak berada
  di origin layar.
- **Kondisi pemicu:** Semua pemakaian `GetCursorPos`/`SetCursorPos` saat window digeser dari
  pojok kiri-atas layar.
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + verifikasi round-trip `SetCursorPos`→`GetCursorPos`
  konsisten dengan offset window (`SDL_GetWindowPosition`).
- **Status kerja:** `todo`

### PDR-012 — `GetFileTime`/`FileTimeToSystemTime` return `TRUE` tanpa mengisi output

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Kernel/Path.cpp:263-264`
- **Dampak di macOS:** Kedua fungsi langsung `return TRUE` tanpa menulis ke output. Pemanggil
  menerima status sukses dengan `FILETIME`/`SYSTEMTIME` berisi nilai tak terinisialisasi,
  sehingga waktu modifikasi file salah tanpa terdeteksi.
- **Kondisi pemicu:** Pemanggil: `Alchemy/Kernel/Path.cpp:543,552` (`fileGetModTime`).
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + unit test `GetFileTime`/`FileTimeToSystemTime` menghasilkan
  waktu non-nol dan konsisten dengan `stat()` file uji.
- **Status kerja:** `todo`

### PDR-013 — `WIN32_FIND_DATA::ftLastWriteTime` menyimpan epoch Unix

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Kernel/Path.cpp:183`
- **Dampak di macOS:** `pData->ftLastWriteTime = (FILETIME)st.st_mtime;` menyimpan detik epoch
  Unix mentah ke field bertipe `FILETIME` (`#define FILETIME unsigned long long` di
  `Alchemy/Kernel/Path.cpp:33`). Nilai ini bukan FILETIME (100-ns sejak 1601) sehingga
  perbandingan/konversi waktu file salah; konsumen `FileTimeToSystemTime` akan menghasilkan
  tanggal yang tidak berarti.
- **Kondisi pemicu:** `FindFirstFile`/`FindNextFile` diikuti pemakaian `ftLastWriteTime`
  (mis. urutan save game terbaru, cek waktu mod resource).
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + unit test konversi `ftLastWriteTime` → `SYSTEMTIME`
  menghasilkan tanggal yang sesuai file uji.
- **Status kerja:** `todo`

### PDR-014 — `CopyFile` mengabaikan `bFailIfExists`

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Kernel/Path.cpp:139`
- **Dampak di macOS:** Implementasi selalu memanggil `copyfile(..., COPYFILE_ALL)` tanpa
  memeriksa `bFailIfExists`. Pemanggil yang mengandalkan penolakan saat tujuan sudah ada akan
  menimpa file secara diam-diam (mis. menimpa save game atau config).
- **Kondisi pemicu:** Pemanggil: `Alchemy/Kernel/Path.cpp:406` (`fileCopy` dengan
  `bFailIfExists = FALSE`, jadi aman); risiko muncul pada pemanggil yang meminta `TRUE`.
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + unit test `CopyFile` gagal saat `bFailIfExists` dan tujuan
  sudah ada, serta sukses menimpa saat `bFailIfExists = FALSE`.
- **Status kerja:** `todo`

### PDR-015 — `SHGetFolderPath` tanpa komponen `Kronosaur`

- **Status temuan:** `confirmed`
- **Prioritas:** `P1`
- **Lokasi:** `Alchemy/Kernel/Path.cpp:44-100` dan fallback `Alchemy/Kernel/Path.cpp:1135-1165`
- **Dampak di macOS:** `CSIDL_APPDATA`/`CSIDL_LOCAL_APPDATA` dipetakan ke
  `~/Library/Application Support` **tanpa** komponen `Kronosaur/Transcendence`, sementara
  `Transcendence/Transcendence/Platform/AppCore.cpp:182-202` (`GetAppLogPath`) memakai
  `~/Library/Application Support/Kronosaur/Transcendence`. Hasilnya ada **dua root app-data**:
  save/settings (`CGameSettings` → `folderAppData`, lihat `CTranscendenceModel.cpp:1091` dan
  banyak pemanggil di `CTranscendenceController.cpp`) tertulis ke root yang berbeda dari log
  dan dari ekspektasi paket aplikasi.
- **Kondisi pemicu:** Setiap pembacaan/penulisan save, settings, atau log pada build macOS.
- **Fase perbaikan:** Fase 2 — PR `fix/macos-port-p1-functional-fs`
- **Gate verifikasi:** Gate standar + unit test `SHGetFolderPath(CSIDL_APPDATA)` mengembalikan
  root yang sama dengan direktori `GetAppLogPath()`.
- **Status kerja:** `todo`

---

## Fase 3 — Platform/Event & Performa (P2)

Fokus: menyelaraskan semantik message/event platform dengan Win32 dan menghapus pemaksaan
performa yang tidak lagi diperlukan.

### PDR-016 — `PlatformPostMessage` memotong `WPARAM` ke `int`

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Transcendence/Transcendence/Platform/AppCore.cpp:781-790`; deklarasi
  `Transcendence/Transcendence/Platform/AppCore.h:82`; shim `Alchemy/Include/Kernel.h:883-884`
- **Dampak di macOS:** Signature `PlatformPostMessage(int msg, int wParam, void* lParam)`
  memaksa `WPARAM` menjadi `int`. `PostMessage` di `Kernel.h:884` juga meng-cast
  `(int)wParam`. Nilai `WPARAM` 64-bit (pointer/handle/flag gabungan) terpotong, sehingga
  pesan yang membawa data lebar kehilangan bit tinggi.
- **Kondisi pemicu:** `PostMessage` dengan `wParam` bernilai lebih dari 32-bit.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + unit test `PostMessage`/`PeekMessage` round-trip
  mempertahankan nilai `WPARAM` lebar penuh.
- **Status kerja:** `todo`

### PDR-017 — `PeekMessage` tidak mengisi `hwnd`/`time`/`pt`

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Alchemy/Include/Kernel.h:899` (inline `PeekMessage`), struktur `tagMSG` di
  `Alchemy/Include/Kernel.h:903-904` (`SMsgCompat`)
- **Dampak di macOS:** `PeekMessage` hanya mengisi `message`, `wParam`, dan `lParam`.
  Field `hwnd`, `time`, dan `pt` tetap tak terinisialisasi. Konsumen yang membaca `msg.time`
  (timestamp) atau `msg.pt` (posisi kursor saat pesan) menerima sampah.
- **Kondisi pemicu:** Semua loop pesan yang memeriksa `msg.time`/`msg.pt`.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + unit test memastikan `hwnd`, `time`, dan `pt` terisi
  deterministik (bukan sampah).
- **Status kerja:** `todo`

### PDR-018 — `PlatformSendMessage` asinkron

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Transcendence/Transcendence/Platform/AppCore.cpp:793-808`
- **Dampak di macOS:** `PlatformSendMessage` mengantre pesan ke queue (`PlatformPostMessage`)
  lalu langsung kembali, padahal kontrak Win32 `SendMessage` bersifat **sinkron**: handler
  selesai dijalankan sebelum fungsi kembali. Kode yang bergantung pada efek samping langsung
  (mis. mengambil hasil dari `lParam` setelah `SendMessage`) akan membaca nilai yang belum
  diproses.
- **Kondisi pemicu:** Semua pemanggilan `SendMessage` yang mengandalkan eksekusi sinkron.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + unit test `SendMessage` menjalankan handler sebelum
  kembali (observable ordering).
- **Status kerja:** `todo`

### PDR-019 — `MAKELONG` untuk koordinat mouse

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Transcendence/Transcendence/Platform/AppCore.cpp:650-703`
  (`WM_MOUSEMOVE`, tombol mouse, `WM_MOUSEWHEEL` di `:689`, `WM_SIZE` di `:698`,
  `WM_MOVE` di `:703`); `MAKELONG` lokal di `AppCore.cpp:25-26`, definisi nyata di
  `Alchemy/Include/Kernel.h:1363`
- **Dampak di macOS:** Koordinat mouse dan delta wheel dipaketkan dengan `MAKELONG`, yang
  menyimpan dua nilai 16-bit. Pada konfigurasi multi-monitor atau koordinat negatif, nilai
  X/Y di luar rentang 16-bit terpotong/wrap. `WM_MOUSEWHEEL` juga memaketkan flag `MK_*`
  bersama delta wheel di `lParam` (`AppCore.cpp:690`), yang semantiknya berbeda dari Win32
  (di Windows, `lParam` high word = delta, low word = key flags, dan posisi ada di `wParam`).
- **Kondisi pemicu:** Mouse di monitor sekunder, koordinat negatif, atau scroll wheel saat
  window digeser.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + unit test packing/unpacking koordinat untuk nilai
  negatif dan nilai > 32767 tetap utuh.
- **Status kerja:** `todo`

### PDR-020 — Crash handler `siglongjmp` dari signal context

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Transcendence/Transcendence/Platform/AppCore.cpp:1005-1040`
  (`crashHandlerWithRecovery`), instalasi handler di `:1037`
- **Dampak di macOS:** Handler sinyal memanggil `siglongjmp` langsung dari signal context.
  Ini bukan async-signal-safe dan menghasilkan perilaku tak terdefinisi (deadlock pada lock
  yang dipegang thread yang terinterupsi, state rusak, atau crash berantai saat logging).
- **Kondisi pemicu:** Setiap sinyal fatal (`SIGSEGV`, `SIGBUS`, `SIGFPE`, …).
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + jalur handler yang hanya memakai operasi
  async-signal-safe (tulis ke fd mentah) atau `sigaltstack`/`SA_ONSTACK` yang benar.
- **Status kerja:** `todo`

### PDR-021 — `PlatformSetTimerCompat` race

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Transcendence/Transcendence/Platform/AppCore.cpp:833-875`; `TimerThunk` di
  `AppCore.cpp:251-256` yang mem-post `PLATFORM_WM_TIMER`
- **Dampak di macOS:** `PlatformSetTimerCompat` memegang `g_TimerCS` sambil memodifikasi state
  timer, sementara `TimerThunk` (dari thread timer) membaca/mem-post `PLATFORM_WM_TIMER`.
  Penguncian yang tidak konsisten pada jalur ini menimbulkan race: timer ganda, timer hilang,
  atau pemakaian state setelah dibebaskan.
- **Kondisi pemicu:** Set/clear timer bersamaan dengan timer yang sedang menyala.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + unit test set/clear timer berulang secara bersamaan tanpa
  timer ganda/hilang.
- **Status kerja:** `todo`

### PDR-022 — `VirtualAlloc` `MEM_COMMIT` tidak commit/zero

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Alchemy/Include/Kernel.h:260-277`
- **Dampak di macOS:** Untuk `lpAddress != NULL` dan `flAllocationType & MEM_COMMIT`, fungsi
  langsung `return lpAddress` tanpa meng-commit atau men-zero region. Padahal semantik Win32
  `MEM_COMMIT` menjamin memori ter-commit dan ter-zero. Pemanggil menerima pointer yang
  isinya sampah, bukan nol.
- **Kondisi pemicu:** `VirtualAlloc` dengan alamat eksplisit dan `MEM_COMMIT`.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + unit test `VirtualAlloc(MEM_COMMIT)` mengembalikan region
  berisi nol.
- **Status kerja:** `todo`

### PDR-023 — `ShellExecute` blocking `waitpid`

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Alchemy/Include/Kernel.h:1226-1245`
- **Dampak di macOS:** `ShellExecute` melakukan `fork` + `execlp("open", ...)` lalu menunggu
  proses anak dengan `waitpid` secara blocking. Membuka URL/file eksternal memblokir thread
  UI selama proses `open` berjalan.
- **Kondisi pemicu:** Setiap `ShellExecute` (buka browser, folder, atau file eksternal).
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + `ShellExecute` kembali segera (non-blocking) dan tetap
  membuka target.
- **Status kerja:** `todo`

### PDR-024 — `posixResolvePathCase` noise stderr + O(n) syscall

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Alchemy/Include/Kernel.h:437-480` (`fprintf(stderr, "Warning: case mismatch ...")`
  di `:471`)
- **Dampak di macOS:** Setiap komponen path yang kapitalisasinya berbeda memicu
  `fprintf(stderr, ...)` (noise pada log/terminal), dan resolusi dilakukan dengan
  `opendir`/`readdir` per komponen — biaya syscall O(n) pada setiap pembukaan file yang
  kapitalisasinya tidak persis cocok.
- **Kondisi pemicu:** Pemanggil: `Alchemy/Kernel/Path.cpp:217,274`,
  `Alchemy/Kernel/CFileDirectory.cpp:78`, dan `CreateFile` di `Alchemy/Include/Kernel.h:505`.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + verifikasi tidak ada output stderr saat resolusi case
  berhasil, dan hasil lookup tetap benar (termasuk cache per direktori bila ditambahkan).
- **Status kerja:** `todo`

### PDR-025 — MT background paint dipaksa off

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Mammoth/TSE/CSFXOptions.cpp:211-219`
- **Dampak di macOS:** Blok `#if defined(__APPLE__)` memaksa `m_iMaxBkrndPaintWorkers = 0` dan
  `m_bUseMTBkrndPaint = false`, sehingga background painting selalu single-threaded. Ini
  adalah workaround untuk crash intro-render; setelah bug render tertutup, performa macOS
  tetap terbatas secara tidak perlu.
- **Kondisi pemicu:** Semua frame dengan background paint (menu/intro/gameplay).
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + evaluasi ulang pemaksaan setelah bug render tertutup;
  bila belum aman, pertahankan dengan komentar eksplisit dan alasan terdokumentasi.
- **Status kerja:** `todo`

### PDR-026 — `bForceSTPaint` dipaksa `true`

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Mammoth/TSE/CSystem.cpp:529-535`
- **Dampak di macOS:** Setelah membaca `Ctx.bForceSTPaint = ...IsForceSTPaintEnabled()`,
  blok `#if defined(__APPLE__)` menimpanya menjadi `true`, sehingga object painting selalu
  single-threaded. Sama seperti PDR-025, ini workaround untuk crash intro yang menekan
  performa.
- **Kondisi pemicu:** Semua paint viewport/objek.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + keputusan terdokumentasi: hapus pemaksaan, atau
  pertahankan dengan tiket/kriteria keluar yang jelas.
- **Status kerja:** `todo`

### PDR-027 — Deteksi monokrom O(W·H)

- **Status temuan:** `confirmed`
- **Prioritas:** `P2`
- **Lokasi:** `Alchemy/Graphics/DIBSDL.cpp:26-49`
- **Dampak di macOS:** `DetectBitmapType` memindai **setiap piksel** untuk menentukan apakah
  gambar monokrom. Untuk atlas/tekstur besar ini menambah biaya O(W·H) pada setiap load
  bitmap, termasuk pada gambar yang jelas berwarna (deteksi baru berhenti saat menemukan
  piksel berwarna pertama, tetapi kasus terburuk tetap penuh).
- **Kondisi pemicu:** Pemuatan bitmap besar pada jalur SDL.
- **Fase perbaikan:** Fase 3 — PR `fix/macos-port-p2-platform-perf`
- **Gate verifikasi:** Gate standar + verifikasi hasil deteksi tipe bitmap tidak berubah
  (paritas benar/salah) setelah optimasi, dengan sampling/early-exit terbatas.
- **Status kerja:** `todo`

---

## Fase 4 — Hardening & Minor (P3)

Fokus: perbaikan defensif, kebenaran tipe, dan pembersihan diagnostik. Tidak ada perubahan
perilaku yang diharapkan selain hilangnya bug tepi.

### PDR-028 — `_fcvt_s` over-read

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:1074-1082`
- **Dampak di macOS:** Implementasi melakukan `snprintf(buf, len, "%.*f", decimals, absVal)`
  lalu `memmove(dot, dot+1, strlen(dot))`. Parameter `len` adalah panjang buffer **dan** nilai
  yang dipakai pemanggil sebagai panjang string (`Alchemy/Kernel/CString.cpp:1767` memanggil
  `GetWritePointer(_CVTBUFSIZE)` yang men-set length ke 309). Ketika output terformat lebih
  panjang dari `len`, `snprintf` memotong tanpa NUL di posisi yang diharapkan sehingga
  `strlen(dot)` membaca melewati batas buffer.
- **Kondisi pemicu:** `_fcvt_s` dengan nilai yang menghasilkan digit lebih banyak daripada
  `len`. Terkonfirmasi secara empiris dengan probe ASan/UBSan terpisah: output `'0500'` untuk
  `0.5`, `'12340'` untuk `12.34`, `'0125'` untuk `-0.125`, `'9900'` untuk `9.9` — sisa byte
  stale setelah terminator, konsisten dengan over-read saat buffer lebih kecil dari string
  terformat.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + unit test `_fcvt_s` dengan buffer kecil tidak membaca
  melewati batas (hasil deterministik, tidak ada byte stale).
- **Status kerja:** `todo`

### PDR-029 — `wsprintf` asumsi buffer 4096

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:1052-1058`
- **Dampak di macOS:** `wsprintf` memanggil `vsnprintf(buf, 4096, ...)` dengan panjang tetap
  4096 dan mengabaikan ukuran buffer pemanggil. Pemanggil yang memakai buffer lebih kecil
  (mis. `char szBuffer[1024]` di `Transcendence/TransData/EncounterTable.cpp:96-97`,
  `char szBuffer[256]` di `Transcendence/Transcendence/CTranscendenceWnd.cpp:277`) berisiko
  overflow.
- **Kondisi pemicu:** `wsprintf` dengan output lebih panjang dari buffer pemanggil.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + audit semua pemanggil `wsprintf`; panjang buffer eksplisit
  atau helper aman yang menerima `sizeof(buf)`.
- **Status kerja:** `todo`

### PDR-030 — `CreateFile` abaikan `dwShareMode`

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:495-496`
- **Dampak di macOS:** Parameter `dwShareMode` dan `dwFlags` diabaikan sepenuhnya. Pemanggil
  yang mengandalkan penolakan/berbagi akses tidak mendapat jaminan apa pun, dan atribut
  (`FILE_ATTRIBUTE_*`) tidak berpengaruh.
- **Kondisi pemicu:** `CreateFile` dengan `dwShareMode`/`dwFlags` non-default.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + audit pemanggil dan dokumentasi batasan eksplisit
  (atau penerapan `flock`/`O_EXCL` sesuai kebutuhan).
- **Status kerja:** `todo`

### PDR-031 — `SetFilePointer`/`GetFileSize` 32-bit

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:631-637` (`SetFilePointer`),
  `Alchemy/Include/Kernel.h:639-650` (`GetFileSize`)
- **Dampak di macOS:** Keduanya memakai tipe pengembalian `DWORD` sehingga posisi/ukuran file
  di atas 4 GiB terpotong. `SetFilePointer` juga tidak memanfaatkan `pHighWord` untuk
  mempertahankan bagian tinggi.
- **Kondisi pemicu:** File resource/save berukuran > 4 GiB (jarang tetapi mungkin pada paket
  resource gabungan).
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + unit test posisi/ukuran 64-bit (menggunakan file sparse
  bila memungkinkan) tidak terpotong.
- **Status kerja:** `todo`

### PDR-032 — `MoveFile`/`GetTempPath`/`FreePIDL`/`SHGetMalloc`

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:1225` (`MoveFile` = `rename` saja),
  `Alchemy/Kernel/Path.cpp:265` (`GetTempPath` hardcode `/tmp`),
  `Alchemy/Kernel/Path.cpp:377-378` (`SHGetMalloc`),
  `Alchemy/Kernel/Path.cpp:1457` (`FreePIDL`)
- **Dampak di macOS:** `MoveFile` gagal lintas volume (POSIX `rename` tidak menyeberangi
  filesystem) dan tidak menghormati semantik overwrite Win32; `GetTempPath` mengabaikan
  `TMPDIR` macOS (sandboxing/`NSTemporaryDirectory`); `SHGetMalloc`/`FreePIDL` adalah stub
  yang tidak melepaskan apa pun.
- **Kondisi pemicu:** Pindah file antar volume, penggunaan direktori temp yang benar, atau
  jalur shell yang memakai `PIDL`.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + `MoveFile` fallback copy+unlink saat `rename` gagal
  (EXDEV), dan `GetTempPath` menghormati `TMPDIR`.
- **Status kerja:** `todo`

### PDR-033 — `SDL_SetHint` setelah `SDL_CreateWindow`, HIGHDPI, log `"w"`, `Crash.log` di CWD

- **Status temuan:** `likely`
- **Prioritas:** `P3`
- **Lokasi:** `Transcendence/Transcendence/Platform/MetalRenderer.cpp:48`;
  `Transcendence/Transcendence/Platform/AppCore.cpp:220` (log dibuka mode `"w"`);
  `Transcendence/Transcendence/Platform/AppCore.cpp:877` (`CRASH_LOG_FILE = "Crash.log"`)
- **Dampak di macOS:** `SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal")` dipanggil di dalam
  `MetalRenderer_Init`, yaitu setelah window dibuat, sehingga hint tidak lagi berpengaruh
  (dan `MetalRenderer_Init` saat ini tidak punya pemanggil — `AppCore.cpp:428-441` memakai
  `SDL_CreateRenderer` langsung; status efektif `latent`/`dead-code`). Selain itu tidak ada
  pemakaian `SDL_HINT_VIDEO_HIGHDPI_DISABLED`/`SDL_HINT_VIDEO_ALLOW_HIGHDPI` sehingga
  koordinat Retina belum ditangani eksplisit. Log dibuka dengan mode `"w"` (setiap start
  menimpa log sebelumnya), dan `Crash.log` ditulis relatif terhadap CWD (bukan lokasi
  app-data) sehingga tidak dapat ditemukan saat aplikasi diluncurkan dari Finder.
- **Kondisi pemicu:** Start aplikasi, dan setiap crash yang perlu didiagnosis.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + hint dipasang sebelum pembuatan window (bila jalur
  Metal diaktifkan), dan log/crash log ditulis ke root app-data (lihat PDR-015).
- **Status kerja:** `todo`

### PDR-034 — `GetSystemInfo`/`GetLogicalProcessorInformationEx` stub

- **Status temuan:** `dead-code`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:1174-1175`
  (`GetLogicalProcessorInformationEx` return `FALSE`, `GetSystemInfo` mengisi 1 CPU)
- **Dampak di macOS:** Stub ini mengembalikan informasi CPU yang salah, tetapi pada build macOS
  jalur yang memakainya (`Alchemy/Kernel/Utilities.cpp:106-254`) berada di cabang `#else` dari
  `#if defined(__APPLE__)`, sehingga tidak dikompilasi/dijalankan. Risiko nyata hanya jika
  cabang Apple dihapus atau kode baru memanggil stub ini.
- **Kondisi pemicu:** Tidak terpicu pada build macOS saat ini.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + audit bahwa tidak ada jalur macOS yang memanggil kedua
  stub; tambahkan komentar/`ASSERT` agar pemanggilan tak sengaja terdeteksi.
- **Status kerja:** `todo`

### PDR-035 — `#pragma clang diagnostic ignored "-Wnon-pod-varargs"` menyembunyikan UB varargs

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Mammoth/TSE/CUniverse.cpp:10`, `Mammoth/TSE/CDesignCollection.cpp:10`,
  `Mammoth/TSE/CreateSystem.cpp:11`, `Mammoth/TSE/CSystem.cpp:12`;
  `CMakeLists.txt:26` (flag global saat ini dikomentari)
- **Dampak di macOS:** Pragma per-file menyembunyikan peringatan bahwa objek non-POD
  (mis. `CString`) dilewatkan ke fungsi varargs — perilaku tak terdefinisi pada ABI arm64 dan
  sumber crash tipe `%p`/`%s` yang sudah pernah terjadi. Menyembunyikan warning berarti
  call-site bermasalah tidak terdeteksi.
- **Kondisi pemicu:** Setiap pemanggilan varargs dengan argumen non-POD pada file tersebut.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + hapus pragma lalu perbaiki seluruh call-site yang
  memicu warning (tanpa menambah `-Wno-` global).
- **Status kerja:** `todo`

### PDR-036 — `DebugLog` mati di `CLanguageDataBlock.cpp`

- **Status temuan:** `confirmed`
- **Prioritas:** `P3`
- **Lokasi:** `Mammoth/TSE/CLanguageDataBlock.cpp:286-293`
- **Dampak di macOS:** Pada `#if defined(__APPLE__) && defined(_DEBUG)`, `DebugLog` langsung
  `return;` tanpa melakukan apa pun, sehingga logging terjemahan (translation) hilang pada
  build debug macOS — menyulitkan diagnosis masalah lokalisasi/teks.
- **Kondisi pemicu:** Build Debug macOS dengan jalur terjemahan.
- **Fase perbaikan:** Fase 4 — PR `fix/macos-port-p3-hardening`
- **Gate verifikasi:** Gate standar + `DebugLog` mengeluarkan output yang sama dengan jalur
  non-Apple (atau alasan eksplisit bila sengaja dinonaktifkan).
- **Status kerja:** `todo`

---

## Fase 5 — Risiko Lintas Platform (Deferred, Tidak Diperbaiki)

Temuan berikut adalah regresi Windows, bukan blocker macOS. Sesuai lingkup rencana
(macOS-only), keduanya **hanya dicatat** dan ditandai `deferred`; tidak ada perubahan kode.

### PDR-037 — `#define WINAPI` kosong di luar guard `_WIN32`

- **Status temuan:** `latent`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Include/Kernel.h:1256` (dibandingkan `#define WINAPI __stdcall` di
  `Alchemy/Include/Kernel.h:330` di dalam guard `_WIN32`)
- **Dampak di macOS:** Pada build macOS, `WINAPI` menjadi kosong sehingga deklarasi fungsi
  kehilangan calling convention. Untuk macOS (System V/AAPCS arm64) ini tidak masalah karena
  tidak ada `__stdcall`, tetapi bila header ini dipakai kembali pada build Windows, fungsi
  yang dideklarasikan `WINAPI` akan memakai konvensi default yang berbeda → mismatch ABI.
- **Kondisi pemicu:** Kompilasi header ini pada toolchain Windows.
- **Fase perbaikan:** Fase 5 — tidak diperbaiki (`deferred`)
- **Gate verifikasi:** Tidak ada; dicatat sebagai risiko lintas platform.
- **Status kerja:** `deferred`

### PDR-038 — `CMemoryStream.cpp` stub menggantikan implementasi Windows

- **Status temuan:** `latent`
- **Prioritas:** `P3`
- **Lokasi:** `Alchemy/Kernel/CMemoryStream.cpp:1-2`
  ("Stub implementation for non-Windows platforms")
- **Dampak di macOS:** File stub menggantikan implementasi asli tanpa guard platform
  (`#ifndef _WIN32`), sehingga build Windows apa pun yang memakai file ini akan kehilangan
  implementasi asli. Pada macOS sendiri ini adalah satu-satunya implementasi aktif, dan
  perbaikan Fase 1 (PDR-002/PDR-003) dilakukan pada file ini.
- **Kondisi pemicu:** Kompilasi file ini pada toolchain Windows.
- **Fase perbaikan:** Fase 5 — tidak diperbaiki (`deferred`)
- **Gate verifikasi:** Tidak ada; dicatat sebagai risiko lintas platform.
- **Status kerja:** `deferred`

---

## Rencana Perbaikan Bertahap

Setiap fase dikerjakan pada branch + PR terpisah dari `osx` (sesuai `AGENTS.md`), dan
**tidak** digabungkan ke `main`/`master` tanpa persetujuan eksplisit.

| Fase | Branch | Temuan | Deliverable |
|---|---|---|---|
| 0 | `docs/port-defect-register` | — | dokumen register + registrasi ke `index.md`, `task-backlog.md`, `change-log.md`, `../bug_fix_plan.md`, `../macOS_port_status.md` |
| 1 | `fix/macos-port-p0-input-memory` | PDR-001..PDR-006 | pemetaan VK lengkap + helper `PlatformVKToScancode`, perbaikan pertumbuhan/akuntansi `CMemoryStream`, magic check handle event, pembersihan map `SDLBitmapDestroy` |
| 2 | `fix/macos-port-p1-functional-fs` | PDR-007..PDR-015 | DIB yang hilang, keputusan MCI/video/audio, kursor & capture, waktu file, `bFailIfExists`, satu root app-data |
| 3 | `fix/macos-port-p2-platform-perf` | PDR-016..PDR-027 | truncation/isi struct pesan, packing koordinat mouse, crash handler, race timer, semantik `VirtualAlloc`, `ShellExecute` non-blocking, noise `posixResolvePathCase`, evaluasi ulang pemaksaan single-thread paint |
| 4 | `fix/macos-port-p3-hardening` | PDR-028..PDR-036 | perbaikan defensif dan pembersihan |
| 5 | — | PDR-037..PDR-038 | hanya `deferred` di register; tanpa perubahan kode |

### Acceptance Criteria Umum per Fase

- `git diff --check` bersih.
- Tidak ada perubahan perilaku Windows: setiap perubahan bersifat platform-netral atau
  dibungkus `#ifndef _WIN32`/`__APPLE__`.
- Gate verifikasi standar (build `macos-debug` + `ctest -R mac-portability`) lulus.
- Setiap entri register yang diselesaikan diperbarui statusnya (`todo` → `done`) pada PR
  yang sama.
- Utilitas publik baru (mis. `PlatformVKToScancode`) didokumentasikan di `docs/`.

### Test Plan

Gate per fase (tanpa menjalankan game):

1. `cmake --preset macos-debug`
2. `cmake --build --preset macos-debug`
3. `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure`

Penambahan tes di `Transcendence/Tests/MacPortabilityTests.cpp` (harness `main()` + `Check()`
yang sudah ada):

- **`CMemoryStream`:** tulis melebihi `2 × m_iMaxSize` tanpa korupsi; `Seek` maju menghasilkan
  byte nol; `m_iCommittedSize` konsisten.
- **`SDLBitmap`:** create → destroy → lookup mengembalikan `null`; map tidak tumbuh setelah
  siklus berulang.
- **`PlatformVKToScancode`:** setiap VK pada `DefaultKeyMappings.h` (`VK_LEFT`, `VK_RIGHT`,
  `VK_SPACE`, `VK_TAB`, `VK_PAUSE`, `VK_F1/F2/F6/F7/F8/F9`, `VK_LBUTTON`, `VK_RBUTTON`,
  `VK_MBUTTON`) terpetakan ke scancode benar dan `PlatformGetAsyncKeyState` mengembalikan
  `0x8000` saat scancode aktif.
- **Filesystem:** `GetFileTime`/`FileTimeToSystemTime` menghasilkan waktu non-nol dan
  konsisten; `CopyFile` gagal saat `bFailIfExists` dan tujuan ada; `SHGetFolderPath`
  mengembalikan root yang sama dengan `GetAppLogPath()`.
- **DIB:** `dibCreate24bitDIB`/`dibCrop` mengembalikan `NOERROR` dengan output valid (atau
  tes menegaskan pemanggil sudah dialihkan).

## Asumsi & Default

- Bahasa dokumen: Indonesia; lokasi: `docs/migrate_to_osx/`.
- Cakupan perbaikan: macOS saja. Regresi Windows (PDR-037, PDR-038) hanya dicatat `deferred`.
- Tidak ada langkah `npm run lint`/`pint` karena repo ini C++ murni (tidak ada `package.json`
  atau `composer.json`); gate kualitas = build + `ctest`.
- Temuan berlabel `likely`/`latent`/`dead-code` yang masih perlu konfirmasi pemanggil dicatat
  apa adanya dan diverifikasi saat fase terkait dikerjakan.
- Branch `osx` tetap bersih sampai implementasi disetujui; PR dibuat per fase, tidak
  digabungkan langsung.
