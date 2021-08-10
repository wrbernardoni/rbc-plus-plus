#ifndef WRB_BOT_INFERENCE
#define WRB_BOT_INFERENCE

#include "../reconUtils/BotBase.h"
#include "../utilities/BoardManager.h"
#include "../utilities/EngineBase.h"

#include <set>

namespace WRB_Bot
{
	class Inference : public WRB_Chess::BotBase
	{
	private:
		WRB_Chess::BoardManager boards;
		WRB_Chess::Color c;
		unsigned int turnCount;

		std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash> movePolicy;
		WRB_Chess::Move mv;
	public:
		WRB_Chess::EngineBase* engine;
		Inference(WRB_Chess::EngineBase*);
		void handle_game_start(WRB_Chess::Color color, WRB_Chess::Bitboard board, std::string opponent_name);
		void handle_opponent_move_result(bool pieceCaptured, int square);
		short choose_sense(std::vector<short> sense_actions, std::vector<WRB_Chess::Move> move_actions, double seconds_left);
		void handle_sense_result(std::vector<std::pair<short, WRB_Chess::ColorPiece>>);
		WRB_Chess::Move choose_move(std::vector<WRB_Chess::Move> move_actions, double seconds_left);
		void handle_move_result(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, bool captured_opponent_piece, short capture_square);
		void handle_game_end(WRB_Chess::Color winner_color, WRB_Chess::GameHistory game_history);
	};
}
#endif
