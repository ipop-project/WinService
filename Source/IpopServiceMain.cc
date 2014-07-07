#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include "WinException.h"
#include "SystemInfo.h"
#include "EventLog.h"
#include "ServiceConfig.h"
#include "IpopService.h"

using namespace std;

namespace ipop
{
namespace win
{
fstream dbgfile;// ("c:/temp/debug.txt", ios::in | ios::out | ios::trunc);
EventLog EventLogger(L"IPoPService");
SystemInfo SysInfo;
ServiceConfig ServiceCfg;
}
}

using namespace ipop::win;
static const wchar_t * const IpopServiceName = L"IPoPService";
static const wchar_t * const IpopServiceDisplayName = L"IPoP VPN Service";
static const wchar_t * const IpopServiceDescription = L"Provides an IP virtual private network (VPN) via a peer to peer overlay network.";


int wmain(int argc, wchar_t * argv[], wchar_t *envp[])
{
	IpopService * svc = NULL;
	int status = 0;
	try 
	{
		dbgfile.open("c:\\ipop_dbg.txt", ios::in | ios::out | ios::trunc); 
		SysInfo.DiscoverCapabilities();
		ServiceCfg.LoadServiceConfiguration();
		if (argc >= 2)
		{
			if (0 == (wstring(argv[1])).compare(wstring(L"--debug")) ||
				0 == (wstring(argv[1])).compare(wstring(L"-d"))){
				//run as console app
				svc = new IpopService(IpopServiceName, IpopServiceDisplayName, SERVICE_WIN32_OWN_PROCESS);
				svc->Debug();
				delete svc;
			}
			else if (0 == (wstring(argv[1])).compare(wstring(L"--install")) ||
				0 == (wstring(argv[1])).compare(wstring(L"-i"))){
				//install service
				unique_ptr<wchar_t[]> name = make_unique<wchar_t[]>(MAX_PATH*sizeof(wchar_t));
				if (GetModuleFileName(NULL, name.get(), MAX_PATH*sizeof(wchar_t)) == 0)
					throw WINEXCEPT("Failed GetModuleFilename");

				EventLog::AddEventSource(L"IPoPService", name.get());
				IpopService::Install(IpopServiceName, IpopServiceDisplayName, IpopServiceDescription);
				cout << "The IPoP Service was successfully installed" << endl;
			}
			else if (0 == (wstring(argv[1])).compare(wstring(L"--remove")) ||
				0 == (wstring(argv[1])).compare(wstring(L"-r"))){
				//remove the service
				IpopService::Remove(IpopServiceName);
				cout << "The IPoP Service was successfully removed" << endl;
			}
			else if (0 == (wstring(argv[1])).compare(L"/?") ||
				(0 == wstring(argv[1]).compare(L"--help")))
			{
				cout << "ipop_svc.exe --help\tto display this list" << endl
					<< "ipop_svc.exe --install\tto install the service" << endl
					<< "ipop_svc.exe --remove\tto remove the service" << endl
					<< "ipop_svc.exe --debug\tto run as a console app for debugging" << endl;
			}
			else { cout << "Try \"/\?\"" << endl; }
		}
		else
		{
			svc = new IpopService(IpopServiceName, IpopServiceDisplayName, SERVICE_WIN32_OWN_PROCESS);
			//start the service
			BEGIN_SERVICE_MAP
				SERVICE_MAP_ENTRY(IpopService, IPoPService)
			END_SERVICE_MAP
			//service ends
			delete svc;
		}
	}
	catch (exception& e)
	{
		status = -1;
		dbgfile << e.what() << endl;
	}

	dbgfile.close();
	return status;
}