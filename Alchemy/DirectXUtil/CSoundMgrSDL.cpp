//	CSoundMgrSDL.cpp
//	SDL_mixer-backed sound manager for macOS

#include "Alchemy.h"
#include "DirectXUtil.h"

#ifdef TARGET_PLATFORM_MACOS
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

static bool g_bOwnsAudio = false;

static void LogSoundMgrEvent(const CString &sMessage)
{
    ::kernelDebugLogString(sMessage);
}

static void LogSoundMgrPattern(const char *pszPrefix, const CString &sValue)
{
    CString sMessage(pszPrefix);
    sMessage.Append(sValue);
    ::kernelDebugLogString(sMessage);
}

static void LogSoundMgrSDLString(const char *pszPrefix, const char *pszValue)
{
    CString sMessage(pszPrefix);
    if (pszValue)
        sMessage.Append(CString(pszValue));
    else
        sMessage.Append(CONSTLIT("<null>"));

    ::kernelDebugLogString(sMessage);
}

static int CalcSDLMixerVolume(int iLevel)
{
    int iClamped = Max(0, Min(iLevel, 10));
    return (iClamped * MIX_MAX_VOLUME) / 10;
}

static int CalcSDLEffectVolume(int iDSVolume, int iMasterLevel)
{
    int iMaster = CalcSDLMixerVolume(iMasterLevel);

    int iClampedDS = Min(0, Max(iDSVolume, -10000));
    int iAtten = 10000 + iClampedDS;

    return (iMaster * iAtten) / 10000;
}

static CString NormalizeAudioFilespec(const CString &sFilespec)
{
    CString sNormalized = sFilespec;
    const int iLen = sNormalized.GetLength();
    char *pPos = sNormalized.GetWritePointer(iLen);
    char *pEnd = pPos + iLen;
    while (pPos < pEnd)
        {
        if (*pPos == '\\')
            *pPos = '/';
        pPos++;
        }

    return sNormalized;
}

static CString ResolveAudioFilespec(const CString &sFilespec)
{
    CString sNormalized = NormalizeAudioFilespec(sFilespec);
    if (sNormalized.IsBlank() || pathExists(sNormalized))
        return sNormalized;

    TArray<CString> Candidates;
    Candidates.Insert(sNormalized);

    CString sFilename = pathGetFilename(sNormalized);
    if (!sFilename.IsBlank() && !strEquals(sFilename, sNormalized))
        Candidates.Insert(sFilename);

    Candidates.Insert(pathAddComponent(CONSTLIT("Transcendence/Game"), sNormalized));
    Candidates.Insert(pathAddComponent(CONSTLIT("../Transcendence/Game"), sNormalized));
    Candidates.Insert(pathAddComponent(CONSTLIT("../../Transcendence/Game"), sNormalized));
    Candidates.Insert(pathAddComponent(CONSTLIT("Transcendence/TransCore/Resources"), sFilename.IsBlank() ? sNormalized : sFilename));
    Candidates.Insert(pathAddComponent(CONSTLIT("../Transcendence/TransCore/Resources"), sFilename.IsBlank() ? sNormalized : sFilename));
    Candidates.Insert(pathAddComponent(CONSTLIT("../../Transcendence/TransCore/Resources"), sFilename.IsBlank() ? sNormalized : sFilename));
    Candidates.Insert(pathAddComponent(CONSTLIT("Contents/Resources/Game"), sNormalized));

    if (!sFilename.IsBlank())
        {
        Candidates.Insert(pathAddComponent(CONSTLIT("Transcendence/Game"), sFilename));
        Candidates.Insert(pathAddComponent(CONSTLIT("../Transcendence/Game"), sFilename));
        Candidates.Insert(pathAddComponent(CONSTLIT("../../Transcendence/Game"), sFilename));
        Candidates.Insert(pathAddComponent(CONSTLIT("Contents/Resources/Game"), sFilename));
        }

    if (const char *pBasePath = SDL_GetBasePath())
        {
        CString sBasePath(pBasePath);
        Candidates.Insert(pathAddComponent(sBasePath, sNormalized));
        Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("Game")), sNormalized));
        Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("../Resources/Game")), sNormalized));
        Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("../../Resources/Game")), sNormalized));
        Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("../TransCore/Resources")), sFilename.IsBlank() ? sNormalized : sFilename));
        Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("../../TransCore/Resources")), sFilename.IsBlank() ? sNormalized : sFilename));

        if (!sFilename.IsBlank())
            {
            Candidates.Insert(pathAddComponent(sBasePath, sFilename));
            Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("Game")), sFilename));
            Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("../Resources/Game")), sFilename));
            Candidates.Insert(pathAddComponent(pathAddComponent(sBasePath, CONSTLIT("../../Resources/Game")), sFilename));
            }
        }

    for (int i = 0; i < Candidates.GetCount(); i++)
        if (pathExists(Candidates[i]))
            return Candidates[i];

    return sNormalized;
}

