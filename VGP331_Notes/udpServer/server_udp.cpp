

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")  // needed to link with the proper winsock libray in windows 


#define BuffLen 512
//#define Default_Port 27018

using namespace std;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "usage: " << argv[0] << " server-port\n";
		return 1;
	}
	
	u_short port = static_cast<u_short>(strtoul(argv[1], nullptr, 0));
	
	// initialize winsock
	WSADATA wsadata;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error " << iResult << endl;
		return 1;
	}

	cout << "WSAStartup successful\n";

	SOCKET commSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (commSocket == INVALID_SOCKET)
	{
		cout << "listenning socket creation failed with error " << WSAGetLastError();
		WSACleanup();
		return 1;
	}
	cout << "Socket created\n";

	//setting non blocking property on the socket:
	DWORD nonBlocking{ 1 };
	if (ioctlsocket(commSocket, FIONBIO, &nonBlocking) != 0)
	{
		cout << "ioctlsocket failed with error " << WSAGetLastError() << endl;
		closesocket(commSocket);
		WSACleanup();
		return 1;
	}

	struct sockaddr_in servAddr {};
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(port);

	if (bind(commSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		cout << "bind failed with error " << WSAGetLastError() << endl;
		closesocket(commSocket);
		WSACleanup();
		return 1;
	}
	cout << "Binding done\n";

	// start listening for data 
	struct sockaddr_in clientAddr {};
	int slen{ sizeof(clientAddr) };
	while (true)
	{
		fflush(stdout);

		int recvLen{ 0 };
		char buf[BuffLen]{};
		//memset(buf, '\0', BuffLen); // not necessary?

		if ((recvLen = recvfrom(commSocket, buf, BuffLen, 0, (struct sockaddr*)&clientAddr, &slen)) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSAEWOULDBLOCK)
				cout << "Error recvfrom with error " << WSAGetLastError() << endl;
			Sleep(100);
			continue;
		}
		cout << "Data Received  from ip=" << inet_ntoa(clientAddr.sin_addr) << ", port=" <<
			ntohs(clientAddr.sin_port) << endl;
		cout << buf << endl;
		
		cout << "Echo back the message to the client:\n";
		//use sendto
		if (sendto(commSocket, buf, recvLen, 0, (struct sockaddr*)&clientAddr, slen) == SOCKET_ERROR)
		{
			cout << "sendto failed with error code " << WSAGetLastError() << endl;
			closesocket(commSocket);
			WSACleanup();
			return 1;
		}
	}

	closesocket(commSocket);
	WSACleanup();
	return 0;
}