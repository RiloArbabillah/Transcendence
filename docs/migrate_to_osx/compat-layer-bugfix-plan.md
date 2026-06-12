# Rencana Perbaikan Bug Porting macOS

Dokumen ini merangkum semua bug yang ditemukan pada layer kompatibilitas Windows-ke-macOS
beserta rencana perbaikannya. Bug dikelompokkan per fase berdasarkan prioritas dampak.

Status: `[ ]` belum dikerjakan, `[x]` selesai.

---

## Fase 1 — Blocker: File System (game tidak bisa menemukan resource)

### 1.1 Path separator masih backslash
- **File:** `Alchemy/Kernel/Path.cpp` (baris ~66)
- **Bug:** `STR_PATH_SEPARATOR` didefinisikan sebagai `"\\"`. Semua path hasil
  `pathAddComponent` dkk. tidak valid di POSIX (`Game\Collection\file.tdb`).
- **Perbaikan:**
  - [x] Ubah menjadi `"/"` di build non-Windows (gunakan `#ifdef _WIN32`).
  - [x] Audit semua fungsi `path*` (`pathGetExtension`, `pathStripExtension`,
    `pathGetPath`, `pathGetFilename`) agar menerima `/` sekaligus `\` sebagai
    separator input (resource lama dari TDB bisa berisi `\`).
  - [x] Tambahkan fungsi normalisasi `pathNormalizeSeparators()` yang mengubah
    `\` → `/` saat membaca path dari XML/TDB.

### 1.2 `FindFirstFile` stub mengembalikan `nullptr` + struct tak terinisialisasi
- **File:** `Alchemy/Kernel/Path.cpp` (shim di atas file, dipakai `fileGetFileList`)
- **Bug:** Stub return `nullptr`, tapi pemanggil membandingkan dengan
  `INVALID_HANDLE_VALUE` (`(HANDLE)-1`). Kode lanjut ke loop `do/while` dengan
  `WIN32_FIND_DATA FileInfo` **tidak terinisialisasi** → baca memori sampah/crash.
  Efek: daftar save game, extension, dan mod selalu kosong atau crash.
- **Perbaikan:**
  - [x] Implementasi nyata berbasis `opendir()`/`readdir()`/`fnmatch()`:
    buat struct handle internal yang menyimpan `DIR*` + pattern.
  - [x] `FindFirstFile` gagal harus return `INVALID_HANDLE_VALUE`, bukan `nullptr`.
  - [x] Isi `dwFileAttributes` dari `stat()` (`FILE_ATTRIBUTE_DIRECTORY` untuk dir).
  - [x] Alternatif lebih bersih: tulis ulang `fileGetFileList` versi POSIX murni
    di blok `#else`, tanpa meniru API Win32.

### 1.3 `GetFullPathName` stub salah dan rawan overflow
- **File:** `Alchemy/Kernel/Path.cpp`
- **Bug:** Hanya `strcpy(lpBuffer, lpFileName)` — tidak meresolve path relatif,
  tidak menghormati `nBufferLength` (buffer overflow).
- **Perbaikan:**
  - [x] Gunakan `realpath()` untuk path yang sudah ada; untuk path yang belum ada,
    gabungkan dengan `getcwd()` lalu normalisasi `.`/`..` manual.
  - [x] Selalu cek `nBufferLength` (gunakan `strlcpy`).

### 1.4 `SHGetFolderPath` selalu `E_FAIL`
- **File:** `Alchemy/Kernel/Path.cpp`
- **Bug:** Folder save/AppData/Documents tidak pernah resolve → lokasi save game rusak.
- **Perbaikan:**
  - [x] Map CSIDL ke lokasi macOS:
    - `CSIDL_LOCAL_APPDATA` / `CSIDL_APPDATA` → `~/Library/Application Support/Transcendence`
    - `CSIDL_PERSONAL` → `~/Documents`
    - `CSIDL_MYPICTURES` → `~/Pictures`, `CSIDL_MYMUSIC` → `~/Music`
  - [x] Gunakan `getenv("HOME")` + fallback `getpwuid(getuid())->pw_dir`.
  - [x] Buat direktori jika belum ada (`mkdir -p` semantics).

### 1.5 Case sensitivity filesystem
- **Bug:** APFS bisa case-sensitive; referensi resource yang beda kapitalisasi
  gagal load padahal jalan di Windows.
- **Perbaikan:**
  - [x] Tambahkan fallback lookup case-insensitive di resource loader
    (`CResourceDb` / `CResourcePathResolver`): jika `stat()` gagal, scan direktori
    dengan perbandingan `strcasecmp`.
  - [x] Log warning saat fallback terpakai agar aset bisa dirapikan.

