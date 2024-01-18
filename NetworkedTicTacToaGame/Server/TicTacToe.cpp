#include "TicTacToe.h"

TicTacToe::TicTacToe()
{

}

TicTacToe::~TicTacToe()
{
}

void TicTacToe::Draw()
{
	cout << "Tic Tac Toe " << endl;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;

}

bool TicTacToe::Input(int a)
{
	if (a >= 10)
	{
		return false;
	}
	if (a == 1)
	{
		if (matrix[0][0] == '1')
			matrix[0][0] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 2)
	{
		if (matrix[0][1] == '2')
			matrix[0][1] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 3)
	{
		if (matrix[0][2] == '3')
			matrix[0][2] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 4)
	{
		if (matrix[1][0] == '4')
			matrix[1][0] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 5)
	{
		if (matrix[1][1] == '5')
			matrix[1][1] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 6)
	{
		if (matrix[1][2] == '6')
			matrix[1][2] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 7)
	{
		if (matrix[2][0] == '7')
			matrix[2][0] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 8)
	{
		if (matrix[2][1] == '8')
			matrix[2][1] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	else if (a == 9)
	{
		if (matrix[2][2] == '9')
			matrix[2][2] = player;
		else
		{
			cout << "Field is already in use try again!" << endl;
			return false;
		}
	}
	return true;
}

void TicTacToe::TogglePlayer()
{
	if (player == 'X')
		player = 'O';
	else
		player = 'X';
}
bool TicTacToe::CheckXWin()
{
	//first player
	if (matrix[0][0] == 'X' && matrix[0][1] == 'X' && matrix[0][2] == 'X')
	{
		return true;
	}

	else if (matrix[1][0] == 'X' && matrix[1][1] == 'X' && matrix[1][2] == 'X')
	{
		return true;
	}

	else if (matrix[2][0] == 'X' && matrix[2][1] == 'X' && matrix[2][2] == 'X')
	{
		return true;
	}

	else if (matrix[0][0] == 'X' && matrix[1][0] == 'X' && matrix[2][0] == 'X')
	{
		return true;
	}
	else if (matrix[0][1] == 'X' && matrix[1][1] == 'X' && matrix[2][1] == 'X')
	{
		return true;
	}
	else if (matrix[0][2] == 'X' && matrix[1][2] == 'X' && matrix[2][2] == 'X')
	{
		return true;
	}
	else if (matrix[0][0] == 'X' && matrix[1][1] == 'X' && matrix[2][2] == 'X')
	{
		return true;
	}
	else if (matrix[2][0] == 'X' && matrix[1][1] == 'X' && matrix[0][2] == 'X')
	{
		return true;
	}
	return false;
}

bool TicTacToe::CheckOWin()
{
	//second player
	if (matrix[0][0] == 'O' && matrix[0][1] == 'O' && matrix[0][2] == 'O')
		return true;
	else if (matrix[1][0] == 'O' && matrix[1][1] == 'O' && matrix[1][2] == 'O')
		return true;
	else if (matrix[2][0] == 'O' && matrix[2][1] == 'O' && matrix[2][2] == 'O')
		return true;
	else if (matrix[0][0] == 'O' && matrix[1][0] == 'O' && matrix[2][0] == 'O')
		return true;
	else if (matrix[0][1] == 'O' && matrix[1][1] == 'O' && matrix[2][1] == 'O')
		return true;
	else if (matrix[0][2] == 'O' && matrix[1][2] == 'O' && matrix[2][2] == 'O')
		return true;
	else if (matrix[0][0] == 'O' && matrix[1][1] == 'O' && matrix[2][2] == 'O')
		return true;
	else if (matrix[2][0] == 'O' && matrix[1][1] == 'O' && matrix[0][2] == 'O')
		return true;
	return false;
}

void TicTacToe::ResetBoard()
{
	matrix[0][0] = '1';
	matrix[0][1] = '2';
	matrix[0][2] = '3';
	matrix[1][0] = '4';
	matrix[1][1] = '5';
	matrix[1][2] = '6';
	matrix[2][0] = '7';
	matrix[2][1] = '8';
	matrix[2][2] = '9';
}

int TicTacToe::IncTurn()
{
	return mTurn++;
}

void TicTacToe::SetTurn(int num)
{
	mTurn = num;
}
