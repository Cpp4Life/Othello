#include "Game.h"

const static int weightBoard[8][8] = {
	{ 4, -3,  2,  2,  2,  2, -3,  4},
	{-3, -4, -1, -1, -1, -1, -4, -3},
	{ 2, -1,  1,  0,  0,  1, -1,  2},
	{ 2, -1,  0,  1,  1,  0, -1,  2},
	{ 2, -1,  0,  1,  1,  0, -1,  2},
	{ 2, -1,  1,  0,  0,  1, -1,  2},
	{-3, -4, -1, -1, -1, -1, -4, -3},
	{ 4, -3,  2,  2,  2,  2, -3,  4}
};

Game::Game() {}

void Game::Setup(std::string _message, bool& gameOver)
{
	std::cout << _message << std::endl;

	std::istringstream iss(_message);
	std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

	auto it = std::find(parsed.begin(), parsed.end(), "winner");
	if (it != parsed.end())
	{
		gameOver = true;
		return;
	}

	std::string currentPlayer = parsed[parsed.size() - 1];

	char _Othelloboard[8][8];
	int i = 0, j = 0;
	for (int temp = 7; temp <= 70; temp++)
	{
		if (parsed[temp] == "W") //WHITE = 1
			_Othelloboard[i][j++] = 1;
		else if (parsed[temp] == "B") //BLACK = 2
			_Othelloboard[i][j++] = 2;
		else if (parsed[temp] == "E") //EMPYT = 0
			_Othelloboard[i][j++] = 0;
		if (j == BOARDSIZE)
		{
			j = 0;
			if (++i == BOARDSIZE)
				break;
		}
	}

	int _myTurn = (currentPlayer == "WHITE") ? WHITE : BLACK;

	board = Board(_Othelloboard, _myTurn);
}

//void Game::Play()
std::string Game::Play()
{
	srand(time(NULL));
	board.Print();
	return Game::smartMove();
}

//  heuristic evaluation of board state
//  accounts for number of pieces of each color, corners,
//  potential corners (that the opponent can capture),
//  frontier pieces, mobility, and edges
int Game::heuristic(Board b)
{
	double piececount, corners, potentialCorners, edges, frontier, mobility;
	int mine, opp;

	int opponent = (maxPlayer == WHITE) ? BLACK : WHITE;

	//set weights of heuristic parameters
	int pW = 2 * (b.score[maxPlayer] + b.score[opponent]);
	int cW = 5500;
	int pcW = 2500;
	int fW = 200;
	int eW = 125;
	int mW = 10 * (100 - (b.score[maxPlayer] + b.score[opponent]));

	//piece count
	piececount = (100.0 * b.score[maxPlayer]) / (b.score[maxPlayer] + b.score[opponent]);

	//corners
	mine = 0, opp = 0;
	if (b.board[0][0] == maxPlayer) mine++;
	else if (b.board[0][0] == opponent) opp++;
	if (b.board[0][7] == maxPlayer) mine++;
	else if (b.board[0][7] == opponent) opp++;
	if (b.board[7][0] == maxPlayer) mine++;
	else if (b.board[7][0] == opponent) opp++;
	if (b.board[7][7] == maxPlayer) mine++;
	else if (b.board[7][7] == opponent) opp++;
	corners = 25.0 * (mine - opp);

	//edges and frontier
	int myEdges = 0, oppEdges = 0;
	int myFrontier = 0, oppFrontier = 0;
	for (int i = 0; i < BOARDSIZE; i++)
	{
		for (int j = 0; j < BOARDSIZE; j++)
		{
			if (i == 0 || i == BOARDSIZE - 1 || j == 0 || j == BOARDSIZE - 1)
			{
				if (b.board[i][j] == maxPlayer) myEdges++;
				else if (b.board[i][j] == opponent) oppEdges++;
			}
			else if (b.board[i][j] != '0')
			{
				if (b.OnFrontier(i, j))
				{
					if (b.board[i][j] == maxPlayer) myFrontier++;
					else if (b.board[i][j] == opponent) oppFrontier++;
				}
			}
		}
	}
	edges = 100.0 * myEdges / (myEdges + oppEdges);
	frontier = -100 * (myFrontier - oppFrontier); //frontier pieces are bad!

	//mobility
	std::vector<Board::Move> myLegal = board.LegalMoves(maxPlayer);
	std::vector<Board::Move> oppLegal = board.LegalMoves(opponent);
	mobility = 100.0 * myLegal.size() / (myLegal.size() + oppLegal.size());

	//potential corners (pseudo-expand node)
	opp = 0;
	for (int i = 0; i < oppLegal.size(); i++)
	{
		if (oppLegal[i].square.y == 0 && oppLegal[i].square.x == 0) opp++;
		else if (oppLegal[i].square.y == 0 && oppLegal[i].square.x == (BOARDSIZE - 1)) opp++;
		else if (oppLegal[i].square.y == (BOARDSIZE - 1) && oppLegal[i].square.x == 0) opp++;
		else if (oppLegal[i].square.y == (BOARDSIZE - 1) && oppLegal[i].square.x == (BOARDSIZE - 1)) opp++;
	}
	potentialCorners = -25.0 * opp;

	return pW * piececount + cW * corners + pcW * potentialCorners + eW * edges + fW * frontier + mW * mobility;
}

