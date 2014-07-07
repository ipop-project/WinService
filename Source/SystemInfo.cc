#include "SystemInfo.h"
#include <memory>
#include "WinException.h"
using namespace std;

namespace ipop
{
namespace win
{


SystemInfo::SystemInfo() :
	WOWProc(false),
	INET4(false),
	INET6(false),
	PROTO_UDP(false),
	Win64(false)
{}

SystemInfo::~SystemInfo()
{}

int SystemInfo::DiscoverCapabilities()
{
	try
	{
		QueryWOW64();
		QueryNetProtocolFacility();
	}
	catch (exception & e)
	{
		UNREFERENCED_PARAMETER(e);
	}
	return 0;
}

bool SystemInfo::IsInet4Capable() const
{
	return INET4;
}

bool SystemInfo::IsInet6Capable() const
{
	return INET6;
}

bool SystemInfo::IsUDPCapable() const
{
	return PROTO_UDP;
}

bool SystemInfo::IsWin64() const
{
#if defined(_WIN64)
	return TRUE;
#elif defined(_WIN32)
	return IsWOW64Process();
#else
	return false;
#endif
}

bool SystemInfo::IsWOW64Process() const
{
	return WOWProc;
}

void SystemInfo::QueryWOW64()
{
	BOOL f64 = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	HMODULE mh = GetModuleHandle(L"kernel32");
	if (mh)
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(mh, "IsWow64Process");
	
	if (fnIsWow64Process)
		WOWProc = fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
	
	return;
}

void SystemInfo::QueryNetProtocolFacility()
{
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		throw WINEXCEPT("Could not initialize Winsock");
	}

	unique_ptr<int[]> prot = make_unique<int[]>(2);
	prot[0] = IPPROTO_UDP;
	prot[1] = 0;

	DWORD bufLen = sizeof(WSAPROTOCOL_INFO)* 8;
	unique_ptr<WSAPROTOCOL_INFO[]>protoInfo = make_unique<WSAPROTOCOL_INFO[]>(8);

	int numinfo = WSAEnumProtocols(prot.get(), protoInfo.get(), &bufLen);
	if (numinfo == SOCKET_ERROR)
	{
		WSACleanup();
		throw WINEXCEPT("Could not enumerate network capability");
	}
	else
	{
		for (int i = 0; i < numinfo; i++)
		{
			if (protoInfo[i].iAddressFamily == AF_INET)
				INET4 = PROTO_UDP = true;
			if (protoInfo[i].iAddressFamily == AF_INET6)
				INET6 = PROTO_UDP = true;
		}
	}
	WSACleanup();
}

}
}