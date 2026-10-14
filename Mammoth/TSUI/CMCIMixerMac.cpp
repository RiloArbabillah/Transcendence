//	CMCIMixerMac.cpp
//
//	CMCIMixer for macOS.
//
//	This is a port of CMCIMixer.cpp: the same request queue, processing
//	thread, fade timing and soundtrack notifications, with the MCIWnd calls
//	replaced by SDL_mixer. MCI tells its parent window when the mode changes;
//	SDL_mixer has no equivalent we may act on (its music-finished hook runs on
//	the audio thread, where SDL_mixer must not be called), so the processing
//	thread polls for the end of a track instead.

#include "stdafx.h"
#include "Soundtrack.h"

#ifdef TARGET_PLATFORM_MACOS

#define CMD_SOUNDTRACK_DONE						CONSTLIT("cmdSoundtrackDone")
#define CMD_SOUNDTRACK_NOW_PLAYING				CONSTLIT("cmdSoundtrackNowPlaying")
#define CMD_SOUNDTRACK_UPDATE_PLAY_POS			CONSTLIT("cmdSoundtrackUpdatePlayPos")

const DWORD CHECK_INTERVAL =					50;
const int FADE_LENGTH =							2000;
const int NORMAL_VOLUME =						1000;
const int FADE_DELAY =							25;

//	MCI modes reported by MCIWndGetMode.

enum EMusicMode
	{
	modeStopped,
	modePlaying,
	modePaused,
	};

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <cstdio>

static bool g_bInitialized = false;
static Mix_Music* g_pCurrentMusic = nullptr;		//	The open track (MCIWndOpen); used only by the processing thread
static bool g_bOwnsAudio = false;

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

//	SDL_mixer equivalents of the MCIWnd calls. There is a single channel, so
//	the window handle is ignored. Volume is 0-1000, as with MCIWndSetVolume.

static bool MusicOpen (const CString &sFilespec)
	{
	CString sResolved = ResolveMusicFilespec(sFilespec);
	g_pCurrentMusic = Mix_LoadMUS(sResolved.GetASCIIZPointer());
	if (g_pCurrentMusic == nullptr)
		{
		LogMixerPattern("CMCIMixer Mix_LoadMUS failed: ", CString(Mix_GetError()));
		return false;
		}

	return true;
	}

static void MusicClose (void)
	{
	if (g_pCurrentMusic)
		{
		Mix_HaltMusic();
		Mix_FreeMusic(g_pCurrentMusic);
		g_pCurrentMusic = nullptr;
		}
	}

static bool MusicPlay (int iPos)
	{
	if (g_pCurrentMusic == nullptr || Mix_PlayMusic(g_pCurrentMusic, 0) != 0)
		{
		LogMixerPattern("CMCIMixer Mix_PlayMusic failed: ", CString(Mix_GetError()));
		return false;
		}

	if (iPos > 0)
		Mix_SetMusicPosition((double)iPos / 1000.0);

	return true;
	}

static void MusicSetVolume (int iVolume)
	{
	Mix_VolumeMusic(Max(0, Min(MIX_MAX_VOLUME, iVolume * MIX_MAX_VOLUME / NORMAL_VOLUME)));
	}

static EMusicMode MusicGetMode (void)
	{
	if (g_pCurrentMusic == nullptr || !Mix_PlayingMusic())
		return modeStopped;
	else if (Mix_PausedMusic())
		return modePaused;
	else
		return modePlaying;
	}

CMCIMixer::CMCIMixer (int iChannels) :
		m_hParent(NULL),
		m_iDefaultVolume(NORMAL_VOLUME),
		m_iCurChannel(-1),
		m_pNowPlaying(NULL),
		m_hProcessingThread(INVALID_HANDLE_VALUE),
		m_hWorkEvent(INVALID_HANDLE_VALUE),
		m_hResultEvent(INVALID_HANDLE_VALUE),
		m_hQuitEvent(INVALID_HANDLE_VALUE),
		m_bNoStopNotify(false),
		m_bDebugMode(false)