### 1.6 `fileDelete` dengan recycle selalu gagal
- **File:** `Alchemy/Kernel/Path.cpp`
- **Bug:** `SHFileOperation` stub return 1 (gagal).
- **Perbaikan:**
  - [x] Minimal: fallback ke `unlink()` saat `bRecycle=true` di macOS, atau
  - [x] Implementasi trash via `NSFileManager trashItemAtURL` (butuh file .mm Obj-C++).

---

## Fase 2 — Blocker: Ukuran Tipe Data & Serialisasi

### 2.1 `LONG` = 64-bit di macOS arm64
- **File:** `Alchemy/Include/Kernel.h` (`typedef long LONG;`)
- **Bug:** Di Windows `LONG` = 32-bit. Akibatnya:
  - `POINT`/`SIZE`/`RECT` jadi 16/32 byte → layout struct berubah.
  - Save game & format binary (`.tdb`, `CDataFile`, `CArchiver`) yang menulis
    struct mentah jadi korup / tidak kompatibel lintas platform.
- **Perbaikan:**
  - [x] Ubah ke `typedef std::int32_t LONG;` dan `typedef std::uint32_t ULONG;`.
  - [ ] Rebuild penuh + jalankan self-test serialisasi (tulis-baca save game).
  - [x] Grep semua `sizeof(LONG)`, cast `(long)`, dan `memcpy` struct yang
    mengandung `LONG` untuk verifikasi.

### 2.2 `LARGE_INTEGER` union rusak
- **File:** `Alchemy/Include/Kernel.h`
- **Bug:** `HighPart` bertipe `LONG` 64-bit → aligned ke offset 8, sehingga
  `QuadPart` tidak lagi overlap dengan pasangan Low/HighPart. Semua kode yang
  mengisi Low/High lalu membaca QuadPart (timer, ukuran file) menghasilkan nilai salah.
- **Perbaikan:**
  - [x] Otomatis benar setelah 2.1 (`LONG` jadi 32-bit).
  - [x] Tambahkan `static_assert(sizeof(LARGE_INTEGER) == 8)` sebagai pengaman.
  - [x] Guard `#ifndef LARGE_INTEGER` tidak berfungsi untuk typedef — ganti dengan
    komentar yang menjelaskan `#pragma once` memberikan proteksi.

### 2.3 `SOCKET` unsigned
- **File:** `Alchemy/Include/Kernel.h`
- **Bug:** `typedef unsigned int SOCKET` padahal fd POSIX adalah `int` (bisa -1);
  perbandingan dengan `SOCKET_ERROR (-1)` pada tipe unsigned berisiko salah.
- **Perbaikan:**
  - [x] Ubah ke `typedef int SOCKET;` dengan `INVALID_SOCKET (-1)`.
  - [x] Audit pengecekan `== INVALID_SOCKET` / `== SOCKET_ERROR` di `Internets.h`,
    `CHTTPClientSession.cpp`, `NetUtil`. — Semua aman: SOCKET sekarang `int` (signed),
    comparasi dengan `-1` valid.

### 2.4 `WCHAR` 16-bit vs `wchar_t` 32-bit
- **File:** `Alchemy/Include/Kernel.h`
- **Bug:** `typedef std::uint16_t WCHAR;` tidak kompatibel dengan literal `L"..."`
  (32-bit di macOS).
- **Perbaikan:**
  - [x] Pertahankan `WCHAR` 16-bit (UTF-16) untuk kompatibilitas data, tapi
    grep semua penggunaan `L"..."` yang di-assign ke `WCHAR*` dan ganti dengan
    `u"..."` (char16_t) atau konversi eksplisit.

---

## Fase 3 — Blocker: Networking

### 3.1 `htons`/`ntohs` jadi fungsi identity
- **File:** `Alchemy/Include/Kernel.h` (baris ~47-50)
- **Bug:** macOS arm64 little-endian; tanpa byte-swap, `sin_port` salah endian
  (port 80 → 20480). Semua fitur online (Multiverse/Hexarc, HTTP) gagal connect.
- **Perbaikan:**
  - [x] Hapus `#undef htons` / `#undef ntohs` dan shim identity-nya.
  - [x] Pakai implementasi sistem dari `<arpa/inet.h>` apa adanya.
  - [x] Audit juga penggunaan `htonl`/`ntohl` jika ada shim serupa. —
    Tidak ada shim: hanya satu penggunaan di `CNetServer.cpp` (`htonl(INADDR_ANY)`),
    memanggil sistem langsung.

### 3.2 Redefinisi konstanta socket
- **File:** `Alchemy/Include/Kernel.h`
- **Bug:** `#define AF_INET 2`, `SOCK_STREAM 1`, dst. setelah include
  `<sys/socket.h>` → redefinition warning dan rapuh jika nilai sistem berbeda.
- **Perbaikan:**
  - [x] Hapus semua `#define` konstanta socket yang sudah disediakan header sistem;
    bungkus sisanya dengan `#ifndef`.

