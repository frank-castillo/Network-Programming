#pragma once

class TicTacToe
{
public:
	TicTacToe();
	~TicTacToe();

	int IncTurn();
	int GetTurn() const { return mTurn; }
	std::string GetStringAtIndex(int row, int column);

	bool Input(int sel);
	bool CheckPlayerWin(std::string who);

	void SetTurn(int num);
	void SetStringAtIndex(int row, int index, char icon[]);
	void TogglePlayer();
	void Draw() const;
	void ResetBoard();
	void SaveSession();
	bool LoadSession();

private:
	std::string matrix[4][4] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
	char player{ 'X' };
	int mTurn{ 1 };
};