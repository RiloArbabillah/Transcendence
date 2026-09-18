# Utilitas Input Platform macOS

Dokumen ini menjelaskan utilitas publik pada layer platform macOS yang dipakai untuk
menerjemahkan input SDL menjadi semantik Win32 yang dipanggil oleh engine.

## Document Status

- Version: v1.0
- Last Updated: 2026-09-19
- Branch: `osx`
- Terkait: `port-defect-register.md` (`PDR-001`), `compat-layer-bugfix-plan.md`

## Ruang Lingkup

Engine memanggil `GetAsyncKeyState()`/`GetKeyState()` dengan *virtual-key code* Win32
(`VK_*`), lalu menguji bit `0x8000` untuk menentukan apakah tombol sedang ditekan
(lihat `Alchemy/Include/Kernel.h`, `uiIsKeyDown`). Pada build macOS tidak ada
`GetAsyncKeyState()` native, sehingga layer platform harus menerjemahkan `VK_*` ke
state keyboard/mouse SDL.

Sebelum `PDR-001` ditutup, pemetaan ini hanya menangani sebagian kecil `VK_*`
(`VK_SHIFT`, `VK_CONTROL`, `VK_MENU`, `VK_NUMLOCK`, `A`–`Z`, `0`–`9`, dan beberapa tombol
navigasi). Akibatnya seluruh kontrol default pada
`Transcendence/Transcendence/DefaultKeyMappings.h` — termasuk `VK_LEFT`, `VK_RIGHT`,
`VK_SPACE`, `VK_TAB`, `VK_PAUSE`, `VK_F1/F2/F6/F7/F8/F9`, `VK_LBUTTON`, dan `VK_RBUTTON` —
selalu terbaca "tidak ditekan".

## Berkas

| Berkas | Peran |
|---|---|
| `Transcendence/Transcendence/Platform/PlatformInput.h` | deklarasi utilitas, definisi `VK_*` yang belum ada di `Kernel.h` |
| `Transcendence/Transcendence/Platform/PlatformInput.cpp` | tabel pemetaan `VK_*` → `SDL_Scancode` dan query state |

Keduanya dikompilasi hanya pada jalur CMake macOS (`platform_sdl` dan target tes
`mac_portability_tests`); berkas ini tidak dirujuk oleh proyek Visual Studio, sehingga
perilaku build Windows tidak berubah.

## API

### `SDL_Scancode PlatformVKToScancode(int vk)`

Fungsi murni tanpa efek samping dan tanpa dependensi SDL runtime: menerjemahkan
*virtual-key code* Win32 ke `SDL_Scancode` untuk tombol fisik yang sama.

- Menerima `'A'`–`'Z'` dan `'0'`–`'9'` (nilai `VK_*` untuk huruf/digit sama dengan kode
  ASCII huruf besar).
- Menerima tombol kontrol, navigasi, numpad, `F1`–`F12`, `VK_OEM_*`, dan seterusnya.
- Mengembalikan `SDL_SCANCODE_UNKNOWN` untuk tombol mouse (tidak punya scancode) dan untuk
  kode yang tidak dipetakan.

Karena fungsi ini murni, pemetaan bisa diuji unit tanpa menjalankan game, tanpa window,
dan tanpa inisialisasi subsistem video SDL.

### `SHORT PlatformAsyncKeyStateForState(int vk, const Uint8 *pKeyState, Uint32 dwMouseButtons, SDL_Keymod mod)`

Semantik `GetAsyncKeyState()` (mengembalikan `0x8000` bila tombol sedang ditekan)
dievaluasi terhadap *snapshot* state yang diberikan pemanggil, bukan terhadap state SDL
global. Urutan evaluasi:

1. `VK_LBUTTON`/`VK_RBUTTON`/`VK_MBUTTON` → bit pada `dwMouseButtons`
   (`SDL_BUTTON(SDL_BUTTON_*)`).
2. `VK_SHIFT`/`VK_CONTROL`/`VK_MENU`/`VK_NUMLOCK` → bit pada `mod` (`KMOD_*`); bila
   modifier state belum diperbarui, jatuh ke langkah 3.
3. `VK_SHIFT`/`VK_CONTROL`/`VK_MENU` → menerima kedua sisi fisik (`L`/`R`) pada
   `pKeyState`, sesuai perilaku Win32.
4. `pKeyState[PlatformVKToScancode(vk)]` → `0x8000` bila scancode tersebut aktif.

Mengembalikan `0` bila `pKeyState` bernilai `NULL` atau tidak ada kondisi yang terpenuhi.
Entry point ini ada agar pemetaan tombol dapat diuji tanpa window atau perangkat input
nyata.

### `SHORT PlatformGetAsyncKeyState(int vk)`

Pembungkus tipis yang memanggil `PlatformAsyncKeyStateForState` dengan state SDL live:
`SDL_GetKeyboardState(NULL)`, `SDL_GetMouseState(NULL, NULL)`, dan `SDL_GetModState()`.
Dipanggil oleh `Kernel.h` sebagai pengganti `GetAsyncKeyState()` Win32.

### `SHORT PlatformGetKeyState(int vk)`

Sama dengan `PlatformGetAsyncKeyState`, ditambah bit toggle `0x0001` untuk `VK_NUMLOCK`
bila `KMOD_NUM` aktif (meniru perilaku `GetKeyState` Win32 untuk lampu NumLock).

## Verifikasi

Gate untuk perubahan pada berkas ini:

1. `cmake --preset macos-debug`
2. `cmake --build --preset macos-debug`
3. `ctest --test-dir build/macos-debug -R mac-portability --output-on-failure`
4. `git diff --check`

Tes di `Transcendence/Tests/MacPortabilityTests.cpp` memverifikasi:

- setiap `VK_*` yang dipakai `DefaultKeyMappings.h` (`DEFAULT_MAP` dan `WASD_MAP`)
  terpetakan ke scancode yang benar;
- `PlatformAsyncKeyStateForState` mengembalikan `0` saat tombol naik dan `0x8000` saat
  scancode/bit tombol mouse terkait aktif;
- tombol mouse dan modifier `VK_SHIFT`/`VK_CONTROL`/`VK_MENU`/`VK_NUMLOCK` berperilaku
  sesuai semantik Win32.

## Catatan

- Perubahan pada berkas ini hanya menyentuh jalur macOS; build Windows tidak memakai
  `PlatformInput.cpp`.
- `PlatformAsyncKeyStateForState` sengaja memisahkan state dari SDL global agar perilaku
  input dapat diuji secara deterministik; jangan menambahkan panggilan SDL langsung ke
  fungsi ini.