int Game::alphabeta(Board board, int depth, int alpha, int beta, bool maxPlayer)
{
	int a = alpha, b = beta, msize;

	//do a quick check on time limit and depth
	if ((((float)(clock() - startTime)) / CLOCKS_PER_SEC) > TIMECUTOFF * timeLimit)
	{
		timeout = true;
		return heuristic(board);
	}
	else if (depth == 0)
		return heuristic(board);
	else
		depth--;

	std::vector<Board::Move> m = board.LegalMoves(board.currentPlayer); //expand
	msize = m.size();

	if (msize == 0)
	{ //no legal moves
		if (board.TerminalState(true))
		{ //check terminal state
			Board child = board;
			child.NextPlayer(false);
			return heuristic(child);
		}
		else
		{ //if pass is only move, continue search with pass
			Board child = board;
			child.NextPlayer(true);
			return alphabeta(child, depth, alpha, beta, !maxPlayer);
		}
	}

	if (maxPlayer)
	{ //maximize alpha
		int v = INT_MIN;
		for (int i = 0; i < msize; i++)
		{
			Board child = board;
			child.ApplyMove(m[i]);
			child.NextPlayer(false);

			int eval = alphabeta(child, depth, a, b, false);
			v = MAX(v, eval);

			//if opponent can make a move that will give max
			//a lower score than alpha, this branch is not
			//worth exploring
			if (v >= beta)
				return v;
			a = MAX(a, v);
		}
		return v;
	}
	else { //minimize beta
		int v = INT_MAX;
		for (int i = 0; i < msize; i++)
		{
			Board child = board;
			child.ApplyMove(m[i]);
			child.NextPlayer(false);

			int eval = alphabeta(child, depth, a, b, true);
			v = MIN(v, eval);

			//if opponent can make a move that will give max
			//a lower score than alpha, this branch is not
			//worth exploring
			if (v <= a)
				return v;
			b = MIN(b, v);
		}
		return v;
	}
}

struct Coord
{
	int Ox;	//alphabet
	int Oy; //numeric
};

//bool Game::smartMove()
std::string Game::smartMove()
{
	int depth, eval, moveNum = 0;
	int depthLimit = NUMSQUARES - (board.score[BLACK] + board.score[WHITE]);
	Board::Move move, bestMove;

	startTime = clock();
	maxPlayer = board.currentPlayer;

	//expand layer 1
	std::vector<Board::Move> legal = board.LegalMoves(board.currentPlayer);

	if (legal.size() == 0)
		return "NULL";

	char alphabet, numeric;
	std::vector<Coord> possibleMoves;
	for (int i = 0; i < legal.size(); i++)
	{
		alphabet = legal[i].square.x;
		numeric = legal[i].square.y;
		Coord temp;
		temp.Ox = alphabet;
		temp.Oy = numeric;
		possibleMoves.push_back(temp);
	}

	//increment depth of search until time runs out
	//look for the move with the MAX evaluation
	for (depth = 0; (((float)(clock() - startTime)) / CLOCKS_PER_SEC < timeLimit / 2.0) && depth < depthLimit; depth++)
	{
		int alpha = INT_MIN, beta = INT_MAX, randMove = 1;
		timeout = false; //reset timeout

		for (int i = 0; i < legal.size(); i++)
		{ //maximize alpha
			Board child = board;
			child.ApplyMove(legal[i]);
			child.NextPlayer(false);
			eval = alphabeta(child, depth, alpha, beta, false);

			//if this depth timed out, use the best move from the previous depth
			if (timeout)
				break;

			if (eval > alpha)
			{
				move = legal[i];
				moveNum = i;
				alpha = eval;
			}
			else if (eval == alpha)
			{
				//use the new move instead of the existing one with uniform probabilty
				if (((rand() % randMove++) - 1) == 0)
				{
					move = legal[i];
					moveNum = i;
				}
			}
		}
		bestMove = move;
	}
	std::cout << "Searched to depth: " << depth << " in " << ((float)(clock() - startTime)) / CLOCKS_PER_SEC << " seconds" << std::endl;
	
	int maxWeight = INT_MIN;
	Board::Move idealMove;
	for (int i = 0; i < possibleMoves.size(); i++)
	{
		if (maxWeight < weightBoard[possibleMoves[i].Oy][possibleMoves[i].Ox])
		{
			maxWeight = weightBoard[possibleMoves[i].Oy][possibleMoves[i].Ox];
			idealMove.square.x = possibleMoves[i].Ox;
			idealMove.square.y = possibleMoves[i].Oy;
		}
	}

	if (weightBoard[bestMove.square.x][bestMove.square.y] < weightBoard[idealMove.square.y][idealMove.square.x])
	{
		bestMove.square.x = idealMove.square.x;
		bestMove.square.y = idealMove.square.y;
	}

	alphabet = bestMove.square.x;
	numeric = bestMove.square.y;

	std::string coord(1, alphabet + 'a');
	coord += numeric + '1';
	return coord;
}