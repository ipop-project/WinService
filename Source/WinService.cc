#include <stdexcept>
#include <iostream>
using namespace std;
#include "WinException.h"
#include "winservice.h"

namespace ipop
{
namespace win
{

const int WinService::NUMEVENTS = 4;
const int WinService::MAX_SERVICENAME_LEN = 256;


WinService::WinService(
	const wstring & SvcName, 
	const wstring & SvcDisplay, 
	unsigned int Type) : 
		m_Type(Type), 
		m_Name(SvcName),
		m_Display(SvcDisplay),
		m_ServiceStatus(NULL),
		m_WatcherThread(NULL),
		m_State(0),
		m_RequestedControl(0),
		m_ControlsAccepted(0),
		m_Checkpoint(0),
		m_WaitHint(0)
{
	// Set up class critical section
	InitializeCriticalSection(&m_cs);
	// Initialize event handles to NULL
	memset(&m_Events, 0x0, WinService::NUMEVENTS);
	mSelf = this;
}

WinService::~WinService()
{
	DeleteCriticalSection(&m_cs);
}

/*--
FUNCTION:
	PreInit()
PURPOSE:
	Facilitates code execution before the class is initialized
PARAMETERS:
	None
RETURN VALUE:
	None
THROWS:
	bad_alloc
COMMENTS:
++*/
void WinService::PreInit(LPHANDLER_FUNCTION HandlerFunc){ 
	// Initialize Events
	for(int i = 0 ; i < NUMEVENTS ; i++)
	{
		m_Events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(!m_Events[i]){
			for(int j = i-1; j >= 0; j--) 
				CloseHandle(m_Events[j]);
			throw WINEXCEPT("Failed on CreateEvent");
		}	      
	}
		SetupHandler(HandlerFunc);
		Thread::Start(NULL);
}

/*--
FUNCTION:
	DeInit
PURPOSE:
	Prepares the service for shutdown by attempting to orderly release allocated resources
PARAMETERS:
	None
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
	The function waits a abounded period of time for the watcher thread to terminate 
	before closing the handle. This is done so as to prevent an indefinate wait if the
	watcher thread failed to terminate.
++*/
void WinService::DeInit()
{
   // Closes the watcher thread handle
   if(m_WatcherThread)
   {
      // Wait a reasonable amount of time for watcher thread to terminate
      WaitForSingleObject(m_WatcherThread, 10000);
      //CloseHandle(m_WatcherThread);
   }

   // Uninitialize any resources created in Init()
   for(int i = 0 ; i < WinService::NUMEVENTS ; i++)
   {
      if(m_Events[i])
         CloseHandle(m_Events[i]);
   }
}

/*--
FUNCTION:
	ServiceMainWorker
PURPOSE:
	Starts the service
PARAMETERS:
	argc - the number of parameter passed to function
	argv - array of arguments as C strings
	HandlerFunc - Event handler routine
	WatcherFunc - Watcher routine
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
++*/
void WinService::ServiceMainWorker(unsigned int argc, wchar_t** argv, 
								   LPHANDLER_FUNCTION HandlerFunc){ 
	unsigned int Err = 0;
	try{
		PreInit(HandlerFunc);
		Init();
		CallRun();
		DeInit();
	}
	catch(exception& e){
		//eLog.LogEvent( SERVICE_NAME, ei.MessageID );
		UNREFERENCED_PARAMETER(e);
	}
	SetStatus(SERVICE_STOPPED, 0, 0, 0, 0, 0);

	return;
}

/*--
FUNCTION:
	SetupHandler
PURPOSE:
	Registers the control handler for the service
PARAMETERS:
	HandlerFunc - Function responsible for handling service control events
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
++*/
void WinService::SetupHandler(LPHANDLER_FUNCTION HandlerFunc) 
{
	m_ServiceStatus = RegisterServiceCtrlHandler(m_Name.c_str(), HandlerFunc);
	if(!m_ServiceStatus){
		throw WINEXCEPT("Failed on RegisterServiceCtrlHandler");
	}
	SetStatus(SERVICE_START_PENDING, 1, 5000);
	return;
}

/*--
FUNCTION:
	HandlerWorker
PURPOSE:
	This is the handler routine for the service
PARAMETERS:
	
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
	This function intercepts the control notifications from the host system. It then
	sets the appropriate event which wakes the watcher thread worker.
++*/
void WinService::HandlerWorker(unsigned int Control)
{
	unsigned int PriorControlState = 0;
	// Keep an additional control request of the same type
	//  from coming in when you're already handling it
	if(m_RequestedControl == Control)
		return;

	switch (Control) {
	case SERVICE_CONTROL_STOP:
		m_RequestedControl = Control;
		// Notify the service to stop...
		SetEvent(m_Events[STOP]);
		break;

	case SERVICE_CONTROL_PAUSE:
		m_RequestedControl = Control;
		// Notify the service to pause...
		SetEvent(m_Events[PAUSE]);
		break;

	case SERVICE_CONTROL_CONTINUE:
		if(GetStatus() != SERVICE_RUNNING)
		{
			m_RequestedControl = Control;
			// Notify the service to continue...
			SetEvent(m_Events[CONTINUE]);
		}
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		m_RequestedControl = Control;
		SetEvent(m_Events[SHUTDOWN]);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		// Return current status on interrogation
		//SetStatus(GetStatus());
		break;

	default: // User Defined
		PriorControlState = m_RequestedControl;
		m_RequestedControl = Control;
		HandleUserDefined(Control);
		m_RequestedControl = PriorControlState;
	}
}


/*--
FUNCTION:
	WatcherThreadWorker
PURPOSE:
	Listens for control events and triggers the appropriate handler
PARAMETERS:
	None
RETURN VALUE:
	None
THROWS:
	None
COMMENTS:
++*/
void WinService::WatcherThreadWorker()
{
	unsigned int Wait = 0;
	bool ListenForEvents = true;

	// Wait for any events to signal
	while(ListenForEvents)
	{
		Wait = WaitForMultipleObjects(NUMEVENTS, m_Events, FALSE, INFINITE);

		switch(Wait - WAIT_OBJECT_0)
		{
		case STOP:
			CallStop();
			ListenForEvents = false;
			break;

		case PAUSE:
			CallPause();
			ResetEvent(m_Events[PAUSE]);
			break;

		case CONTINUE:
			CallContinue();
			ResetEvent(m_Events[CONTINUE]);
			break;

		case SHUTDOWN:
			CallShutdown();
			ListenForEvents = false;
			break;
		default:
			throw WINEXCEPT("Invalid switch option");
		}
	}
	return;
}

/*--
FUNCTION:
	SetStatus
PURPOSE:
	Updates the service status to the one specified
PARAMETERS:
	None
RETURN VALUE:
	None
THROWS:
	invalid_argument
	runtime_error
COMMENTS:
	The functions checks to ensure that parameters are valid for the requested state change.
	For Example, if a pending state is specifeied then a hint and check point must be specified.
++*/
void WinService::SetStatus(unsigned int State, unsigned int CheckPoint, 
						   unsigned int WaitHint, unsigned int AcceptedControls, 
						   unsigned int ExitCode, unsigned int SpecificExit){ 
	// The only state that can set Exit Codes is STOPPED
	// Fix if necessary, just in case it is not properly set.
	if(State != SERVICE_STOPPED)
	{
		ExitCode = S_OK;
		SpecificExit = 0;
	}

	// Only pending states can set checkpoints or wait hints,
	//  and pending states *must* set wait hints
	if(State == SERVICE_STOPPED || State == SERVICE_PAUSED || State == SERVICE_RUNNING){	
		// Requires hint and checkpoint == 0
		// Fix it so that NO_CHANGE from previous state doesn't cause nonzero
		WaitHint = 0;
		CheckPoint = 0;
	}
	else{	
		// Requires hint and checkpoint != 0
		if(WaitHint == 0 || CheckPoint ==0){
			throw WINEXCEPT("Pending statuses require a valid hint and checkpoint");
		}
	}
	 
	// Function can be called by multiple threads - protect member data
	EnterCriticalSection(&m_cs);

	// Alter states if changing
	m_State = State;

//	if(CheckPoint != STATE_NO_CHANGE)
	m_Checkpoint = CheckPoint;

//	if(WaitHint != STATE_NO_CHANGE)
	m_WaitHint = WaitHint;

//	if(AcceptedControls != STATE_NO_CHANGE)
	m_ControlsAccepted = AcceptedControls;

	SERVICE_STATUS ss = { m_Type, m_State, m_ControlsAccepted,
		ExitCode, SpecificExit, m_Checkpoint, m_WaitHint };

	if(!SetServiceStatus(m_ServiceStatus, &ss)) {
		LeaveCriticalSection(&m_cs);
		throw WINEXCEPT("Failed on SetServiceStatus");
	}	
	LeaveCriticalSection(&m_cs);


}


void 
WinService::Install(
	const wchar_t * const & SvcName,
	const wchar_t * const & SvcDisplayName,
	const wchar_t * const & SvcDescription,
	unsigned long ServiceType,
	unsigned long StartType)
{
	
    SC_HANDLE   Service = NULL;
    SC_HANDLE   SCManager = NULL;

	wchar_t *Path = new wchar_t[1024];
	memset(Path, 0x0, 1024);

    if (GetModuleFileName(NULL, Path, 1024) == 0){
        throw WINEXCEPT("Failed GetModuleFilename");
    }

    SCManager = OpenSCManager(NULL,						// machine (NULL == local)
							NULL,						// database (NULL == default)
							SC_MANAGER_ALL_ACCESS);		// type of access required
    if (!SCManager){
		throw WINEXCEPT("Failed on OpenSCManager");
	}
    Service = CreateService(
        SCManager,					// SCManager database
        SvcName,					// name of service
        SvcDisplayName,				// name to display
        SERVICE_ALL_ACCESS,         // desired access
        ServiceType,				// service type
        StartType,					// start type
        SERVICE_ERROR_NORMAL,       // error control type
        Path,						// service's binary
        NULL,                       // no load ordering group
        NULL,                       // no tag identifier
        NULL,						// dependencies
        NULL,                       // LocalSystem account
        NULL);                      // no password

	delete []Path;

	if (!Service){
		CloseServiceHandle(SCManager);
		throw WINEXCEPT("Failed on CreateService");
	}else{
		SERVICE_DESCRIPTION svcdesc;
		svcdesc.lpDescription = const_cast<LPWSTR>(SvcDescription);
		ChangeServiceConfig2(Service, SERVICE_CONFIG_DESCRIPTION, &svcdesc);
		CloseServiceHandle(Service);
		CloseServiceHandle(SCManager);
	}
	return;
}

void 
WinService::Remove(
	const wchar_t * const & SvcName)
{
    SC_HANDLE   Service;
    SC_HANDLE   SCManager;
	SERVICE_STATUS Status;

    SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    
	if (!SCManager){
		throw WINEXCEPT("Remove service failed - Failed on OpenSCManager");
	}
    Service = OpenService(SCManager, SvcName, SERVICE_ALL_ACCESS);

    if (!Service){
		CloseServiceHandle(SCManager);
		throw WINEXCEPT("Remove service failed - Failed on OpenService");
	}


	if (ControlService(Service, SERVICE_CONTROL_STOP, &Status)){
		cout << "Stopping " << SvcName;
		Sleep(1000);

		while(QueryServiceStatus(Service, &Status)){
			if (Status.dwCurrentState == SERVICE_STOP_PENDING){
				cout << ".";
				Sleep(1000);
			}
			else
				break;
		}

		if (Status.dwCurrentState == SERVICE_STOPPED)
			cout << endl << SvcName << " stopped." << endl;
		else
			cout << endl << SvcName << " failed to stop." << endl;

	}


	if (!DeleteService(Service)){
		CloseServiceHandle(Service);
		CloseServiceHandle(SCManager);
		throw WINEXCEPT("Remove Service failed - Failed on DeleteService");
	}
        
	CloseServiceHandle(Service);
	CloseServiceHandle(SCManager);
   
}

void WinService::Debug(){
	cout << "Ready..." << endl;
	Run();
	//SetConsoleCtrlHandler(ControlHandler, TRUE);
}

void WinService::Init(){
	m_Checkpoint = 1;
	SetStatus(SERVICE_START_PENDING, m_Checkpoint, 2000);
}

void WinService::CallRun(){
	SetStatus( SERVICE_RUNNING, 0, 0, m_ControlsAccepted);
	Run();
}

void WinService::CallStop(){
	m_Checkpoint = 1;
	SetStatus(SERVICE_STOP_PENDING, m_Checkpoint++, 300000);
	OnStop();
}

void WinService::CallShutdown(){
	m_Checkpoint = 1;
	SetStatus(SERVICE_STOP_PENDING, m_Checkpoint++, 300000);
	OnShutdown();
}

void WinService::CallPause(){
	m_Checkpoint = 1;
	SetStatus(SERVICE_PAUSE_PENDING, m_Checkpoint++, 300000);
	OnPause();
	SetStatus(SERVICE_PAUSED, 0, 300000);
}

void WinService::CallContinue(){
	SetStatus(SERVICE_CONTINUE_PENDING, m_Checkpoint, 300000);
	OnContinue();
	SetStatus(SERVICE_RUNNING, 0, 0, SERVICE_ACCEPT_STOP| SERVICE_ACCEPT_SHUTDOWN);
}

void WinService::CallHandleUserDefined(unsigned int Control){
	HandleUserDefined(Control);
}

int WinService::Execute(void*){
	WatcherThreadWorker();
	return 0;
}

}
}