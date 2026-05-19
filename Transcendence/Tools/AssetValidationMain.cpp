#include <SDL2/SDL.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "Alchemy.h"
#include "DirectXUtil.h"
#include "TSE.h"
#include "TSEResourceDb.h"

namespace fs = std::filesystem;

namespace
    {
    struct SValidationFailure
        {
        std::string sCategory;
        std::string sAsset;
        std::string sReason;
        };

    struct SValidationStats
        {
        int iTDBImages = 0;
        int iExternalImages = 0;
        int iTDBSounds = 0;
        int iExternalSounds = 0;
        int iMusicTracks = 0;
        int iVideoAssets = 0;
        std::vector<SValidationFailure> Failures;
        };

    bool HasExtension(const fs::path &Path, std::initializer_list<const char *> Exts)
        {
        std::string sExt = Path.extension().string();
        for (char &ch : sExt)
            ch = (char)std::tolower((unsigned char)ch);

        for (const char *pExt : Exts)
            if (sExt == pExt)
                return true;

        return false;
        }

    bool FileExists(const fs::path &Path)
        {
        std::error_code ec;
        return fs::exists(Path, ec);
        }

    fs::path FindRepoRoot(const fs::path &Start)
        {
        fs::path Current = fs::absolute(Start);
        while (!Current.empty())
            {
            if (FileExists(Current / "CMakeLists.txt")
                    && FileExists(Current / "Transcendence/Game/Transcendence.tdb"))
                return Current;

            if (Current == Current.root_path())
                break;

            Current = Current.parent_path();
            }

        return {};
        }

    CString ToCString(const fs::path &Path)
        {
        return CString(Path.string().c_str());
        }

    CString ToCString(const std::string &sValue)
        {
        return CString(sValue.c_str());
        }

    void AddFailure(SValidationStats &Stats, const std::string &sCategory, const std::string &sAsset, const CString &sReason)
        {
        Stats.Failures.push_back({ sCategory, sAsset, sReason.GetASCIIZPointer() });
        }

    void AddFailureLiteral(SValidationStats &Stats, const std::string &sCategory, const std::string &sAsset, const char *pReason)
        {
        Stats.Failures.push_back({ sCategory, sAsset, (pReason ? std::string(pReason) : std::string()) });
        }

    bool ValidateTDBResource(CResourceDb &Resources, const CString &sResource, SValidationStats &Stats)
        {
        CString sExt = strToLower(pathGetExtension(sResource));

        if (strEquals(sExt, CONSTLIT("png"))
                || strEquals(sExt, CONSTLIT("jpg"))
                || strEquals(sExt, CONSTLIT("jpeg"))
                || strEquals(sExt, CONSTLIT("bmp")))
            {
            TUniquePtr<CG32bitImage> pImage;
            CString sError;
            if (Resources.LoadImageFile(sResource, NULL_STR, pImage, false, &sError) != NOERROR || !pImage)
                {
                AddFailure(Stats, "image", sResource.GetASCIIZPointer(), (sError.IsBlank() ? CONSTLIT("LoadImageFile failed.") : sError));
                return false;
                }

            Stats.iTDBImages++;
            return true;
            }
        else if (strEquals(sExt, CONSTLIT("wav")))
            {
            CString sFolder = pathGetPath(sResource);
            CString sFilename = pathGetFilename(sResource);

            CSoundMgr SoundMgr;
            CString sError;
            if (SoundMgr.Init(NULL) != NOERROR)
                {
                AddFailureLiteral(Stats, "audio", sResource.GetASCIIZPointer(), "CSoundMgr::Init failed for embedded sound validation.");
                return false;
                }

            int iChannel = -1;
            ALERROR error = Resources.LoadSound(SoundMgr, sFolder, sFilename, &iChannel);
            if (error != NOERROR || iChannel < 0)
                {
                AddFailureLiteral(Stats, "audio", sResource.GetASCIIZPointer(), "LoadSound failed for embedded WAV resource.");
                SoundMgr.CleanUp();
                return false;
                }

            SoundMgr.Delete(iChannel);
            SoundMgr.CleanUp();
            Stats.iTDBSounds++;
            return true;
            }

        return true;
        }

    bool ValidateExternalWAV(const fs::path &Path, SValidationStats &Stats)
        {
        CSoundMgr SoundMgr;
        if (SoundMgr.Init(NULL) != NOERROR)
            {
            AddFailureLiteral(Stats, "audio", Path.string(), "CSoundMgr::Init failed for external WAV validation.");
            return false;
            }

        int iChannel = -1;
        ALERROR error = SoundMgr.LoadWaveFile(ToCString(Path), &iChannel);
        if (error != NOERROR || iChannel < 0)
            {
            AddFailureLiteral(Stats, "audio", Path.string(), "LoadWaveFile failed.");
            SoundMgr.CleanUp();
            return false;
            }

        SoundMgr.Delete(iChannel);
        SoundMgr.CleanUp();
        Stats.iExternalSounds++;
        return true;
        }

    bool ValidateExternalImage(const fs::path &Path, SValidationStats &Stats)
        {
        TUniquePtr<CG32bitImage> pImage(new CG32bitImage);

        if (!pImage->CreateFromFile(ToCString(Path), NULL_STR, 0))
            {
            AddFailureLiteral(Stats, "image", Path.string(), "CG32bitImage::CreateFromFile failed.");
            return false;
            }

        if (pImage->IsEmpty())
            {
            AddFailureLiteral(Stats, "image", Path.string(), "Image decoded but returned empty bitmap.");
            return false;
            }

        Stats.iExternalImages++;
        return true;
        }

    bool ValidateMusicTrack(const fs::path &Path, SValidationStats &Stats)
        {
        CSoundMgr SoundMgr;
        if (SoundMgr.Init(NULL) != NOERROR)
            {
            AddFailureLiteral(Stats, "music", Path.string(), "CSoundMgr::Init failed for music validation.");
            return false;
            }

        CString sError;
        if (!SoundMgr.PlayMusic(ToCString(Path), 0, &sError))
            {
            AddFailure(Stats, "music", Path.string(), (sError.IsBlank() ? CONSTLIT("PlayMusic failed.") : sError));
            SoundMgr.CleanUp();
            return false;
            }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        CSoundMgr::SMusicPlayState State;
        if (!SoundMgr.GetMusicPlayState(&State) || State.sFilename.IsBlank())
            {
            AddFailureLiteral(Stats, "music", Path.string(), "GetMusicPlayState did not report an active track.");
            SoundMgr.CleanUp();
            return false;
            }

        SoundMgr.StopMusic();
        SoundMgr.CleanUp();
        Stats.iMusicTracks++;
        return true;
        }

    void PrintSummary(const fs::path &RepoRoot, const SValidationStats &Stats)
        {
        std::cout
            << "repo_root=" << RepoRoot.string() << "\n"
            << "tdb_images=" << Stats.iTDBImages << "\n"
            << "external_images=" << Stats.iExternalImages << "\n"
            << "tdb_embedded_sounds=" << Stats.iTDBSounds << "\n"
            << "external_wavs=" << Stats.iExternalSounds << "\n"
            << "music_tracks=" << Stats.iMusicTracks << "\n"
            << "video_assets=" << Stats.iVideoAssets << "\n";

        if (Stats.Failures.empty())
            {
            std::cout << "status=ok\n";
            }
        else
            {
            std::cout << "status=failed\n";
            for (const auto &Failure : Stats.Failures)
                std::cout
                    << "failure[" << Failure.sCategory << "] "
                    << Failure.sAsset << " :: "
                    << Failure.sReason << "\n";
            }
        }
    }