static void ApplyPanToChannel(int iChannel, int iPan)
{
    if (iChannel < 0)
        return;

    const int iClampedPan = Max(-10000, Min(iPan, 10000));
    Uint8 byLeft = 255;
    Uint8 byRight = 255;

    if (iClampedPan < 0)
        byRight = (Uint8)(255 + ((255 * iClampedPan) / 10000));
    else if (iClampedPan > 0)
        byLeft = (Uint8)(255 - ((255 * iClampedPan) / 10000));

    Mix_SetPanning(iChannel, byLeft, byRight);
}

static int MillisecondsFromSeconds(double rSeconds)
{
    if (rSeconds <= 0.0)
        return 0;

    return (int)(rSeconds * 1000.0);
}

static double SecondsFromMilliseconds(int iMilliseconds)
{
    if (iMilliseconds <= 0)
        return 0.0;

    return ((double)iMilliseconds / 1000.0);
}

static CString g_sCurrentMusicFilename;
static double g_rCurrentMusicLength = 0.0;

void CSoundMgr::AddMusicFolder(const CString &sFolder, TArray<CString> *retCatalog)
{
    CFileDirectory Dir(pathAddComponent(sFolder, CONSTLIT("*.*")));
    while (Dir.HasMore())
        {
        SFileDesc FileDesc;
        Dir.GetNextDesc(&FileDesc);

        if (FileDesc.bHiddenFile || FileDesc.bSystemFile)
            continue;

        if (*FileDesc.sFilename.GetASCIIZPointer() == '.')
            continue;

        CString sFilepath = pathAddComponent(sFolder, FileDesc.sFilename);
        if (FileDesc.bFolder)
            AddMusicFolder(sFilepath, retCatalog);
        else
            retCatalog->Insert(sFilepath);
        }
}

int CSoundMgr::AllocChannel(void)
{
    SChannel *pChannel = m_Channels.Insert();
    pChannel->pBuffer = NULL;
    pChannel->bLooping = false;
    pChannel->sFilename = NULL_STR;
    pChannel->pNext = NULL;
    return (m_Channels.GetCount() - 1);
}

CSoundMgr::CSoundMgr(void)
{
    m_pDS = NULL;
    m_hMusic = NULL;
    m_iSoundVolume = 10;
    m_iMusicVolume = 10;
}

CSoundMgr::~CSoundMgr(void)
{
    CleanUp();
}

ALERROR CSoundMgr::Init(HWND hWnd)
{
    (void)hWnd;

    if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) == 0)
        {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
            {
            LogSoundMgrPattern("CSoundMgr::Init SDL_InitSubSystem(SDL_INIT_AUDIO) failed: ", CString(SDL_GetError()));
            return ERR_FAIL;
            }
        }

    LogSoundMgrSDLString("CSoundMgr::Init SDL audio driver: ", SDL_GetCurrentAudioDriver());

    int iInitFlags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
    int iMixerFlags = Mix_Init(iInitFlags);
    if ((iMixerFlags & iInitFlags) != iInitFlags)
        LogSoundMgrPattern("CSoundMgr::Init Mix_Init partial support: ", CString(Mix_GetError()));

    int iFreq;
    Uint16 iFormat;
    int iChannels;
    if (Mix_QuerySpec(&iFreq, &iFormat, &iChannels) == 0)
        {
        struct SAudioSpec
            {
            int iFrequency;
            Uint16 wFormat;
            int iChannels;
            int iChunkSize;
            };

        static constexpr SAudioSpec OPEN_SPECS[] =
            {
                { 44100, AUDIO_S16SYS, 2, 1024 },
                { 48000, AUDIO_S16SYS, 2, 1024 },
                { 44100, AUDIO_S16SYS, 2, 2048 },
                { 48000, AUDIO_S16SYS, 2, 2048 },
                { 22050, AUDIO_S16SYS, 2, 1024 },
            };

        bool bOpened = false;
        for (const auto &Spec : OPEN_SPECS)
            {
            if (Mix_OpenAudio(Spec.iFrequency, Spec.wFormat, Spec.iChannels, Spec.iChunkSize) == 0)
                {
                bOpened = true;
                break;
                }

            LogSoundMgrPattern("CSoundMgr::Init Mix_OpenAudio failed: ", CString(Mix_GetError()));
            }

        if (!bOpened)
            return ERR_FAIL;

        g_bOwnsAudio = true;
        }
    else
        g_bOwnsAudio = false;

    Mix_AllocateChannels(32);
    Mix_Volume(-1, CalcSDLMixerVolume(m_iSoundVolume));
    Mix_VolumeMusic(CalcSDLMixerVolume(m_iMusicVolume));
    return NOERROR;
}

