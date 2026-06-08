//	CScreenMgrSDL.cpp
//
//	SDL2-based screen manager for macOS
//	Provides the same interface as CScreenMgr3D but uses SDL2 instead of DirectX

#ifdef __APPLE__

#include "CScreenMgrSDL.h"

CScreenMgrSDL::CScreenMgrSDL (void)

//	CScreenMgrSDL constructor

	{
	}

CScreenMgrSDL::~CScreenMgrSDL (void)

//	CScreenMgrSDL destructor

	{
	CleanUp();
	}

void CScreenMgrSDL::CleanUp (void)

//	CleanUp
//
//	Free resources

	{
	m_bReady = false;
	}

void CScreenMgrSDL::Init (int cxScreen, int cyScreen, CString *retsError)

//	Init
//
//	Initialize screen manager with given dimensions

	{
	m_cxScreen = cxScreen;
	m_cyScreen = cyScreen;

	PlatformResizeScreen(cxScreen, cyScreen);

	//	Get the platform framebuffer info
	SPlatformScreenInfo info = PlatformGetScreenInfo();

	//	If dimensions match, use the platform framebuffer directly
	if (info.cxWidth == cxScreen && info.cyHeight == cyScreen)
		{
		m_Screen.CreateFromExternalBuffer((CG32bitPixel*)info.pPixels, cxScreen, cyScreen, info.cbPitch);
		}
	else
		{
		//	Create our own buffer if sizes don't match
		m_Screen.Create(cxScreen, cyScreen);
		}

	m_bReady = true;
	}

CG32bitImage &CScreenMgrSDL::GetScreen (void)

//	GetScreen
//
//	Returns the screen buffer

	{
	return m_Screen;
	}

void CScreenMgrSDL::Render (void)

//	Render
//
//	Present the screen

	{
	if (!m_bReady)
		return;

	PlatformPresentScreen();
	}

#endif // __APPLE__
