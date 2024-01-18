
#include <iostream>
#include <stdlib.h>
#include <string>
#include "TicTacToe.h"
#include "fstream"

using namespace std;
TicTacToe::TicTacToe() :player{ 'X' }, mTurn{ 1 }
{
}

TicTacToe::~TicTacToe()
{
}

void TicTacToe::Draw() const
{
	cout << "Tic Tac Toe: " << endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

bool TicTacToe::Input(int sel)
{
	if (sel < 1 || sel > 16) // invalid input
	{
		return false;
	}

	int index = sel - 1;

	if (matrix[index / 4][index % 4] == std::to_string(sel))
	{
		matrix[index / 4][index % 4] = player;
	}
	else
	{
		cout << "Field is already in use try again!" << endl;
		return false;
	}

	return true;
}

void TicTacToe::TogglePlayer()
{
	if (player == 'X')
	{
		player = 'O';
	}
	else if (player == 'O')
	{
		player = 'Y';
	}
	else
	{
		player = 'X';
	}
}
bool TicTacToe::CheckPlayerWin(std::string who)
{
	// row match wins
	for (int i = 0; i < 4; ++i)
	{
		if (matrix[i][0] == who && matrix[i][1] == who && matrix[i][2] == who)
		{
			return true;
		}

		if (matrix[i][1] == who && matrix[i][2] == who && matrix[i][3] == who)
		{
			return true;
		}
	}

	// column match wins
	for (int i = 0; i < 4; ++i)
	{
		if (matrix[0][i] == who && matrix[1][i] == who && matrix[2][i] == who)
		{
			return true;
		}

		if (matrix[1][i] == who && matrix[2][i] == who && matrix[3][i] == who)
		{
			return true;
		}
	}

	// diagonal match wins
	auto CheckDiagonal = [&](const int x, const int y)
	{
		if (matrix[x][y] == who && matrix[x - 1][y + 1] == who && matrix[x + 1][y - 1] == who) {
			return true;
		}

		if (matrix[x][y] == who && matrix[x - 1][y - 1] == who && matrix[x + 1][y + 1] == who) {
			return true;
		}
		return false;
	};

	if (CheckDiagonal(1, 1)) { return true; }
	if (CheckDiagonal(1, 2)) { return true; }
	if (CheckDiagonal(2, 1)) { return true; }
	if (CheckDiagonal(2, 2)) { return true; }

	return false;
}

void TicTacToe::ResetBoard()
{
	matrix[0][0] = "1";
	matrix[0][1] = "2";
	matrix[0][2] = "3";
	matrix[0][3] = "4";

	matrix[1][0] = "5";
	matrix[1][1] = "6";
	matrix[1][2] = "7";
	matrix[1][3] = "8";

	matrix[2][0] = "9";
	matrix[2][1] = "10";
	matrix[2][2] = "11";
	matrix[2][3] = "12";

	matrix[3][0] = "13";
	matrix[3][1] = "14";
	matrix[3][2] = "15";
	matrix[3][3] = "16";
}

int TicTacToe::IncTurn()
{
	return mTurn++;
}

std::string TicTacToe::GetStringAtIndex(int row, int column)
{
	return matrix[row][column];
}

void TicTacToe::SetTurn(int num)
{
	mTurn = num;
}

void TicTacToe::SetStringAtIndex(int row, int index, char icon[])
{
	matrix[row][index] = icon;
}

void TicTacToe::SaveSession()
{
	ofstream fw("../session.txt", ofstream::out);

	if (fw.is_open())
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				fw << matrix[i][j] << "\n";
			}
		}
	}
	else
	{
		cout << "Error while opening file" << endl;
	}
}

bool TicTacToe::LoadSession()
{
	string line;
	ifstream session("../session.txt");

	if (session.is_open())
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				getline(session, line);
				matrix[i][j] = line;
			}
		}

		return true;
	}
	else
	{
		cout << "Error while opening file" << endl;
		return false;
	}
}