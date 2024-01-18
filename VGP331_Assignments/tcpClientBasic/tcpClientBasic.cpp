//#undef UNICODE
// window includes needed for socket  programming
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#pragma warning(disable: 4996)
#pragma comment(lib, "Ws2_32.lib")  // needed to link with the proper winsock libray in windows 

#define Default_BuffLen 512
#define DefaultPort 27020

using namespace std;

int main(int argc, char** argv)
{
	// check to see if server ip address has been provided:
	if (argc < 2)
	{
		cout << "usage: " << argv[0] << " server-name\n";
		return 1;
	}

	// Convert port to u_short
	u_short port = static_cast<u_short>(strtoul(argv[2], nullptr, 0));

	// initialize winsock
	WSADATA wsadata;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error " << iResult << endl;
		return 1;
	}

	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		cout << "socket creation failed with error " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	struct sockaddr_in servaddr;
	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(port);

	if ((connect(connectSocket, (struct sockaddr*)&servaddr, sizeof(servaddr))) == SOCKET_ERROR)
	{
		cout << "connect failed for this socket with error " << WSAGetLastError() << ". Try another addrinfo\n";
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
		return 1;
	}

	string messageToServer{};
	bool wasTerminated = false;

	while(!wasTerminated)
	{
		getline(cin, messageToServer);
		iResult = send(connectSocket, messageToServer.c_str(), (int)messageToServer.length(), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Connection with server lost " << WSAGetLastError() << endl;
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		} 

		cout << "Successfully sent " << iResult << " bytes.\n";

		if(messageToServer == "close" || messageToServer.empty())
		{
			wasTerminated = true;
			break;
		}

		char recvbuf[Default_BuffLen];
		int recvbuflen{ Default_BuffLen };

		ZeroMemory(recvbuf, recvbuflen);
		iResult = recv(connectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			cout << "Server: " << iResult << ": " << recvbuf << endl;
		}
		else if (iResult == 0 || strupr(recvbuf) == "close")
		{
			cout << "Connection closed\n";
			wasTerminated = true;
			break;
		}
		else
		{
			cout << "recv failed with error " << WSAGetLastError() << endl;
		}
	}

	// clean up
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}
