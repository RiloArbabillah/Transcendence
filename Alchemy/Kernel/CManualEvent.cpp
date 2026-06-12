//	CManualEvent.cpp
//
//	CManualEvent class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CManualEvent::Create (void)

//	Create
//
//	Create the event

	{
	Create(NULL_STR); 
	}

void CManualEvent::Create (const CString &sName, bool *retbExists)

//	Create
//
//	Create the event

	{
	Close();

	//	Create the event handle using the real CreateEvent implementation

	m_hHandle = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hHandle == NULL)
		throw CException(ERR_MEMORY);

	if (retbExists)
		*retbExists = false;
	}