//	CMCIMixer constructor

	{
	//	Initialize events so we can post work even before our processing thread
	//	is running.

	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWorkEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hResultEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hAbortEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadDoneEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

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

		g_bInitialized = true;
		}
	}

CMCIMixer::~CMCIMixer (void)

//	CMCIMixer destructor

	{
	Shutdown();

	//	Free up our events

	::CloseHandle(m_hWorkEvent);
	::CloseHandle(m_hResultEvent);
	::CloseHandle(m_hQuitEvent);
	::CloseHandle(m_hAbortEvent);
	::CloseHandle(m_hThreadDoneEvent);

	if (g_bInitialized)
		{
		MusicClose();
		if (g_bOwnsAudio)
			Mix_CloseAudio();
		Mix_Quit();
		g_bInitialized = false;
		g_bOwnsAudio = false;
		}
	}

void CMCIMixer::AbortAllRequests (void)

//	AbortAllRequests
//
//	Remove all requests

	{
	CSmartLock Lock(m_cs);
	m_Request.DeleteAll();
	}

bool CMCIMixer::Boot (void)

//	Boot
//
//	Starts our processing thread running.

	{
	if (m_hProcessingThread == INVALID_HANDLE_VALUE && g_bInitialized)
		{
		::ResetEvent(m_hThreadDoneEvent);
		m_hProcessingThread = ::kernelCreateThread(ProcessingThread, this);
		}

	return true;
	}

void CMCIMixer::CheckForTrackEnd (void)

//	CheckForTrackEnd
//
//	MCI reports the end of a track as a mode change to MCI_MODE_STOP; this is
//	where we detect it. Called only by the processing thread.

	{
	if (m_iCurChannel == -1)
		return;

	if (m_Channels[m_iCurChannel].iState == statePlaying && MusicGetMode() == modeStopped)
		OnNotifyMode(NULL, modeStopped);
	}

bool CMCIMixer::CreateParentWindow (void)

//	CreateParentWindow
//
//	There is no notification window on macOS.

	{
	return true;
	}

void CMCIMixer::EnqueueRequest (ERequestType iType, CMusicResource *pTrack, int iPos, bool bReplace)

//	EnqueueRequest
//
//	Enqueue a request

	{
	CSmartLock Lock(m_cs);
	int i;

	//	If we're replacing a request, the see if this request type is already in the queue

	if (bReplace)
		{
		for (i = 0; i < m_Request.GetCount(); i++)
			if (m_Request[i].iType == iType)
				{
				m_Request[i].pTrack = pTrack;
				m_Request[i].iPos = iPos;
				return;
				}
		}

	//	Add the request

	SRequest &NewRequest = m_Request.EnqueueAndOverwrite();
	NewRequest.iType = iType;
	NewRequest.pTrack = pTrack;
	NewRequest.iPos = iPos;

	::SetEvent(m_hWorkEvent);
	}

void CMCIMixer::FadeAtPos (int iPos)

//	FadeAtPos
//
//	Fades starting before the given position so that the fade is complete by the
//	time we reach iPos.

	{
	EnqueueRequest(typeWaitForPos, NULL, iPos - FADE_LENGTH);
	EnqueueRequest(typeFadeOut, NULL, iPos);
	EnqueueRequest(typeStop);
	}

void CMCIMixer::FadeNow (void)

//	FadeNow
//
//	Fades right now.

	{
	EnqueueRequest(typeFadeOut, NULL, Min(GetCurrentPlayPos() + FADE_LENGTH, GetCurrentPlayLength()));
	EnqueueRequest(typeStop);
	}

bool CMCIMixer::FindChannel (HWND hMCI, SChannel **retpChannel)

//	FindChannel
//
//	Finds the channel by child window

	{
	int i;

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].hMCI == hMCI)
			{
			if (retpChannel)
				*retpChannel = &m_Channels[i];
			return true;
			}

	return false;
	}

int CMCIMixer::GetCurrentPlayLength (void)