void CSoundMgr::CleanUp(void)
{
    StopMusic();

    for (int i = 0; i < m_Channels.GetCount(); i++)
        Delete(i);

    m_Channels.DeleteAll();
    if (g_bOwnsAudio)
        Mix_CloseAudio();
    Mix_Quit();
    g_bOwnsAudio = false;
}

void CSoundMgr::Delete(int iChannel)
{
    if (iChannel < 0 || iChannel >= m_Channels.GetCount())
        return;

    SChannel *pChannel = GetChannel(iChannel);
    if (pChannel->pBuffer)
        {
        Mix_FreeChunk(reinterpret_cast<Mix_Chunk *>(pChannel->pBuffer));
        pChannel->pBuffer = NULL;
        }
}

ALERROR CSoundMgr::LoadWaveFile(const CString &sFilename, int *retiChannel)
{
    CString sResolved = ResolveAudioFilespec(sFilename);
    LogSoundMgrPattern("CSoundMgr::LoadWaveFile resolved filespec: ", sResolved);

    Mix_Chunk *pChunk = Mix_LoadWAV(sResolved.GetASCIIZPointer());
    if (!pChunk)
        {
        LogSoundMgrPattern("CSoundMgr::LoadWaveFile Mix_LoadWAV failed: ", CString(Mix_GetError()));
        return ERR_FAIL;
        }

    int iChannel = AllocChannel();
    SChannel *pChannel = GetChannel(iChannel);
    pChannel->pBuffer = reinterpret_cast<LPDIRECTSOUNDBUFFER>(pChunk);
    pChannel->sFilename = sFilename;

    if (retiChannel)
        *retiChannel = iChannel;

    return NOERROR;
}

ALERROR CSoundMgr::LoadWaveFromBuffer(IReadBlock &Data, int *retiChannel)
{
    SDL_RWops *pRW = SDL_RWFromConstMem(Data.GetPointer(0, -1), Data.GetLength());
    if (!pRW)
        return ERR_FAIL;

    Mix_Chunk *pChunk = Mix_LoadWAV_RW(pRW, 1);
    if (!pChunk)
        return ERR_FAIL;

    int iChannel = AllocChannel();
    SChannel *pChannel = GetChannel(iChannel);
    pChannel->pBuffer = reinterpret_cast<LPDIRECTSOUNDBUFFER>(pChunk);
    pChannel->sFilename = NULL_STR;

    if (retiChannel)
        *retiChannel = iChannel;

    return NOERROR;
}

void CSoundMgr::Play(int iChannel, int iVolume, int iPan, bool bLoop)
{
    if (iChannel < 0 || iChannel >= m_Channels.GetCount())
        return;

    SChannel *pChannel = GetChannel(iChannel);
    if (!pChannel->pBuffer)
        return;

    Mix_Chunk *pChunk = reinterpret_cast<Mix_Chunk *>(pChannel->pBuffer);
    int iPlayChannel = Mix_PlayChannel(-1, pChunk, (bLoop ? -1 : 0));
    if (iPlayChannel >= 0)
        {
        Mix_Volume(iPlayChannel, CalcSDLEffectVolume(iVolume, m_iSoundVolume));
        ApplyPanToChannel(iPlayChannel, iPan);
        }
    else
        LogSoundMgrPattern("CSoundMgr::Play Mix_PlayChannel failed: ", CString(Mix_GetError()));
}

void CSoundMgr::Stop(int iChannel)
{
    (void)iChannel;
    Mix_HaltChannel(-1);
}

