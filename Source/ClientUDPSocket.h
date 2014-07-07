/**********************************************************************************
						ClientUDPSocket
					------------------------
	description			: Wapper class for Win32 IP v4 & v6 UDP communications. 
	
***********************************************************************************/  

#pragma once
#include <string>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
using namespace std;

namespace ipop
{
	namespace win
	{

		class ClientUDPSocket
		{
		public:
			ClientUDPSocket(
				const string & IPAddress,
				short Port,
				ADDRESS_FAMILY AF
				);

			~ClientUDPSocket(
				);

			void SendMsg(
				const string & Message
				);

			void ReceiveMsg(
				string & Message
				);

			static void QueryNetProtocolFacility(
				);
				
		private:
			union {
				SOCKADDR_IN mServerAddr4;
				SOCKADDR_IN6 mServerAddr6;
			};
			int mSocAddrLen;
			struct sockaddr * mRef;
			SOCKET mSock;
		};

	}
}