//	GetCurrentPlayLength
//
//	Returns the length of the current track.
//
//	NOTE: These can only be called by other threads. It should never be called by
//	any processing functions because it will not return.

	{
	//	This is always stored in the channel and updated when we start playing.

	CSmartLock Lock(m_cs);
	if (m_iCurChannel == -1)
		return 0;

	return m_Channels[m_iCurChannel].iCurLength;
	}

int CMCIMixer::GetCurrentPlayPos (DWORD dwTimeout)

//	GetCurrentPlayPos
//
//	Returns the position of the current track.
//
//	NOTE: These can only be called by other threads. It should never be called by
//	any processing functions because it will not return.

	{
	//	Enqueue a request. We add TRUE to indicate that we should replace
	//	any existing GetPlayPos command.

	::ResetEvent(m_hResultEvent);
	EnqueueRequest(typeGetPlayPos, NULL, 0, true);

	//	Wait for result

	::WaitForSingleObject(m_hResultEvent, dwTimeout);

	//	Whether we succeed or not, get the result from the channel structure.

	CSmartLock Lock(m_cs);
	if (m_iCurChannel == -1)
		return 0;

	return m_Channels[m_iCurChannel].iCurPos;
	}

void CMCIMixer::GetDebugInfo (TArray<CString> *retLines) const

//	GetDebugInfo
//
//	Returns debug information about our current state

	{
	}

int CMCIMixer::GetPlayLength (HWND hMCI)

//	GetPlayLength
//
//	MCIWndGetLength equivalent (milliseconds)

	{
	if (g_pCurrentMusic == nullptr)
		return 0;

	double rSeconds = Mix_MusicDuration(g_pCurrentMusic);
	return (rSeconds > 0.0 ? (int)(rSeconds * 1000.0) : 0);
	}

int CMCIMixer::GetPlayPos (HWND hMCI)

//	GetPlayPos
//
//	MCIWndGetPosition equivalent (milliseconds)

	{
	if (g_pCurrentMusic == nullptr)
		return 0;

	double rSeconds = Mix_GetMusicPosition(g_pCurrentMusic);
	return (rSeconds > 0.0 ? (int)(rSeconds * 1000.0) : 0);
	}

CString CMCIMixer::GetRequestDesc (const SRequest &Request) const

//	GetRequestDesc
//
//	Returns a description of a request

	{
	switch (Request.iType)
		{
		case typePlay:
			return strPatternSubst(CONSTLIT("ProcessPlay: %s"), Request.pTrack->GetFilespec());

		case typeStop:
			return CONSTLIT("ProcessStop");

		case typePlayPause:
			return CONSTLIT("ProcessPlayPause");

		case typeWaitForPos:
			return strPatternSubst(CONSTLIT("ProcessWaitForPos: %d"), Request.iPos);

		case typeFadeIn:
			return strPatternSubst(CONSTLIT("ProcessFadeIn: %s"), Request.pTrack->GetFilespec());

		case typeFadeOut:
			return CONSTLIT("ProcessFadeOut");

		case typeSetPaused:
		case typeSetUnpaused:
			return CONSTLIT("ProcessSetPlayPaused");

		case typeSetVolume:
			return CONSTLIT("ProcessSetVolume");

		case typeGetPlayPos:
			return CONSTLIT("typeGetPlayPos");

		default:
			return CONSTLIT("Idle");
		}
	}

bool CMCIMixer::InitChannels (void)

//	InitChannels
//
//	Initializes all channels

	{
	CSmartLock Lock(m_cs);

	//	For now we only support 1 channel

	m_Channels.InsertEmpty(1);
	m_Channels[0].hMCI = NULL;
	m_Channels[0].iState = stateNone;
	m_Channels[0].iCurPos = 0;
	m_Channels[0].iCurLength = 0;

	m_iCurChannel = 0;

	return true;
	}

void CMCIMixer::LogDebug (const CString &sMsg)

//	LogDebug
//
//	Log debug info

	{
	if (!m_bDebugMode)
		return;

	::kernelDebugLogPattern("MCI DEBUG: %s", sMsg);
	}

