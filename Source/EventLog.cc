#include <string>
using namespace std;
#include <windows.h>
#include <winbase.h>
#include "WinException.h"
#include "EventLog.h"

namespace ipop
{
namespace win
{

EventLog::EventLog(
	const wstring & ApplicationName, 
	const wstring & UNCServerName)
{
    mLog = RegisterEventSource(NULL,						// uses local computer 
								ApplicationName.c_str());	// source name 
	if (!mLog)
		throw WINEXCEPT("Failed to register event log source");
	return;
}

EventLog::~EventLog()
{
	DeregisterEventSource(mLog); 

}

void
EventLog::AddEventSource(
	const wstring & ApplicationName, 
	const wstring & MessageFile, 
	unsigned long TypesSupported)
{
	HKEY hk; 
	DWORD disp;
    // Add your source name as a subkey under the Application 
    // key in the EventLog registry key. 
	wstring app(L"System\\CurrentControlSet\\Services\\EventLog\\Application\\");
	app += ApplicationName;
	if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, app.c_str(), 0, NULL,
								REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, &disp))
        throw WINEXCEPT("Failed to configure event logging."); 
 
    // Add the name to the EventMessageFile subkey. 
    if (ERROR_SUCCESS != RegSetValueEx(hk,												// subkey handle 
										L"EventMessageFile",							// value name 
										0,												// must be zero 
										REG_EXPAND_SZ,									// value type 
										(LPBYTE) MessageFile.c_str(),					// pointer to value data 
										(MessageFile.length() + 1)*sizeof(wchar_t)))	// length of value data 
	{
		RegCloseKey(hk);
        throw WINEXCEPT("Failed to configure event logging."); 
	}   
 
    if (ERROR_SUCCESS != RegSetValueEx(hk,							// subkey handle 
										L"TypesSupported",			// value name 
										0,							// must be zero 
										REG_DWORD,					// value type 
										(LPBYTE) &TypesSupported,	// pointer to value data 
										sizeof(unsigned long)))		// length of value data 
	{
		RegCloseKey(hk);
		throw WINEXCEPT("Failed to configure event logging."); 
	}
    RegCloseKey(hk);
	return;
}

//takes 3 strings to merge
void EventLog::LogEvent(unsigned long msgId, const TCHAR * const arg1, const TCHAR * const arg2, const TCHAR * const arg3)
{
    const TCHAR *array[4];
	int nargs = 3;


    array[0] = arg1;
    array[1] = arg2;
    array[2] = arg3;
    array[3] = '\0';

    ProcessEvent(msgId, nargs, array); 
}

void EventLog::LogEvent(unsigned long msgId, const TCHAR * const arg1, const TCHAR * const arg2)
{
    const TCHAR *array[3];
    int      nargs = 2;

    array[0] = arg1;
    array[1] = arg2;
    array[2] = '\0';
 
    ProcessEvent(msgId, nargs, array);
     
} 

void EventLog::LogEvent(unsigned long msgId, const TCHAR * const arg1)
{
    const TCHAR *array[2];
    int      nargs = 1;

    array[0] = arg1;
    array[1] = '\0';
 
    ProcessEvent(msgId, nargs, array);
} 

void EventLog::LogEvent(unsigned long msgId)
{
    ProcessEvent(msgId, 0, NULL);
} 

int EventLog::ProcessEvent(unsigned long msgId, int nbargs, const TCHAR** Messages) 
{

    
    WORD     eventtype;

    // get the event type from msg Id
    eventtype = (WORD)((msgId & 0xC0000000) >> 30);

    switch (eventtype) {
       case STATUS_SEVERITY_INFORMATIONAL:
          eventtype = EVENTLOG_INFORMATION_TYPE;
          break;
       case STATUS_SEVERITY_WARNING:
          eventtype = EVENTLOG_WARNING_TYPE;
          break;
       case STATUS_SEVERITY_ERROR:
          eventtype = EVENTLOG_ERROR_TYPE;
          break;
       case STATUS_SEVERITY_SUCCESS:
          eventtype = EVENTLOG_AUDIT_SUCCESS;
          break;
	   default:
		   break;
     }
	{
		ipop::win::Synchronized sync(mCS);
		if (!ReportEvent(mLog,		// event log handle 
						eventtype,  // event type 
						0,          // category zero 
						msgId,      // event identifier 
						NULL,       // no user security identifier 
						nbargs,     // nbargs substitution strings 
						0,			// no data 
						Messages,	// array of strings to merge 
						NULL))      // pointer to data 
		
			return GetLastError();
	}
	return 0;
  
}

}
}