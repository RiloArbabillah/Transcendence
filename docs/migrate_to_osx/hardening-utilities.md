# Utilitas Hardening & Kebenaran Tipe macOS

Dokumen ini menjelaskan utilitas publik dan keputusan yang dihasilkan oleh Fase 4 register
temuan port macOS (`PDR-028`..`PDR-036`, ditambah `PDR-039` yang ditemukan saat fase ini
dikerjakan): konversi angka ke string, `wsprintf` yang sadar ukuran buffer, pointer/ukuran
file 64-bit, pemindahan file lintas volume, direktori temporer, informasi CPU, dan lokasi
log/crash log.

## Document Status

- Version: v1.0
- Last Updated: 2026-09-19
- Branch: `osx`
- Terkait: `port-defect-register.md` (`PDR-028`..`PDR-036`, `PDR-039`),
  `platform-input-utilities.md`, `functional-fs-utilities.md`,
  `platform-event-utilities.md`, `compat-layer-bugfix-plan.md`

## Ruang Lingkup

Fase 4 adalah fase hardening: tidak ada perilaku baru yang diharapkan selain hilangnya bug
tepi dan hilangnya informasi diagnostik yang salah. Perubahan menyentuh:

- `Alchemy/Include/Kernel.h` — `_fcvt_s`, `wsprintf`, `SetFilePointer`, `MoveFile`,
  `GetSystemInfo`/`GetLogicalProcessorInformationEx`, `CreateFile`.
- `Alchemy/Include/PathCompat.h`, `Alchemy/Kernel/Path.cpp` — `GetTempPath`, `SHGetMalloc`,
  `FreePIDL`.
- `Transcendence/Transcendence/Platform/AppCore.cpp` — mode buka log dan lokasi `Crash.log`.
- `Transcendence/Transcendence/Platform/MetalRenderer.cpp` — pemasangan hint renderer.
- `Mammoth/TSE/CLanguageDataBlock.cpp` — `DebugLog` pada build debug macOS.
- `Mammoth/TSE/CUniverse.cpp`, `CDesignCollection.cpp`, `CreateSystem.cpp`, `CSystem.cpp` —
  penghapusan pragma `-Wnon-pod-varargs`.

Semua perubahan bersifat platform-netral atau berada di dalam guard macOS
(`TARGET_PLATFORM_MACOS`, `__APPLE__`, `#ifndef _WIN32`), sehingga build Windows tidak
berubah.

## Berkas

| Berkas | Peran |
|---|---|
| `Alchemy/Include/Kernel.h` | shim `_fcvt_s`/`_gcvt_s`/`wsprintf`, `SetFilePointer`, `MoveFile`/`posixMoveFileAcrossVolumes`, `GetSystemInfo`, `CreateFile` |
| `Alchemy/Include/PathCompat.h` | deklarasi publik `GetTempPath` (dipromosikan dari `inline` lokal agar dapat diuji) |
| `Alchemy/Kernel/Path.cpp` | implementasi `GetTempPath`, catatan `SHGetMalloc`/`FreePIDL` |
| `Transcendence/Transcendence/Platform/AppCore.cpp` | mode buka log, `GetCrashLogPath()` |
| `Transcendence/Transcendence/Platform/MetalRenderer.cpp` | penghapusan `SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...)` yang tidak efektif |
| `Mammoth/TSE/CLanguageDataBlock.cpp` | `DebugLog` tanpa stub kosong pada build Apple debug |

## API

### `int _fcvt_s(char* buf, int len, double value, int decimals, int* dec, int* sign)`

Konversi `value` menjadi digit desimal tanpa titik, dengan `decimals` angka di belakang
koma. Satu-satunya pemanggil adalah `Kernel::strFromDouble` (`Alchemy/Kernel/CString.cpp`)
yang menyerahkan `CString::GetWritePointer(_CVTBUFSIZE)`, yaitu buffer dengan panjang
terdeklarasi 309 byte.

Kontrak keluaran mengikuti CRT dan karena itu sama dengan build Windows:

| Parameter | Arti |
|---|---|
| `dec` | indeks titik desimal di dalam `buf` |
| `sign` | `1` bila `value` negatif, selain itu `0` |