void CMCIMixer::LogError (HWND hMCI, const CString &sState, const CString &sFilespec)

//	LogError
//
//	Log an error

	{
	if (!sFilespec.IsBlank())
		::kernelDebugLogPattern("MCI ERROR %s [%s]: %s", sState, sFilespec, CString(Mix_GetError()));
	else
		::kernelDebugLogPattern("MCI ERROR %s: %s", sState, CString(Mix_GetError()));
	}

LONG CMCIMixer::OnNotifyMode (HWND hWnd, int iMode)

//	OnNotifyMode
//
//	Equivalent of MCIWNDM_NOTIFYMODE (called on the processing thread).

	{
	CSmartLock Lock(m_cs);

	SChannel *pChannel;
	if (!FindChannel(hWnd, &pChannel))
		return 0;

	switch (iMode)
		{
		case modePlaying:
			pChannel->iState = statePlaying;
			pChannel->iCurLength = GetPlayLength(pChannel->hMCI);
			pChannel->iCurPos = GetPlayPos(pChannel->hMCI);

			//	Notify that we're playing

			if (g_pHI)
				g_pHI->HIPostCommand(CMD_SOUNDTRACK_NOW_PLAYING, m_pNowPlaying);
			break;

		case modeStopped:
			pChannel->iState = stateStopped;
			pChannel->iCurLength = 0;
			pChannel->iCurPos = 0;

			//	Notify that we're done with this track

			if (g_pHI && !m_bNoStopNotify)
				g_pHI->HIPostCommand(CMD_SOUNDTRACK_DONE);
			break;
		}

	return 0;
	}

LONG CMCIMixer::OnNotifyPos (HWND hWnd, int iPos)

//	OnNotifyPos
//
//	Notify that we've played to a certain position.

	{
	CSmartLock Lock(m_cs);

	SChannel *pChannel;
	if (!FindChannel(hWnd, &pChannel))
		return 0;

	if (g_pHI)
		g_pHI->HIPostCommand(CMD_SOUNDTRACK_UPDATE_PLAY_POS, (void *)(intptr_t)iPos);

	return 0;
	}

LONG APIENTRY CMCIMixer::ParentWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	ParentWndProc
//
//	There is no notification window on macOS.

	{
	return 0;
	}

bool CMCIMixer::Play (CMusicResource *pTrack, int iPos)

//	Play
//
//	Stops all channels and begins playing the given track.

	{
	EnqueueRequest(typePlay, pTrack, iPos);
	return true;
	}

bool CMCIMixer::PlayFadeIn (CMusicResource *pTrack, int iPos)

//	Play
//
//	Stops all channels and begins playing the given track.

	{
	EnqueueRequest(typeFadeIn, pTrack, iPos);
	return true;
	}

void CMCIMixer::ProcessFadeIn (const SRequest &Request)

//	ProcessFadeIn
//
//	Fade in the given track starting at the given position.

	{
	//	Stop all channels

	ProcessStop(Request, true);

	//	Play on some channel

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	//	Open new file

	CString sFilespec = Request.pTrack->GetFilespec();
	if (!MusicOpen(sFilespec))
		{
		LogError(hMCI, CONSTLIT("ProcessFadeIn MCIWndOpen"), sFilespec);
		return;
		}

	//	Set state (we need to do this before we play because the callback inside
	//	MCIWndPlay needs m_pNowPlaying to be valid).

	m_pNowPlaying = Request.pTrack;

	//	Play it at the proper position

	MusicSetVolume(0);
	if (!MusicPlay(Request.iPos))
		{
		LogError(hMCI, CONSTLIT("ProcessFadeIn MCIWndPlay"), sFilespec);
		return;
		}

	OnNotifyMode(hMCI, modePlaying);

	//	Fade in util we reach this position

	int iPrevPos = -1;
	while (true)
		{
		//	Wait a little bit

		if (!Wait(FADE_DELAY))
			return;

		//	How far into the fade

		int iCurPos = GetPlayPos(hMCI);
		UpdatePlayPos(m_iCurChannel, iCurPos);

		int iPlaying = iCurPos - Request.iPos;
		if (iPlaying <= 0 
				|| iPlaying >= FADE_LENGTH
				|| iCurPos == iPrevPos)
			{
			MusicSetVolume(m_iDefaultVolume);
			return;
			}

		//	The volume should be proportional to how much we have left

		int iVolume = m_iDefaultVolume * iPlaying / FADE_LENGTH;
		MusicSetVolume(iVolume);

		//	Remember our previous position to make sure we're making progress

		iPrevPos = iCurPos;
		}
	}

