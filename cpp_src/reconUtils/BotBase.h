#ifndef WRB_BOT_BASECLASS
#define WRB_BOT_BASECLASS

#include "chess/board.h"
#include "GameHistory.h"
#include <string>
#include <vector>

namespace WRB_Chess
{
	class BotBase
	{
	public:
		virtual void handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name) {};
		virtual void handle_opponent_move_result(bool pieceCaptured, int square) {};
		virtual short choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left) = 0;
		virtual void handle_sense_result(std::vector<std::pair<short, WRB_Chess::ColorPiece>>) {};
		virtual WRB_Chess::Move choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left) = 0;
		virtual void handle_move_result(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, bool captured_opponent_piece, short capture_square) {};
		virtual void handle_game_end(WRB_Chess::Color winner_color, WRB_Chess::GameHistory game_history) {};
	};
}

#endif