---

## Fase 4 — High: Shim File I/O Berbahaya

Semua di `Alchemy/Include/Win32Compat.h` (lihat juga Fase 6 soal status file ini)
dan duplikatnya di header lain.

### 4.1 `GetFileSize` menggeser file pointer
- **Bug:** Macro `lseek(fd, 0, SEEK_END)` memindahkan posisi baca ke EOF sebagai
  efek samping; read berikutnya dapat 0 byte (di Windows tidak menggeser pointer).
- **Perbaikan:**
  - [x] Ganti dengan `fstat(fd, &st)` dan return `st.st_size`, atau
    simpan posisi → seek end → restore posisi.

### 4.2 `CreateFileMapping` + `MapViewOfFile` dobel-mmap
- **Bug:** `CreateFileMapping` sudah return pointer hasil `mmap`, lalu
  `MapViewOfFile` memanggil `mmap` lagi dengan pointer itu sebagai fd → EBADF/crash.
  `MAP_PRIVATE` juga berarti tulisan tidak pernah masuk ke file.
- **Perbaikan:**
  - [x] Jadikan `CreateFileMapping` hanya menyimpan fd + ukuran + proteksi dalam
    struct handle; `MapViewOfFile` yang melakukan `mmap` sebenarnya.
  - [x] Gunakan `MAP_SHARED` saat `FILE_MAP_WRITE`.
  - [x] Cek hasil `mmap` terhadap `MAP_FAILED`, bukan `NULL`.

### 4.3 `UnmapViewOfFile` → `munmap(ptr, 0)`
- **Bug:** `munmap` dengan length 0 gagal (EINVAL) di macOS → leak setiap unmap.
- **Perbaikan:**
  - [x] Simpan length mapping di handle (peta `ptr → size`) dan panggil
    `munmap(ptr, size)`.

### 4.4 `ReadFile`/`WriteFile` selalu return TRUE
- **Bug:** `ssize_t -1` saat error disimpan ke `DWORD` (jadi 4294967295) dan
  macro tetap return `TRUE` → error I/O tidak pernah terdeteksi.
- **Perbaikan:**
  - [x] Ganti macro dengan inline function yang mengecek hasil `read`/`write`,
    return `FALSE` saat `< 0`, dan menangani partial read/write (loop sampai habis).

### 4.5 `CloseHandle(fd)` → `close(fd)`
- **Bug:** `HANDLE` adalah `void*`; cast implisit ke `int` tidak valid/truncation.
- **Perbaikan:**
  - [x] Standardisasi representasi handle file: bungkus fd dalam
    `(HANDLE)(intptr_t)fd` di seluruh shim, dan unwrap secara konsisten.

---

## Fase 5 — High: Stub yang Gagal Diam-diam

### 5.1 Konversi Unicode return 0
- **File:** `Alchemy/Include/Kernel.h`
- **Bug:** `MultiByteToWideChar`/`WideCharToMultiByte` stub return 0 → string
  hasil konversi selalu kosong (input non-ASCII, nama file UTF-8).
- **Perbaikan:**
  - [x] Implementasi UTF-8 ↔ UTF-16 nyata (manual encoder/decoder kecil, atau
    pakai utilitas yang sudah ada di `Alchemy/Kernel/Unicode.cpp`).

### 5.2 `GetClientRect` hardcode 800x600
- **File:** `Alchemy/Include/Win32Compat.h`
- **Perbaikan:**
  - [x] Routing ke ukuran window SDL aktual (`SDL_GetWindowSize` /
    `SDL_GL_GetDrawableSize` untuk Retina) via `CScreenMgrSDL`.

### 5.3 GDI stubs return nullptr
- **File:** `Alchemy/Include/Kernel.h` (`CreateFont`, `CreateDIBitmap`,
  `CreateDIBSection`, `SetDIBits`, dll.)
- **Perbaikan:**
  - [x] Pastikan tidak ada jalur runtime macOS yang masih memanggilnya
    (grep call-site, arahkan semua ke `DIBSDL.cpp` / `SDLBitmap.cpp`).
    — `CreateDIBitmap`/`CreateDIBSection`/`SetDIBits`/`SelectPalette`/`RealizePalette`
    tidak dipanggil dari macOS code path. `CreateFont`/`DeleteObject` dipanggil
    dari fallback path (null-guarded, aman).
  - [x] Tambahkan `ASSERT(false)` / log di stub agar pemanggilan tak sengaja
    terdeteksi saat debug, bukan gagal diam-diam.

### 5.4 `CStringStub.cpp`
- **Bug:** Berisi `strToLower` yang return string apa adanya; saat ini sudah
  dikecualikan dari CMake tapi masih ada di tree.
- **Perbaikan:**
  - [x] Hapus file dari source tree dan hapus komentar di CMakeLists.txt.

---

