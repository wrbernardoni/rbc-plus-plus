#include "inference.h"

#include <cstdlib> 
#include <ctime>
#include <iostream>
#include <unordered_map>
#include <vector>

std::string sensePartition(WRB_Chess::Bitboard brd, short s)
{
	return "";
}

void WRB_Bot::Inference::handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name)
{
	srand(time(0));
	boards.Initialize(color, board);
	c = color;
	turnCount = 0;
}

void WRB_Bot::Inference::handle_opponent_move_result(bool pieceCaptured, int square)
{
	std::cout << "Current number of boards: " << boards.size() << std::endl;
	if ((turnCount != 0) || (c == WRB_Chess::Color::Black))
	{
		boards.OpponentMove(square);
	}
	std::cout << "Current number of boards: " << boards.size() << std::endl;
};

short WRB_Bot::Inference::choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	short minSense = -1;
	int maxSize = -1;
	for (int j = 1; j < 7; j++)
	{
		for (int i = 1; i < 7; i++)
		{
			std::unordered_map<WRB_Chess::Bitboard, std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>, WRB_Chess::BoardHash> partition;
			for (auto it = boards.boards.begin(); it != boards.boards.end(); it++)
			{
				WRB_Chess::Bitboard masked = (*it).senseMask(i + 8*j);
				partition[masked].emplace((*it));
			}
	
			int localMax = -1;
			
			for (auto it = partition.begin(); it != partition.end(); it++)
			{
				int pSize = (*it).second.size();
				if (pSize > localMax)
				{
					localMax = pSize;
				}
			}
	
			if ((maxSize == -1) || (localMax < maxSize))
			{
				maxSize = localMax;
				minSense = i + 8 * j;
			}
		}
	}
	
	std::cout << "Max number of boards: " << maxSize << std::endl;;

	return minSense;
}

void WRB_Bot::Inference::handle_sense_result(std::vector<std::pair<short, WRB_Chess::ColorPiece>> sR)
{
	boards.SenseResult(sR);
	std::cout << "Current number of boards: " << boards.size() << std::endl;
};

WRB_Chess::Move WRB_Bot::Inference::choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	return (move_actions[rand() % move_actions.size()]);
}


void WRB_Bot::Inference::handle_move_result(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, bool captured_opponent_piece, short capture_square) 
{
	boards.TakenMove(requested_move, taken_move, capture_square);
	turnCount++;
	std::cout << "Current number of boards: " << boards.size() << std::endl;
};

void WRB_Bot::Inference::handle_game_end(WRB_Chess::Color winner_color, WRB_Chess::GameHistory game_history)
{

};