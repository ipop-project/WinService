#include <iostream>
#include <fstream>
#include "WinException.h"
#include "EventLog.h"
#include "ServiceConfig.h"
#include "IpopService.h"

using namespace std;

namespace ipop
{
namespace win
{

DEFINE_SERVICE(IpopService)
extern EventLog EventLogger;
extern ServiceConfig ServiceCfg;
extern fstream dbgfile;

IpopService::IpopService(
	const wstring & SvcName,
	const wstring & DisplayName,
	unsigned int SvcType) :
	WinService(SvcName, DisplayName, SvcType)
{
	m_This = this;
	SetAcceptedControls(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN );
	mSvcExitEv = CreateEvent(NULL, FALSE, FALSE, L"IPoPService Exit Event");
	if (!mSvcExitEv)
	{
		throw WINEXCEPT("Failed to create the IPoP service exit event");
	}
}


IpopService::~IpopService()
{
	if (mSvcExitEv)
		CloseHandle(mSvcExitEv);
}

void IpopService::Init(){
	//call base init
	WinService::Init();
}

/*
FUNCTION:
IpopService::Run()
PURPOSE:
Starts tincan and controller processes and periodically checks their health. If 
either process has stoppped or fails to respond to the heartbeat query this service
will attempt to terminate both processes and restart them. The polling interval
is currently fixed at 60 secs.
PARAMETERS:
None.
RETURN VALUE:
int - the status code supplied by the child class implementation of Execute()
THROWS:
None
COMMENTS:
*/
void IpopService::Run(){
	//initial process startup
	try{
		mWDog.StartIPoPProcesses();
		while (true){
			unsigned long wait = WaitForSingleObject(mSvcExitEv, ServiceCfg.GetHealthCheckInterval()); //60sec
			switch (wait){
			case WAIT_TIMEOUT:
				//check processes status
				mWDog.CheckHealth();
				break;
			case WAIT_OBJECT_0:
				//exit event is set, service is shutting down
				mWDog.TerminateIPoPProcesses();
				return;
			default:
				break;
			}

		}
	}
	catch (exception & e)
	{
		dbgfile << e.what() << endl;
	}
	return;
}

void IpopService::OnStop(){
	EventLogger.LogEvent(IPOP_STOPPING);
	if (!SetEvent(mSvcExitEv)){
		throw WINEXCEPT("Failed to set exit event");
	}
}

void IpopService::OnShutdown(){
	OnStop();
}

void IpopService::OnPause(){
	EventLogger.LogEvent(IPOP_PAUSE);

}

void IpopService::OnContinue(){
	EventLogger.LogEvent(IPOP_CONTINUE);

}

void IpopService::HandleUserDefined(unsigned int Control){
	/*
	switch (Control) {

	default:
		//log invalid User control
		break;
	}
	*/
}

void IpopService::Debug(){
	SetConsoleCtrlHandler(ControlHandler, TRUE);
	WinService::Debug();
}


/*
FUNCTION:ControlHandler
PURPOSE: Handles keyboard signals
PARAMETERS: The signal code 
RETURN VALUE: None
THROWS: None
COMMENTS:
++*/
BOOL __stdcall IpopService::ControlHandler(DWORD CtrlType){
	switch (CtrlType){
	case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
	case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
		cout << "Stopping... " << endl;
		m_This->OnStop();
		return(TRUE);
	}
	return(FALSE);
}

}
}