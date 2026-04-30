//	CMCIMixerStub.h
//
//	Stub implementation of CMCIMixer for non-Windows platforms
//	This provides minimal stubs so the game can link and run without audio

#pragma once

#include "Soundtrack.h"

class CMCIMixerStub : public CMCIMixer
	{
	public:
		CMCIMixerStub(int iChannels = 1) : CMCIMixer(iChannels) { }
		~CMCIMixerStub(void) { }

		void AbortAllRequests(void) { }
		bool Boot(void) { return true; }
		void FadeAtPos(int iPos) { }
		void FadeNow(void) { }
		int GetCurrentPlayLength(void) { return 0; }
		int GetCurrentPlayPos(DWORD dwTimeout = 5000) { return 0; }
		void GetDebugInfo(TArray<CString> *retLines) const { }
		bool Play(CMusicResource *pTrack, int iPos = 0) { return true; }
		bool PlayFadeIn(CMusicResource *pTrack, int iPos = 0) { return true; }
		void SetPlayPaused(bool bPlay) { }
		void SetVolume(int iVolume) { }
		void Shutdown(void) { }
		void Stop(void) { }
		void TogglePausePlay(void) { }
	};

//	Use the stub on non-Windows platforms
#define CMCIMixer CMCIMixerStub