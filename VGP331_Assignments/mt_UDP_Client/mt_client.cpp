#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>

#pragma warning(disable: 4996)
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024

struct ServerInfo
{
	SOCKET socket;
	int id;
	char receivedMsg[DEFAULT_BUFLEN];
	int size;
};

using namespace std;

void processReceived(ServerInfo& server, sockaddr_in const& serverAddr)
{
	int errorCount{ 3 };

	while (true)
	{
		memset(server.receivedMsg, 0, DEFAULT_BUFLEN);

		if (recvfrom(server.socket, server.receivedMsg, strlen(server.receivedMsg), 0, (struct sockaddr*)&serverAddr, &server.size) == SOCKET_ERROR)
		{
			cout << "recv() failed: error = " << WSAGetLastError() << endl;
			break;
		}
		else
		{
			string receivedMessage{ server.receivedMsg };
			string checker{};
			string message{};
			string delimiter = ">=";
			size_t pos_start{ 0 }, pos_end, delim_len = delimiter.length();

			while ((pos_end = checker.find(delimiter, pos_start)) != string::npos)
			{
				checker = checker.substr(pos_start, pos_end - pos_start);
				pos_start = pos_end + delim_len;
				message = checker.substr((pos_start));
			}

			while (true)
			{
				if (sendto(server.socket, checker.c_str(), strlen(checker.c_str()), 0, (struct sockaddr*)&serverAddr, server.size) == SOCKET_ERROR)
				{
					cout << "send() failed " << WSAGetLastError() << endl;
					break;
				}

				if (recvfrom(server.socket, server.receivedMsg, strlen(server.receivedMsg), 0, (struct sockaddr*)&serverAddr, &server.size) == SOCKET_ERROR)
				{
					cout << "receive failed. Connection lost. Closing... " << WSAGetLastError() << endl;
					break;
				}

				if (strcmp("confirmed", server.receivedMsg) == 0)
				{
					cout << "Received message: " << message << endl;
					break;
				}
				else
				{
					++errorCount;
				}

				if (errorCount >= 3)
				{
					cout << "Loss of data, will try to connect to server again" << endl;
				}
			}
		}
	} // while 

	if (WSAGetLastError() == WSAECONNRESET)
		cout << "The server has disconnected\n";
}

void errNdie(const char* msg)
{
	std::cout << msg << std::endl;
	exit(1);
}


int main(int argc, char** argv)
{
	if (argc < 3)
		errNdie("Command line error: Usage: >HttpClient.exe serverAddress Port");
	char* address = argv[1];

	u_short port = static_cast<u_short>(strtoul(argv[2], nullptr, 0));

	WSADATA wsadata;

	// initializing winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		errNdie("error starting socket engine\n");

	cout << "Connecting to server....\n";

	//resolving the server address and port:
	ServerInfo server{ INVALID_SOCKET, -1, "" };

	//create socket to connect to server:
	server.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server.socket == INVALID_SOCKET)
		errNdie("error creating socket \n");

	DWORD nonBlocking{ 1 };
	if (ioctlsocket(server.socket, FIONBIO, &nonBlocking) != 0)
	{
		cout << "ioctlsocket failed with error " << WSAGetLastError() << endl;
		closesocket(server.socket);
		WSACleanup();
		return 1;
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(port);
	int slen{ sizeof(serverAddress) };
	server.size = slen;

	/*if (bind(server.socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		closesocket(server.socket);
		WSACleanup();
		errNdie("cannot connect");
	}*/

	string initialMessage{ "Hello" };
	if (sendto(server.socket, initialMessage.c_str(), strlen(initialMessage.c_str()), 0, (struct sockaddr*)&serverAddress, slen) == SOCKET_ERROR)
	{
		cout << "send() failed " << WSAGetLastError() << endl;
		closesocket(server.socket);
		WSACleanup();
		return -1;
	}

	cout << "Successfully connected\n";

	while (true)
	{
		if (recvfrom(server.socket, server.receivedMsg, strlen(server.receivedMsg), 0, (struct sockaddr*)&serverAddress, &slen) == SOCKET_ERROR)
		{
			Sleep(100);
			continue;
		}

		break;
	}
	// obtain id from server:

	string receivedMessage{ server.receivedMsg };
	string checker{};
	string message{};
	string delimiter = ">=";
	size_t pos_start{ 0 }, pos_end, delim_len = delimiter.length();

	while ((pos_end = checker.find(delimiter, pos_start)) != string::npos)
	{
		checker = checker.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		message = checker.substr((pos_start));
	}

	if (sendto(server.socket, initialMessage.c_str(), strlen(initialMessage.c_str()), 0, (struct sockaddr*)&serverAddress, slen) == SOCKET_ERROR)
	{
		cout << "send() failed " << WSAGetLastError() << endl;
		closesocket(server.socket);
		WSACleanup();
		return -1;
	}

	while (true)
	{
		if (recvfrom(server.socket, server.receivedMsg, strlen(server.receivedMsg), 0, (struct sockaddr*)&serverAddress, &slen) == SOCKET_ERROR)
		{
			Sleep(100);
			continue;
		}

		break;
	}

	if (message != "Server is full" || message == "confirmed")
	{
		server.id = atoi(message.c_str());
		std::thread receiveThread = std::thread(processReceived, std::ref(server), std::ref(serverAddress));

		// main thread takes care of sending:
		string msg2Send{ "" };
		while (true)
		{
			getline(cin, msg2Send);
			if (sendto(server.socket, initialMessage.c_str(), strlen(initialMessage.c_str()), 0, (struct sockaddr*)&serverAddress, slen) == SOCKET_ERROR)
			{
				cout << "send() failed " << WSAGetLastError() << endl;
				break;
			}
		}

		receiveThread.detach();
	}
	else
		cout << server.receivedMsg << endl;

	cout << "Shutting down socket..." << endl;
	shutdown(server.socket, SD_SEND);

	closesocket(server.socket);
	WSACleanup();
	return 0;
}