`buf` di-zero lebih dulu, lalu `snprintf` dijalankan dan hasilnya dipindai hanya sebatas
jumlah byte yang benar-benar ditulis (`snprintf` selalu men-*terminate* buffer, juga saat
terpotong). Titik desimal dihapus dengan `memmove` yang dibatasi pada jumlah byte itu. Bila
`buf` `NULL` atau `len <= 0`, fungsi mengembalikan `-1`.

**`PDR-039` (temuan baru Fase 4):** shim lama menulis `sign` ke parameter kelima dan indeks
titik desimal ke parameter keenam — kebalikan dari kontrak CRT. Pemanggil
(`strFromDouble`) membaca `&iDecimalPoint, &iSign` sesuai urutan CRT, sehingga setiap
`strFromDouble(value, decimals)` dengan jumlah desimal eksplisit mengembalikan string negatif
yang korup:

| Ekspresi | Sebelum | Sesudah |
|---|---|---|
| `strFromDouble(12.34, 2)` | `-0.1234` | `12.34` |
| `strFromDouble(12.34, 0)` | `-0.12` | `12.0` |
| `strFromDouble(-0.125, 3)` | `-0.125` (kebetulan benar) | `-0.125` |
| `strFromDouble(9.9, 0)` | `-0.10` | `10.0` |
| `strFromDouble(0.5, 0)` | `-0.0` | `0.0` |

Jalur `iDecimals == -1` memakai `_gcvt_s` dan tidak terpengaruh. `strFromDouble` dipakai
antara lain oleh `CPerformanceCounters.cpp`, `CDiagnosticsCommand.cpp`, `CHexarc.cpp`,
`CShipClass.cpp`, `CItemType.cpp`, `CWeaponClass.cpp`, dan `CLanguage.cpp`, sehingga
temuan ini sebelumnya merusak angka yang ditampilkan maupun yang dipakai untuk diagnosis.

### `template <size_t N> int wsprintf(char (&buf)[N], const char* format, ...)`

Win32 tidak memberi cara bagi `wsprintf` untuk mengetahui ukuran tujuan, sehingga shim lama
memakai batas tetap 4096 byte yang bukan batas sama sekali. Overload ini menerima tujuan
sebagai **referensi array**, sehingga ukuran aslinya diketahui kompilator:

- `vsnprintf(buf, N, ...)` tidak pernah menulis melewati `buf`;
- bila hasil terpotong, fungsi mengembalikan `N - 1` (jumlah karakter yang benar-benar ada,
  karena pemanggil memakai nilai balik itu sebagai panjang `CString`);
- bila `vsnprintf` gagal, `buf` dikosongkan dan fungsi mengembalikan `0`.

Audit menemukan **62 pemanggil**, semuanya menyerahkan array `char[256]` atau `char[1024]`;
karena bentuk referensi-array menolak pointer telanjang, bentuk tak terbatas tidak dapat
kembali. Pada Windows overload ini tidak dikompilasi (`#ifndef _WIN32`) dan pemanggil tetap
memakai `wsprintf` milik `user32`.

### `DWORD SetFilePointer(HANDLE hFile, LONG lDist, LONG* pHighWord, DWORD dwWhence)`

Kontrak Win32: tipe balik `DWORD` **memang** benar (Windows juga memakai `DWORD` dan
melaporkan separuh tinggi lewat `pHighWord`), jadi yang diperbaiki bukan tipe baliknya
melainkan bagian **masukan** yang hilang (`PDR-031`). Dengan `FILE_BEGIN`, Win32 membaca 32
bit atas offset baru dari `*pHighWord`; shim lama mengabaikannya sehingga setiap permintaan
seek 64-bit mendarat di offset yang salah. Sekarang:

- `FILE_BEGIN` → offset = `((DWORD)*pHighWord << 32) | (DWORD)lDist`;
- `FILE_CURRENT`/`FILE_END` → jarak 32-bit bertanda, seperti Windows;
- `lseek` gagal → `pHighWord` di-set `0` dan fungsi mengembalikan
  `INVALID_SET_FILE_POINTER` (bukan nilai `errno` yang terpotong seolah-olah posisi file);
- sukses → `*pHighWord` selalu diisi separuh tinggi posisi hasil.