## Fase 6 — Medium: Bersihkan `Win32Compat.h` (dead code yang tidak bisa compile)

- **File:** `Alchemy/Include/Win32Compat.h` — tidak di-include di mana pun,
  dan akan langsung error jika di-include:
  - `typedef LONGLONG LONGLONG;` (self-typedef, dipakai sebelum didefinisikan)
  - `typedef LONGLONG ULONGLONG;` konflik dengan `typedef DWORDLONG ULONGLONG;`
  - `typedef LONG_PTR SPONG_PTR;` — typo **SPONG_PTR**, `LONG_PTR` belum didefinisikan
  - `struct D3DRECT` didefinisikan dua kali
  - Macro `RGB` tanda kurungnya tidak seimbang
  - `LPARAM`/`LRESULT` di-typedef dua kali dengan tipe berbeda
- **Perbaikan:**
  - [x] Putuskan satu sumber kebenaran: konsolidasikan semua shim ke `Kernel.h`
    (atau sebaliknya), lalu **hapus** file yang tidak dipakai.
  - [x] Macro `min`/`max` di file ini menabrak `std::min`/`std::max` — hapus dan
    ganti call-site dengan `std::min`/`std::max` atau `Min`/`Max` milik engine.

---

## Fase 7 — Medium: Timing

### 7.1 `GetTickCount` berbasis epoch
- **File:** `Alchemy/Include/Win32Compat.h` / shim aktif di `Kernel.h`
- **Bug:** `gettimeofday` sejak epoch di-truncate ke `DWORD` → wrap tiap ~49,7 hari
  dengan titik wrap yang arbitrer; berbeda semantik dengan Windows (sejak boot).
- **Perbaikan:**
  - [x] Implementasi dengan `clock_gettime(CLOCK_MONOTONIC)` atau
    `mach_absolute_time`, dikurangi timestamp startup proses (sehingga mulai
    dari ~0 seperti Windows).

### 7.2 `CPeriodicWaiter` mencampur clock
- **File:** `Alchemy/Kernel/CPeriodicWaiter.cpp`
- **Bug:** Mencampur `QueryPerformanceCounter` dan `GetTickCount` pada basis
  waktu berbeda.
- **Perbaikan:**
  - [x] Satukan ke satu sumber monotonic; pastikan shim
    `QueryPerformanceCounter`/`QueryPerformanceFrequency` konsisten
    (mach_absolute_time + timebase).

---

## Fase 8 — Low: Build System (`CMakeLists.txt`)

- [x] Ganti path Homebrew hardcoded (`/opt/homebrew/Cellar/zlib/1.3.2/...`,
  `minizip/1.3.2_1`) dengan `find_package(ZLIB REQUIRED)` dan
  `pkg_check_modules(MINIZIP minizip)`.
- [x] Hapus variabel `MAMMOTH_TSE_SOURCES` yang didefinisikan tapi tidak dipakai
  (target `mammoth_tse` mendaftar source-nya sendiri) — membingungkan.
- [x] Tinjau `-Wno-non-pod-varargs`: warning ini sering menandakan bug nyata
  (CString dipassing ke varargs); idealnya perbaiki call-site lalu hapus flag.
  — Fixed: added 6 missing typed overloads for `kernelDebugLogPattern`,
  fixed 6 individual call-sites, removed `-Wno-non-pod-varargs` flag.

---

## Urutan Eksekusi yang Disarankan

| Urutan | Fase | Alasan |
|--------|------|--------|
| 1 | Fase 1 (File System) | Tanpa ini game tidak menemukan resource/save sama sekali |
| 2 | Fase 2 (Tipe Data) | Mencegah korupsi save/serialisasi sebelum data dibuat |
| 3 | Fase 4 (File I/O) | Dibutuhkan loader TDB/resource yang andal |
| 4 | Fase 5 (Silent stubs) | Menghilangkan kegagalan diam-diam saat debugging fase lain |
| 5 | Fase 3 (Networking) | Fitur online; bisa ditunda jika target awal offline |
| 6 | Fase 7 (Timing) | Kualitas frame pacing |
| 7 | Fase 6 + 8 (Cleanup) | Higienis, mencegah regresi di masa depan |

## Kriteria Verifikasi

- [x] `static_assert` ukuran tipe: `LONG`==4, `DWORD`==4, `LARGE_INTEGER`==8,
  `POINT`==8 byte.
- [ ] Save game bisa ditulis lalu dibaca ulang tanpa korupsi (round-trip test).
- [ ] `fileGetFileList` menemukan file di direktori test (termasuk pattern `*.sav`).
- [ ] Folder save terbuat di `~/Library/Application Support/Transcendence`.
- [ ] HTTP request ke Multiverse mengembalikan respons valid (cek port endian).
- [x] Build bersih tanpa warning redefinition di `Kernel.h`.
