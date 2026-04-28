//	TSUISessions.cpp
//
//	Transcendence UI Engine - Session classes
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "TSUI.h"

CMessageSession::CMessageSession (CHumanInterface &HI, const CString &sTitle, const CString &sMessage, const CString &sCommand) :
		IHISession(HI),
		m_sTitle(sTitle),
		m_sMessage(sMessage),
		m_sCommand(sCommand)
	{
	}

ALERROR CMessageSession::OnCommand (const CString &sCmd, void *pData)
	{
	if (strEquals(sCmd, CONSTLIT("cmdDone")))
		CmdDone();
	else
		return ERR_NOTFOUND;

	return NOERROR;
	}

ALERROR CMessageSession::OnInit (CString *retsError)
	{
	CreateDlgMessage(NULL);
	return NOERROR;
	}

void CMessageSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)
	{
	CmdDone();
	}

void CMessageSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)
	{
	}

void CMessageSession::OnReportHardCrash (CString *retsMessage)
	{
	}

void CMessageSession::CmdDone (void)
	{
	}

void CMessageSession::CreateDlgMessage (IAnimatron **retpDlg)
	{
	if (retpDlg)
		*retpDlg = NULL;
	}