#include <memory>
#include <iostream>
#include <fstream>
#include "WatchDog.h"
#include "WinException.h"
#include "EventLog.h"
#include "SystemInfo.h"
#include "ServiceConfig.h"

namespace ipop
{
namespace win
{

extern EventLog EventLogger;
extern SystemInfo SysInfo;
extern ServiceConfig ServiceCfg;

extern fstream dbgfile;

WatchDog::WatchDog() :
	mHealthCheckInterval(60000),
	mTincanPort(5800),
	mControllerPort(5801)
{
	if (SysInfo.IsInet6Capable())
	{
		mTincanAddress.assign("::1");
		mAF = AF_INET6;
	}
	else
	{
		mTincanAddress.assign("127.0.0.1");
		mAF = AF_INET6;
	}
	mControllerAddress.assign(mTincanAddress),
	SetCurrentDirectory(ServiceCfg.GetIPoPPath().c_str());

}
		
WatchDog::~WatchDog()
{}


void 
makeMBStr(
	wstring & in, 
	string & out)
{
	PSZ outp;
	//int inLenUnicode = lstrlenW(in.c_str()); // Convert all UNICODE characters
	int outLen = WideCharToMultiByte(CP_ACP, 0, in.c_str(), -1, NULL, 0, NULL, NULL);

	outp = new char[outLen];
	WideCharToMultiByte(CP_ACP, 0, in.c_str(), -1, outp, outLen, NULL, NULL);
	out = outp;
	delete []outp;

}

size_t 
WatchDog::GetHealthCheckInterval()
{
	return mHealthCheckInterval;
}

void 
WatchDog::StartIPoPProcesses()
{
	DWORD err = 0;
	STARTUPINFO si1, si2;
	memset(&si1, 0x00, sizeof(STARTUPINFO));
	memset(&si2, 0x00, sizeof(STARTUPINFO));
	si1.cb = sizeof(STARTUPINFO);
	si2.cb = sizeof(STARTUPINFO);
	try
	{
		wstring cmdline = L"ipop-tincan.exe";
		cout << "starting ipop...";
		BOOL rc = CreateProcess(NULL, const_cast<LPWSTR>(cmdline.c_str()), NULL, NULL, TRUE,
			CREATE_NO_WINDOW | INHERIT_PARENT_AFFINITY, NULL, ServiceCfg.GetIPoPPath().c_str(), &si1, &mTincanPI);
		if (!rc)
		{
			EventLogger.LogEvent(IPOP_NO_TINCAN);
			throw WINEXCEPT("Failed to create tincan process.");
		}
		//Task: wait until tincan starts
		//Tincan is a console app so it cannot WaitForInputIdle. 
		//Will have to modify tincan to sent an explicit wait event.
		//Or have controller retry it's connection attempt to Tincan
		Sleep(5000);	//undesireable

		cmdline.clear();
		cmdline.append(ServiceCfg.GetPythonPath()).append(L"python.exe");
		cmdline.append(L" ").append(ServiceCfg.GetIPoPPath()).append(L"\\gvpn_controller.py -c config.json");

		rc = CreateProcess(NULL, const_cast<LPWSTR>(cmdline.c_str()), NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE | INHERIT_PARENT_AFFINITY, NULL, ServiceCfg.GetIPoPPath().c_str(), &si2, &mCtrlerPI);
		if (!rc)
		{
			EventLogger.LogEvent(IPOP_NO_CNTRL);
			throw WINEXCEPT("Failed to create controller process.");
		}

		cout << "done" << endl;
	}
	catch (exception & e)
	{
		//UNREFERENCED_PARAMETER(e);
		cout << "failed" << endl;
		dbgfile << e.what() << endl;
	}
	return;
}

void WatchDog::TerminateIPoPProcesses()
{
	cout << "terminating ipop...";
	if (TerminateProcess(mTincanPI.hProcess, -1))
	{
		DWORD wait = WaitForSingleObject(mTincanPI.hProcess, 3000);
		switch (wait)
		{
		case WAIT_OBJECT_0:
			//success, terminate controller next
			break;
		case WAIT_TIMEOUT:
			throw WINEXCEPT("IPoP tincan failed to terminate within timeout");
		case WAIT_FAILED:
		default:
			break;
		}
	}
	CloseHandle(mTincanPI.hProcess);
	
	if (TerminateProcess(mCtrlerPI.hProcess, -1))
	{
		DWORD wait = WaitForSingleObject(mCtrlerPI.hProcess, 3000);
		switch (wait)
		{
		case WAIT_OBJECT_0:
			//success
			break;
		case WAIT_TIMEOUT:
			throw WINEXCEPT("IPoP controller failed to terminate within timeout");
		case WAIT_FAILED:
		default:
			break;
		}
	}
	CloseHandle(mCtrlerPI.hProcess);
	cout << "done" << endl;
}

void WatchDog::CheckControllerHealth(bool & Status)
{
	Status = false;
	try{
		string respmsg;
		ClientUDPSocket soc(mControllerAddress, mControllerPort, mAF);
		soc.SendMsg(ServiceCfg.GetControllerRequest());
		soc.ReceiveMsg(respmsg);

		if (string::npos != respmsg.find("\x2\x1{\"type\": \"echo_reply\"}"))
			Status = true;
	}
	catch (exception & e){
		dbgfile << e.what() << endl;
		//UNREFERENCED_PARAMETER(e);
		Status = false;
	}
	return;
}

void WatchDog::CheckTincanHealth(bool & Status)
{
	Status = false;
	try{
		string respmsg;
		ClientUDPSocket soc(mTincanAddress, mTincanPort, mAF);
		soc.SendMsg(ServiceCfg.GetTincanRequest());
		soc.ReceiveMsg(respmsg);
		if (string::npos != respmsg.find("\x2\x1{\n   \"_fpr\""))
			Status = true;
	}
	catch (exception & e){
		dbgfile << e.what() << endl;
		//UNREFERENCED_PARAMETER(e);
		Status = false;
	}
	return;
}

bool WatchDog::IsServiceStateGood()
{
	bool tcstatus = false, ctlstatus = false;
	CheckControllerHealth(ctlstatus);
	CheckTincanHealth(tcstatus);
		
	return ctlstatus && tcstatus;
}

/*
FUNCTION:CheckHealth
PURPOSE: Checks the existence and state of the Tincan and controller processes.
If either process fails the health check they are both terminated and restarted.
PARAMETERS: None
RETURN VALUE: None
THROWS: exception if processes cannot be successfully started
COMMENTS: The IPoP processes' state are evaluated via a UPD socket call to each process.
This routine aborts and terminates after THRESHOLD consecutive failed attempts.
*/
void WatchDog::CheckHealth()
{
	cout << "health check...";
	if (!IsServiceStateGood())
	{
		cout << "failed" << endl;
		if (mFailureCount >= ServiceCfg.GetMaxSvcStartAttempts())
		{
			EventLogger.LogEvent(IPOP_MAX_RETRIES);
			throw WINEXCEPT("The IPoP processes could not be started in a reliable state. The service will now exit.");
		}
		EventLogger.LogEvent(IPOP_RESTART_REQ);
		mFailureCount++;
		TerminateIPoPProcesses();
		StartIPoPProcesses();
	}
	else
	{
		cout << "OK" << endl;
		mFailureCount = 0;
	}
	return;
}

}
}