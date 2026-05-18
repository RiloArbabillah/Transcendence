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
- Verifikasi lanjutan untuk first-frame dilakukan dengan dua cara:
  - `CTranscendenceController::OnInit` sekarang mengembalikan error jika `m_HI.ShowSession(new CLoadingSession(...))` gagal, sehingga kegagalan loading session tidak lagi diam-diam diabaikan.
  - `CLoadingSession::OnInit` diberi log sempit ke stderr.
- Hasilnya:
  - `CLoadingSession::OnInit` sukses penuh: title image, stargate image, dan stargate mask semua berhasil ditemukan dan dimuat.
  - Tetapi log `CLoadingSession::OnPaint` pertama tidak pernah muncul pada smoke run.
  - Di bawah `lldb`, proses berhenti pada thread background selama `CTranscendenceModel::InitBackground -> CUniverse::Init -> CExtensionCollection::LoadBaseFile` sebelum breakpoint `CLoadingSession::OnPaint` pernah terpukul.
- Interpretasi saat ini:
  - loading session berhasil dibuat,
  - asset menu utama berhasil dimuat,
  - tetapi first paint belum terjadi atau belum mencapai breakpoint paint sebelum background universe init memicu stop di `LoadBaseFile` path.
  - Jadi blocker aktif yang paling kuat sekarang kembali ke background initialization, bukan lagi loading-screen resource lookup.
- Investigasi `lldb` lanjutan pada jalur background init memberi lokasi crash yang lebih konkret:
  - Setelah log `LoadBaseFile` disederhanakan, stop di background thread bergeser menjadi `EXC_BAD_ACCESS` nyata, bukan lagi berhenti di log line.
  - Backtrace aktif sekarang masuk ke:
    - `SHAUpdate`
    - `CDigest::CDigest`
    - `cryptoCreateDigest`
    - `fileCreateDigest`
    - `CResourceDb::ComputeFileDigest`
    - `CExtensionCollection::LoadBaseFile`
  - Ini berarti blocker aktif saat ini berada pada perhitungan digest file base (`Transcendence.xml`/resource db) di background init, sebelum load embedded extension berjalan jauh dan sebelum first paint terkonfirmasi.
  - Dengan kata lain, first-frame blocker sekarang punya akar teknis yang lebih sempit: digest or file-read path di `SecureHashAlgorithm.cpp` / `CResourceDb::ComputeFileDigest`, bukan loading-session setup.

**Hasil sementara Task 2:**
- Instrumentasi error context untuk embedded extension load sudah ditambahkan.
- Build tetap hijau.
- `LoadBaseFile` terbukti benar-benar dieksekusi di background thread.
- Root cause awal yang terkonfirmasi oleh `lldb` adalah crash pada logging varargs, bukan embedded extension load itu sendiri.
- Blokir path resource `Title.JPG` yang terlihat di stdout sudah ditangani melalui normalisasi separator path di macOS.
- `CLoadingSession::OnInit` kini terbukti berhasil, tetapi `OnPaint` pertama belum terbukti tercapai.
- Task tetap `in_progress` karena jalur background `LoadBaseFile` sekarang terlokalisasi lebih jauh ke crash digest file di `SecureHashAlgorithm.cpp` dan `CResourceDb::ComputeFileDigest`.
- Fresh rebuild dari folder `build` yang dibersihkan mengonfirmasi bahwa diagnosis tidak bergantung pada artefak build lama.
- Setelah fix mmap/digest pada `CFileReadBlock.cpp`, blocker bergerak lebih jauh ke load embedded extension dan sekarang berhenti konsisten di `CEffectGroupCreator::OnEffectCreateFromXML`.
- Stop aktif terjadi pada pembentukan sub-UNID effect group:
  - `CString sSubUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, i);`
