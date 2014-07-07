/**************************************************************************
						ServiceConfig
					-------------------
	description			: Discovers relevant system capabilties. 
	
***************************************************************************/  

#pragma once

#include <ws2tcpip.h>
#include <ws2ipdef.h>
namespace ipop
{
	namespace win
	{

		class SystemInfo
		{
		public:
			SystemInfo();
			~SystemInfo();
			int DiscoverCapabilities();
			bool IsInet4Capable() const;
			bool IsInet6Capable() const;
			bool IsUDPCapable() const;
			bool IsWin64() const;
			bool IsWOW64Process() const;
		private:
			void QueryNetProtocolFacility();
			void QueryWOW64();
			bool INET4;
			bool INET6;
			bool PROTO_UDP;
			bool Win64;
			bool WOWProc;
			typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
		};

	}
}