`GetFileSize` tetap memakai bentuk `DWORD` + `pHighWord` yang sama dan sudah benar; tes
menambahkan verifikasi pada file sparse 5 GiB.

### `BOOL MoveFile(const char* pSrc, const char* pDst)`

`PDR-032`: `rename()` tidak dapat menyeberangi batas filesystem, sehingga pemindahan antar
volume (mis. save file di disk eksternal) gagal dengan `EXDEV` — padahal `MoveFile` Win32
menangani kasus itu dengan menyalin lalu menghapus sumbernya. Alur sekarang:

1. `rename(pSrc, pDst)`; bila berhasil, selesai.
2. Bila `errno != EXDEV`, gagal.
3. Selain itu `posixMoveFileAcrossVolumes(pSrc, pDst)`.

Semantik overwrite mengikuti `rename` POSIX: tujuan yang sudah ada **diganti**, sama dengan
`MoveFileEx(MOVEFILE_REPLACE_EXISTING)` dan sesuai dengan pemanggil engine
(`Kernel::fileMove`).

`BOOL posixMoveFileAcrossVolumes(const char* pSrc, const char* pDst)` adalah helper murni
yang menyalin isi berkas lewat buffer 64 KiB dengan loop tulis yang aman `EINTR`, memakai
mode sumber sebagai mode tujuan, menghapus tujuan bila penyalinan gagal, dan baru menghapus
sumber setelah penyalinan sukses. Helper ini diekspos agar fallback `EXDEV` dapat diuji unit
tanpa dua volume nyata.

### `DWORD GetTempPath(DWORD nBufferLength, char *lpBuffer)`

`PDR-032`: shim lama meng-hardcode `/tmp`. macOS memberi setiap proses direktori temporer
privat lewat `TMPDIR` (dan aplikasi ber-sandbox tidak dapat menulis ke `/tmp` sama sekali),
jadi fungsi sekarang:

- mengembalikan `TMPDIR` bila di-set dan tidak kosong, selain itu `/tmp`;
- mengembalikan **panjang path yang ditulis**, atau **panjang yang dibutuhkan** (tanpa
  terminator) bila buffer pemanggil terlalu kecil — buffer dibiarkan tak tersentuh.

Deklarasinya dipromosikan dari `inline` lokal di `Path.cpp` ke `Alchemy/Include/PathCompat.h`
agar dapat dipanggil dari unit test; ini satu-satunya perubahan signature di fase ini.
`Kernel::pathGetTempPath()` memakai fungsi ini, sehingga perilaku trailing slash mengikuti
nilai `TMPDIR` seperti halnya Windows mengembalikan trailing backslash.

### `void GetSystemInfo(SYSTEM_INFO* pInfo)`

`PDR-034`: stub lama melaporkan tepat satu prosesor, yang salah pada setiap mesin Apple
Silicon dan akan dipakai apa adanya oleh pemanggil yang memercayainya. Nilai yang benar-benar
dapat diandalkan kini dibaca dari host: `dwNumberOfProcessors` dari
`sysconf(_SC_NPROCESSORS_ONLN)`, `dwPageSize` dari `sysconf(_SC_PAGESIZE)`,
`dwAllocationGranularity` = ukuran halaman, dan `dwActiveProcessorMask` menutup seluruh
prosesor (atau semua bit bila jumlahnya ≥ lebar `DWORD_PTR`).

`GetLogicalProcessorInformationEx` **tetap** stub yang mengembalikan `FALSE` dengan
`*pLength = 0`. Topologi prosesor tidak dapat dipetakan ke host tanpa mengarang data, dan
satu-satunya konsumennya di macOS (`Alchemy/Kernel/Utilities.cpp`) mengambil cabang
`__APPLE__` sehingga tidak pernah memanggilnya; `FALSE` + panjang nol adalah jawaban jujur
untuk "tidak ada informasi". Catatan di kode menjelaskan hal ini agar tidak ada yang
menambahkan `ASSERT` di sana — `ASSERT` baru didefinisikan jauh setelah titik itu di
`Kernel.h`.

### `HANDLE CreateFile(...)`

