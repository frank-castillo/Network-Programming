#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>
#include <array>

#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")  // needed to link with the proper winsock libray in windows 

#define BuffLen 512
#define Default_Port 27018

const int MAX_CLIENTS{ 10 };
int currentClient{ 0 };

using namespace std;

struct ClientInfo
{
	string name{ "empty" };
	int id{ -1 };
	struct sockaddr_in sockAdrr {};
	int slen{};

	char* ip{};
	int clientPort{};

	void Set(string sName, struct sockaddr_in sock, int sLen, char* IP, int port, int ID = -1)
	{
		name = sName;
		sockAdrr = sock;
		slen = sLen;
		ip = IP;
		clientPort = port;
		id = ID;
	}
};

string SetUserName(struct sockaddr_in& clientAddr, int slen, SOCKET& commSocket)
{
	string mes = "Please input a username";
	char ans[BuffLen]{};
	//memset(ans, '\0', BuffLen);

	if (sendto(commSocket, mes.c_str(), (int)mes.length(), 0, (struct sockaddr*)&clientAddr, slen) == SOCKET_ERROR)
	{
		cout << "sendto failed with error code " << WSAGetLastError() << endl;
		Sleep(100);
		return string{};
	}

	if ((recvfrom(commSocket, ans, BuffLen, 0, (struct sockaddr*)&clientAddr, &slen)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			cout << "Error recvfrom with error " << WSAGetLastError() << endl;
		Sleep(100);
		return string{};
	}

	string returnName = ans;

	return returnName;
}

int main()
{
	// initialize
	WSADATA wsadata;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error " << iResult << endl;
		return 1;
	}

	cout << "WSAStartup successful\n";

	array<ClientInfo, MAX_CLIENTS> activeClients;

	SOCKET commSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (commSocket == INVALID_SOCKET)
	{
		cout << "listening socket creation failed with error " << WSAGetLastError();
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
	servAddr.sin_port = htons(Default_Port);

	if (bind(commSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
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
		//memset(buf, '\0', BuffLen);
		string generalMessage{};

		if ((recvLen = recvfrom(commSocket, buf, BuffLen, 0, (struct sockaddr*)&clientAddr, &slen)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				cout << "Error recvfrom with error " << WSAGetLastError() << endl;
			Sleep(100);
			continue;
		}

		cout << "Data Received  from ip=" << inet_ntoa(clientAddr.sin_addr) << ", port=" << ntohs(clientAddr.sin_port) << endl;
		cout << buf << endl;

		if (strcmp(buf, "bye bye") == 0)
		{
			ClientInfo tempClient;
			tempClient.Set(string{}, clientAddr, slen, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			for (size_t i = 0; i < activeClients.size(); i++)
			{
				if ((activeClients[i].ip == tempClient.ip) && (activeClients[i].clientPort == tempClient.clientPort))
				{
					char byeMessage[BuffLen] = "bye";

					if (sendto(commSocket, byeMessage, BuffLen, 0, (struct sockaddr*)&activeClients[i].sockAdrr, activeClients[i].slen) == SOCKET_ERROR)
					{
						cout << "sendto failed with error code " << WSAGetLastError() << endl;
						cout << "Let client know bye section" << endl;
						Sleep(100);
					}

					string generalByeMessage = "User: " + activeClients[i].name + " has left the chat.";

					for (ClientInfo& _client : activeClients)
					{
						if (sendto(commSocket, byeMessage, BuffLen, 0, (struct sockaddr*)&_client.sockAdrr, _client.slen) == SOCKET_ERROR)
						{
							cout << "sendto failed with error code " << WSAGetLastError() << endl;
							cout << "Close connection section to let all know" << endl;
							Sleep(100);
							continue;
						}
					}

					ZeroMemory(&activeClients[i], sizeof(ClientInfo));
					activeClients[i].id = -1;
					break;
				}
			}
		}

		ClientInfo newClient;
		ClientInfo currentClient;
		bool userExists = false;

		newClient.Set(string{}, (struct sockaddr_in)clientAddr, slen, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		for (ClientInfo& _client : activeClients)
		{
			if (_client.ip == newClient.ip && _client.clientPort == newClient.clientPort)
			{
				userExists = true;
				currentClient = _client;
				break;
			}
		}

		if (!userExists)
		{
			string ans = buf;
			string message{};
			bool allNamesChecked = false;

			while (!allNamesChecked)
			{
				//ans = SetUserName(clientAddr, slen, commSocket);

				for (auto it = activeClients.begin(); it < activeClients.end(); ++it)
				{
					if (strcmp(ans.c_str(), it->name.c_str()) == 0)
					{
						message = "Name already taken or not valid, please choose another one";

						if (sendto(commSocket, message.c_str(), (int)message.length(), 0, (struct sockaddr*)&clientAddr, slen) == SOCKET_ERROR)
						{
							cout << "sendto failed with error code " << WSAGetLastError() << endl;
							cout << "Name already taken section" << endl;
							Sleep(100);
							break;
						}

						break;
					}
				}

				allNamesChecked = true;
			}

			newClient.name = ans;

			int tempId{ -1 };
			for (size_t i = 0; i < activeClients.size(); i++)
			{
				if (activeClients[i].id == -1)
				{
					tempId = i;
					break;
				}
			}

			if (tempId == -1)
			{
				message = "Server full. Try connecting at another time";
				if (sendto(commSocket, message.c_str(), (int)message.length(), 0, (struct sockaddr*)&clientAddr, slen) == SOCKET_ERROR)
				{
					cout << "sendto failed with error code " << WSAGetLastError() << endl;
					cout << "Server is full" << endl;
					Sleep(100);
					continue;
				}
			}

			newClient.id = tempId;
			activeClients[tempId] = newClient;
			currentClient = newClient;

			generalMessage = "Welcome to the chat! In order to message someone directly, write your message and at the end with no spaces, add the delimiters >= followed by their username. ";

			if (sendto(commSocket, generalMessage.c_str(), (int)generalMessage.length(), 0, (struct sockaddr*)&newClient.sockAdrr, newClient.slen) == SOCKET_ERROR)
			{
				cout << "sendto failed with error code " << WSAGetLastError() << endl;
				cout << "New client Section" << endl;
				Sleep(100);
				continue;
			}

			generalMessage = "New User: " + newClient.name + " has joined the chat";

			for (ClientInfo& _client : activeClients)
			{
				if (_client.name != newClient.name && _client.id != -1)
				{
					if (sendto(commSocket, generalMessage.c_str(), (int)generalMessage.length(), 0, (struct sockaddr*)&_client.sockAdrr, _client.slen) == SOCKET_ERROR)
					{
						cout << "sendto failed with error code " << WSAGetLastError() << endl;
						cout << "New client Section" << endl;
						Sleep(100);
						continue;
					}
				}
			}

			cout << generalMessage << endl;

			continue;
		}

		string checker = buf;
		string delimiter = ">=";
		string token{};
		size_t pos_start{ 0 }, pos_end, delim_len = delimiter.length(), times_split{ 0 };

		while ((pos_end = checker.find(delimiter, pos_start)) != string::npos)
		{
			token = checker.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			token = checker.substr((pos_start));
			++times_split;
		}

		if (times_split > 0) // Sent to someone directly
		{
			for (ClientInfo& _client : activeClients)
			{
				if (_client.name == token)
				{
					string directMessage = _client.name + " directly messaged you: " + buf;

					if (sendto(commSocket, directMessage.c_str(), (int)directMessage.length(), 0, (struct sockaddr*)&_client.sockAdrr, _client.slen) == SOCKET_ERROR)
					{
						cout << "sendto failed with error code " << WSAGetLastError() << endl;
						cout << "Send to all Section" << endl;
						closesocket(commSocket);
						WSACleanup();
						return 1;
					}
				}
			}
		}
		else // Sent to everyone
		{
			for (ClientInfo& _client : activeClients)
			{
				if (_client.id != -1 && _client.id != currentClient.id)
				{
					string directMessage = _client.name + " says: " + buf;

					if (sendto(commSocket, directMessage.c_str(), (int)directMessage.length(), 0, (struct sockaddr*)&_client.sockAdrr, _client.slen) == SOCKET_ERROR)
					{
						cout << "sendto failed with error code " << WSAGetLastError() << endl;
						cout << "Send to all Section" << endl;
						closesocket(commSocket);
						WSACleanup();
						return 1;
					}
				}
			}

			char myMessage[BuffLen]{};
			cout << "Enter your username: ";
			cin.getline(myMessage, BuffLen);

			for (ClientInfo& _client : activeClients)
			{
				if (_client.id != -1 && _client.id != currentClient.id)
				{
					string directMessage = _client.name + " says: " + buf;

					if (sendto(commSocket, myMessage, BuffLen, 0, (struct sockaddr*)&_client.sockAdrr, _client.slen) == SOCKET_ERROR)
					{
						cout << "sendto failed with error code " << WSAGetLastError() << endl;
						cout << "Send to all Section" << endl;
						closesocket(commSocket);
						WSACleanup();
						return 1;
					}
				}
			}
		}
	}

	closesocket(commSocket);
	WSACleanup();
	return 0;
}