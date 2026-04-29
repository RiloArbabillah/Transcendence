//	CHTML.h
//
//	CHTML class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#ifndef CHTML_INCLUDED
#define CHTML_INCLUDED

class CHTML
	{
	public:
		static bool FindStdEntity (const CString &sEntity, CString *retsValue);
		static CString TranslateStdEntity (const CString &sEntity);
	};

#endif
