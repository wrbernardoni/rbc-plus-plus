#include "RemoteGame.h"

#include <chrono>
#include <thread>

#include "../utilities/json.hpp"
using json = nlohmann::json;

#include <iostream>

#define NUM_RETRIES 4
#define RETRY_DELAY std::chrono::milliseconds(1000)

WRB_Chess::RemoteGame::RemoteGame(httplib::Client* c, std::string server_url, int gameID, std::string user, std::string pass)
{
	gID = gameID;

	cli = c;

	std::string rt = "/api/games/" + std::to_string(gID) + "/color";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			if (rep["color"].get<bool>())
			{
				activeColor = WRB_Chess::Color::White;
			}
			else
			{
				activeColor = WRB_Chess::Color::Black;
			}
			break;
		} else {
			std::cout << "Error in RemoteGame Constructor" << std::endl;
			
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
}

std::string WRB_Chess::RemoteGame::get_opponent_name()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/opponent_name";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			return rep["opponent_name"].get<std::string>();
		} else {
			std::cout << "Error in RemoteGame get_opponent_name" << std::endl;

			if (i == NUM_RETRIES)
			{
				
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return "";
}

std::vector<short> WRB_Chess::RemoteGame::sense_actions()
{
	std::vector<short> senseAct;

	std::string rt = "/api/games/" + std::to_string(gID) + "/sense_actions";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			for (auto it = rep["sense_actions"].begin(); it != rep["sense_actions"].end(); it++)
			{
				senseAct.push_back(it->get<short>());
			}
			break;
		} else {
			std::cout << "Error in RemoteGame sense_actions" << std::endl;
			
			if (i == NUM_RETRIES)
			{
				
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return senseAct;
}

std::vector<WRB_Chess::Move> WRB_Chess::RemoteGame::move_actions()
{
	std::vector<WRB_Chess::Move> mvs;

	std::string rt = "/api/games/" + std::to_string(gID) + "/move_actions";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			for (auto it = rep["move_actions"].begin(); it != rep["move_actions"].end(); it++)
			{
				mvs.push_back(WRB_Chess::UCI_to_move((*it)["value"].get<std::string>()));
			}
			break;
		} else {
			std::cout << "Error in RemoteGame move_actions" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return mvs;
}

double WRB_Chess::RemoteGame::get_seconds_left()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/seconds_left";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			return rep["seconds_left"].get<double>();
		} else {
			std::cout << "Error in RemoteGame get_seconds_left" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return 0;
}

void WRB_Chess::RemoteGame::start()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/ready";
	cli->Post(rt.c_str());
}

short WRB_Chess::RemoteGame::opponent_move_results()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/opponent_move_results";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			if (rep["opponent_move_results"].is_null())
				return -1;
			else
				return rep["opponent_move_results"].get<short>();
		} else {
			std::cout << "Error in RemoteGame opponent_move_results" << std::endl;
			if (i == NUM_RETRIES)
				{
					std::cout << "Throwing error " << res.error() << std::endl;
					throw(res.error());
				}
				else
					std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return -1;
}

std::vector<std::pair<short, WRB_Chess::ColorPiece>> WRB_Chess::RemoteGame::sense(short s)
{
	json senseR;
	senseR["square"] = s;

	std::string rt = "/api/games/" + std::to_string(gID) + "/sense";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Post(rt.c_str(), senseR.dump(), "application/json")) {
			json rep = json::parse(res->body);
			
			std::vector<std::pair<short, WRB_Chess::ColorPiece>> sR;
	
			for (auto it = rep["sense_result"].begin(); it != rep["sense_result"].end(); it++)
			{
				short sQ = (*it)[0].get<short>();
				WRB_Chess::ColorPiece pA;
				if (!(*it)[1].is_null())
				{
					std::string vA = (*it)[1]["value"].get<std::string>();
	
					if (vA == "p")
					{
						pA.piece = WRB_Chess::Piece::Pawn;
						pA.color = WRB_Chess::Color::Black;
					}
					else if (vA == "b")
					{
						pA.piece = WRB_Chess::Piece::Bishop;
						pA.color = WRB_Chess::Color::Black;
					}
					else if (vA == "r")
					{
						pA.piece = WRB_Chess::Piece::Rook;
						pA.color = WRB_Chess::Color::Black;
					}
					else if (vA == "n")
					{
						pA.piece = WRB_Chess::Piece::Knight;
						pA.color = WRB_Chess::Color::Black;
					}
					else if (vA == "q")
					{
						pA.piece = WRB_Chess::Piece::Queen;
						pA.color = WRB_Chess::Color::Black;
					}
					else if (vA == "k")
					{
						pA.piece = WRB_Chess::Piece::King;
						pA.color = WRB_Chess::Color::Black;
					}
					else if (vA == "P")
					{
						pA.piece = WRB_Chess::Piece::Pawn;
						pA.color = WRB_Chess::Color::White;
					}
					else if (vA == "B")
					{
						pA.piece = WRB_Chess::Piece::Bishop;
						pA.color = WRB_Chess::Color::White;
					}
					else if (vA == "R")
					{
						pA.piece = WRB_Chess::Piece::Rook;
						pA.color = WRB_Chess::Color::White;
					}
					else if (vA == "N")
					{
						pA.piece = WRB_Chess::Piece::Knight;
						pA.color = WRB_Chess::Color::White;
					}
					else if (vA == "Q")
					{
						pA.piece = WRB_Chess::Piece::Queen;
						pA.color = WRB_Chess::Color::White;
					}
					else if (vA == "K")
					{
						pA.piece = WRB_Chess::Piece::King;
						pA.color = WRB_Chess::Color::White;
					}
				}
	
				sR.push_back(std::pair<short, WRB_Chess::ColorPiece>(sQ, pA));
			}
	
			return sR;
		} else {
			std::cout << "Error in RemoteGame sense" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return std::vector<std::pair<short, WRB_Chess::ColorPiece>>();
}

