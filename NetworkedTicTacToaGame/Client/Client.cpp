//#define WIN32_LEAN_AND_MEAN
#include <iostream>
//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string>
#include "../TicTacToe_logic/TicTacToe.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996)

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std;

int main(int argc, char** argv)
{
	TicTacToe tictactoe;
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	const char* sendbuf = "Start";
	std::string sendchar;
	char recvbuf[DEFAULT_BUFLEN];
	char arr[DEFAULT_BUFLEN] = { 'y','e','s' };
	int iResult;
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	bool isPlayer01 = false;
	bool firstPass = true;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server. If connection to server fails, close socket, set to INVALID_SOCKET and start the process again
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Add load
	memset(recvbuf, '\0', DEFAULT_BUFLEN);
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (strcmp(recvbuf, "-1") == 0)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				memset(recvbuf, '\0', DEFAULT_BUFLEN);
				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				if (iResult == SOCKET_ERROR)
				{
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				else
				{
					tictactoe.SetStringAtIndex(i, j, recvbuf);
				}
			}
		}

		tictactoe.Draw();
	}
	else
	{
		cout << "No previous session found" << endl;
	}

	memset(recvbuf, '\0', DEFAULT_BUFLEN);
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	cout << "You are player: " << recvbuf << "\n";

	if (iResult > 0)
	{
		if (strcmp(recvbuf, "1") == 0)
		{
			//Client Starts game first
			isPlayer01 = true;
			cout << "---Multiplayer Tic Tac Toe--- \n";
			cout << "----Type in 20 to pause the game--- \n";
			cout << "Turn: " << tictactoe.IncTurn() << "\n";
			tictactoe.Draw();
			cout << "It's your turn. Choose a digit in the field: ";

			do
			{
				getline(std::cin, sendchar);
			} while (!tictactoe.Input(std::stoi(sendchar)));

			tictactoe.Draw();
			tictactoe.TogglePlayer();
			cout << "Sending " << sendchar << "\n";
			cout << "Waiting For Opponent Input....\n";

			iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
		}
		else
		{
			cout << "Waiting for other players to make a selection" << endl;
			tictactoe.SetTurn(1);
		}
	}

	// Receive until the peer closes the connection
	do
	{
		if (isPlayer01)
		{
			if (!firstPass)
			{
				tictactoe.TogglePlayer();
			}

			memset(recvbuf, '\0', DEFAULT_BUFLEN);
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0)
			{
				if (stoi(recvbuf) == 0)
				{
					cout << "Game ended!" << endl;
					iSendResult = send(ConnectSocket, recvbuf, recvbuflen, 0);
					closesocket(ConnectSocket);
					WSACleanup();
					return 0;
				}

				if (stoi(recvbuf) == 20)
				{
					// Pause game
					cout << "A player has asked to pause the game, do you want to pause as well?" << endl;
					getline(std::cin, sendchar);

					if (sendchar == "yes")
					{
						iSendResult = send(ConnectSocket, recvbuf, recvbuflen, 0);
						if (iSendResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}

						memset(recvbuf, '\0', DEFAULT_BUFLEN);
						iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0); // Receives from player 2
						if (iResult > 0)
						{
							if (stoi(recvbuf) == 20)
							{
								cout << "Game ended, session will be saved" << endl;
								closesocket(ConnectSocket);
								WSACleanup();
								return 0;
							}
							else
							{
								cout << "Game ended but session wont be saved" << endl;
								closesocket(ConnectSocket);
								WSACleanup();
								return 0;
							}
						}
					}
					else
					{
						cout << "Closing Connection\nGame will not be saved";
						sendchar = "0";

						iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
						if (iResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}

						Sleep(5000);
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}
				}

				tictactoe.IncTurn();
				tictactoe.Input(stoi(recvbuf));
			}

			tictactoe.TogglePlayer();
		}

		memset(recvbuf, '\0', DEFAULT_BUFLEN);
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		//cout <<"Recieved Input: " << recvbuf << "\n";
		if (iResult > 0)
		{
			tictactoe.IncTurn();
			tictactoe.Input(stoi(recvbuf));
			tictactoe.Draw();

			if (stoi(recvbuf) == 0)
			{
				cout << "Game ended!" << endl;
				iSendResult = send(ConnectSocket, recvbuf, recvbuflen, 0);
				closesocket(ConnectSocket);
				WSACleanup();
				return 0;
			}

			if (stoi(recvbuf) == 20)
			{
				// Pause game
				cout << "A player has asked to pause the game, do you want to pause as well?" << endl;
				getline(std::cin, sendchar);

				if (sendchar == "yes")
				{
					iSendResult = send(ConnectSocket, recvbuf, recvbuflen, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}

					memset(recvbuf, '\0', DEFAULT_BUFLEN);
					iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0); // Receives from player 2
					if (iResult > 0)
					{
						if (stoi(recvbuf) == 20)
						{
							cout << "Game ended, session will be saved" << endl;
							closesocket(ConnectSocket);
							WSACleanup();
							return 0;
						}
						else
						{
							cout << "Game ended but session wont be saved" << endl;
							closesocket(ConnectSocket);
							WSACleanup();
							return 0;
						}
					}
				}
				else
				{
					cout << "Closing Connection\nGame will not be saved";
					sendchar = "0";

					iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					if (iResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}

					Sleep(5000);
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}

			if (stoi(recvbuf) == 18 || stoi(recvbuf) == 19)
			{
				cout << "Opponent Requests a rematch (yes or no)? ";
				getline(std::cin, sendchar);
				if (sendchar == "yes")
				{
					sendchar = "19";
				}
				else
				{
					cout << "Rematch Declined Closing Connection\n";
					Sleep(5000);
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}

				tictactoe.TogglePlayer();
				tictactoe.ResetBoard();
				tictactoe.SetTurn(1);
			}
			else if (tictactoe.GetTurn() > 16)
			{
				cout << "Draw! \n";
				cout << "Do you want a rematch? (yes or no): ";
				getline(std::cin, sendchar);
				if (sendchar == "yes")
				{
					sendchar = "18";
				}
				else
				{
					cout << "Closing Connection\n";
					sendchar = "0";

					iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					if (iResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}

					Sleep(5000);
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				tictactoe.ResetBoard();
				tictactoe.SetTurn(0);
			}
			else if (tictactoe.CheckPlayerWin("O") || tictactoe.CheckPlayerWin("X") || tictactoe.CheckPlayerWin("Y"))
			{
				cout << "You Lose!\n";
				cout << "Do you want a rematch? (yes or no): ";
				getline(std::cin, sendchar);
				if (sendchar == "yes")
				{
					sendchar = "18";
				}
				else
				{
					cout << "Closing Connection\n";
					sendchar = "0";

					iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					if (iResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}

					Sleep(5000);
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}

				tictactoe.TogglePlayer();
				tictactoe.ResetBoard();
				tictactoe.SetTurn(0);
			}
			else
			{
				cout << "----Type in 20 to pause the game--- \n";
				cout << "Turn: " << tictactoe.IncTurn() << "\n";
				tictactoe.TogglePlayer();
				cout << "It's your turn. Press the number of the field: ";

				do
				{
					getline(std::cin, sendchar);
				} while (!tictactoe.Input(std::stoi(sendchar)));

				tictactoe.Draw();

				if (tictactoe.CheckPlayerWin("X") || tictactoe.CheckPlayerWin("O"))
				{
					cout << "You win!\n";
				}
			}

			//cout << "Sending " << sendchar << "\n";
			cout << "Waiting For Opponent Input....\n";

			iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}

			if (!isPlayer01)
			{
				tictactoe.TogglePlayer(); // If second player, here the toggle should be Y

				memset(recvbuf, '\0', DEFAULT_BUFLEN);
				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				if (iResult > 0)
				{
					if (stoi(recvbuf) == 0)
					{
						cout << "Game ended!" << endl;
						iSendResult = send(ConnectSocket, recvbuf, recvbuflen, 0);
						closesocket(ConnectSocket);
						WSACleanup();
						return 0;
					}
					if (stoi(recvbuf) == 20)
					{
						// Pause game
						cout << "A player has asked to pause the game, do you want to pause as well?" << endl;
						getline(std::cin, sendchar);

						if (sendchar == "yes")
						{
							iSendResult = send(ConnectSocket, recvbuf, recvbuflen, 0);
							if (iSendResult == SOCKET_ERROR) {
								printf("send failed with error: %d\n", WSAGetLastError());
								closesocket(ConnectSocket);
								WSACleanup();
								return 1;
							}

							memset(recvbuf, '\0', DEFAULT_BUFLEN);
							iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0); // Receives from player 2
							if (iResult > 0)
							{
								if (stoi(recvbuf) == 20)
								{
									cout << "Game ended, session will be saved" << endl;
									closesocket(ConnectSocket);
									WSACleanup();
									return 0;
								}
								else
								{
									cout << "Game ended but session wont be saved" << endl;
									closesocket(ConnectSocket);
									WSACleanup();
									return 0;
								}
							}
						}
						else
						{
							cout << "Closing Connection\nGame will not be saved";
							sendchar = "0";

							iSendResult = send(ConnectSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							if (iResult == SOCKET_ERROR) {
								printf("send failed with error: %d\n", WSAGetLastError());
								closesocket(ConnectSocket);
								WSACleanup();
								return 1;
							}

							Sleep(5000);
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}
					}

					tictactoe.IncTurn();
					tictactoe.Input(stoi(recvbuf));
				}

				tictactoe.TogglePlayer();
			}
		}
		else if (iResult == 0)
		{
			cout << "Server Closed Connection\n";
		}
		else
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}