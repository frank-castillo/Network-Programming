#undef UNICODE

//#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
#include <string>
#include "../TicTacToe_logic/TicTacToe.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996)
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#include <iostream>
using namespace std;

int main(void)
{
	WSADATA wsaData;
	int iResult;

	TicTacToe tictactoe;
	SOCKET ListenSocket = INVALID_SOCKET;		// Socket used to allow connections to the server
	SOCKET PlayerOneSocket = INVALID_SOCKET;		// Socket used to connect to the client
	SOCKET PlayerTwoSocket = INVALID_SOCKET;		// Socket used to connect to the client
	struct addrinfo* result = NULL;				// Pointer to store the servers information
	struct addrinfo hints;						// Struct used to store server's info

	std::string sendchar;
	bool YourTurn = false;
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port. Store it in result pointer
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server. Use the result pointer to extract the server's info.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket -> Binding
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	// Setup the TCP listening socket -> Listening. Server is ready to accept clients.
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server ready to accept clients:\n";

	// Accept a client socket
	PlayerOneSocket = accept(ListenSocket, NULL, NULL);
	if (PlayerOneSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Player One is Ready!" << endl;

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Server waiting to accept second player:\n";

	PlayerTwoSocket = accept(ListenSocket, NULL, NULL);
	if (PlayerTwoSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		closesocket(PlayerOneSocket);
		WSACleanup();
		return 1;
	}

	cout << "All players on-line!" << endl;

	// If session exists, send it to the other players
	if (tictactoe.LoadSession())
	{
		sendchar = "-1";

		iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(PlayerOneSocket);
			closesocket(PlayerTwoSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(PlayerOneSocket);
			closesocket(PlayerTwoSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				string icon = tictactoe.GetStringAtIndex(i, j);

				iSendResult = send(PlayerOneSocket, icon.c_str(), strlen(icon.c_str()), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(PlayerOneSocket);
					closesocket(PlayerTwoSocket);
					closesocket(ListenSocket);
					WSACleanup();
					return 1;
				}
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				string icon = tictactoe.GetStringAtIndex(i, j);

				iSendResult = send(PlayerTwoSocket, icon.c_str(), strlen(icon.c_str()), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(PlayerOneSocket);
					closesocket(PlayerTwoSocket);
					closesocket(ListenSocket);
					WSACleanup();
					return 1;
				}
			}
		}
	}
	else
	{
		sendchar = "-2";

		iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(PlayerOneSocket);
			closesocket(PlayerTwoSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(PlayerOneSocket);
			closesocket(PlayerTwoSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
	}

	sendchar = "1";

	iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(PlayerOneSocket);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	sendchar = "2";

	iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(PlayerOneSocket);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "----3 player Tic Tac Toe--- \n";
	cout << "----Type in 20 to pause the game--- \n";

	cout << "Connection Established with players...\n";
	tictactoe.SetTurn(1);
	tictactoe.Draw();
	cout << "Waiting for Opponent Input...\n";

	// Server accepts clients inputs and sends back their input 
	do
	{
		//Stops NullCharacter from appearing
		memset(recvbuf, '\0', DEFAULT_BUFLEN);
		iResult = recv(PlayerOneSocket, recvbuf, recvbuflen, 0); // Receives from player one
		cout << "Received Input: " << recvbuf << "\n";

		if (iResult > 0)
		{
			if (stoi(recvbuf) == 0)
			{
				cout << "Game ended!" << endl;
				iSendResult = send(PlayerOneSocket, recvbuf, recvbuflen, 0);
				iSendResult = send(PlayerTwoSocket, recvbuf, recvbuflen, 0);
				closesocket(PlayerOneSocket);
				closesocket(PlayerTwoSocket);
				closesocket(ListenSocket);
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
					iSendResult = send(PlayerTwoSocket, recvbuf, recvbuflen, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(PlayerOneSocket);
						closesocket(PlayerTwoSocket);
						closesocket(ListenSocket);
						WSACleanup();
						return 1;
					}

					memset(recvbuf, '\0', DEFAULT_BUFLEN);
					iResult = recv(PlayerTwoSocket, recvbuf, recvbuflen, 0); // Receives from player 2
					if (iResult > 0)
					{
						if (stoi(recvbuf) == 20)
						{
							cout << "Game ended and session will be saved!" << endl;
							sendchar = "0";
							iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							closesocket(PlayerOneSocket);
							closesocket(PlayerTwoSocket);
							closesocket(ListenSocket);
							WSACleanup();
							tictactoe.SaveSession();
							//Save
							return 0;
						}
						else
						{
							cout << "Game ended but session wont be saved" << endl;
							sendchar = "0";
							iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							closesocket(PlayerOneSocket);
							closesocket(PlayerTwoSocket);
							closesocket(ListenSocket);
							WSACleanup();
							return 0;
						}
					}
				}
				else
				{
					sendchar = "0";
					iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(PlayerOneSocket);
						closesocket(PlayerTwoSocket);
						closesocket(ListenSocket);
						WSACleanup();
						return 1;
					}
				}
			}

			tictactoe.Input(stoi(recvbuf));
			tictactoe.TogglePlayer();

			// Sends the information of player 1 to player 2
			iSendResult = send(PlayerTwoSocket, recvbuf, recvbuflen, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(PlayerOneSocket);
				closesocket(PlayerTwoSocket);
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}
		}

		memset(recvbuf, '\0', DEFAULT_BUFLEN);
		iResult = recv(PlayerTwoSocket, recvbuf, recvbuflen, 0); // Receives from player 2
		cout << "Received Input: " << recvbuf << "\n";

		// Sends the information of player 2 to player 1
		iSendResult = send(PlayerOneSocket, recvbuf, recvbuflen, 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(PlayerOneSocket);
			closesocket(PlayerTwoSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		if (iResult > 0)
		{
			if (stoi(recvbuf) == 0)
			{
				cout << "Game ended!" << endl;
				iSendResult = send(PlayerOneSocket, recvbuf, recvbuflen, 0);
				iSendResult = send(PlayerTwoSocket, recvbuf, recvbuflen, 0);
				closesocket(PlayerOneSocket);
				closesocket(PlayerTwoSocket);
				closesocket(ListenSocket);
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
					iSendResult = send(PlayerOneSocket, recvbuf, recvbuflen, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(PlayerOneSocket);
						closesocket(PlayerTwoSocket);
						closesocket(ListenSocket);
						WSACleanup();
						return 1;
					}

					memset(recvbuf, '\0', DEFAULT_BUFLEN);
					iResult = recv(PlayerOneSocket, recvbuf, recvbuflen, 0); // Receives from player 2
					if (iResult > 0)
					{
						if (stoi(recvbuf) == 20)
						{
							cout << "Game ended and session will be saved!" << endl;
							sendchar = "0";
							iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							closesocket(PlayerOneSocket);
							closesocket(PlayerTwoSocket);
							closesocket(ListenSocket);
							WSACleanup();
							tictactoe.SaveSession();
							return 0;
						}
						else
						{
							cout << "Game ended but session wont be saved" << endl;
							sendchar = "0";
							iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
							closesocket(PlayerOneSocket);
							closesocket(PlayerTwoSocket);
							closesocket(ListenSocket);
							WSACleanup();
							return 0;
						}
					}
				}
				else
				{
					sendchar = "0";
					iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(PlayerOneSocket);
						closesocket(PlayerTwoSocket);
						closesocket(ListenSocket);
						WSACleanup();
						return 1;
					}
				}
			}

			tictactoe.IncTurn(); // Two players already played before, so we have to increase accounting for that extra player

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
					closesocket(PlayerOneSocket);
					closesocket(PlayerTwoSocket);
					closesocket(ListenSocket);
					WSACleanup();
					return 1;
				}

				tictactoe.ResetBoard();
				tictactoe.SetTurn(1);
			}
			else
			{
				if (stoi(recvbuf) == 19)
				{
					cout << "Rematch Accepted...\n";
				}
				else
				{
					tictactoe.Input(stoi(recvbuf));
				}

				tictactoe.Draw();

				// checking status of the game
				if (tictactoe.IncTurn() > 16)
				{
					cout << "Draw! \n";
					cout << "Do you want a rematch? (yes or no):  ";
					getline(std::cin, sendchar);

					if (sendchar == "yes")
					{
						sendchar = "18";
					}
					else
					{
						cout << "Closing Connection\n";
						Sleep(5000);
						closesocket(PlayerOneSocket);
						closesocket(PlayerTwoSocket);
						closesocket(ListenSocket);
						WSACleanup();
						return 1;
					}

					tictactoe.ResetBoard();
					tictactoe.SetTurn(0);
				}
				else if (tictactoe.CheckPlayerWin("X") || tictactoe.CheckPlayerWin("Y"))
				{
					cout << "You Lose!\n";
					cout << "Do you want a rematch? (yes or no): ";
					getline(std::cin, sendchar);
					cout << "Sending " << sendchar << "\n";

					if (sendchar == "yes")
					{
						sendchar = "18";
					}
					else
					{
						cout << "Closing Connection\n";
						Sleep(5000);
						closesocket(PlayerOneSocket);
						closesocket(PlayerTwoSocket);
						closesocket(ListenSocket);
						WSACleanup();
						return 1;
					}

					tictactoe.ResetBoard();
					tictactoe.SetTurn(0);
				}
				else
				{
					tictactoe.TogglePlayer();
					cout << "Turn: " << tictactoe.IncTurn() << "\n";
					cout << "It's your turn. Choose a number in the field: ";

					do
					{
						getline(std::cin, sendchar);
					} while (!tictactoe.Input(std::stoi(sendchar)));

					tictactoe.Draw();

					if (tictactoe.CheckPlayerWin("O"))
					{
						cout << "You win!\n";
					}

					tictactoe.TogglePlayer();
				}
			}

			cout << "Waiting For Opponent Input....\n";

			// Echo the buffer back to the sender and other player
			iSendResult = send(PlayerTwoSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(PlayerOneSocket);
				closesocket(PlayerTwoSocket);
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}

			iSendResult = send(PlayerOneSocket, sendchar.c_str(), strlen(sendchar.c_str()), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(PlayerOneSocket);
				closesocket(PlayerTwoSocket);
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}
		}
		else if (iResult == 0)
		{
			cout << "Client Closed Connection\n";
		}
		else
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(PlayerOneSocket);
			closesocket(PlayerTwoSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	// cleanup
	closesocket(PlayerOneSocket);
	closesocket(PlayerTwoSocket);
	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}