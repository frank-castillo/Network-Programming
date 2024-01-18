#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

#define DEFAULT_BUFLEN 512

struct ServerInfo
{
	SOCKET socket;
	int id;
	char receivedMsg[DEFAULT_BUFLEN];
};

using namespace std;

void processReceives(ServerInfo& server)
{
	while (true)
	{
		memset(server.receivedMsg, 0, DEFAULT_BUFLEN);
		if (server.socket != INVALID_SOCKET)
		{
			int iResult = recv(server.socket, server.receivedMsg, DEFAULT_BUFLEN, 0);
			if (iResult != SOCKET_ERROR)
				cout << server.receivedMsg << endl;
			else
			{
				cout << "recv() failed: error = " << WSAGetLastError() << endl;
				break;
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
	if (argc < 2)
		errNdie("Command line error: Usage: >HttpClient.exe serverAddress");
	char* address = argv[1];

	WSADATA wsadata;
	struct addrinfo* result = nullptr;
	struct addrinfo* ptr = nullptr;

	// initializing winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		errNdie("error starting socket engine\n");

	cout << "Connecting to server....\n";

	//resolving the server address and port:
	ServerInfo server{ INVALID_SOCKET, -1, "" };

	//create socket to connect to server:
	server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server.socket == INVALID_SOCKET)
		errNdie("error creating socket \n");

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(3504);
	if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0)
		errNdie("inet_pton error\n");

	// try to connect to server:
	if (connect(server.socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
	{
		closesocket(server.socket);
		server.socket = INVALID_SOCKET;
		cout << "unable to connect to server\n";
		WSACleanup();
		return -1;
	}

	//setup interactive mode for the socket:
	const char OPTION_VALUE{ 1 };
	setsockopt(server.socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int));

	cout << "Successfully connected\n";

	// obtain id from server:
	recv(server.socket, server.receivedMsg, DEFAULT_BUFLEN, 0);
	string message{ server.receivedMsg };
	if (message != "Server is full")
	{
		server.id = atoi(server.receivedMsg);
		std::thread receiveThread = std::thread(processReceives, std::ref(server));
		// main thread takes care of sending:
		string msg2Send{ "" };
		while (true)
		{
			getline(cin, msg2Send);
			if (send(server.socket, msg2Send.c_str(), strlen(msg2Send.c_str()), 0) < 0)
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