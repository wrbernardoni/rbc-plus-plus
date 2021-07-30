#include "fullRandom.h"

#include <cstdlib> 
#include <ctime>

void WRB_Bot::FullRandom::handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name)
{
	srand(time(0));
}

short WRB_Bot::FullRandom::choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	return (sense_actions[rand() % sense_actions.size()]);
}

WRB_Chess::Move WRB_Bot::FullRandom::choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left)
{
	return (move_actions[rand() % move_actions.size()]);
}
