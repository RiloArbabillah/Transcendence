//	CScreenMgrSDL.h
//
//	SDL2-based screen manager for macOS
//	Provides the same interface as CScreenMgr3D but uses SDL2 instead of DirectX

#pragma once

#ifdef __APPLE__

#include "DXImage32.h"

class CScreenMgrSDL
	{
	public:
		CScreenMgrSDL (void);
		~CScreenMgrSDL (void);

		bool CheckIsReady (void) { return m_bReady; }
		void CleanUp (void);
		void ClientToLocal (int x, int y, int *retx, int *rety) const { if (retx) *retx = x; if (rety) *rety = y; }
		void Flip (void) { }
		int GetHeight (void) const { return m_cyScreen; }
		bool GetInvalidRect (RECT *retrcRect) { retrcRect->left = 0; retrcRect->top = 0; retrcRect->right = m_cxScreen; retrcRect->bottom = m_cyScreen; return true; }
		CG32bitImage &GetScreen (void);
		int GetWidth (void) const { return m_cxScreen; }
		void GlobalToLocal (int x, int y, int *retx, int *rety) const { ClientToLocal(x, y, retx, rety); }
		void Init (int cxScreen, int cyScreen, CString *retsError = NULL);
		void Invalidate (void) { }
		void Invalidate (const RECT &rcRect) { }
		bool IsMinimized (void) const { return m_bMinimized; }
		void LocalToClient (int x, int y, int *retx, int *rety) const { if (retx) *retx = x; if (rety) *rety = y; }
		void LocalToGlobal (int x, int y, int *retx, int *rety) const { if (retx) *retx = x; if (rety) *rety = y; }
		void OnWMActivateApp (bool bActivate) { }
		void OnWMDisplayChange (int iBitDepth, int cxWidth, int cyHeight) { }
		void OnWMMove (int x, int y) { }
		void OnWMSize (int cxWidth, int cyHeight, int iSize) { }
		void Render (void);
		void StopDX (void) { }
		void Validate (void) { }

	private:
		CG32bitImage m_Screen;			//	Main screen buffer
		int m_cxScreen = 0;
		int m_cyScreen = 0;
		bool m_bMinimized = false;
		bool m_bReady = false;
	};

#endif // __APPLE__
