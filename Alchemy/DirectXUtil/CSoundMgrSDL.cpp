//	CSoundMgrSDL.cpp
//	Stub sound manager for macOS SDL platform

#include "Alchemy.h"
#include "DirectXUtil.h"

CSoundMgr::CSoundMgr(void)
{
    m_iSoundVolume = 100;
    m_iMusicVolume = 100;
}

CSoundMgr::~CSoundMgr(void)
{
}

ALERROR CSoundMgr::Init(HWND hWnd)
{
    return NOERROR;
}

void CSoundMgr::CleanUp(void)
{
}

void CSoundMgr::Delete(int iChannel)
{
}

ALERROR CSoundMgr::LoadWaveFile(const CString &sFilename, int *retiChannel)
{
    return NOERROR;
}

ALERROR CSoundMgr::LoadWaveFromBuffer(IReadBlock &Data, int *retiChannel)
{
    return NOERROR;
}

void CSoundMgr::Play(int iChannel, int iVolume, int iPan, bool bLoop)
{
}

void CSoundMgr::Stop(int iChannel)
{
}

bool CSoundMgr::CanPlayMusic(const CString &sFilename)
{
    return false;
}

void CSoundMgr::GetMusicCatalog(const CString &sMusicFolder, TArray<CString> *retCatalog)
{
}

bool CSoundMgr::GetMusicPlayState(SMusicPlayState *retState)
{
    return false;
}

bool CSoundMgr::PlayMusic(const CString &sFilename, int iPos, CString *retsError)
{
    return false;
}

int CSoundMgr::SetMusicVolume(int iVolumeLevel)
{
    m_iMusicVolume = iVolumeLevel;
    return m_iMusicVolume;
}

void CSoundMgr::StopMusic(void)
{
}

void CSoundMgr::TogglePlayPaused(void)
{
}