- Ini mempersempit blocker aktif berikutnya ke kombinasi `CString` / `strPatternSubst` pada jalur effect XML loading di Apple Silicon.
- Slice berikutnya menormalkan resource path gambar di `CObjectImage.cpp`, karena `lldb` menunjukkan `m_sBitmap` masih berisi path Windows-style seperti `Resources\DockScreenBackground.jpg`.
- Setelah normalisasi backslash -> slash pada path gambar, blocker maju lagi. Stop aktif berikutnya sekarang berada di `CWeaponFireDesc::InitFromXML` saat memanggil `strPatternSubst("%s:e", m_sUNID)` untuk membentuk UNID effect weapon.
- Ini menguatkan pola bahwa beberapa jalur `strPatternSubst` dengan `CString` pada background init masih rapuh di Apple Silicon, sementara blocker path resource gambar spesifik sudah berhasil dilewati.
- Patch berikutnya mengganti pembentukan `"%s:e"` di `CWeaponFireDesc::InitFromXML` dengan append manual, dan jalur tersebut berhasil dilewati.
- Setelah rebuild dan `lldb`, blocker maju lagi ke jalur serupa pada weapon effects:
  - `CWeaponFireDesc::InitFromXML`
  - `strPatternSubst("%s:h", m_sUNID)` untuk `m_pHitEffect.LoadEffect(...)`
- Ini mengonfirmasi pola yang konsisten: blocker aktif bukan satu bug tunggal pada data, melainkan beberapa titik pembentukan ID turunan berbasis `strPatternSubst` dan `CString` di background design loading pada Apple Silicon.
- Sweep lanjutan dilakukan pada sekumpulan file efek/topologi yang membentuk derived UNID dengan pola `"%s/..."`, `"%s:..."`, atau `"%s/%d"`, termasuk `SFXVariants.cpp`, `SFXSequencer.cpp`, `CTopologyDesc.cpp`, `CTableTopologyProc.cpp`, `CRandomPointsProc.cpp`, `CPartitionNodesProc.cpp`, `CObjectEffectDesc.cpp`, `CLocateNodesProc.cpp`, `CGroupTopologyProc.cpp`, `CFillNodesProc.cpp`, dan `CConquerNodesProc.cpp`.
- Setelah sweep itu, blocker berpindah dari effect or particle UNID builders ke `CTradingDesc::ComputeID`, tepatnya pada `strPatternSubst(CONSTLIT("%s:%s"), sService, sCriteria)`.
- Ini menunjukkan sweep pada jalur active background design loading berhasil mendorong init jauh lebih dalam, dan pola rapuh sekarang meluas dari UNID turunan ke pembentukan ID trading berbasis `CString`.
- Sweep tambahan berikutnya mencakup `CSystemMap::OnCreateFromXML`, `CHexarc.cpp`, `CHexarcSession.cpp`, `CGameStats.cpp`, dan `CTradingDesc.cpp` untuk menghapus pola string-builder sejenis yang masih tersisa di jalur aktif.
- Setelah rebuild dan rerun `lldb`, blocker bergerak lagi dari `CTradingDesc::ComputeID` ke `CLanguageDataBlock::InitFromXML`, tepatnya pada pembentukan ID script:
  - `sID = strPatternSubst(CONSTLIT("%s%s"), SCRIPT_ID_PREFIX, sID);`
- Ini menunjukkan bahwa jalur background init terus maju dan pola masalah kini mencakup prefiks string berbasis `CString`, bukan hanya suffix atau path-derived IDs.
- Sweep berikutnya juga mencakup titik sejenis yang tersisa di `CLanguageDataBlock.cpp`, `CSystemMap.cpp`, dan helper terkait, lalu build dan `lldb` dijalankan lagi sekali.
- Setelah sweep tersebut, background init bergerak lebih jauh lagi, melewati load design XML dan masuk ke eksekusi globals CodeChain.
- Blocker aktif terbaru sempat berada di `CCLambda::initDesc`:
  - `m_sDesc = strPatternSubst(CONSTLIT("(%s %s)\n\n%s\n"), sKey, pLambdaArgs->Print(PRFLAG_NO_LIST_LAMBDA_ARGS), sHelp);`