int main(int argc, char *argv[])
    {
    fs::path RepoRoot;
    if (argc > 1)
        RepoRoot = FindRepoRoot(argv[1]);
    else
        RepoRoot = FindRepoRoot(fs::current_path());

    if (RepoRoot.empty())
        {
        std::cerr << "Unable to find repo root containing Transcendence/Game/Transcendence.tdb\n";
        return 2;
        }

    setenv("SDL_AUDIODRIVER", "dummy", 0);

    if (!kernelInit(KERNEL_FLAG_INTERNETS))
        {
        std::cerr << "kernelInit failed\n";
        return 2;
        }

    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        {
        std::cerr << "SDL_Init(SDL_INIT_AUDIO) failed: " << SDL_GetError() << "\n";
        kernelCleanUp();
        return 2;
        }

    SValidationStats Stats;
    int iResult = 0;

    {
        CString sError;
        const fs::path MainXml = RepoRoot / "Transcendence/TransCore/Transcendence.xml";
        const fs::path MainTdb = RepoRoot / "Transcendence/Game/Transcendence.tdb";
        CResourceDb Resources(ToCString(MainTdb));
        if (Resources.Open(DFOPEN_FLAG_READ_ONLY, &sError) != NOERROR)
            {
            AddFailure(Stats, "resource-db", MainXml.string(), sError);
            iResult = 1;
            }
        else
            {
            for (int i = 0; i < Resources.GetResourceCount(); i++)
                {
                CString sResource = Resources.GetResourceFilespec(i);
                if (!ValidateTDBResource(Resources, sResource, Stats))
                    iResult = 1;
                }
            }
    }

    const fs::path ExternalSoundRoot = RepoRoot / "Transcendence/TransCore/Resources";
    if (FileExists(ExternalSoundRoot))
        {
        for (const auto &Entry : fs::recursive_directory_iterator(ExternalSoundRoot))
            {
            if (!Entry.is_regular_file())
                continue;

            const fs::path Path = Entry.path();
            if (HasExtension(Path, { ".png", ".jpg", ".jpeg", ".bmp" }))
                {
                if (!ValidateExternalImage(Path, Stats))
                    iResult = 1;
                }
            else if (HasExtension(Path, { ".wav" }))
                {
                if (!ValidateExternalWAV(Path, Stats))
                    iResult = 1;
                }
            else if (HasExtension(Path, { ".mov", ".mp4", ".m4v", ".bik", ".ogv", ".webm" }))
                Stats.iVideoAssets++;
            }
        }

    const fs::path GameRoot = RepoRoot / "Transcendence/Game";
    if (FileExists(GameRoot))
        {
        for (const auto &Entry : fs::recursive_directory_iterator(GameRoot))
            {
            if (!Entry.is_regular_file())
                continue;

            const fs::path Path = Entry.path();
            if (HasExtension(Path, { ".png", ".jpg", ".jpeg", ".bmp" }))
                {
                if (!ValidateExternalImage(Path, Stats))
                    iResult = 1;
                }
            else if (HasExtension(Path, { ".mp3", ".ogg", ".flac" }))
                {
                if (!ValidateMusicTrack(Path, Stats))
                    iResult = 1;
                }
            else if (HasExtension(Path, { ".mov", ".mp4", ".m4v", ".bik", ".ogv", ".webm" }))
                Stats.iVideoAssets++;
            }
        }

    PrintSummary(RepoRoot, Stats);

    SDL_Quit();
    kernelCleanUp();
    return iResult;
    }
