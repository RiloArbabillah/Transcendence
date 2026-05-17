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

	//	Create the semaphore/event handle. On macOS we currently use Win32
	//	compatibility stubs, so ::CreateEvent() is not a real kernel object and
	//	returns NULL. For the single-threaded compatibility path we only need a
	//	stable non-null sentinel handle here.

#ifdef TARGET_PLATFORM_MACOS
	m_hHandle = reinterpret_cast<HANDLE>(this);
	if (retbExists)
		*retbExists = false;
#else
	m_hHandle = ::CreateEvent(NULL, 
			TRUE, 
			FALSE, 
			(sName.IsBlank() ? NULL : (LPSTR)sName));
	if (m_hHandle == NULL)
		throw CException(ERR_MEMORY);

	//	See if the semaphore already exists

	if (retbExists)
		*retbExists = (::GetLastError() == ERROR_ALREADY_EXISTS);
#endif
	}
