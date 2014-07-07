#include <memory>
#include "ClientUDPSocket.h"
#include "WinException.h"
#include "ServiceConfig.h"
using namespace std;

namespace ipop
{
namespace win
{

extern ServiceConfig ServiceCfg;

ClientUDPSocket::ClientUDPSocket(
	const string & IPAddress,
	short Port,
	ADDRESS_FAMILY AF) :
	mSock(INVALID_SOCKET)
{
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		throw WINEXCEPT("Could not initialize Winsock");
	}


	if (AF == AF_INET6)
	{
		memset((char *)&mServerAddr6, 0, sizeof(mServerAddr6));
		mServerAddr6.sin6_family = AF;
		mServerAddr6.sin6_port = htons(Port);
		/* the server IP address, in network byte order */
		int rc = InetPtonA(AF_INET6, IPAddress.c_str(), &mServerAddr6.sin6_addr);
		mSocAddrLen = sizeof(mServerAddr6);
		mRef = (struct sockaddr *) &mServerAddr6;
	}
	else if (AF == AF_INET)
	{
		memset((char *)&mServerAddr4, 0, sizeof(mServerAddr4));
		mServerAddr4.sin_family = AF;
		mServerAddr4.sin_port = htons(Port);
		mServerAddr4.sin_addr.S_un.S_addr = inet_addr(IPAddress.c_str());
		mSocAddrLen = sizeof(mServerAddr4);
		mRef = (struct sockaddr *) &mServerAddr4;
	}
			 
	else
		throw WINEXCEPT("No UDP protocol capability identified");

			
	// Create a SOCKET for connecting to server
	if ((mSock = socket(AF, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		WSACleanup();
		throw WINEXCEPT("UDP socket failed");
	}
}


ClientUDPSocket::~ClientUDPSocket()
{
	closesocket(mSock);
	WSACleanup();
}

void ClientUDPSocket::SendMsg(const string & Message)
{
	if (SOCKET_ERROR == sendto(mSock, Message.c_str(), Message.length(), 0, mRef, mSocAddrLen))
	{
		throw WINEXCEPT("Failed to send UDP message");
	}
	return;
}

void ClientUDPSocket::ReceiveMsg(string & Message)
{
	unique_ptr<char[]> msgbuf = make_unique<char[]>(ServiceCfg.GetMaxStateMsgLen());
	int rc = recvfrom(mSock, msgbuf.get(), ServiceCfg.GetMaxStateMsgLen(), 0, mRef, &mSocAddrLen);
	if (SOCKET_ERROR == rc)
	{
		throw WINEXCEPT("Failed to receive UDP message");
	}

	Message.assign(msgbuf.get(), rc);
	return;
}

}
}