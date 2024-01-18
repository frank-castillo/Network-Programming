#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <array>

#pragma comment(lib, "Ws2_32.lib")

#define Default_port 3504
#define Default_BUFLEN 512

using std::array;
using std::cout;
using std::endl;
using std::string;
using std::thread;

string checker = "13342>=";

struct ClientInfo
{
	thread th;
	struct sockaddr sockAdrr {};

	int id{ -1 };
	int slen{};
	char* ip{};
	int clientPort{};

	ClientInfo() : id(-1), slen(-1), clientPort(INT_MIN)
	{
		ZeroMemory(&sockAdrr, sizeof(sockAdrr));
		ZeroMemory(&ip, 0);
	}

	void Set(struct sockaddr sock, int sLen, char* IP, int port, int ID = -1)
	{
		sockAdrr = sock;
		slen = sLen;
		ip = IP;
		clientPort = port;
		id = ID;
	}
};

void errNdie(const char* msg)
{
	std::cout << msg << std::endl;
	exit(1);
}

const int MAX_CLIENTS{ 10 };

void process_client(int id, std::array<ClientInfo, MAX_CLIENTS>& clients, SOCKET const& serverSocket)
{
	ClientInfo& client = clients[id];
	string msg{};
	int recvLen{ 0 };
	char tmpmsg[Default_BUFLEN]{ "" };

	// client chat session
	while (true)
	{
		memset(tmpmsg, 0, Default_BUFLEN);
		if (client.id != -1)
		{
			recvLen = recvfrom(serverSocket, tmpmsg, Default_BUFLEN, 0, (struct sockaddr*)&client.sockAdrr, &client.clientPort);

			if (recvLen != SOCKET_ERROR)
			{
				if (strcmp("", tmpmsg) != 0)
				{
					msg = checker + "Client #" + std::to_string(client.id) + ": " + tmpmsg;
					std::cout << msg << std::endl;
				}
				else if (strcmp("", tmpmsg) == 0 || strcmp("close", tmpmsg))
				{
					msg = checker + "Client #" + std::to_string(client.id) + " Disconnected";
					std::cout << msg << std::endl;
					ZeroMemory(&clients[id], sizeof(ClientInfo));
					clients[id].id = -1;

					// broadcast the disconnection message to the other clients
					for (int i = 0; i < MAX_CLIENTS; ++i)
					{
						if ((clients[i].id != -1))
							sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&clients[i].sockAdrr, clients[i].slen);
					}

					break; // stop chat session for this client
				}

				//broadcast the msg to the other clients:
				for (int i = 0; i < clients.size(); ++i)
				{
					if (clients[i].id != -1 && client.id != i)
						sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&clients[i].sockAdrr, clients[i].slen);
				}
			}
			else
			{
				msg = checker + "Client #" + std::to_string(client.id) + " Disconnected";
				std::cout << msg << std::endl;
				ZeroMemory(&clients[id], sizeof(ClientInfo));
				clients[id].id = -1;

				// broadcast the disconnection message to the other clients
				for (int i = 0; i < MAX_CLIENTS; ++i)
				{
					if ((clients[i].id != -1))
						sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&clients[i].sockAdrr, clients[i].slen);
				}

				break; // stop chat session for this client
			}
		}

		// Check for reception confirmation
		for (int i = 0; i < MAX_CLIENTS; ++i)
		{
			if ((clients[i].id != -1))
			{
				recvLen = recvfrom(serverSocket, tmpmsg, Default_BUFLEN, 0, (struct sockaddr*)&clients[i].sockAdrr, &clients[i].clientPort);

				if (strcmp(tmpmsg, "13342") == 0)
				{
					string confimration{ "confirmed" };
					sendto(serverSocket, confimration.c_str(), strlen(confimration.c_str()), 0, (struct sockaddr*)&clients[i].sockAdrr, clients[i].slen);
				}
			}
		}
	}
}