`PDR-030`: `dwShareMode` dan `dwFlags` (`FILE_ATTRIBUTE_*`) tetap diabaikan karena tidak ada
padanan POSIX yang dapat diterapkan saat `open()`. Audit menemukan setiap pemanggil hanya
memakai `FILE_SHARE_READ` atau `FILE_SHARE_READ|FILE_SHARE_WRITE`, tidak ada yang
mengandalkan penegakan share mode, sehingga batasan ini **didokumentasikan** alih-alih
diemulasi dengan `flock()` (yang akan menambahkan advisory locking yang tidak pernah diminta
pemanggil).

Yang diperbaiki adalah *creation disposition*-nya: `CREATE_NEW` dan `TRUNCATE_EXISTING`
sebelumnya jatuh ke cabang default dan diam-diam berperilaku seperti `OPEN_EXISTING`.

| Disposition | Perilaku sekarang |
|---|---|
| `CREATE_NEW` (1) | gagal bila berkas sudah ada, selain itu `O_CREAT\|O_EXCL` |
| `CREATE_ALWAYS` (2) | `O_CREAT\|O_TRUNC` |
| `OPEN_EXISTING` (3) | gagal bila berkas tidak ada |
| `TRUNCATE_EXISTING` (5) | `O_TRUNC`; **tidak** membuat berkas yang tidak ada |

Konstanta `CREATE_NEW` dan `TRUNCATE_EXISTING` ditambahkan dengan guard `#ifndef` seperti
konstanta disposition lainnya.

### `void CLanguageDataBlock::DebugLog(...)`

`PDR-036`: pada `#if defined(__APPLE__) && defined(_DEBUG)` fungsi ini dikompilasi sebagai
stub kosong (`return;`), sehingga logging terjemahan hilang pada build debug macOS. Alasannya
dulu adalah `CString` non-POD yang diserahkan ke formatter varargs (didiagnosis clang sebagai
`-Wnon-pod-varargs`); badan fungsinya sejak itu ditulis ulang memakai `CString::Append` dan
pemanggilan pola hanya melewatkan integer, jadi stub tidak lagi diperlukan dan kini dihapus.

### `const char* GetCrashLogPath()` (internal `AppCore.cpp`)

`PDR-033`: crash log dulu bernama relatif `"Crash.log"`, sehingga laporan mendarat di
direktori tempat proses diluncurkan — yang bahkan belum tentu dapat ditulis — bukan di
samping log game. Sekarang path disusun ke root app-data yang sama dengan
`GetAppLogPath()`: `$HOME/Library/Application Support/Kronosaur/Transcendence/Crash.log`,
dengan dua `mkdir()` untuk membuat root tersebut dan fallback ke nama relatif bila `HOME`
tidak di-set.

Path disusun memakai panggilan libc biasa (`snprintf`/`mkdir`) dengan sengaja:
`installCrashHandler()` berjalan **sebelum** `kernelInit()`, jadi helper string/path engine
belum terinisialisasi dan tidak boleh dipakai di titik itu.

## Perbaikan Lain di Fase Ini

| Temuan | Perbaikan |
|---|---|
| `PDR-032` | `SHGetMalloc`/`FreePIDL` tetap stub, kini dengan catatan eksplisit: jalur shell macOS tidak pernah mengalokasikan `ITEMIDLIST`, jadi tidak ada yang dimiliki dan tidak ada yang dibebaskan; `SHGetMalloc` mengembalikan token, bukan allocator yang dapat dipakai |
| `PDR-033` | log dibuka dengan mode `"a"` (sebelumnya `"w"`, sehingga setiap restart menghapus log sebelumnya); `SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal")` di `MetalRenderer_Init` dihapus karena dipanggil setelah window dibuat **dan** fungsi itu tidak punya pemanggil, serta memaksa `"metal"` secara global akan mematahkan fallback software |
| `PDR-033` | high-DPI drawable **sengaja tidak** diminta (`SDL_WINDOW_ALLOW_HIGHDPI`/`SDL_HINT_VIDEO_ALLOW_HIGHDPI`); renderer menggambar framebuffer berukuran logis tetap lalu `SDL_RenderCopy` meregangkannya ke seluruh window, sehingga ruang koordinat game, pemetaan mouse, dan konversi screen/client semuanya memakai ukuran point. Drawable 2× memerlukan audit skala di ketiga tempat itu dan tidak dapat divalidasi oleh gate build + unit test fase ini |
| `PDR-035` | empat blok `#pragma clang diagnostic ignored "-Wnon-pod-varargs"` dihapus dari `CUniverse.cpp`, `CDesignCollection.cpp`, `CreateSystem.cpp`, dan `CSystem.cpp`; build menghasilkan **nol** peringatan `non-pod-varargs` baru, sehingga tidak ada call-site yang perlu diperbaiki dan tidak ada `-Wno-` global yang ditambahkan |
| `PDR-028` | `_fcvt_s` tidak lagi melakukan `strchr`/`strlen` atas seluruh buffer setelah `snprintf` yang dapat terpotong; pemindaian dibatasi jumlah byte yang benar-benar ditulis dan buffer di-zero lebih dulu sehingga hasilnya deterministik. Catatan: varian byte stale `'0500'`/`'12340'` yang diklaim pada entri register awal **tidak** dapat direproduksi dengan buffer berbentuk pemanggil; efek nyata yang dapat direproduksi adalah kebalikan parameter di `PDR-039` |

