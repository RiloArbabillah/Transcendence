# Rencana Eksekusi: Port macOS Apple Silicon

## Ringkasan

Dokumen ini memecah pekerjaan port macOS Apple Silicon menjadi task kecil yang bisa diverifikasi dan mudah diperbarui selama implementasi berlangsung. Fokusnya adalah menyelesaikan jalur release-ready dari baseline yang sudah bisa build dan launch sampai menjadi `.app` yang layak diuji.

## Status Umum

- Status: `in_progress`
- Baseline source of truth:
  - `docs/macOS_port_status.md`
  - `docs/migrate_to_osx/minimax-m27-port-completion-plan.md`
  - `docs/migrate_to_osx/release-ready-execution-plan.md`
  - `docs/migrate_to_osx/qa-test-matrix.md`
- Strategi aktif: compatibility-first, pertahankan engine/gameplay, pakai SDL untuk shell, pertahankan software framebuffer untuk jalur aktif.

## Grafik Dependensi

```text
Build baseline (CMake presets + app target)
    |
    +-- Runtime baseline lock
    |
    +-- Visible frame + base/extension load stability
    |       |
    |       +-- Menu/input operable
    |       |       |
    |       |       +-- First playable gameplay
    |       |
    |       +-- Resource path correctness
    |
    +-- Save/settings writable path parity
    |
    +-- Native audio backend
    |
    +-- .app packaging + Finder launch
    |
    +-- Lifecycle/Retina/fullscreen QA
    |
    `-- Release QA gate
