#ifndef WRB_BOT_BASECLASS
#define WRB_BOT_BASECLASS

#include "chess/board.h"
#include <string>
#include <vector>

class BotBase
{
public:
	virtual void handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name) = 0;
	virtual void handle_opponent_move_result(bool pieceCaptured, int square) = 0;
};

#endif
