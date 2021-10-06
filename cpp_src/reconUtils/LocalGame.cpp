#include "LocalGame.h"

WRB_Chess::LocalGame::LocalGame(double seconds_per_player)
{
	this->whiteSeconds = seconds_per_player;
	this->blackSeconds = seconds_per_player;
	currentTurn = WRB_Chess::Color::White;
	opponentMvRes = -1;

	hist.j["fens_after_move"]["false"] = json::array();
	hist.j["fens_after_move"]["true"] = json::array();
	hist.j["fens_before_move"]["false"] = json::array();
	hist.j["fens_before_move"]["true"] = json::array();
}

std::vector<short> WRB_Chess::LocalGame::sense_actions()
{
	std::vector<short> senseAct;
	for (int i = 0; i < 64; i++)
	{
		senseAct.push_back(i);
	}
	return senseAct;
}

std::vector<WRB_Chess::Move> WRB_Chess::LocalGame::move_actions()
{
	return this->board.AvailableMoves(currentTurn);
}

double WRB_Chess::LocalGame::get_seconds_left()
{
	if (this->currentTurn == WRB_Chess::Color::White)
		return this->whiteSeconds - ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - turnStart)).count();
	else
		return this->blackSeconds - ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - turnStart)).count();
}

void WRB_Chess::LocalGame::start()
{
	turnStart = std::chrono::steady_clock::now();
}

short WRB_Chess::LocalGame::opponent_move_results()
{
	return opponentMvRes;
}

std::vector<std::pair<short, WRB_Chess::ColorPiece>> WRB_Chess::LocalGame::sense(short s)
{
	return board.sense(s);
}

std::tuple<WRB_Chess::Move, WRB_Chess::Move, short> WRB_Chess::LocalGame::move(WRB_Chess::Move m)
{
	std::tuple<WRB_Chess::Move, WRB_Chess::Move, short> mv;
	bool capture;
	short cS;
	std::string turn = (currentTurn == WRB_Chess::White ? "true" : "false");
	hist.j["fens_before_move"][turn].push_back(board.fen());
	WRB_Chess::Move taken = board.ApplyMove(m, capture, cS);
	hist.j["fens_after_move"][turn].push_back(board.fen());

	opponentMvRes = cS;
	
	return std::tuple<WRB_Chess::Move, WRB_Chess::Move, short>(m, taken, cS);
}

void WRB_Chess::LocalGame::end_turn()
{
	if (currentTurn == WRB_Chess::Color::White)
	{
		currentTurn = WRB_Chess::Color::Black;
		this->whiteSeconds -= ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - turnStart)).count();
	}
	else
	{
		currentTurn = WRB_Chess::Color::White;
		this->blackSeconds -= ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - turnStart)).count();
	}

	turnStart = std::chrono::steady_clock::now();
}

bool WRB_Chess::LocalGame::is_over()
{
	if ((this->whiteSeconds <= 0) || (this->blackSeconds <= 0))
		return true;
	if ((this->board.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King) == 0) || (this->board.Pieces(WRB_Chess::Color::Black, WRB_Chess::Piece::King) == 0))
		return true;

	return false;
}

WRB_Chess::Color WRB_Chess::LocalGame::get_winner_color()
{
	if (this->whiteSeconds <= 0)
		return WRB_Chess::Color::Black;
	else if (this->blackSeconds <= 0)
		return WRB_Chess::Color::White;
	else if (this->board.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King) == 0)
		return WRB_Chess::Color::Black;
	else if (this->board.Pieces(WRB_Chess::Color::Black, WRB_Chess::Piece::King) == 0)
		return WRB_Chess::Color::White;

	return WRB_Chess::Color::NoColor;
}

WRB_Chess::GameHistory WRB_Chess::LocalGame::get_game_history()
{
	WRB_Chess::Color winner = get_winner_color();
	if (winner != WRB_Chess::Color::NoColor)
		hist.j["winner_color"] = (winner == WRB_Chess::Color::White);
	return hist;
}