```

## Fase 1: Baseline dan blocker runtime teratas

### Task 1: Kunci baseline build dan runtime saat ini

**Status:** `completed`

**Deskripsi:** Pastikan baseline yang tertulis di dokumen masih benar di tree saat ini, supaya task berikutnya tidak mengejar asumsi yang stale.

**Acceptance criteria:**
- [x] `cmake --preset macos-debug` berhasil
- [x] `cmake --build "build" -j8` berhasil
- [x] `./build/Transcendence` membuka window dan masuk main loop
- [x] Top blocker runtime saat ini tercatat eksplisit dari hasil run nyata

**Verifikasi:**
- [x] Build lulus: `cmake --preset macos-debug && cmake --build "build" -j8`
- [x] Smoke run: `./build/Transcendence`
- [x] Cek manual minimum via runtime log: init selesai dan app masuk main loop; observasi visual GUI penuh masih perlu saat Task 3

**Dependensi:** Tidak ada

**File yang kemungkinan disentuh:**
- `CMakeLists.txt`
- `CMakePresets.json`
- `docs/macOS_port_status.md`
- `docs/migrate_to_osx/execution-task-plan.md`

**Perkiraan scope:** XS

**Catatan eksekusi:**
- Percobaan awal `cmake --preset macos-debug` gagal karena `build/macos-debug` sebelumnya dikonfigurasi dengan `Unix Makefiles`, sementara preset saat ini memakai `Ninja`.
- Direktori `build/macos-debug` dihapus lalu dikonfigurasi ulang dengan preset aktif, setelah itu build berhasil.
- Build berhasil dengan warning yang banyak tetapi tidak blocking.
- Smoke run `./build/macos-debug/Transcendence` menghasilkan log berikut:
  - `App_Run: start`
  - `App_Init: SDL_Init OK`
  - `App_Init: window created`
  - `App_Init: using SDL software renderer`
  - `App_Run: entering main loop`
- Dari hasil run saat ini, baseline build dan launch masih valid.
- Blocker aktif berikutnya tetap berada di runtime visual dan initialization path setelah main loop, terutama visible first frame atau menu presentation serta kestabilan load base or embedded extensions, sesuai status di `docs/macOS_port_status.md`.

**Hasil Task 1:**
- Preset `macos-debug` valid setelah reset direktori build preset.
- Target debug build berhasil di `build/macos-debug/Transcendence`.
- Aplikasi launch dan masuk SDL main loop pada jalur software renderer aktif.
- Tidak ada blocker baru di fase configure atau link; fokus berikutnya pindah ke blocker runtime visual dan load path.

### Task 2: Stabilkan load base file dan embedded extensions

**Status:** `in_progress`

**Deskripsi:** Hilangkan kegagalan inisialisasi background universe yang saat ini dilaporkan terpusat di `CExtensionCollection::LoadBaseFile`, karena tanpa ini frame/menu bisa tetap hitam atau startup tidak lengkap.

**Acceptance criteria:**
- [ ] Base file dari `Transcendence.xml` termuat tanpa crash fatal
- [ ] Embedded extensions tidak gagal di path init utama
- [ ] Startup tidak lagi berhenti di blocker yang sama

**Verifikasi:**
- [x] Build lulus: `cmake --build "build" -j8`
- [ ] Run di debugger bila perlu: `lldb ./build/Transcendence`
- [ ] Cek manual: startup melewati fase load universe/base data

**Dependensi:** Task 1

**File yang kemungkinan disentuh:**
- `Mammoth/TSUI/CExtensionCollection*`
- `Alchemy/CodeChain/*`
- `Alchemy/Kernel/CSymbolTable.cpp`
- `Alchemy/Kernel/CDictionary.cpp`

**Perkiraan scope:** M

**Catatan eksekusi:**
- Slice pertama untuk Task 2 difokuskan ke instrumentasi sempit di `Mammoth/TSE/CExtensionCollection.cpp` agar load error embedded extension punya konteks file yang lebih jelas.
- Perubahan yang ditambahkan:
  - log nama file atau elemen embedded extension sebelum `LoadEmbeddedExtension`
  - isi `Ctx.sErrorFilespec` dengan format `base#embedded-file` atau `base#<tag:unid>`
  - propagasi `sErrorFilespec` ke `ExtCtx`
  - pemulihan entity resolver lebih aman pada jalur gagal `CreateExtension`
- Rebuild `macos-debug` berhasil setelah perubahan.
- Smoke run masih hanya menunjukkan app berhasil init dan masuk main loop; `build/macos-debug/Debug.log` belum memuat log baru dari `LoadBaseFile`, jadi blocker sekarang belum cukup terlokalisasi dari run non-interaktif ini.
- Kesimpulan sementara: belum ada bukti bahwa crash atau failure aktif sudah terpicu dalam smoke run headless yang dipakai sekarang. Langkah berikutnya adalah menjalankan di `lldb` atau menambah titik log lebih dekat ke jalur background init yang benar-benar dieksekusi.
- Hasil `lldb` mengubah diagnosis blocker aktif:
  - Crash pertama yang benar-benar terpicu bukan di `LoadBaseFile`, tetapi di `CTranscendenceController::CleanUpUpgrade` saat memanggil `kernelDebugLogPattern("Unable to delete file: %s.", FilesToDelete[i])`.
  - Di macOS arm64, overload `kernelDebugLogPattern` yang meneruskan `CString` ke varargs menghasilkan argumen rusak; crash muncul di `Kernel::CString::Append` saat formatting log.
  - Untuk membuka jalan ke blocker berikutnya, log cleanup itu dipersempit menjadi indeks file saja di `Transcendence/Transcendence/CTranscendenceController.cpp`.
  - Setelah workaround tersebut, app tidak crash pada cleanup dan `lldb` berhasil mencapai breakpoint `CExtensionCollection::LoadBaseFile` di thread background.
  - Run yang sama juga mengeluarkan error visual penting dari macOS image loader: `Resources\Title.JPG` tidak bisa dibuka karena path masih memakai backslash Windows di path resource (`.../Resources\Title.JPG`).
- Artinya blocker aktif sudah bergeser lagi:
  1. varargs logging dengan `CString` di macOS arm64 tidak aman dan perlu hardening lebih luas,
  2. resource path normalization Windows-to-macOS masih memblokir first visible frame.
- Slice berikutnya memperbaiki normalisasi separator di `Alchemy/Kernel/Path.cpp`.
  - `pathAddComponent` sekarang memakai `/` pada `TARGET_PLATFORM_MACOS` alih-alih selalu menambahkan `\`.
  - Setelah rebuild, error stdout dari macOS image loader untuk `Resources\Title.JPG` tidak muncul lagi pada smoke run berikutnya.
  - Jalur `LoadBaseFile` tetap tercapai di `lldb`, sehingga path fix tidak memutus background initialization.
  - Smoke run setelah fix masih masuk main loop, tetapi belum memberi bukti cukup bahwa title or menu frame sudah benar-benar visible dalam run non-interaktif.

**Hasil sementara Task 2:**
- Instrumentasi error context untuk embedded extension load sudah ditambahkan.
- Build tetap hijau.
- `LoadBaseFile` terbukti benar-benar dieksekusi di background thread.
- Root cause awal yang terkonfirmasi oleh `lldb` adalah crash pada logging varargs, bukan embedded extension load itu sendiri.
- Blokir path resource `Title.JPG` yang terlihat di stdout sudah ditangani melalui normalisasi separator path di macOS.
- Task tetap `in_progress` karena visible first frame dan jalur background load penuh masih perlu dibuktikan setelah path fix ini.

### Checkpoint: Setelah Task 1-2

- [ ] Build debug tetap hijau
- [ ] Blocker aktif sudah bergeser dari init/load failure ke UI/render/input atau gameplay
- [ ] Ada catatan observasi runtime yang bisa dijadikan baseline berikutnya

## Fase 2: Menu dan input usable

### Task 3: Tampilkan first visible frame/title/menu secara konsisten

**Status:** `pending`

**Deskripsi:** Setelah inisialisasi stabil, fokus pada keluarnya frame pertama yang benar sehingga user bisa melihat title/menu, bukan black window.

**Acceptance criteria:**
- [ ] Window tidak lagi hitam pada first frame
- [ ] Title/loading/menu frame tampil konsisten di run normal
- [ ] Tidak ada corrupt warna/alpha yang membuat menu tidak terbaca

**Verifikasi:**
- [ ] Build lulus: `cmake --build "build" -j8`
- [ ] Run: `./build/Transcendence`
- [ ] Cek manual: title/menu terlihat dan readable

**Dependensi:** Task 2

**File yang kemungkinan disentuh:**
- `Transcendence/Transcendence/GameUIBridge.cpp`
- `AppCore.cpp`
- `Mammoth/TSUI/CHumanInterfaceMac.cpp`

**Perkiraan scope:** S-M

### Task 4: Rapikan packing dan dispatch mouse messages ala Win32

**Status:** `pending`

**Deskripsi:** Selaraskan SDL bridge dengan kontrak Win32-style yang diharapkan engine, terutama `wParam/lParam`, mapping button, dan dispatch handler HI.

**Acceptance criteria:**
- [ ] Mouse move/click/wheel dipack dan didecode konsisten
- [ ] Right/middle button tidak tertukar
- [ ] Handler `WM_*` yang dipost benar-benar di-dispatch ke handler yang sesuai

**Verifikasi:**
- [ ] Build lulus: `cmake --build --preset macos-debug --target transcendence_app`
- [ ] Cek manual: hover, left click, right click, wheel pada menu/settings flow
- [ ] Cek manual: setelah resize, hit testing tetap benar

**Dependensi:** Task 3

**File yang kemungkinan disentuh:**
- `AppCore.cpp`
- `Transcendence/Transcendence/GameUIBridge.cpp`
- `Mammoth/TSUI/CHumanInterfaceMac.cpp`
- `Mammoth/Include/TSUI.h`

**Perkiraan scope:** M

### Task 5: Rapikan keyboard mapping dan text input

**Status:** `pending`

**Deskripsi:** Pastikan event keyboard SDL diterjemahkan ke virtual-key/command model engine tanpa menyebabkan text double insert atau hotkey salah.

**Acceptance criteria:**
- [ ] Navigasi menu via keyboard bekerja
- [ ] Enter/escape/confirm/cancel bekerja
- [ ] Satu flow text entry menerima input tanpa double insert
- [ ] Command/navigation key tidak menyuntik teks

**Verifikasi:**
- [ ] Build lulus: `cmake --build --preset macos-debug --target transcendence_app`
- [ ] Cek manual: menu nav, satu hotkey huruf/angka, satu text-entry flow
- [ ] Cek manual: key repeat tidak flood dan tidak skip liar

**Dependensi:** Task 4

**File yang kemungkinan disentuh:**
- `Transcendence/Transcendence/CGameKeys.cpp`
- `Transcendence/Transcendence/GameSessionInput.cpp`
- `Alchemy/DirectXUtil/CAniTextInput.cpp`

**Perkiraan scope:** M

### Checkpoint: Setelah Task 3-5

- [ ] M4 minimum usable-menu checks tercapai
- [ ] Menu terlihat dan bisa dipakai dengan keyboard dan mouse
- [ ] Satu text flow tervalidasi

## Fase 3: First playable

### Task 6: Buka New Game sampai masuk gameplay

**Status:** `pending`

**Deskripsi:** Validasi jalur vertikal pertama dari menu ke gameplay, tanpa memperluas scope ke semua sistem gameplay.

**Acceptance criteria:**
- [ ] User bisa pilih New Game
- [ ] In-game state tercapai tanpa crash fatal
- [ ] Kontrol dasar kapal atau gerak merespons

**Verifikasi:**
- [ ] Build lulus: `cmake --build "build" -j8`
- [ ] Cek manual: launch -> New Game -> masuk gameplay
- [ ] Ulangi flow minimal 3 kali

**Dependensi:** Task 5

**File yang kemungkinan disentuh:**
- `Transcendence/Transcendence/CIntroSession.cpp`
- `Transcendence/Transcendence/IntroScreen.cpp`

**Perkiraan scope:** S-M

### Task 7: Pulihkan HUD, dock, dan overlay kritis yang dibutuhkan first playable

**Status:** `pending`

**Deskripsi:** Lengkapi coverage rendering dan UI gameplay minimum yang benar-benar dibutuhkan untuk sesi main singkat.

**Acceptance criteria:**
- [ ] HUD terlihat dan terbaca
- [ ] Dock, map, atau overlay representatif bisa dibuka bila flow mencapainya
- [ ] Tidak ada crash kritis pada surface UI gameplay utama

**Verifikasi:**
- [ ] Build lulus: `cmake --build "build" -j8`
- [ ] Cek manual: gameplay singkat, buka HUD atau overlay bila reachable
- [ ] Cek manual: pause, help, atau menu in-game tidak crash

**Dependensi:** Task 6

**File yang kemungkinan disentuh:**
- file HUD, dock, map, dan UI gameplay terkait

**Perkiraan scope:** M

### Task 8: Stabilkan short play loop

**Status:** `pending`

**Deskripsi:** Ubah first-playable dari "sekali masuk" menjadi "bisa dimainkan singkat dengan cukup stabil".

**Acceptance criteria:**
- [ ] Launch -> New Game -> play briefly -> quit berhasil minimal 3 kali
- [ ] Satu sesi 20-30 menit basic play tidak crash kritis, atau blocker baru terdokumentasi jelas
- [ ] Crash tersisa sudah terklasifikasi, bukan random atau belum ditriase

**Verifikasi:**
- [ ] Run berulang manual
- [ ] Jika crash: stack trace LLDB dan klasifikasi akar masalah
- [ ] Cek manual: gameplay inti tetap usable

**Dependensi:** Task 7

**File yang kemungkinan disentuh:**
- tergantung crash aktif

**Perkiraan scope:** S-M

### Checkpoint: Setelah Task 6-8

- [ ] M5 required checks tercapai atau ada exception non-blocking yang terdokumentasi
- [ ] Ada vertical slice penuh menu -> gameplay -> quit

## Fase 4: Runtime parity macOS

### Task 9: Pisahkan resource root dan writable root

**Status:** `pending`

**Deskripsi:** Jadikan resource path dan write path platform-owned, sehingga debug run tidak bergantung pada current working directory dan bundle run tidak menulis ke bundle.

**Acceptance criteria:**
- [ ] Resource lookup berhasil dari repo run dan non-repo CWD
- [ ] Writable path tidak lagi mengikuti bundle atau CWD
- [ ] Logging path terpilih cukup jelas untuk diagnosis

**Verifikasi:**
- [ ] Launch dari repo root
- [ ] Launch dari working directory lain
- [ ] Cek manual: resource tetap ketemu

**Dependensi:** Task 8

**File yang kemungkinan disentuh:**
- `Transcendence/Transcendence/CResourcePathResolver.cpp`
- platform path helper code

**Perkiraan scope:** S-M

### Task 10: Implement save/settings path macOS

**Status:** `pending`

**Deskripsi:** Pindahkan save dan settings ke lokasi writable standar macOS seperti `~/Library/Application Support/Transcendence/`.

**Acceptance criteria:**
- [ ] Save file dibuat di user-writable macOS path
- [ ] Save bisa diload ulang setelah restart
- [ ] Settings persisten setelah restart
- [ ] Tidak ada file runtime yang dibuat di `.app` bundle

**Verifikasi:**
- [ ] Cek manual: save, quit, relaunch, load
- [ ] Cek manual: ubah setting, relaunch, setting tetap ada
- [ ] Inspeksi filesystem: file muncul di Application Support, bukan bundle

**Dependensi:** Task 9

**File yang kemungkinan disentuh:**
- `Transcendence/Transcendence/CGameSettings.cpp`
- `Transcendence/Transcendence/GameSettings.h`
- `Mammoth/TSUI/CUserSettings.cpp`
- `Mammoth/TSUI/CListSaveFilesTask.cpp`
- `Mammoth/TSE/CGameFile.cpp`

**Perkiraan scope:** M

### Checkpoint: Setelah Task 9-10

- [ ] M6 path-related checks lulus
- [ ] Runtime tidak lagi tergantung repo CWD untuk perilaku inti
- [ ] Save, load, dan settings punya parity macOS minimum

## Fase 5: Audio native

### Task 11: Audit requirement audio nyata

**Status:** `pending`

**Deskripsi:** Sebelum mengganti stub, tentukan format aset dan perilaku playback minimum yang benar-benar dibutuhkan release.

**Acceptance criteria:**
- [ ] Format file SFX dan music yang dipakai teridentifikasi
- [ ] Kebutuhan overlap, loop, pause/resume, volume/mute terdokumentasi
- [ ] Ada keputusan backend kecil yang cukup untuk memenuhi kebutuhan

**Verifikasi:**
- [ ] Audit code dan asset references
- [ ] Catat hasil di session report atau plan update

**Dependensi:** Task 8

**File yang kemungkinan disentuh:**
- `Mammoth/TSUI/CSoundtrackManager.cpp`
- `Alchemy/DirectXUtil/Sound.cpp`
- `docs/migrate_to_osx/execution-task-plan.md`

**Perkiraan scope:** XS-S

### Task 12: Ganti stub dengan backend audio minimal yang release-usable

**Status:** `pending`

**Deskripsi:** Implementasikan backend terkecil yang memenuhi kebutuhan nyata, sambil menjaga kontrak high-level soundtrack manager.

**Acceptance criteria:**
- [ ] Menu atau UI SFX bekerja bila ada
- [ ] Gameplay SFX bekerja
- [ ] Music play, stop, pause, dan resume memadai
- [ ] Tidak ada dependency MCI atau DirectSound aktif di path macOS

**Verifikasi:**
- [ ] Build lulus
- [ ] Cek manual: menu audio
- [ ] Cek manual: gameplay audio
- [ ] Cek manual: volume atau mute bila ada setting

**Dependensi:** Task 11

**File yang kemungkinan disentuh:**
- `Mammoth/TSUI/CMCIMixerStub.cpp`
- `Mammoth/TSUI/CSoundtrackManager.cpp`
- `Alchemy/DirectXUtil/Sound.cpp`

**Perkiraan scope:** M

### Checkpoint: Setelah Task 11-12

- [ ] M6 audio checks lulus atau exception non-blocking jelas
- [ ] Audio tidak lagi silent-stub untuk release candidate path

## Fase 6: Packaging dan QA release

### Task 13: Lengkapi bundle `.app` dan packaging resource

**Status:** `pending`

**Deskripsi:** Hasil build harus menjadi `.app` yang bisa berjalan di luar terminal dan di luar repo, dengan resource minimum terpaket.

**Acceptance criteria:**
- [ ] Executable berada di `.app/Contents/MacOS/`
- [ ] Resource wajib berada di `.app/Contents/Resources/`
- [ ] Dependency runtime SDL terselesaikan untuk Finder launch
- [ ] Save/settings tetap mengarah ke lokasi writable user

**Verifikasi:**
- [ ] Build release: `cmake --preset macos-release && cmake --build --preset macos-release --target transcendence_app`
- [ ] Cek isi bundle
- [ ] Launch via path `.app` dari luar repo

**Dependensi:** Task 10, Task 12

**File yang kemungkinan disentuh:**
- `CMakeLists.txt`
- `CMakePresets.json`
- packaging/resource-copy rules
- `Transcendence/Transcendence/CResourcePathResolver.cpp`

**Perkiraan scope:** M

### Task 14: Validasi Finder launch, lifecycle, Retina, resize, relaunch

**Status:** `pending`

**Deskripsi:** Tutup gap perilaku desktop native yang umum sebelum menyebut build ini release-candidate.

**Acceptance criteria:**
- [ ] Launch dari Finder berhasil
- [ ] Relaunch setelah quit berhasil
- [ ] Resize, minimize, focus loss-regain tidak crash
- [ ] Hit testing tetap benar setelah resize atau Retina
- [ ] Fullscreen stabil atau limitation-nya terdokumentasi

**Verifikasi:**
- [ ] Cek manual: Finder launch
- [ ] Cek manual: resize, minimize, focus, relaunch
- [ ] Cek manual: pointer accuracy setelah resize atau high-DPI

**Dependensi:** Task 13

**File yang kemungkinan disentuh:**
- platform SDL or macOS window files
- presenter sizing files
- bundle metadata or config

**Perkiraan scope:** S-M

### Task 15: Eksekusi QA gate M2-M7 dan susun RC blocker list

**Status:** `pending`

**Deskripsi:** Jalankan gate wajib sesuai `qa-test-matrix.md` dan hasilkan daftar blocker final yang benar-benar tersisa.

**Acceptance criteria:**
- [ ] Semua required check M2-M7 dieksekusi
- [ ] Setiap fail punya evidence dan owner
- [ ] Ada keputusan jelas: RC ready atau belum

**Verifikasi:**
- [ ] Checklist QA lengkap
- [ ] Bukti build, run, dan manual test tercatat
- [ ] Known issues dibedakan antara blocking vs non-blocking

**Dependensi:** Task 14

**File yang kemungkinan disentuh:**
- `docs/macOS_port_status.md`
- `docs/migrate_to_osx/qa-test-matrix.md`
- release notes atau status docs

**Perkiraan scope:** S

### Checkpoint: Selesai

- [ ] Semua acceptance criteria task inti terpenuhi
- [ ] Ada vertical slice penuh dari launch sampai quit
- [ ] `.app` bisa dipakai tester tanpa setup dev
- [ ] Siap review manusia untuk cut release candidate

## Risiko dan Mitigasi

| Risiko | Dampak | Mitigasi |
|---|---|---|
| Crash arm64 tersisa di pointer atau int storage lama | Tinggi | Debug dengan LLDB, klasifikasikan dulu, perbaiki sempit di path aktif |
| Black screen ternyata gabungan renderer dan resource init | Tinggi | Pisahkan validasi deterministic frame, lalu real frame, lalu resource-backed UI |
| Input terlihat hidup tapi kontrak Win32 bridge masih salah | Sedang | Pakai checklist M4 lengkap, bukan smoke test seadanya |
| Path debug vs bundle bercampur | Tinggi | Pisahkan resource root dan writable root sebelum packaging final |
| Audio backend terlalu besar scope-nya | Sedang | Audit format dulu, pilih backend minimum yang cukup |
| Hardcoded build paths atau Homebrew path di CMake | Sedang | Bereskan sebelum packaging dianggap reproducible |

## Catatan Sesi

Gunakan format ini setiap selesai satu slice kerja:

```text
Slice:
Files changed:
Build command:
Build result:
Runtime/manual test:
Observed result:
Remaining blockers:
Next recommended slice:
```