- Slice lanjutan mengganti builder deskripsi lambda di `Alchemy/CodeChain/CCLambda.cpp` menjadi append manual untuk menghindari formatting multi-arg berbasis `CString` pada Apple Silicon.
- Setelah rebuild dan rerun `lldb`, crash tidak lagi berhenti di `CCLambda::initDesc`; background init maju lagi sampai merge XML di `Alchemy/XMLUtil/CXMLElement.cpp`.
- Stop aktif berikutnya muncul di `CXMLElement::SetAttributesFromMerge` pada pembentukan keyword atribut:
  - `m_Keywords.Atomize(strPatternSubst(CONSTLIT("attrib.%s"), A.GetAttributeName(iAPos)))`
- Patch berikutnya mengganti pembentukan `attrib.<name>` itu dengan append manual.
- Setelah rebuild dan rerun `lldb`, app tidak lagi crash pada dua titik tersebut selama jendela observasi 120 detik.
- Smoke run biasa sekarang konsisten mencapai:
  - `CLoadingSession::OnInit done`
  - `App_Run: entering main loop`
  - `App_Run: exit main loop`
  - `App_Shutdown: done`
- Namun log `CLoadingSession::OnPaint first paint` masih belum muncul, sehingga blocker aktif tidak lagi berupa crash background init yang sama, tetapi belum terkonfirmasi menjadi visible first frame/title-menu paint.
- Slice lanjutan pada bridge macOS menyamakan inisialisasi `CHumanInterface` dengan jalur Win32 minimum: `WMCreate` sekarang menginisialisasi sound manager, background processors, visuals, dan `CScreenMgrSDL` sebelum sesi HI dipakai.
- Setelah parity init itu aktif, crash berikutnya berpindah dari bridge awal ke pembuatan `CIntroSession`.
- Root cause yang terkonfirmasi adalah `STranscendenceSessionCtx` belum diisi sebelum `CMD_MODEL_INIT_DONE` memanggil `new CIntroSession(...)`; akibatnya `CreateCtx.pSettings`/pointer lain masih `NULL` saat konstruktor dan `OnInit` intro dipanggil.
- `CTranscendenceController::OnInit` sekarang mengisi `m_SessionCtx.pHI`, `pModel`, `pSettings`, `pDebugConsole`, dan `pSoundtrack` segera setelah `g_pTrans` dibuat, sebelum background init dan sebelum sesi apa pun dapat diluncurkan.
- Verifikasi setelah patch:
  - `cmake --build --preset macos-debug --target transcendence_app` tetap berhasil.
  - `build/macos-debug/Debug.log` sekarang maju melewati:
    - `CMD_MODEL_INIT_DONE: showing intro session`
    - `CIntroSession::OnInit start`
    - `CIntroSession::OnInit visuals OK`
    - `CIntroSession::OnInit cursor set`
    - `CIntroSession::OnInit options OK`
    - `CIntroSession::OnInit widescreen rect OK`
    - `CIntroSession::OnInit screen=1024x768 bar=128`
- Ini menutup blocker `m_SessionCtx`/`pSettings` null pada intro init. Blocker aktif berikutnya kembali ke first visible frame/menu presentation setelah intro session berhasil mulai inisialisasi.
- Slice berikutnya memajukan blocker visual pertama dengan dua perbaikan terpisah:
  - sweep pointer-safe object reference untuk CodeChain/TSE digeser dari `CreateInteger((intptr_t)...)` ke helper object-pointer path pada jalur aktif (`CCExtensions.cpp`, `CTranscendenceModel.cpp`, dan helper terkait), sehingga crash dereference `CreateShipObjFromItem -> CreateObjFromItem` di arm64 tidak lagi terpicu saat intro/game stats memakai object refs.
  - loader JPEG non-Windows di `Alchemy/IntelJPEGUtil/Load.cpp` tidak lagi mengembalikan pointer mentah ke buffer `CString`, tetapi sekarang membungkus hasil decode ke `SDLBitmap`/`HBITMAP` kompatibel yang valid untuk `dibGetInfo` dan `CG32bitImage::CreateFromBitmap`.
- Verifikasi setelah rebuild menunjukkan startup sekarang konsisten melewati:
  - `CLoadingSession::OnPaint first paint`
  - `CIntroSession::OnAnimate first frame`
  - `CIntroSession::Paint first frame`
