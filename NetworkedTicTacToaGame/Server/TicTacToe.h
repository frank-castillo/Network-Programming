#pragma once
#include <iostream>
using namespace std;
class TicTacToe
{
public:
	TicTacToe();
	~TicTacToe();

	void Draw();
	bool Input(int a);
	void TogglePlayer();
	bool CheckXWin();
	bool CheckOWin();
	void ResetBoard();
	int IncTurn();
	void SetTurn(int num);
private:
	char matrix[3][3] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	char player = 'X';
	int mTurn = 1;
};