## Verifikasi

Gate untuk perubahan pada berkas-berkas ini:

1. `cmake --preset macos-debug`
2. `cmake --build --preset macos-debug`
3. `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure`
4. `git diff --check`

Tes di `Transcendence/Tests/MacPortabilityTests.cpp` memverifikasi:

- `_fcvt_s` dengan buffer berukuran pemanggil (309 byte, pra-isi `0xAA`) menghasilkan digit
  yang diharapkan, melaporkan indeks titik desimal dan tanda di posisi yang benar, tidak
  meninggalkan byte stale di dalam rentang terdeklarasi, dan tidak menulis melewati panjang
  yang dideklarasikan; buffer yang memotong tetap ter-*terminate* dan buffer `NULL` ditolak;
- `Kernel::strFromDouble` mengembalikan tanda dan titik desimal di posisi CRT untuk tujuh
  kasus, termasuk `12.34`/2 → `12.34` dan `-0.125`/3 → `-0.125` (`PDR-039`);
- `wsprintf` memotong ke ukuran array tujuan, melaporkan panjang terpotong, dan menulis hasil
  utuh bila muat;
- `CreateFile` menolak `CREATE_NEW` saat berkas ada, mengosongkan berkas dengan
  `TRUNCATE_EXISTING`, tidak membuat berkas yang tidak ada lewat `TRUNCATE_EXISTING`, dan
  membuatnya lewat `CREATE_NEW`;
- `SetFilePointer` menerapkan high word masukan, posisi 64-bit bolak-balik, `GetFileSize`
  melaporkan ukuran di atas 4 GiB pada berkas sparse 5 GiB, dan seek yang gagal melaporkan
  `INVALID_SET_FILE_POINTER`;
- `MoveFile` mengganti tujuan yang ada, menghapus sumber, dan gagal untuk sumber yang tidak
  ada; `posixMoveFileAcrossVolumes` menyalin isi dan menghapus sumber;
- `GetTempPath` mengikuti `TMPDIR`, melaporkan panjang yang dibutuhkan untuk buffer terlalu
  kecil tanpa menyentuh buffer, dan mengikuti perubahan `TMPDIR`;
- `GetSystemInfo` melaporkan jumlah prosesor ≥ 1, ukuran halaman yang masuk akal, granularity
  = ukuran halaman, dan mask prosesor yang tidak kosong; `GetLogicalProcessorInformationEx`
  mengembalikan `FALSE` dengan panjang nol.

## Catatan

- Perubahan pada berkas-berkas ini hanya menyentuh jalur macOS; build Visual Studio tetap
  memakai `user32`/CRT untuk `wsprintf`/`_fcvt_s` dan tidak mengompilasi shim di `Kernel.h`.
- Gate berikut hanya dapat diverifikasi dengan menjalankan game dan **tidak** diklaim selesai
  oleh dokumen ini: keluaran logging terjemahan yang sesungguhnya, perilaku direktori
  temporer di dalam sandbox, pemindahan berkas lintas volume nyata, koordinat Retina/HiDPI,
  variasi `TMPDIR`, penempatan `Crash.log` saat aplikasi diluncurkan dari Finder, dan setiap
  keluaran `strFromDouble` yang belum pernah dibandingkan dengan build Windows.
