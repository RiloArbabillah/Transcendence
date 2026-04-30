//	CMCIMixerStub.cpp
//
//	Stub implementation of CMCIMixer for macOS
//	Provides minimal stubs so the game can link without audio

#include "stdafx.h"
#include "Soundtrack.h"

#ifdef TARGET_PLATFORM_MACOS

CMCIMixer::CMCIMixer(int iChannels) :
		m_iDefaultVolume(1000),
		m_iCurChannel(-1),
		m_pNowPlaying(NULL),
		m_bNoStopNotify(false),
		m_bDebugMode(false)
	{
	}

CMCIMixer::~CMCIMixer(void)
	{
	}

void CMCIMixer::AbortAllRequests(void)
	{
	}

bool CMCIMixer::Boot(void)
	{
	return true;
	}

void CMCIMixer::FadeAtPos(int iPos)
	{
	}

void CMCIMixer::FadeNow(void)
	{
	}

int CMCIMixer::GetCurrentPlayLength(void)
	{
	return 0;
	}

int CMCIMixer::GetCurrentPlayPos(DWORD dwTimeout)
	{
	return 0;
	}

void CMCIMixer::GetDebugInfo(TArray<CString> *retLines) const
	{
	if (retLines)
		retLines->DeleteAll();
	}

bool CMCIMixer::Play(CMusicResource *pTrack, int iPos)
	{
	return true;
	}

bool CMCIMixer::PlayFadeIn(CMusicResource *pTrack, int iPos)
	{
	return true;
	}

void CMCIMixer::SetPlayPaused(bool bPlay)
	{
	}

void CMCIMixer::SetVolume(int iVolume)
	{
	}

void CMCIMixer::Shutdown(void)
	{
	}

void CMCIMixer::Stop(void)
	{
	}

void CMCIMixer::TogglePausePlay(void)
	{
	}

#endif // TARGET_PLATFORM_MACOS