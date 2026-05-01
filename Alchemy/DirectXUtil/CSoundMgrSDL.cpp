//	CSoundMgrSDL.cpp
//	SDL_mixer-backed sound manager for macOS

#include "Alchemy.h"
#include "DirectXUtil.h"

#ifdef TARGET_PLATFORM_MACOS
#include <SDL2/SDL_mixer.h>

static bool g_bOwnsAudio = false;

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

    int iInitFlags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
    Mix_Init(iInitFlags);

    int iFreq;
    Uint16 iFormat;
    int iChannels;
    if (Mix_QuerySpec(&iFreq, &iFormat, &iChannels) == 0)
        {
        if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) != 0)
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
    Mix_Chunk *pChunk = Mix_LoadWAV(sFilename.GetASCIIZPointer());
    if (!pChunk)
        return ERR_FAIL;

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
        Mix_Volume(iPlayChannel, CalcSDLEffectVolume(iVolume, m_iSoundVolume));
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

    retState->bPlaying = (Mix_PlayingMusic() != 0);
    retState->bPaused = (Mix_PausedMusic() != 0);
    retState->iPos = 0;
    retState->iLength = 0;
    return true;
}

bool CSoundMgr::PlayMusic(const CString &sFilename, int iPos, CString *retsError)
{
    (void)iPos;

    Mix_Music *pMusic = Mix_LoadMUS(sFilename.GetASCIIZPointer());
    if (!pMusic)
        {
        if (retsError)
            *retsError = CONSTLIT("Unable to load music file.");
        return false;
        }

    StopMusic();
    m_hMusic = reinterpret_cast<HWND>(pMusic);

    if (Mix_PlayMusic(pMusic, 0) != 0)
        {
        Mix_FreeMusic(pMusic);
        m_hMusic = NULL;
        if (retsError)
            *retsError = CONSTLIT("Unable to play music file.");
        return false;
        }

    Mix_VolumeMusic(CalcSDLMixerVolume(m_iMusicVolume));
    return true;
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
