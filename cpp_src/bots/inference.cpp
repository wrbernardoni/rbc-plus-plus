#include "inference.h"

#include <cstdlib> 
#include <ctime>

std::string sensePartition(WRB_Chess::Bitboard brd, short s)
{
	return "";
}

void WRB_Bot::Inference::handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name)
{
	srand(time(0));
	boards.Initialize(color, board);
	c = color;
}

void WRB_Bot::Inference::handle_opponent_move_result(bool pieceCaptured, int square)
{

};

short WRB_Bot::Inference::choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	return (sense_actions[rand() % sense_actions.size()]);
}

void WRB_Bot::Inference::handle_sense_result(std::vector<std::pair<short, WRB_Chess::ColorPiece>>)
{

};

WRB_Chess::Move WRB_Bot::Inference::choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	return (move_actions[rand() % move_actions.size()]);
}


void WRB_Bot::Inference::handle_move_result(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, bool captured_opponent_piece, short capture_square) 
{

};

void WRB_Bot::Inference::handle_game_end(WRB_Chess::Color winner_color, WRB_Chess::GameHistory game_history)
{

};