std::tuple<WRB_Chess::Move, WRB_Chess::Move, short> WRB_Chess::RemoteGame::move(WRB_Chess::Move m)
{
	std::string mString = WRB_Chess::move_to_UCI(m);

	json mJson;
	if (mString == "0000")
	{
		mJson["requested_move"] = nullptr;
	}
	else
	{
		mJson["requested_move"]["type"] = "Move";
		mJson["requested_move"]["value"] = mString;
	}

	std::string rt = "/api/games/" + std::to_string(gID) + "/move";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Post(rt.c_str(), mJson.dump(), "application/json")) {
			//std::cout << mJson << std::endl;
			//std::cout << res->body << std::endl;
			auto rep = json::parse(res->body);
			WRB_Chess::Move rmv;
			WRB_Chess::Move tmv;
			short cS = -1;
	
			if (!rep["move_result"][0].is_null())
			{
				rmv = UCI_to_move(rep["move_result"][0]["value"].get<std::string>());
			}
	
			if (!rep["move_result"][1].is_null())
			{
				tmv = UCI_to_move(rep["move_result"][1]["value"].get<std::string>());
			}
	
			if (!rep["move_result"][2].is_null())
			{
				cS = rep["move_result"][2].get<short>();
			}
	
			return std::tuple<WRB_Chess::Move, WRB_Chess::Move, short>(rmv, tmv, cS);
		} else {
			std::cout << "Error in RemoteGame move" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return std::tuple<WRB_Chess::Move, WRB_Chess::Move, short>();
}

void WRB_Chess::RemoteGame::end_turn()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/end_turn";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Post(rt.c_str())) {
		} else {
			std::cout << "Error in RemoteGame end_turn" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
}

void WRB_Chess::RemoteGame::error_resign()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/error_resign";
	cli->Post(rt.c_str());
}

bool WRB_Chess::RemoteGame::is_over()
{
	while (true)
	{
		std::string rt = "/api/games/" + std::to_string(gID) + "/game_status";
		for (int i = 0; i <= NUM_RETRIES; i++)
		{
			if (auto res = cli->Get(rt.c_str())) {
				auto rep = json::parse(res->body);
				if (rep["is_my_turn"].get<bool>())
					return false;

				if (rep["is_over"].get<bool>())
					return true;
				
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				break;
			} else {
				std::cout << "Error in RemoteGame is_over" << std::endl;
				if (i == NUM_RETRIES)
				{
					std::cout << "Throwing error " << res.error() << std::endl;
					throw(res.error());
				}
				else
					std::this_thread::sleep_for(RETRY_DELAY);
			}
		}
	}
	

	return false;
}

WRB_Chess::Color WRB_Chess::RemoteGame::get_winner_color()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/winner_color";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			if (rep["winner_color"].is_null())
			{
				return WRB_Chess::Color::NoColor;
			}
			else if (rep["winner_color"].get<bool>())
				return WRB_Chess::Color::White;
			else
				return WRB_Chess::Color::Black;
		} else {
			std::cout << "Error in RemoteGame get_winner_color" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}
	return WRB_Chess::Color::NoColor;
}

WRB_Chess::Color WRB_Chess::RemoteGame::turn()
{
	std::string rt = "/api/games/" + std::to_string(gID) + "/game_status";
	for (int i = 0; i<= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			if (rep["is_my_turn"].get<bool>())
				return activeColor;
			else
				return OPPOSITE_COLOR(activeColor);
		} else {
			std::cout << "Error in RemoteGame turn" << std::endl;
			if (i == NUM_RETRIES)
			{
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}

	return WRB_Chess::Color::NoColor;
}

WRB_Chess::GameHistory WRB_Chess::RemoteGame::get_game_history()
{
	WRB_Chess::GameHistory g;

	std::vector<short> senseAct;

	std::string rt = "/api/games/" + std::to_string(gID) + "/game_history";
	for (int i = 0; i <= NUM_RETRIES; i++)
	{
		if (auto res = cli->Get(rt.c_str())) {
			auto rep = json::parse(res->body);
			g.j = rep;
			break;
		} else {
			std::cout << "Error in RemoteGame get_game_history" << std::endl;
			
			if (i == NUM_RETRIES)
			{
				
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			else
				std::this_thread::sleep_for(RETRY_DELAY);
		}
	}

	return g;
}
