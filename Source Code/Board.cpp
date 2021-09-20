#include "Board.h"

const int iterateModes[4] = { 1, 2, 3, 4 };
const int iterateDirections[2] = { -1, 1 };

//empty Square constructor
Board::Square::Square() {}

//Square constructor to assign coordinates
Board::Square::Square(char _y, char _x)
{
	this->y = _y;
	this->x = _x;
}

//empty Move constructor
Board::Move::Move() {}

//Move constructor to assign coordinates
Board::Move::Move(char _y, char _x)
{
	this->square.y = _y;
	this->square.x = _x;
	this->valid = false;
}

//empty Board constructor
Board::Board()
{
	//initialize empty board
	for (int i = 0; i < BOARDSIZE; i++) {
		for (int j = 0; j < BOARDSIZE; j++) {
			board[i][j] = 0;
		}
	}

	//initialize starting pieces
	board[4][3] = BLACK;
	board[3][4] = BLACK;
	board[3][3] = WHITE;
	board[4][4] = WHITE;

	currentPlayer = BLACK;
	playerPassed = false;
	score[BLACK] = 2;
	score[WHITE] = 2;
}

//Board copy constructor
Board::Board(Board& other)
{
	for (int i = 0; i < BOARDSIZE; i++)
		for (int j = 0; j < BOARDSIZE; j++)
			this->board[i][j] = other.board[i][j];

	for (int i = 0; i < 3; i++)
		this->score[i] = other.score[i];

	this->currentPlayer = other.currentPlayer;
	this->playerPassed = other.playerPassed;
}

//constructor to initialize board with current state and current player
Board::Board(char boardState[8][8], int currentPlayer)
{
	score[BLACK] = 0;
	score[WHITE] = 0;
	for (int i = 0; i < BOARDSIZE; i++) {
		for (int j = 0; j < BOARDSIZE; j++) {
			board[i][j] = boardState[i][j];
			if (boardState[i][j] == WHITE)
				score[WHITE]++;
			else if (boardState[i][j] == BLACK)
				score[BLACK]++;
		}
	}
	this->currentPlayer = currentPlayer;
	playerPassed = false;
}

//Check whether coordinate is on the board or not
bool Board::m_onBoard(const char _y, const char _x)
{
	return (_x >= 0) && (_x < BOARDSIZE) && (_y >= 0) && (_y < BOARDSIZE);
}

//Iterate through the board in specific direction
//modes
//	1 horizontal
//	2 vertical
//	3 right diagonal
//	4 left diagonal
//
//directions
//	+1
//	-1
bool Board::m_iterate(char& y, char& x, const int& mode, const int& direction)
{
	if (direction != 1 && direction != -1)
		return false;

	switch (mode)
	{
	case(1):
		x += direction;
		return true;
	case(2):
		y += direction;
		return true;
	case(3):
		y += direction;
		x += direction;
		return true;
	case(4):
		y += direction;
		x -= direction;
		return true;
	default:
		std::cout << "THROWING BECAUSE mode = " << mode << std::endl;
		throw;
	}
}

void Board::Print()
{
	std::cout << "    a  b  c  d  e  f  g  h" << std::endl;
	std::cout << "   ------------------------" << std::endl;
	for (int i = 0; i < BOARDSIZE; i++)
	{
		std::cout << i + 1 << " |";
		for (int j = 0; j < BOARDSIZE; j++)
		{
			if (board[i][j] == WHITE)
				std::cout << RED;
			else if (board[i][j] == BLACK)
				std::cout << BLUE;
			std::cout << std::setw(2) << (int)board[i][j] << RESET << " ";
		}
		std::cout << "|" << std::endl;
	}
	std::cout << "   ------------------------" << std::endl;
}

//  checks whether a piece is on the frontier
bool Board::OnFrontier(int y, int x)
{
	if (board[y][x] == '0')
		return false;
	for (int _GAMEMODE = 0; _GAMEMODE < NUMMODES; _GAMEMODE++) {
		int mode = iterateModes[_GAMEMODE];
		for (int _GAMEDIR = 0; _GAMEDIR < NUMDIRECTIONS; _GAMEDIR++) {
			int direction = iterateDirections[_GAMEDIR];
			char Y = y, X = x;
			m_iterate(Y, X, mode, direction);
			if (m_onBoard(Y, X))
				if (board[Y][X] != '0')
					return true;
		}
	}
	return false;
}

//  check whether the game is in an end state
bool Board::TerminalState(bool currentPlayerPass)
{
	//if both players pass or the board is full, game over
	if ((playerPassed && currentPlayerPass) || (score[BLACK] + score[WHITE] == NUMSQUARES))
		return true;
	return false;
}

//  method to move to the next player,
//  checking if the game is in an end state
bool Board::NextPlayer(bool currentPlayerPass)
{
	if (TerminalState(currentPlayerPass))
		return true;
	currentPlayer = (currentPlayer == WHITE) ? BLACK : WHITE;
	playerPassed = false;
	return false;
}

//  method to apply a move to the board,
//  flipping the appropriate tiles
void Board::ApplyMove(Board::Move move)
{
	board[move.square.y][move.square.x] = currentPlayer;
	if (currentPlayer == WHITE)
		score[WHITE]++;
	else
		score[BLACK]++;

	for (int i = 0; i < move.flips.size(); i++)
	{
		board[move.flips[i].y][move.flips[i].x] = currentPlayer;
		if (currentPlayer == WHITE)
		{
			score[WHITE]++;
			score[BLACK]--;
		}
		else
		{
			score[BLACK]++;
			score[WHITE]--;
		}
	}
}

//  method to find the legal moves for the current player
std::vector<Board::Move> Board::LegalMoves(int player) {
	std::vector<Board::Move> moves;

	for (int i = 0; i < BOARDSIZE; i++)
	{
		for (int j = 0; j < BOARDSIZE; j++)
		{
			if (board[i][j] != 0) //only open squares can be played
				continue;

			Board::Move move = Board::Move(i, j);

			for (int _GAMEMODE = 0; _GAMEMODE < NUMMODES; _GAMEMODE++)
			{
				int mode = iterateModes[_GAMEMODE];
				for (int _GAMEDIR = 0; _GAMEDIR < NUMDIRECTIONS; _GAMEDIR++)
				{
					int direction = iterateDirections[_GAMEDIR];
					char y = move.square.y, x = move.square.x;
					std::vector<Board::Square> trace;

					m_iterate(y, x, mode, direction);

					//not a valid direction unless opponent's piece is next
					if ((board[y][x] == player) || (board[y][x] == 0))
						continue;

					for (y, x; m_onBoard(y, x); m_iterate(y, x, mode, direction))
					{
						if (board[y][x] == player) {
							//mark move as valid and append trace to flips vector
							move.valid = true;
							move.flips.insert(move.flips.end(), trace.begin(), trace.end());
							break;
						}
						else if (board[y][x] == 0)
							break;
						else {
							trace.push_back(Board::Square(y, x)); //keep track of potential flips
						}
					}
				}
			}
			if (move.valid)
				moves.push_back(move);
		}
	}
	return moves;
}

void Board::GameOver() {
	std::cout << "GAME OVER" << std::endl;
	std::cout << "white: " << score[WHITE] << " black : " << score[BLACK] << std::endl;
	if (score[WHITE] > score[BLACK])
		std::cout << "White wins!" << std::endl;
	else if (score[WHITE] < score[BLACK])
		std::cout << "Black wins!" << std::endl;
	else
		std::cout << "Tie!" << std::endl;
}