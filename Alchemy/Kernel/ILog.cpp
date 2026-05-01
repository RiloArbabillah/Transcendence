//	ILog.cpp
//
//	ILog Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void ILog::LogOutput (DWORD dwFlags, const CString &sLine) const

//	LogOutput
//
//	Output a line to the log

	{
	//	Write the time date

	if (dwFlags & ILOG_FLAG_TIMEDATE)
		{
		SYSTEMTIME time;

		GetLocalTime(&time);

		// Build timestamp string manually instead of using strPatternSubst with %s
		char szTime[64];
		snprintf(szTime, sizeof(szTime), "%04d-%02d-%02d %02d:%02d:%02d\t",
				time.wYear, time.wMonth, time.wDay,
				time.wHour, time.wMinute, time.wSecond);

		CString sOutput(szTime);
		sOutput.Append(sLine);
		Print(sOutput);
		}
	else
		Print(sLine);
	}

void ILog::LogOutput (DWORD dwFlags, char *pszLine, ...) const

//	LogOutput
//
//	Output a line to the log

	{
	CString sParsedLine;
	va_list args;
	va_start(args, pszLine);
	char *pArgs = (char *)args;
	sParsedLine = strPattern(CString(pszLine, ::strlen(pszLine), TRUE), (void **)pArgs);
	va_end(args);

	LogOutput(dwFlags, sParsedLine);
	}