void CMCIMixer::ProcessFadeOut (const SRequest &Request)

//	ProcessFadeOut
//
//	Fade out

	{
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iStartPos = GetPlayPos(hMCI);
	int iFadeLen = Request.iPos - iStartPos;
	if (iFadeLen <= 0)
		return;

	//	Loop until we've reached out desired position (or until the end of the
	//	track).

	int iPrevPos = -1;
	while (true)
		{
		//	Wait a little bit

		if (!Wait(FADE_DELAY))
			return;

		//	How much longer until we fade completely?

		int iCurPos = GetPlayPos(hMCI);
		UpdatePlayPos(m_iCurChannel, iCurPos);

		int iLeft = Request.iPos - iCurPos;
		if (iLeft <= 0 
				|| iCurPos == iPrevPos)
			{
			MusicSetVolume(0);
			return;
			}

		//	The volume should be proportional to how much we have left

		int iVolume = m_iDefaultVolume * iLeft / iFadeLen;
		MusicSetVolume(iVolume);

		//	Remember our previous position to make sure we're making progress

		iPrevPos = iCurPos;
		}
	}

void CMCIMixer::ProcessPlay (const SRequest &Request)

//	ProcessPlay
//
//	Play the file

	{
	//	Stop all channels

	ProcessStop(Request, true);

	//	Play on some channel

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	//	Open new file

	CString sFilespec = Request.pTrack->GetFilespec();
	if (!MusicOpen(sFilespec))
		{
		LogError(hMCI, CONSTLIT("ProcessPlay MCIWndOpen"), sFilespec);
		return;
		}

	//	Set state (we need to do this before we play because the callback inside
	//	MCIWndPlay needs m_pNowPlaying to be valid).

	m_pNowPlaying = Request.pTrack;

	//	Play it at the proper position

	UpdatePlayPos(m_iCurChannel, Request.iPos);
	MusicSetVolume(m_iDefaultVolume);
	if (!MusicPlay(Request.iPos))
		{
		LogError(hMCI, CONSTLIT("ProcessPlay MCIWndPlay"), sFilespec);
		return;
		}

	OnNotifyMode(hMCI, modePlaying);
	}

void CMCIMixer::ProcessPlayPause (const SRequest &Request)

//	ProcessPlayPause
//
//	Pause and play current track.

	{
	EMusicMode iMode = MusicGetMode();
	if (iMode == modePlaying)
		Mix_PauseMusic();
	else if (iMode == modePaused)
		Mix_ResumeMusic();
	}

bool CMCIMixer::ProcessRequest (void)

