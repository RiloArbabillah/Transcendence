//	CMCIMixerStub.cpp
//
//	Stub implementation of CMCIMixer for macOS
//	Provides minimal stubs so the game can link and run

#include "stdafx.h"
#include "Soundtrack.h"

#ifdef TARGET_PLATFORM_MACOS

#include <SDL2/SDL_mixer.h>
#include <cstdio>

static bool g_bInitialized = false;
static bool g_bMusicPlaying = false;
static Mix_Music* g_pCurrentMusic = nullptr;
static int g_iVolume = 1000;
static bool g_bOwnsAudio = false;

CMCIMixer::CMCIMixer(int iChannels) :
		m_iDefaultVolume(1000),
		m_iCurChannel(-1),
		m_pNowPlaying(NULL),
		m_bNoStopNotify(false),
		m_bDebugMode(false)
	{
	if (!g_bInitialized)
		{
		int flags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
		Mix_Init(flags);

		int iFreq;
		Uint16 iFormat;
		int iChannels;
		if (Mix_QuerySpec(&iFreq, &iFormat, &iChannels) == 0)
			{
			if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) != 0)
				return;

			g_bOwnsAudio = true;
			}
		else
			g_bOwnsAudio = false;

		g_bInitialized = true;
		}
	}

CMCIMixer::~CMCIMixer(void)
	{
	if (g_bInitialized)
		{
		if (g_bMusicPlaying)
			{
			Mix_HaltMusic();
			g_bMusicPlaying = false;
			}
		if (g_bOwnsAudio)
			Mix_CloseAudio();
		Mix_Quit();
		g_bInitialized = false;
		g_bOwnsAudio = false;
		}
	}

void CMCIMixer::AbortAllRequests(void)
	{
	if (g_bInitialized && g_bMusicPlaying)
		{
		Mix_HaltMusic();
		g_bMusicPlaying = false;
		}
	}

bool CMCIMixer::Boot(void)
	{
	return g_bInitialized;
	}

void CMCIMixer::FadeAtPos(int iPos)
	{
	if (g_bInitialized && g_bMusicPlaying)
		{
		Mix_FadeOutMusic(500);
		}
	}

void CMCIMixer::FadeNow(void)
	{
	if (g_bInitialized && g_bMusicPlaying)
		{
		Mix_FadeOutMusic(1000);
		}
	}

int CMCIMixer::GetCurrentPlayLength(void)
	{
	if (!g_bInitialized || !g_pCurrentMusic)
		return 0;

	int ms = Mix_MusicDuration(g_pCurrentMusic);
	return ms / 1000;
	}

int CMCIMixer::GetCurrentPlayPos(DWORD dwTimeout)
	{
	if (!g_bInitialized || !g_pCurrentMusic)
		return 0;

	double pos = Mix_GetMusicPosition(g_pCurrentMusic);
	if (pos >= 0)
		return (int)(pos * 1000);
	return 0;
	}

void CMCIMixer::GetDebugInfo(TArray<CString> *retLines) const
	{
	if (retLines)
		{
		retLines->DeleteAll();
		char buf[256];
		snprintf(buf, sizeof(buf), "Audio: %s", g_bInitialized ? "initialized" : "not initialized");
		retLines->Insert(*new CString(buf));
		snprintf(buf, sizeof(buf), "Music playing: %s", g_bMusicPlaying ? "yes" : "no");
		retLines->Insert(*new CString(buf));
		}
	}

bool CMCIMixer::Play(CMusicResource *pTrack, int iPos)
	{
	if (!g_bInitialized || !pTrack)
		return false;

	if (g_bMusicPlaying)
		{
		Mix_HaltMusic();
		g_bMusicPlaying = false;
		}

	CString sFilespec = pTrack->GetFilespec();
	if (sFilespec.IsBlank())
		sFilespec = pTrack->GetFilename();

	if (sFilespec.IsBlank())
		return false;

	const char* pszFile = sFilespec.GetASCIIZPointer();
	Mix_Music* pMusic = Mix_LoadMUS(pszFile);
	if (!pMusic)
		return false;

	if (Mix_PlayMusic(pMusic, 0) != 0)
		{
		Mix_FreeMusic(pMusic);
		return false;
		}

	g_bMusicPlaying = true;
	g_pCurrentMusic = pMusic;
	m_pNowPlaying = pTrack;
	SetVolume(g_iVolume);

	return true;
	}

bool CMCIMixer::PlayFadeIn(CMusicResource *pTrack, int iPos)
	{
	if (!g_bInitialized || !pTrack)
		return false;

	if (g_bMusicPlaying)
		{
		Mix_HaltMusic();
		g_bMusicPlaying = false;
		}

	CString sFilespec = pTrack->GetFilespec();
	if (sFilespec.IsBlank())
		sFilespec = pTrack->GetFilename();

	if (sFilespec.IsBlank())
		return false;

	const char* pszFile = sFilespec.GetASCIIZPointer();
	Mix_Music* pMusic = Mix_LoadMUS(pszFile);
	if (!pMusic)
		return false;

	if (Mix_FadeInMusic(pMusic, 0, 1000) != 0)
		{
		Mix_FreeMusic(pMusic);
		return false;
		}

	g_bMusicPlaying = true;
	g_pCurrentMusic = pMusic;
	m_pNowPlaying = pTrack;
	SetVolume(g_iVolume);

	return true;
	}

void CMCIMixer::SetPlayPaused(bool bPlay)
	{
	if (g_bInitialized)
		{
		if (bPlay)
			Mix_PauseMusic();
		else
			Mix_ResumeMusic();
		}
	}

void CMCIMixer::SetVolume(int iVolume)
	{
	if (g_bInitialized)
		{
		g_iVolume = iVolume;
		int volume = (iVolume * MIX_MAX_VOLUME) / 1000;
		Mix_VolumeMusic(volume);
		}
	}

void CMCIMixer::Shutdown(void)
	{
	if (g_bInitialized && g_bMusicPlaying)
		{
		Mix_HaltMusic();
		g_bMusicPlaying = false;
		}
	if (g_pCurrentMusic)
		{
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
		}
	m_pNowPlaying = nullptr;
	}

void CMCIMixer::Stop(void)
	{
	if (g_bInitialized && g_bMusicPlaying)
		{
		Mix_HaltMusic();
		g_bMusicPlaying = false;
		}
	m_pNowPlaying = nullptr;
	}

void CMCIMixer::TogglePausePlay(void)
	{
	if (g_bInitialized && g_bMusicPlaying)
		{
		if (Mix_PausedMusic())
			Mix_ResumeMusic();
		else
			Mix_PauseMusic();
		}
	}

#endif // TARGET_PLATFORM_MACOS
