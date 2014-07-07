/**************************************************************************
                          WinService
                      -------------------
    description			: Win32 Service API wrapper class.
	Each service will derive from this base class.
	See sample SvcClass2.h and SvcClass2.cpp how to use
	xxxx.h
	*1.Specify Winservice as base clase
	*2.Implement pure virtual functions OnRun(), OnStop(), OnShutDown(), ParseArgs(), OnPause(), OnContinue(), HandleUserDefined()
	*3.Override PreInit(), Init(), DeInit(), Debug()
	*4.Invoke Macro DECLARE_SERVICE(CLASS_NAME)
	xxxx.cpp
	*1.Invoke Macro DEFINE_SERVICE(CLASS_NAME)
	*2.Make assignment m_This = this in constructor
	*3.Implement overridden functions, remember to call base version
	*4.In main, invoke macros : BEGIN_SERVICE_MAP, SERVICE_MAP_ENTRY(ClassName, ServiceName), END_SERVICE_MAP

***************************************************************************/     
#if !defined(WINSERVICE_H)
#define WINSERVICE_H

#include <windows.h>
#include <process.h>
#include <string>
using namespace std;
#include "Thread.h"

namespace ipop
{
namespace win
{

class WinService : virtual public Thread{
public:
	static const int NUMEVENTS;
	static const int MAX_SERVICENAME_LEN;


	WinService(
		const wstring & SvcName, 
		const wstring & DisplayName, 
		unsigned int SvcType);
	virtual ~WinService();
	virtual void Debug();
	static void Install(	
		const wchar_t * const & SvcName,
		const wchar_t * const & SvcDisplayName,
		const wchar_t * const & SvcDescription,
		unsigned long ServiceType = SERVICE_WIN32_OWN_PROCESS,
		unsigned long StartType = SERVICE_DEMAND_START);
	static void Remove(
		const wchar_t * const & SvcName);

	unsigned int   GetStatus()		{ return m_State; }
	unsigned int   GetControls()	{ return m_ControlsAccepted; }
	void SetAcceptedControls(unsigned int Controls){ m_ControlsAccepted = Controls; }
	wstring GetName()		{ return m_Name; }
	wstring GetDisplayName()	{ return m_Display;	}

	// All derived class static ServiceMain functions are delegated here
	void	ServiceMainWorker(unsigned int	argc, wchar_t ** argv,
								LPHANDLER_FUNCTION HandlerFunc);

	// All derived class static handler functions are delegated here
	void	HandlerWorker(unsigned int	Control);


protected:
	void SetupHandler(LPHANDLER_FUNCTION HandlerProc);
	void WatcherThreadWorker();
	void SetStatus(	unsigned int State,
					unsigned int Checkpoint,
					unsigned int WaitHint = 1000,	//one second
					unsigned int AcceptedControls = SERVICE_ACCEPT_STOP| SERVICE_ACCEPT_SHUTDOWN,
					unsigned int ExitCode = NO_ERROR,
					unsigned int SpecificExit = 0);
	void PreInit(LPHANDLER_FUNCTION HandlerFunc);
	void DeInit();
	
	/**Overrideables**/
	//virtual	void ParseArgs(unsigned int argc, LPTSTR* argv);	
	virtual	void Init();
	virtual	void Run() = 0;		//Executes continuously until the service is stopped
	virtual	void OnStop() =	0;
	virtual	void OnShutdown() =	0;
	virtual	void OnPause() = 0;
	virtual	void OnContinue() = 0;
	virtual	void HandleUserDefined(unsigned int Control) = 0;

	/**ATTRIBUTES**/
	CRITICAL_SECTION	m_cs;
	// Status info
	SERVICE_STATUS_HANDLE m_ServiceStatus;
	unsigned int m_State;
	unsigned int m_ControlsAccepted;
	unsigned int m_Checkpoint;
	unsigned int m_WaitHint;
	// Tracks state currently being worked on in Handler
	unsigned int m_RequestedControl;
	// Control Events
	HANDLE m_Events[4];//!!WinService::NUMEVENTS];
	HANDLE m_WatcherThread;
	const wstring & m_Name;
	const wstring & m_Display;
	unsigned int m_Type;
	enum	EVENTS { STOP, PAUSE, CONTINUE,	SHUTDOWN };
	
	//From Class Thread
	int Execute(void*);
private:
	WinService* mSelf;
	void CallRun();
	void CallStop();
	void CallShutdown();
	void CallPause();
	void CallContinue();
	void CallHandleUserDefined(unsigned int Control);
	//static BOOL __stdcall ControlHandler (DWORD CtrlType);

};
/*************************************************************/

// Implements static thread functions for derived classes
#define DECLARE_SERVICE(ClassName) \
public: \
   static ClassName##* m_This; \
   static void WINAPI Main(DWORD argc, LPTSTR* argv); \
   static void WINAPI Handler(unsigned int Control);

/*************************************************************/

#define DEFINE_SERVICE(ClassName) \
ClassName##* ClassName##::m_This = NULL; \
void WINAPI ClassName##::Main(DWORD argc, LPTSTR* argv) {\
	m_This->ServiceMainWorker(argc, argv, (LPHANDLER_FUNCTION)ClassName##::Handler); \
} \
void WINAPI ClassName##::Handler(unsigned int Control) {\
	m_This->HandlerWorker(Control); \
} 

/*************************************************************/

// For implementing a service process
#define BEGIN_SERVICE_MAP \
SERVICE_TABLE_ENTRY svcTable[] = {

#define SERVICE_MAP_ENTRY(ClassName, ServiceName) \
{L"ServiceName", ClassName##::Main},

#define END_SERVICE_MAP \
{NULL, NULL}}; \
StartServiceCtrlDispatcher(svcTable);

/*************************************************************/

}
}
#endif // !defined(WINSERVICE_H