//	ProcessRequest
//
//	Process a single request off the queue. If there are no more requests, then
//	we reset the worker event.

	{
	bool bMoreEvents;
	SRequest Request;

	//	Lock while we pull something off the queue

	m_cs.Lock();
	if (m_Request.GetCount() > 0)
		{
		Request = m_Request.Head();
		m_Request.Dequeue();
		}
	else
		Request.iType = typeNone;

	//	If our queue is empty, then reset the work event

	if (m_Request.GetCount() == 0)
		{
		::ResetEvent(m_hWorkEvent);
		bMoreEvents = false;
		}
	else
		bMoreEvents = true;

	//	Unlock so we can process the event

	m_cs.Unlock();

	//	Now process the event based on the type.

	switch (Request.iType)
		{
		case typeFadeIn:
			ProcessFadeIn(Request);
			break;

		case typeFadeOut:
			ProcessFadeOut(Request);
			break;

		case typeGetPlayPos:
			{
			int iValue = GetPlayPos(m_Channels[m_iCurChannel].hMCI);
			UpdatePlayPos(m_iCurChannel, iValue);
			::SetEvent(m_hResultEvent);
			break;
			}

		case typePlay:
			ProcessPlay(Request);
			break;

		case typePlayPause:
			ProcessPlayPause(Request);
			break;

		case typeSetVolume:
			ProcessSetVolume(Request);
			break;

		case typeStop:
			ProcessStop(Request);
			break;

		case typeWaitForPos:
			ProcessWaitForPos(Request);
			break;

		case typeSetPaused:
		case typeSetUnpaused:
			ProcessSetPlayPaused(Request);
			break;

		default:
			break;
		}

	//	A track may have ended while we were busy.

	CheckForTrackEnd();

	//	Return whether we think there are more events in the queue. This is a
	//	heuristic, because someone might have changed the queue from under us,
	//	but callers just use this as a hint. No harm if we're wrong.

	return bMoreEvents;
	}

void CMCIMixer::ProcessSetPlayPaused (const SRequest &Request)

//	ProcessSetPlayPaused
//
//	Set to play or paused

	{
	EMusicMode iMode = MusicGetMode();

	if (Request.iType == typeSetPaused)
		{
		if (iMode == modePlaying)
			Mix_PauseMusic();
		}
	else if (Request.iType == typeSetUnpaused)
		{
		if (iMode == modePaused)
			Mix_ResumeMusic();
		}
	}

void CMCIMixer::ProcessSetVolume (const SRequest &Request)

//	ProcessSetVolume
//
//	Sets the volume

	{
	m_iDefaultVolume = Max(0, Request.iPos);
	MusicSetVolume(m_iDefaultVolume);
	}

void CMCIMixer::ProcessStop (const SRequest &Request, bool bNoNotify)

//	ProcessStop
//
//	Stop playing

	{
	int i;

	bool bOldNotify = m_bNoStopNotify;
	if (bNoNotify)
		m_bNoStopNotify = true;

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].iState == statePlaying || m_Channels[i].iState == stateStopped)
			{
			//	MCIWndStop notifies the stop synchronously.

			if (m_Channels[i].iState != stateStopped)
				{
				Mix_HaltMusic();
				OnNotifyMode(m_Channels[i].hMCI, modeStopped);
				}

			MusicClose();
			UpdatePlayPos(i, 0);

			m_Channels[i].iState = stateNone;
			}

	m_bNoStopNotify = bOldNotify;
	m_pNowPlaying = NULL;
	}

void CMCIMixer::ProcessWaitForPos (const SRequest &Request)

//	ProcessWaitForPos
//
//	Waits until the current playback position is at the given position.

	{
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iPrevPos = -1;
	while (true)
		{
		//	Keep looping until we reach the given position or until we ask asked
		//	to stop.
		//
		//	We must be in play mode.

		if (MusicGetMode() != modePlaying)
			return;

		//	Get our current position. If we reached the position, then we're done.

		int iCurPos = GetPlayPos(hMCI);
		UpdatePlayPos(m_iCurChannel, iCurPos);

		if (iCurPos >= Request.iPos
				|| iCurPos == iPrevPos)
			return;

		//	Otherwise, we wait for a little bit (and we check to see if someone
		//	wants us to quit.

		if (!Wait(CHECK_INTERVAL))
			return;

		//	Loop and check again, but remember our current position 
		//	to make sure we're making progress

		iPrevPos = iCurPos;
		}
	}

DWORD WINAPI CMCIMixer::ProcessingThread (LPVOID pData)

