//	CHumanInterfaceMac.cpp
//	macOS bridge helpers for CHumanInterface

#include "stdafx.h"

LONG CHumanInterface::OnTimer (DWORD dwID)

//	OnTimer

	{
	m_Timers.FireTimer(m_hWnd, dwID);
	return 0;
	}
