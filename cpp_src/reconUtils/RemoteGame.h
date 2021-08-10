#ifndef WRB_Game_Remote
#define WRB_Game_Remote

#include "GameBase.h"
#include <string>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "../http/httplib.h"


namespace WRB_Chess
{
	class RemoteGame : public GameBase
	{
	private:
		httplib::Client* cli;
		WRB_Chess::Color activeColor;
		int gID;
	public:
		RemoteGame(httplib::Client*, std::string server_url, int gameID, std::string user, std::string pass);
		inline WRB_Chess::Color getColor() { return activeColor; };
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
		WRB_Chess::Color turn();
		WRB_Chess::GameHistory get_game_history();

		std::string get_opponent_name();
		void error_resign();
	};
}

#endif