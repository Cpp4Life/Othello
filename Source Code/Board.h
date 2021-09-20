#ifndef _BOARDGAME_H_
#define _BOARDGAME_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include "Const.h"

class Board
{
public:
	class Square
	{
	public:
		Square();
		Square(char _y, char _x);
		char x, y;
	};

	class Move
	{
	public:
		Move();
		Move(char _y, char _x);
		Board::Square square;
		bool valid;
		std::vector<Square> flips;
	};

	Board();
	Board(Board& other);
	Board(char boardState[8][8], int currentPlayer);
	void Print();
	bool OnFrontier(int y, int x);
	bool TerminalState(bool currentPlayerPass);
	bool NextPlayer(bool currentPlayerPass);
	void ApplyMove(Board::Move move);
	std::vector<Board::Move> LegalMoves(int player);
	void GameOver();

	int		currentPlayer;
	int		score[3];
	char	board[BOARDSIZE][BOARDSIZE];
	bool	playerPassed;

private:
	bool m_onBoard(const char _y, const char _x);
	bool m_iterate(char& y, char& x, const int& mode, const int& direction);
};

#endif //_BOARDGAME_H_