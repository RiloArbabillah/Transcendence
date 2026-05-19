//	CMCIMixerStub.cpp
//
//	Stub implementation of CMCIMixer for macOS
//	Provides minimal stubs so the game can link and run

#include "stdafx.h"
#include "Soundtrack.h"

#ifdef TARGET_PLATFORM_MACOS

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <cstdio>

static bool g_bInitialized = false;
static bool g_bMusicPlaying = false;
static Mix_Music* g_pCurrentMusic = nullptr;
static int g_iVolume = 1000;
static bool g_bOwnsAudio = false;

static void LogMixerEvent(const CString &sMessage)
	{
	::kernelDebugLogString(sMessage);
	}

static void LogMixerPattern(const char *pszPrefix, const CString &sValue)
	{
	CString sMessage(pszPrefix);
	sMessage.Append(sValue);
	::kernelDebugLogString(sMessage);
	}

static void LogMixerPattern(const char *pszPrefix, const char *pszValue)
	{
	CString sMessage(pszPrefix);
	sMessage.Append(CString(pszValue));
	::kernelDebugLogString(sMessage);
	}

static CString ResolveMusicFilespec(const CString &sFilespec)
	{
	if (sFilespec.IsBlank() || pathExists(sFilespec))
		return sFilespec;

	CString sNormalized = sFilespec;
	char *pPos = sNormalized.GetWritePointer(sNormalized.GetLength());
	char *pEnd = pPos + sNormalized.GetLength();
	while (pPos < pEnd)
		{
		if (*pPos == '\\')
			*pPos = '/';
		pPos++;
		}

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

static void OnMusicFinished(void)
	{
	g_bMusicPlaying = false;

	if (g_pCurrentMusic)
		{
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
		}

	if (g_pHI)
		g_pHI->HIPostCommand(CONSTLIT("cmdSoundtrackDone"));
	}

CMCIMixer::CMCIMixer(int iChannels) :
		m_iDefaultVolume(1000),
		m_iCurChannel(-1),
		m_pNowPlaying(NULL),
		m_bNoStopNotify(false),
		m_bDebugMode(false)
	{
	if (!g_bInitialized)
		{
		if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) == 0)
			{
			if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
				{
				LogMixerPattern("CMCIMixer::CMCIMixer SDL_InitSubSystem(SDL_INIT_AUDIO) failed: ", CString(SDL_GetError()));
				return;
				}
			}

		LogMixerPattern("CMCIMixer::CMCIMixer SDL audio driver: ", CString(SDL_GetCurrentAudioDriver()));

		int flags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
		int mixerFlags = Mix_Init(flags);
		if ((mixerFlags & flags) != flags)
			LogMixerPattern("CMCIMixer::CMCIMixer Mix_Init partial support: ", CString(Mix_GetError()));

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

				LogMixerPattern("CMCIMixer::CMCIMixer Mix_OpenAudio failed: ", CString(Mix_GetError()));
				}

			if (!bOpened)
				return;

			g_bOwnsAudio = true;
			}
		else
			g_bOwnsAudio = false;

		Mix_HookMusicFinished(OnMusicFinished);
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
		if (g_pCurrentMusic)
			{
			Mix_FreeMusic(g_pCurrentMusic);
			g_pCurrentMusic = nullptr;
			}
		Mix_HookMusicFinished(NULL);
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
	if (g_pCurrentMusic)
		{
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
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

	double rSeconds = Mix_MusicDuration(g_pCurrentMusic);
	if (rSeconds <= 0.0)
		return 0;

	return (int)(rSeconds * 1000.0);
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
	if (g_pCurrentMusic)
		{
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
		}

	CString sFilespec = pTrack->GetFilespec();
	if (sFilespec.IsBlank())
		sFilespec = pTrack->GetFilename();
	sFilespec = ResolveMusicFilespec(sFilespec);

	if (sFilespec.IsBlank())
		{
		LogMixerEvent(CONSTLIT("CMCIMixer::Play failed: blank filespec."));
		return false;
		}

	LogMixerPattern("CMCIMixer::Play resolved filespec: ", sFilespec);

	const char* pszFile = sFilespec.GetASCIIZPointer();
	Mix_Music* pMusic = Mix_LoadMUS(pszFile);
	if (!pMusic)
		{
		LogMixerPattern("CMCIMixer::Play Mix_LoadMUS failed: ", CString(Mix_GetError()));
		return false;
		}

	if (Mix_PlayMusic(pMusic, 0) != 0)
		{
		LogMixerPattern("CMCIMixer::Play Mix_PlayMusic failed: ", CString(Mix_GetError()));
		Mix_FreeMusic(pMusic);
		return false;
		}

	if (iPos > 0 && Mix_SetMusicPosition((double)iPos / 1000.0) != 0)
		LogMixerPattern("CMCIMixer::Play Mix_SetMusicPosition failed: ", CString(Mix_GetError()));

	g_bMusicPlaying = true;
	g_pCurrentMusic = pMusic;
	m_pNowPlaying = pTrack;
	SetVolume(g_iVolume);
	LogMixerEvent(CONSTLIT("CMCIMixer::Play started."));

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
	if (g_pCurrentMusic)
		{
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
		}

	CString sFilespec = pTrack->GetFilespec();
	if (sFilespec.IsBlank())
		sFilespec = pTrack->GetFilename();
	sFilespec = ResolveMusicFilespec(sFilespec);

	if (sFilespec.IsBlank())
		{
		LogMixerEvent(CONSTLIT("CMCIMixer::PlayFadeIn failed: blank filespec."));
		return false;
		}

	LogMixerPattern("CMCIMixer::PlayFadeIn resolved filespec: ", sFilespec);

	const char* pszFile = sFilespec.GetASCIIZPointer();
	Mix_Music* pMusic = Mix_LoadMUS(pszFile);
	if (!pMusic)
		{
		LogMixerPattern("CMCIMixer::PlayFadeIn Mix_LoadMUS failed: ", CString(Mix_GetError()));
		return false;
		}

	if (Mix_FadeInMusic(pMusic, 0, 1000) != 0)
		{
		LogMixerPattern("CMCIMixer::PlayFadeIn Mix_FadeInMusic failed: ", CString(Mix_GetError()));
		Mix_FreeMusic(pMusic);
		return false;
		}

	g_bMusicPlaying = true;
	g_pCurrentMusic = pMusic;
	m_pNowPlaying = pTrack;
	SetVolume(g_iVolume);
	LogMixerEvent(CONSTLIT("CMCIMixer::PlayFadeIn started."));

	return true;
	}

void CMCIMixer::SetPlayPaused(bool bPlay)
	{
	if (g_bInitialized)
		{
		if (bPlay)
			Mix_ResumeMusic();
		else
			Mix_PauseMusic();
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
	if (g_pCurrentMusic)
		{
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
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
