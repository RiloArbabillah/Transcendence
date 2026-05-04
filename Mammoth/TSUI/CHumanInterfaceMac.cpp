//	CHumanInterfaceMac.cpp
//	macOS bridge helpers for CHumanInterface

#include "stdafx.h"

LONG CHumanInterface::OnTimer (DWORD dwID)

//	OnTimer

	{
	m_Timers.FireTimer(m_hWnd, dwID);
	return 0;
	}

LONG CHumanInterface::WMKeyDown (int iVirtKey, DWORD dwKeyData)

//	WMKeyDown
//
//	Handle WM_KEYDOWN message (called from SDL bridge)

	{
	if (m_pCurSession)
		{
		IHISession *pOldSession = m_pCurSession;

		m_iLastVirtualKey = iVirtKey;
		m_pCurSession->HIKeyDown(iVirtKey, dwKeyData);
		m_iLastVirtualKey = 0;

		if (g_pHI == NULL)
			return 0;

		if (m_pCurSession != pOldSession)
			m_chKeyDown = iVirtKey;
		}

	return 0;
	}

LONG CHumanInterface::WMKeyUp (int iVirtKey, DWORD dwKeyData)

//	WMKeyUp
//
//	Handle WM_KEYUP message (called from SDL bridge)

	{
	if (m_pCurSession)
		m_pCurSession->HIKeyUp(iVirtKey, dwKeyData);

	return 0;
	}

LONG CHumanInterface::WMChar (char chChar, DWORD dwKeyData)

//	WMChar
//
//	Handle WM_CHAR message (called from SDL bridge)

	{
	if (m_chKeyDown && (m_chKeyDown == chChar || m_chKeyDown == (chChar - ('a' - 'A'))))
		{
		m_chKeyDown = '\0';
		return 0;
		}
	else
		m_chKeyDown = '\0';

	if (m_pCurSession)
		m_pCurSession->HIChar(chChar, dwKeyData);

	return 0;
	}

LONG CHumanInterface::WMLButtonDown (int x, int y, DWORD dwFlags)

//	WMLButtonDown

	{
	CaptureMouse();

	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);

		m_iLastVirtualKey = VK_LBUTTON;
		m_pCurSession->HILButtonDown(xLocal, yLocal, dwFlags);
		m_iLastVirtualKey = 0;
		}

	m_bLButtonDown = true;
	return 0;
	}

LONG CHumanInterface::WMLButtonUp (int x, int y, DWORD dwFlags)

//	WMLButtonUp

	{
	ReleaseMouse();

	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HILButtonUp(xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMMouseMove (int x, int y, DWORD dwFlags)

//	WMMouseMove

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);

		m_bMouseMoved = true;
		m_xLastMousePos = x;
		m_yLastMousePos = y;
		m_pCurSession->HIMouseMove(xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMRButtonDown (int x, int y, DWORD dwFlags)

//	WMRButtonDown

	{
	CaptureMouse();

	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);

		m_iLastVirtualKey = VK_RBUTTON;
		m_pCurSession->HIRButtonDown(xLocal, yLocal, dwFlags);
		m_iLastVirtualKey = 0;
		}

	m_bRButtonDown = true;
	return 0;
	}

LONG CHumanInterface::WMRButtonUp (int x, int y, DWORD dwFlags)

//	WMRButtonUp

	{
	ReleaseMouse();

	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HIRButtonUp(xLocal, yLocal, dwFlags);
		}

	m_bRButtonDown = false;
	return 0;
	}

LONG CHumanInterface::WMMButtonDown (int x, int y, DWORD dwFlags)

//	WMMButtonDown

	{
	CaptureMouse();

	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);

		m_iLastVirtualKey = VK_MBUTTON;
		m_pCurSession->HIMButtonDown(xLocal, yLocal, dwFlags);
		m_iLastVirtualKey = 0;
		}

	m_bMButtonDown = true;
	return 0;
	}

LONG CHumanInterface::WMMButtonUp (int x, int y, DWORD dwFlags)

//	WMMButtonUp

	{
	ReleaseMouse();

	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.ClientToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HIMButtonUp(xLocal, yLocal, dwFlags);
		}

	m_bMButtonDown = false;
	return 0;
	}

LONG CHumanInterface::WMMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	WMMouseWheel

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;
		m_ScreenMgr.GlobalToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HIMouseWheel(iDelta, xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMSize (int cxWidth, int cyHeight, int iSize)

//	WMSize

	{
	m_ScreenMgr.OnWMSize(cxWidth, cyHeight, iSize);

	if (m_pCurSession)
		m_pCurSession->HISize(GetScreenWidth(), GetScreenHeight());

	return 0;
	}

LONG CHumanInterface::WMMove (int x, int y)

//	WMMove

	{
	m_ScreenMgr.OnWMMove(x, y);

	if (m_pCurSession)
		m_pCurSession->HISize(GetScreenWidth(), GetScreenHeight());

	return 0;
	}
