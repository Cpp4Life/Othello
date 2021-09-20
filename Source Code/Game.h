#pragma once

#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>
#include <vector>
#include <iterator>
#include <sstream>
#include <string>
#include "Board.h"
#include "Const.h"



class Game {
public:
    Game();
    void Setup(std::string, bool&);
    std::string Play();

private:
    int heuristic(Board board);
    int alphabeta(Board board, int depth, int alpha, int beta, bool maxPlayer);
    std::string smartMove();

    Board   board;
    int     maxPlayer;
    int     timeLimit = 10;
    clock_t startTime;
    bool    timeout;
};

#endif //_GAME_H_