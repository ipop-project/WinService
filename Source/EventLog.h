
/**************************************************************************
                          EventLog
                      -------------------
    description			: Win32 Event Log API wrapper class.
	Wrapper class for Windows event logging. Requires the ServiceMsgs.h resource
	for the message literals and code identifiers.

***************************************************************************/     


#pragma once
#include <string>
using namespace std;
#include "Synchronized.h"

#include "IpopServiceMsg.h"

namespace ipop
{
namespace win
{

class EventLog  
{
public:
	//EventLog();
	
	EventLog(const wstring & ApplicationName, const wstring & UNCServerName = L"");

	virtual ~EventLog();

	static void AddEventSource(const wstring & ApplicationName, 
						const wstring & MessageFile, 
						unsigned long TypesSupported =	EVENTLOG_ERROR_TYPE | 
														EVENTLOG_WARNING_TYPE | 
														EVENTLOG_INFORMATION_TYPE);
	void LogEvent(	unsigned long msgId, 
					const TCHAR * const  arg1, 
					const TCHAR * const  arg2, 
					const TCHAR * const  arg3);

	void LogEvent(	unsigned long msgId, 
					const TCHAR * const arg1, 
					const TCHAR * const arg2);
	
	void LogEvent(	unsigned long msgId, 
					const TCHAR * const arg1);

	void LogEvent(unsigned long msgId);

private:
	int ProcessEvent(	unsigned long msgId, 
						int nbargs, 
						const TCHAR** array);

	HANDLE   mLog;
	ipop::win::CriticalSection mCS;
};

}
}