bool CSoundMgr::CanPlayMusic(const CString &sFilename)
{
    CString sExt = strToLower(pathGetExtension(sFilename));
    return (strEquals(sExt, CONSTLIT("mp3"))
            || strEquals(sExt, CONSTLIT("ogg"))
            || strEquals(sExt, CONSTLIT("wav"))
            || strEquals(sExt, CONSTLIT("flac")));
}

void CSoundMgr::GetMusicCatalog(const CString &sMusicFolder, TArray<CString> *retCatalog)
{
    if (!retCatalog)
        return;

    retCatalog->DeleteAll();
    AddMusicFolder(sMusicFolder, retCatalog);

    int i = 0;
    while (i < retCatalog->GetCount())
        {
        if (!CanPlayMusic(retCatalog->GetAt(i)))
            retCatalog->Delete(i);
        else
            i++;
        }
}

bool CSoundMgr::GetMusicPlayState(SMusicPlayState *retState)
{
    if (!retState)
        return false;

    retState->sFilename = g_sCurrentMusicFilename;
    retState->bPlaying = (Mix_PlayingMusic() != 0);
    retState->bPaused = (Mix_PausedMusic() != 0);
    retState->iLength = MillisecondsFromSeconds(g_rCurrentMusicLength);

    if (m_hMusic)
        {
        double rPos = Mix_GetMusicPosition(reinterpret_cast<Mix_Music *>(m_hMusic));
        retState->iPos = (rPos >= 0.0 ? MillisecondsFromSeconds(rPos) : 0);
        }
    else
        retState->iPos = 0;

    return true;
}

bool CSoundMgr::PlayMusic(const CString &sFilename, int iPos, CString *retsError)
{
    CString sResolved = ResolveAudioFilespec(sFilename);
    LogSoundMgrPattern("CSoundMgr::PlayMusic resolved filespec: ", sResolved);

    Mix_Music *pMusic = Mix_LoadMUS(sResolved.GetASCIIZPointer());
    if (!pMusic)
        {
        LogSoundMgrPattern("CSoundMgr::PlayMusic Mix_LoadMUS failed: ", CString(Mix_GetError()));
        if (retsError)
            *retsError = strPatternSubst(CONSTLIT("Unable to load music file: %s."), sResolved);
        return false;
        }

    StopMusic();
    m_hMusic = reinterpret_cast<HWND>(pMusic);
    g_sCurrentMusicFilename = sResolved;
    g_rCurrentMusicLength = Mix_MusicDuration(pMusic);

    if (Mix_PlayMusic(pMusic, 0) != 0)
        {
        Mix_FreeMusic(pMusic);
        m_hMusic = NULL;
        g_sCurrentMusicFilename = NULL_STR;
        g_rCurrentMusicLength = 0.0;
        if (retsError)
            *retsError = CONSTLIT("Unable to play music file.");
        return false;
        }

    if (iPos > 0)
        {
        if (Mix_SetMusicPosition(SecondsFromMilliseconds(iPos)) != 0)
            {
            // Some codecs do not support seeking; keep playback running from start.
            }
        }

    Mix_VolumeMusic(CalcSDLMixerVolume(m_iMusicVolume));
    return true;
}

void CSoundMgr::SetWaveVolume(int iVolumeLevel)
{
    m_iSoundVolume = Max(0, Min(iVolumeLevel, 10));
    Mix_Volume(-1, CalcSDLMixerVolume(m_iSoundVolume));
}

int CSoundMgr::SetMusicVolume(int iVolumeLevel)
{
    m_iMusicVolume = Max(0, Min(iVolumeLevel, 10));
    Mix_VolumeMusic(CalcSDLMixerVolume(m_iMusicVolume));
    return m_iMusicVolume;
}

void CSoundMgr::StopMusic(void)
{
    Mix_HaltMusic();

    if (m_hMusic)
        {
        Mix_FreeMusic(reinterpret_cast<Mix_Music *>(m_hMusic));
        m_hMusic = NULL;
        }

    g_sCurrentMusicFilename = NULL_STR;
    g_rCurrentMusicLength = 0.0;
}

void CSoundMgr::TogglePlayPaused(void)
{
    if (Mix_PlayingMusic() == 0)
        return;

    if (Mix_PausedMusic())
        Mix_ResumeMusic();
    else
        Mix_PauseMusic();
}

#endif
