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

WRB_Bot::Inference::Inference(WRB_Chess::EngineBase* e)
{
	this->engine = e;
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
	log("Current number of boards: " + std::to_string(boards.size()));
	if ((turnCount != 0) || (c == WRB_Chess::Color::Black))
	{
		boards.OpponentMove(square);
	}
	log("Current number of boards: " + std::to_string(boards.size()));
};

short WRB_Bot::Inference::choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	auto mP = engine->RecommendScanMovePolicy(boards.boards, c);
	movePolicy = mP.second;
	return mP.first;
}

void WRB_Bot::Inference::handle_sense_result(std::vector<std::pair<short, WRB_Chess::ColorPiece>> sR)
{
	boards.SenseResult(sR);
	log("Current number of boards: " + std::to_string(boards.size()));

	WRB_Chess::Bitboard senseMask(sR);
	mv = movePolicy[senseMask];
};

WRB_Chess::Move WRB_Bot::Inference::choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	//return (move_actions[rand() % move_actions.size()]);
	return mv;
}


void WRB_Bot::Inference::handle_move_result(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, bool captured_opponent_piece, short capture_square) 
{
	boards.TakenMove(requested_move, taken_move, capture_square);
	turnCount++;
	log("Current number of boards: " + std::to_string(boards.size()));
};

void WRB_Bot::Inference::handle_game_end(WRB_Chess::Color winner_color, WRB_Chess::GameHistory game_history)
{

};