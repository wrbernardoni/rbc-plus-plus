#ifndef WRB_Game_Local
#define WRB_Game_Local

#include "GameBase.h"
#include <chrono>

namespace WRB_Chess
{
	class LocalGame : public GameBase
	{
	private:
		WRB_Chess::Bitboard board;
		double whiteSeconds;
		double blackSeconds;
		WRB_Chess::Color currentTurn;
		std::chrono::time_point<std::chrono::steady_clock> turnStart;
		short opponentMvRes;
		GameHistory hist;
	public:
		LocalGame(double seconds_per_player);
		std::vector<short> sense_actions();
		std::vector<WRB_Chess::Move> move_actions();
		double get_seconds_left();
		void start();
		short opponent_move_results();
		std::vector<std::pair<short, WRB_Chess::ColorPiece>> sense(short);
		std::tuple<WRB_Chess::Move, WRB_Chess::Move, short> move(WRB_Chess::Move);
		void end_turn();
		bool is_over();
		WRB_Chess::Color get_winner_color();
		WRB_Chess::Color turn() { return currentTurn; };
		WRB_Chess::GameHistory get_game_history();
		WRB_Chess::Bitboard getBoard() { return board; };
	};
}

#endif