- Dengan demikian blocker black-window murni sudah terlewati: first paint title/intro berhasil tercapai pada smoke run normal.
- Namun blocker runtime belum selesai sepenuhnya. `build/macos-debug/Debug.log` sekarang menunjukkan crash lebih lanjut di intro viewport path:
  - `Crash in CalcViewportCtx`
  - `Crash in PaintViewport`
  - `CException: Out of memory.`
- Error `Unable to create bitmap from image: Resources/DeepSpaceBackground.jpg` yang sebelumnya muncul sebelum crash viewport sudah hilang setelah perbaikan JPEG/HBITMAP bridge, jadi blocker aktif berikutnya menyempit ke kalkulasi/presentasi viewport intro, bukan lagi image decode failure atau black first frame.
- Artinya Task 2 secara efektif sudah menutup blocker background init sebelumnya dan memverifikasi first paint, tetapi pekerjaan aktif sekarang bergeser ke Task 3: stabilisasi intro viewport/menu presentation setelah first frame.

### Checkpoint: Setelah Task 1-2

- [x] Build debug tetap hijau
- [x] Blocker aktif sudah bergeser dari init/load failure ke UI/render/input atau gameplay
- [x] Ada catatan observasi runtime yang bisa dijadikan baseline berikutnya

## Fase 2: Menu dan input usable

### Task 3: Tampilkan first visible frame/title/menu secara konsisten

**Status:** `in_progress`

**Deskripsi:** Setelah inisialisasi stabil, fokus pada keluarnya frame pertama yang benar sehingga user bisa melihat title/menu, bukan black window.

**Acceptance criteria:**
- [x] Window tidak lagi hitam pada first frame
- [x] Title/loading/menu frame tampil konsisten di run normal
- [ ] Tidak ada corrupt warna/alpha yang membuat menu tidak terbaca

**Verifikasi:**
- [x] Build lulus: `cmake --build --preset macos-debug --target transcendence_app -j8`
- [x] Run: `./build/macos-debug/Transcendence`
- [ ] Cek manual: title/menu terlihat dan readable

**Dependensi:** Task 2

**File yang kemungkinan disentuh:**
- `Transcendence/Transcendence/GameUIBridge.cpp`
- `AppCore.cpp`
- `Mammoth/TSUI/CHumanInterfaceMac.cpp`

**Perkiraan scope:** S-M

**Catatan eksekusi:**
- Sweep object-reference Apple Silicon pada jalur aktif intro/UI diteruskan agar pointer `CSpaceObject *` tidak lagi dipaksa lewat atom integer 32-bit-style di CodeChain/TSE.
- Perubahan penting yang sekarang aktif:
  - `CCodeChainCtx::DefineSpaceObject(const CSpaceObject &)` tidak lagi memanggil `DefineGlobalInteger((intptr_t)&Obj)`, tetapi memakai helper object-pointer path yang konsisten dengan `CreateObjPointer`.
  - `CCUtil.cpp` dan `CCreatePainterCtx.cpp` sekarang menyimpan referensi object di symbol table melalui `SetAt(..., CreateObjPointer(...))` atau helper ekuivalen, bukan `SetIntegerAt((intptr_t)...)`.
  - Validasi object-ref di `CCExtensions.cpp` untuk jalur data object sekarang membaca pointer lewat `GetObjPointerValue(...)` sebelum `CObject::IsValidPointer(...)`.
- Rebuild verifikasi sukses dengan:
  - `cmake --build --preset macos-debug --target transcendence_app -j8`
- Smoke run terbaru dengan log segar (`rm -f build/macos-debug/Debug.log && ./build/macos-debug/Transcendence`) sekarang kembali konsisten mencapai:
  - `CIntroSession::OnAnimate first frame`
  - `CIntroSession::Paint first frame`
  - `CIntroSession::Paint calling Render`
- Verifikasi tambahan lewat PTY run 60 detik (`./build/macos-debug/Transcendence`) juga menunjukkan jalur loading/intro stabil selama jendela observasi:
  - stdout mencapai `CLoadingSession::OnPaint first paint`
  - `build/macos-debug/Debug.log` kembali mencapai `CIntroSession::Paint calling Render`
  - saat harness menghentikan proses karena timeout, log aplikasi menutup dengan `App_Run: exit main loop` dan `App_Shutdown: done`, bukan dengan signature crash baru