int main()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		errNdie("WSAStartup failed!");
	}

	// let's create our listening server
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET)
		errNdie("socket creation failed!!");

	DWORD nonBlocking{ 1 };
	if (ioctlsocket(serverSocket, FIONBIO, &nonBlocking) != 0)
	{
		cout << "ioctlsocket failed with error " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// assign address to our listening socket
	struct sockaddr_in servaddr;
	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(Default_port);

	std::cout << "Now binding server socket ...\n";
	if (bind(serverSocket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		closesocket(serverSocket);
		WSACleanup();
		errNdie("cannot connect");
	}

	std::array<ClientInfo, MAX_CLIENTS> clients; // will contain client connections

	sockaddr clientAddr;
	int clientAddLen{ sizeof(clientAddr) };
	std::cout << "Server is listening...\n";

	while (true)
	{
		
		ZeroMemory(&clientAddr, sizeof(clientAddr));
		int recvLen{ 0 };
		char buf[Default_BUFLEN];

		if ((recvLen = recvfrom(serverSocket, buf, Default_BUFLEN, 0, (struct sockaddr*)&clientAddr, &clientAddLen)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				cout << "Error recvfrom with error " << WSAGetLastError() << endl;
			Sleep(100);
			continue;
		}

		std::cout << "Accepted a  client\n";

		// find the first clientinfo object with id == -1, to use for this new client.
		int tmp_id{ -1 };
		for (int i = 0; i < clients.size(); ++i)
		{
			if (clients[i].id == -1)
			{
				tmp_id = i;
				break;
			}
		}

		// extract ip address and port of the new client: deal with both IPV4 and IPV6
		char ipstr[INET6_ADDRSTRLEN];
		int clientport{};
		if (clientAddr.sa_family == AF_INET)
		{
			struct sockaddr_in* s = (struct sockaddr_in*)&clientAddr;
			clientport = ntohs(s->sin_port);
			inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
		}
		else // means ipv6
		{
			struct sockaddr_in6* s = (struct sockaddr_in6*)&clientAddr;
			clientport = ntohs(s->sin6_port);
			inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
		}

		std::cout << "Client ip address: " << ipstr << ", client port: " << clientport << std::endl;

		if (tmp_id != -1)
		{
			ClientInfo newClient{};
			newClient.Set((struct sockaddr)clientAddr, clientAddLen, ipstr, clientport, tmp_id);

			// send the id to the client:
			std::string id{ std::to_string(tmp_id) };
			std::string msg{ checker + id };
			int counter = 3;
			char tmpmsg[Default_BUFLEN]{ "" };

			// First check from sending ID and making sure a connection still exists
			while (true || counter > 0)
			{
				sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&newClient.sockAdrr, newClient.slen);

				if ((recvLen = recvfrom(serverSocket, tmpmsg, Default_BUFLEN, 0, (struct sockaddr*)&newClient.sockAdrr, &newClient.clientPort)) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
						cout << "Error recvfrom with error " << WSAGetLastError() << endl;
					Sleep(100);
					continue;
				}

				if (strcmp(tmpmsg, "13342") == 0)
				{
					msg = "confirmed";
					int times = 5;
					while (times > 0)
					{
						sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&newClient.sockAdrr, newClient.slen);
						--times;
					}
					// We break cause we just confirmed the id arrived and client sent header back to us
					continue;
				}

				Sleep(1000);
				--counter;
			}

			// add the client to the client info list:
			clients[tmp_id].Set((struct sockaddr)clientAddr, clientAddLen, ipstr, clientport, tmp_id);
			clients[tmp_id].th = std::thread(process_client, tmp_id, std::ref(clients), std::ref(serverSocket));
		}
		else
		{
			std::string msg("Server is full. Try connecting later");
			int counter = 3;
			char tmpmsg[Default_BUFLEN]{ "" };

			// First check from sending ID and making sure a connection still exists
			while (true || counter > 0)
			{
				sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&clientAddr, clientAddLen);

				if ((recvLen = recvfrom(serverSocket, tmpmsg, Default_BUFLEN, 0, (struct sockaddr*)&clientAddr, &clientAddLen)) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
						cout << "Error recvfrom with error " << WSAGetLastError() << endl;
					Sleep(100);
					continue;
				}

				if (strcmp(tmpmsg, "13342") == 0)
				{
					msg = "confirmed";
					int times = 5;
					while (times > 0)
					{
						sendto(serverSocket, msg.c_str(), strlen(msg.c_str()), 0, (struct sockaddr*)&clientAddr, clientAddLen);
						--times;
					}
					// We break cause we just confirmed the id arrived and client sent header back to us
					continue;
				}

				Sleep(1000);
				--counter;
			}
		}
	}  // end of while 

	// close all client sockets
	for (int i = 0; i < clients.size(); ++i)
	{
		clients[i].th.join();
	}

	// closing listening socket
	closesocket(serverSocket);
	WSACleanup();

	return 0;
}


