#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <array>

#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib") // This line is for adding in the linker properties header 

#define DEFAULT_PORT 3504
#define DEFAULT_BUFLEN 512

using std::cout;
using std::endl;
using std::array;
using std::string;
using std::thread;

struct ClientInfo
{
	int id;
	SOCKET socket;
	sockaddr address;
	thread th;

	ClientInfo() : id(-1), socket(INVALID_SOCKET), address() {}

	void set(int i, SOCKET s, sockaddr a)
	{
		id = i;
		socket = s;
		address = a;
	}
};

// Function used for printing out the error message
void errNdie(const char* msg)
{
	cout << msg << endl;
	exit(1);
}

const int MAX_CLIENTS{ 10 };

void ProcessClient(int id, array<ClientInfo, MAX_CLIENTS>& clients)
{
	string msg{};
	char tempmsg[DEFAULT_BUFLEN]{ "" };
	ClientInfo& client = clients[id];

	// Client chat session
	while (true)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (client.socket != INVALID_SOCKET)
		{
			int iRes = recv(client.socket, tempmsg, DEFAULT_BUFLEN, 0);

			if (iRes != SOCKET_ERROR)
			{
				if (strcmp("", tempmsg) != 0)
				{
					msg = "Client #" + std::to_string(client.id) + ": " + tempmsg;
				}

				cout << msg << endl;

				for (ClientInfo& _client : clients)
				{
					if ((_client.socket != INVALID_SOCKET) && (_client.id != id))
					{
						send(_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
					}
				}
			}
			else // Socket is disconnected and socket is not alive anymore
			{
				msg = "Client #" + std::to_string(client.id) + "has disconnected";

				closesocket(client.socket);
				clients[client.id].socket = INVALID_SOCKET;

				cout << msg << endl;

				for (ClientInfo& _client : clients)
				{
					if ((_client.socket != INVALID_SOCKET) && (_client.id != id))
					{
						send(_client.socket, msg.c_str(), strlen(msg.c_str()), 0);
					}
				}

				break;
			}
		}
	}
}

// Remember to close any open sockets you open or create
int main()
{
	WSADATA wsadata;
	struct addrinfo hints;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		errNdie("WSAStartup failed!");
	}

	array<ClientInfo, MAX_CLIENTS> clients; // This will contain client connections

	// Let's create our listening server
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serverSocket == INVALID_SOCKET)
	{
		WSACleanup();
		errNdie("Failed to create socket!");
	}

	// Set socket options
	// SO_REUSEADDR allows to reuse the socket after 2 minutes of no connection
	// SOL_SOCKET stands for socket layer
	// Instruct the transport layer to not wait and always communicate - TCP_NODELAY
	const char OPTION_VALUE{ 1 };
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int));
	setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int));

	// Assign an address to our socket
	struct sockaddr_in servaddr;
	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DEFAULT_PORT);

	cout << "Now binding server socket" << endl;

	if (bind(serverSocket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		errNdie("Socket binding failed... Can't connect!");
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		errNdie("Failed to listen on server socket!");
	}

	sockaddr clientAddr;
	int clientAddLen{ sizeof(clientAddr) };

	while (true)
	{
		cout << "Server is listening!" << endl;
		ZeroMemory(&clientAddr, clientAddLen);

		// Waiting for new client
		SOCKET incoming = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddLen);

		if (incoming == INVALID_SOCKET)
		{
			cout << "Accept reported invalid socket: error code is: " << WSAGetLastError() << endl;
			cout << "We ignore this for now" << endl;
			continue;
		}

		cout << "Accepted a new client!" << endl;

		// Find the first clientinfo object with id == -1, to use for this new client
		int tempId{ -1 };
		for (size_t i = 0; i < clients.size(); i++)
		{
			if (clients[i].socket == INVALID_SOCKET)
			{
				tempId = i;
				break;
			}
		}

		// Extract IP address and port of the new client: deal with both IPv4 and IPv6
		char ipstr[INET6_ADDRSTRLEN];
		int clientPort{};

		if (clientAddr.sa_family == AF_INET)
		{
			struct sockaddr_in* s = (struct sockaddr_in*)&clientAddr;
			clientPort = ntohs(s->sin_port);
			inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
		}
		else // IPv6
		{
			struct sockaddr_in6* s = (struct sockaddr_in6*)&clientAddr;
			clientPort = ntohs(s->sin6_port);
			inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
		}

		cout << "Client's IP address : " << ipstr << ", Client's Port: " << clientPort << endl;

		if (tempId != -1)
		{
			// Send Id to client
			cout << "Client # " << tempId << "is accepted" << endl;
			string msg{ std::to_string(tempId) };

			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);

			// Add the client to the client info list
			clients[tempId].set(tempId, incoming, clientAddr);
			clients[tempId].th = thread(ProcessClient, tempId, std::ref(clients));
		}
		else
		{
			string msg = "Server is full";
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			closesocket(incoming);
			cout << msg << endl;
		}
	} // End of while loop - listening portion of the server

	// Free resources and close all client sockets
	for (ClientInfo& client : clients)
	{
		client.th.join();
		closesocket(client.socket);
	}

	// Close listening socket
	closesocket(serverSocket);
	WSACleanup();

	cout << "Program has ended successfully" << endl;

	return 0;
}