- Selama jendela observasi proses background ~30+ detik, tidak muncul lagi watched error seperti `Crash in `, `CException:`, `EXC_BAD_ACCESS`, `Unable to create bitmap`, atau `segmentation fault`, dan `Debug.log` tidak bertambah dengan crash intro viewport yang sebelumnya pernah tercatat.
- Dengan demikian, blocker `CalcViewportCtx` / `PaintViewport` / `Out of memory` yang sempat dicatat sebelumnya belum berhasil direproduksi ulang pada smoke run terbaru dan sementara dianggap stale sampai ada reproduksi interaktif baru.
- Slice lanjutan menambahkan dua guardrail runtime khusus macOS untuk mencegah regresi intro viewport sambil menunggu validasi interaktif penuh:
  - `Alchemy/Kernel/Utilities.cpp` sekarang menyediakan jalur `sysctl`/`sysconf` untuk `sysGetProcessorInfo` dan `sysGetProcessorCountLegacy`, sehingga perencanaan thread SFX di macOS tidak lagi bergantung pada API Win32.
  - `CSFXOptions::CalcPaintThreads` sekarang memaksa background painting tetap single-threaded di macOS, sementara `CSystem::CalcViewportCtx` tetap membangun `CThreadPool` background yang valid dengan worker count `0` agar caller lama yang mengasumsikan pool non-NULL tidak crash.
- Verifikasi setelah patch guardrail:
  - `cmake --build --preset macos-debug --target transcendence_app -j8` tetap hijau (`ninja: no work to do` pada rebuild verifikasi terbaru).
  - PTY smoke run 60 detik berikutnya tetap mencapai `CLoadingSession::OnPaint first paint` pada stdout.
  - `build/macos-debug/Debug.log` untuk run yang sama mencapai lagi `CIntroSession::Paint calling Render` tanpa signature error baru seperti `Crash in`, `CException:`, `EXC_BAD_ACCESS`, `Unable to create bitmap`, `segmentation fault`, `CreateShipObjFromItem`, atau `CreateObjFromItem`.
- Reproduksi lanjutan berhasil memunculkan blocker intro yang lebih spesifik dari run normal, bukan hanya dari laporan lama:
  - `Crash in PaintImage`
  - `Crash in PaintViewport`
  - `CException: Out of memory.`
- Ini mempersempit akar masalah aktif ke jalur paint objek atau sprite di viewport intro (`CObjectImageArray::PaintImage` lewat `CSystem::PaintViewport`), bukan lagi ke background init, object-reference CodeChain, atau background thread pool viewport.
- Guardrail berikutnya dipasang di `CSystem::CalcViewportCtx` dengan memaksa `SViewportPaintCtx::bForceSTPaint = true` pada macOS, sehingga object/sprite image paint tidak lagi menjadwalkan worker paint image multithreaded selama port Apple Silicon masih distabilkan.
- Verifikasi setelah guardrail sprite-paint:
  - Rebuild `cmake --build --preset macos-debug --target transcendence_app -j8` berhasil.
  - PTY smoke run 60 detik sesudah patch tetap mencapai `CLoadingSession::OnPaint first paint` pada stdout dan tetap hidup sampai dibunuh harness, alih-alih abort dengan exit code `133`.
  - `build/macos-debug/Debug.log` dari run yang sama berhenti bersih di `CIntroSession::Paint calling Render` tanpa `WARNING:` ataupun crash baru (`Crash in PaintImage`, `Crash in PaintViewport`, `CException:`, `EXC_BAD_ACCESS`, `Unable to create bitmap`).
