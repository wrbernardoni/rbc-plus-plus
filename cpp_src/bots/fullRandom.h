#ifndef WRB_BOT_FULLRANDOM
#define WRB_BOT_FULLRANDOM

#include "reconUtils/BotBase.h"

namespace WRB_Bot
{
	class FullRandom : public WRB_Chess::BotBase
	{
	public:
		void handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name);
		void handle_opponent_move_result(bool pieceCaptured, int square) {};
		short choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left);
		void handle_sense_result(std::vector<std::pair<short, WRB_Chess::ColorPiece>>) {};
		WRB_Chess::Move choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left);
		void handle_move_result(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, bool captured_opponent_piece, short capture_square) {};
		void handle_game_end(WRB_Chess::Color winner_color, WRB_Chess::GameHistory game_history) {};
	};
}
#endif
