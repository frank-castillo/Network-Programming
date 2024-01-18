//#undef UNICODE
// window includes needed for socket  programming
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>
#include <string>
#include "cstring"

#pragma warning(disable: 4996)
#pragma comment(lib, "Ws2_32.lib")  // needed to link with the proper winsock libray in windows 

#define DefaultPort 27500  // default listening port for the server
#define DefaultBuffLen 512

int main()
{
	// initialize winsock
	WSADATA wsadata;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error " << iResult << std::endl;
		return 1;
	}
	
	//create a listening socket directly.
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listenSocket == INVALID_SOCKET)
	{
		std::cout<<"listening socket creation failed with error " << WSAGetLastError()<<std::endl;
		WSACleanup();
		return 1;
	}
		
	struct sockaddr_in servaddr;
	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DefaultPort);
	
	// bind the listenSocket to the address sockaddr_in
	if((bind(listenSocket, (struct sockaddr*)&servaddr, sizeof(servaddr)))< 0)
	{
		std::cout << "bind failed with error " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	
	if((listen(listenSocket, SOMAXCONN))<0)
	{
		std::cout << "listen failed with error " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	
	std::cout << "Start waiting for connection requests to arrive:\n";
	SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed with error " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Connection with client successful, chat is now available" << std::endl;

	std::string message{};
	bool wasTerminated = false;

	// Make so any side can close the server
	while(!wasTerminated)
	{
		// a connection is established. Start communicating:\n";
		char recvbuf[DefaultBuffLen];
		int recvbuflen = DefaultBuffLen;
		ZeroMemory(recvbuf, recvbuflen);

		int receivedDataSize = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (receivedDataSize >= 0)
		{
			std::cout << recvbuf << std::endl;

			if(_strupr(recvbuf) == "CLOSE" || receivedDataSize == 0)
			{
				std::cout << "Connection terminated by server" << std::endl;
				wasTerminated = true;
				break;
			}

			getline(std::cin, message);

			// Echo back the message to the sender:
			int iSendResult = send(clientSocket, message.c_str(), (int)message.length(), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				std::cout << "send failed  with error " << WSAGetLastError() << std::endl;
				closesocket(clientSocket);
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}

			if( message == "close" || message.empty())
			{
				wasTerminated = true;
				break;
			}
		}
	}
	
	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
	closesocket(listenSocket);
	WSACleanup();
	
	return 0;
	
}