//	ProcessingThread
//
//	Processing thread

	{
	CMCIMixer *pThis = (CMCIMixer *)pData;

	pThis->InitChannels();

	//	Loop until we're done. Where Windows waits for MCI notification
	//	messages, we wake up periodically to look for the end of the track.

	while (true)
		{
		HANDLE Handles[2];
		Handles[0] = pThis->m_hQuitEvent;
		const DWORD WAIT_QUIT_EVENT = (WAIT_OBJECT_0);
		Handles[1] = pThis->m_hWorkEvent;
		const DWORD WAIT_WORK_EVENT = (WAIT_OBJECT_0 + 1);

		DWORD dwWait = ::WaitForMultipleObjects(2, Handles, FALSE, CHECK_INTERVAL);

		if (dwWait == WAIT_QUIT_EVENT)
			break;
		else if (dwWait == WAIT_WORK_EVENT)
			{
			//	Process requests from the queue until we're done. NOTE: This
			//	will also reset the work event.

			bool bQuit = false;
			while (pThis->ProcessRequest())
				{
				if (::WaitForSingleObject(pThis->m_hQuitEvent, 0) == WAIT_OBJECT_0)
					{
					bQuit = true;
					break;
					}
				}

			if (bQuit)
				break;
			}
		else
			pThis->CheckForTrackEnd();
		}

	::SetEvent(pThis->m_hThreadDoneEvent);
	return 0;
	}

void CMCIMixer::SetPlayPaused (bool bPlay)

//	SetPlayPaused
//
//	Pause/Play

	{
	EnqueueRequest(bPlay ? typeSetUnpaused : typeSetPaused);
	}

void CMCIMixer::SetVolume (int iVolume)

//	SetVolume
//
//	Sets the volume, where 1000 is normal volume and 0 is no volume.

	{
	EnqueueRequest(typeSetVolume, NULL, iVolume);
	}

void CMCIMixer::Shutdown (void)

//	Shutdown
//
//	Program is quitting, so clean up. NOTE: This is the opposite of Boot, so we
//	don't clean up our events in case Boot gets called again.

	{
	if (m_hProcessingThread != INVALID_HANDLE_VALUE)
		{
		::SetEvent(m_hQuitEvent);
		::WaitForSingleObject(m_hThreadDoneEvent, 5000);

		::CloseHandle(m_hProcessingThread);
		m_hProcessingThread = INVALID_HANDLE_VALUE;
		::ResetEvent(m_hQuitEvent);
		}

	//	Now that the processing thread has stopped, we can clean up all this
	//	without locking.

	if (g_bInitialized)
		MusicClose();

	m_Channels.DeleteAll();
	m_Request.DeleteAll();
	m_iCurChannel = -1;
	m_pNowPlaying = NULL;
	}

void CMCIMixer::Stop (void)

//	Stop
//
//	Stops playing all.

	{
	EnqueueRequest(typeStop);
	}

void CMCIMixer::TogglePausePlay (void)

//	TogglePausePlay
//
//	Pause/Play

	{
	EnqueueRequest(typePlayPause);
	}

void CMCIMixer::UpdatePlayPos (int iChannel, int iPos)

//	UpdatePlayPos
//
//	Updates the play position

	{
	//	If iPos is -1 then we get the position from the MCI object. This should
	//	only be called from inside the processing thread.

	if (iPos == -1)
		iPos = GetPlayPos(m_Channels[iChannel].hMCI);

	//	Update the position

	CSmartLock Lock(m_cs);
	m_Channels[iChannel].iCurPos = iPos;
	}

bool CMCIMixer::Wait (DWORD dwTimeout)

//	Wait
//
//	Waits until the timeout or until we are asked to quit or abort. Returns 
//	FALSE we we need to quit or abort.

	{
	HANDLE Handles[2];
	Handles[0] = m_hQuitEvent;
	const DWORD WAIT_QUIT_EVENT = (WAIT_OBJECT_0);
	Handles[1] = m_hAbortEvent;
	const DWORD WAIT_ABORT_EVENT = (WAIT_OBJECT_0 + 1);

	DWORD dwWait = ::WaitForMultipleObjects(2, Handles, FALSE, dwTimeout);

	//	Quit or abort?

	if (dwWait == WAIT_QUIT_EVENT
			|| dwWait == WAIT_ABORT_EVENT)
		return false;

	//	Done

	return true;
	}

#endif // TARGET_PLATFORM_MACOS
