#ifndef WRB_Game_Base
#define WRB_Game_Base

#include "../chess/board.h"
#include "GameHistory.h"
#include <vector>
#include <tuple>

#define SECONDS_INCREMENT 5
#define REVERSIBLE_MOVE_LIMIT 100


namespace WRB_Chess
{
	class GameBase
	{
	public:
		virtual std::vector<short> sense_actions() = 0;
		virtual std::vector<WRB_Chess::Move> move_actions() = 0;
		virtual double get_seconds_left() = 0;
		virtual void start() = 0;
		virtual short opponent_move_results() = 0;
		virtual std::vector<std::pair<short, WRB_Chess::ColorPiece>> sense(short) = 0;
		virtual std::tuple<WRB_Chess::Move, WRB_Chess::Move, short> move(WRB_Chess::Move) = 0;
		virtual void end_turn() = 0;
		virtual bool is_over() = 0;
		virtual WRB_Chess::Color get_winner_color() = 0;
		virtual WRB_Chess::Color turn() = 0;
		virtual WRB_Chess::GameHistory get_game_history() = 0;
	};
}

#endif