- Slice lanjutan untuk prioritas runtime aset menutup dua gap portability yang masih terbuka di jalur audio/resource macOS:
  - `Mammoth/TSUI/CMCIMixerStub.cpp` tidak lagi murni no-op untuk soundtrack. Backend macOS sekarang memakai `SDL_mixer`, me-resolve filespec musik dari path Windows-style ke kandidat path build tree / bundle (`Transcendence/Game`, `Contents/Resources/Game`, dan base path SDL), membebaskan `Mix_Music` lama secara aman, dan mem-post `cmdSoundtrackDone` saat track selesai.
  - `Transcendence/Transcendence/CResourcePathResolver.cpp` sekarang menambah kandidat root resource yang diturunkan dari `pathGetExecutablePath(NULL)`, termasuk jalur `../Resources`, `../../Resources`, dan fallback bundle `Contents/Resources`, sehingga lookup resource lebih dekat ke parity untuk layout `.app` selain sekadar build tree developer.
- Rebuild verifikasi sesudah slice audio/resource tetap hijau (`ninja: no work to do`).
- Verifikasi runtime lanjutan mengubah blocker aktif lagi. Run normal dari `/tmp` tidak lagi berhenti pada blocker viewport yang lebih lama, tetapi sekarang crash lebih jauh di update intro-world/effect path. Backtrace proses menunjukkan rantai aktif:
  - `CEffectGroupCreator::OnCreatePainter`
  - `CEffectCreator::CreatePainter`
  - `CEffectCreatorRef::CreatePainter`
  - `CWeaponFireDesc::CreateHitEffect`
  - `CMissile::OnDamage` / `CMissile::OnMove`
  - `CSystem::UpdatePhysics` / `CUniverse::Update`
  - `CIntroSession::Update` / `CIntroSession::OnAnimate`
- Artinya blocker aktif berikutnya bukan lagi base-file init, lookup resource title, maupun crash `PaintViewport` yang lama, tetapi crash effect painter pada simulasi intro yang berjalan lebih lama saat missile/hit-effect diproduksi di intro scene.
- Slice lanjutan berikutnya merapikan dua seam portability yang masih relevan dengan blocker Apple Silicon aktif:
  - jalur object-reference CodeChain/TSE diperketat lagi di `CCUtil.cpp`, `CCodeChainCtx.cpp`, `CTLispConvert.cpp`, `CRangeTypeEvent.cpp`, `CUniverse.cpp`, `ShipProperties.cpp`, `CDockScreen.cpp`, dan signature argumen `obj@`/`msn@` di `CCExtensions.cpp`, sehingga pointer `CSpaceObject *` tidak lagi diam-diam jatuh ke integer 32-bit-style atau diterima kembali dari nilai rendah/truncated saat callback effect/property berjalan;
  - bridge `SDLBitmap` sekarang menormalkan surface non-16-bit ke `BGR24` bila perlu, menyimpan metadata `BitsPerPixel`, dan mengembalikan `dibIs16bit`/`dibIs24bit` sesuai bit depth nyata, sehingga caller DIB lama tidak lagi salah membaca surface 24/32-bit sebagai bitmap 16-bit saat asset JPEG/BMP atau mask dipakai di macOS.
- Verifikasi lokal pada slice ini masih sebatas rebuild dan inspeksi log karena rerun PTY dari `/tmp` dibatalkan sebelum eksekusi penuh. Hasil yang terkonfirmasi:
  - `cmake --build --preset macos-debug --target transcendence_app -j8` tetap hijau (`ninja: no work to do`);
  - `build/macos-debug/Debug.log` terbaru tetap menunjukkan startup mencapai `CIntroSession::OnAnimate first frame`, `CIntroSession::Paint first frame`, `CIntroSession::Paint calling Render`, lalu lanjut ke `Initializing adventure: ../../Transcendence/TransCore/Transcendence.xml`.
- Karena itu blocker aktif belum bergeser lagi: kandidat crash bernilai tertinggi tetap jalur `CEffectGroupCreator` / `CWeaponFireDesc::CreateHitEffect` pada simulasi intro yang berjalan lebih lama, sementara validasi manual visual/interaktif menu masih tetap dibutuhkan.
- Task 3 tetap `in_progress` karena acceptance criteria terakhir masih butuh validasi manual visual/interaktif: memastikan frame intro/menu benar-benar readable, tidak ada korupsi alpha/warna, dan input menu berjalan benar pada window nyata macOS.

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
