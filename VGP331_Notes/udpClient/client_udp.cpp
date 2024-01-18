#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")  // needed to link with the proper winsock libray in windows 

#define BuffLen 512
#define Default_Port 27018

using namespace std;

int main(int argc, char** argv)
{
	// check to see if server ip address has been provided:
	if (argc < 3)
	{
		cout << "usage: " << argv[0] << " server-address server-port\n";
		return 1;
	}
	
	u_short port = static_cast<u_short>(strtoul(argv[2], nullptr, 0));
	
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
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(port);
	int slen{ sizeof(servAddr) };

	//start communicating:
	while (1)
	{
		fflush(stdout);

		char message[BuffLen]{};
		cout << "Enter a message: ";
		cin.getline(message, BuffLen);

		// send the message:
		if (sendto(commSocket, message, strlen(message), 0, (struct sockaddr*)&servAddr, slen) == SOCKET_ERROR)
		{
			cout << "failed sendto with error " << WSAGetLastError() << endl;
			closesocket(commSocket);
			WSACleanup();
			return 1;
		}

		while(1){
			char buf[BuffLen]{};
			if (recvfrom(commSocket, buf, BuffLen, 0, (struct sockaddr*)&servAddr, &slen) == SOCKET_ERROR)
			{
				if(WSAGetLastError() != WSAEWOULDBLOCK)
					cout << "failed recvfrom with error " << WSAGetLastError() << endl;
				Sleep(100);
				continue; 
			}
			else
			{
				cout << "Received message: " << buf << endl;
				break;
			}
		}
	}

	closesocket(commSocket);
	WSACleanup();

	return 0;
}
