#include <iostream>
#include <string>

#include <thread>
#include <unordered_map>

#include <exception>
#include <typeinfo>
#include <stdexcept>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "http/httplib.h"

#include "reconUtils/RemoteGame.h"
#include "chess/board.h"
#include "bots/inference.h"

#include "utilities/ExpectimaxEngine.h"

#include "../utilities/json.hpp"
using json = nlohmann::json;


using namespace std;

void PlayGame(httplib::Client* cli, std::string server_url, int invite, std::string user, std::string pass, bool* finished)
{
	*finished = false;
	cout << "Accepting invite to game " << invite << endl;
	int gameID = -1;

	cout << "Accepting" << endl;
	std::string acceptURI = "/api/invitations/" + to_string(invite);
	if (auto res = cli->Post(acceptURI.c_str())) {
		auto rep = json::parse(res->body);
		gameID = rep["game_id"].get<int>();
	} else {
		throw(res.error());
	}

	try
	{
		

		cout << "Instantiating game " << gameID << endl;
		WRB_Chess::RemoteGame game(cli, server_url, gameID, user, pass);
	
		cout << "Game instantiated" << endl;
		WRB_Chess::Expectimax engine(10);
		WRB_Bot::Inference bot(&engine);
	
		WRB_Chess::Color mColor = game.getColor();
		std::string opponent_name = game.get_opponent_name();

		string gameHead = "[" + to_string(gameID) + "|" + opponent_name + "]";
		bot.setPrefix(gameHead + "\t\t");

		cout << "\tPlaying " << (mColor == 0? "White" : "Black") << " versus " << opponent_name << endl;
		bot.handle_game_start(mColor, WRB_Chess::Bitboard(), opponent_name);
		game.start();

		int turnCount = 0;
		while(!game.is_over())
		{
			double timeLeft = game.get_seconds_left();
			cout << endl << gameHead << "\t" << turnCount << " " << (mColor == 0? "White" : "Black") << " turn starts: " << timeLeft << " seconds remaining." << endl;

			auto senseActions = game.sense_actions();
			auto moveActions = game.move_actions();
			short captureSquare = game.opponent_move_results();
			bool wasCaptured = (captureSquare >= 0);
			if (wasCaptured)
				cout << gameHead << "\t" << "Piece was taken at " << WRB_Chess::SquareNames[captureSquare] << endl;
			bot.handle_opponent_move_result(wasCaptured, captureSquare);

			short senseChoice = bot.choose_sense(senseActions, moveActions, game.get_seconds_left());
			if (senseChoice != -1)
			{
				cout << gameHead << "\t" << "Sensing square " << WRB_Chess::SquareNames[senseChoice] << endl;
			}
			else
			{
				cout << gameHead << "\t" << "No sense." << endl;
			}
			
			auto senseResult = game.sense(senseChoice);
			bot.handle_sense_result(senseResult);
			auto mvChoice = bot.choose_move(moveActions, game.get_seconds_left());
			auto mvResult = game.move(mvChoice);
			cout << gameHead << "\t" << "Attempting move " << WRB_Chess::move_to_UCI(mvChoice) << endl;

			game.end_turn();

			bot.handle_move_result(get<0>(mvResult), get<1>(mvResult), (get<2>(mvResult) >= 0), get<2>(mvResult));
			cout << gameHead << "\t" << "Took move " << WRB_Chess::GetPrintable(get<1>(mvResult));
			if (get<2>(mvResult) == -1)
			{
				cout << endl;
			}
			else
			{
				cout << " captured a piece at " << WRB_Chess::SquareNames[get<2>(mvResult)] << endl;
			}

			turnCount += 1;
		}

		cout << "Game " << gameID << " over! ";
		if (game.get_winner_color() == WRB_Chess::Color::NoColor)
		{
			cout << "Draw!" << endl;
		}
		else if (game.get_winner_color() == mColor)
		{
			cout << "We win!" << endl;
		}
		else
		{
			cout << "That's a loss D;" << endl;
		}

		bot.handle_game_end(game.get_winner_color(), game.get_game_history());
	}
	catch(const std::exception &e)
	{
		cout << "Exception: " << e.what() << endl;
		cout << "Error caught. Resigning." << endl;

		std::string finishURI = "/api/games/" + to_string(gameID) + "/error_resign";
		cli->Post(finishURI.c_str());
	}
	catch (...)
	{
		cout << "Error" << endl;
		std::string finishURI = "/api/games/" + to_string(gameID) + "/error_resign";
		cli->Post(finishURI.c_str());
	}

	cout << "Closing invite " << invite << endl;

	std::string finishURI = "/api/invitations/" + to_string(invite) + "/finish";
	cli->Post(finishURI.c_str());
	*finished = true;
}

struct InviteThread
{
	thread* t;
	bool* end;
};


int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "At least 3 arguments needed: call it like: connect user pass" << endl;
		return 1;
	}
	WRB_Chess::BoardHash::Init();
	// TODO Make these cmd line args
	std::string server_url = "https://rbc.jhuapl.edu";
	std::string user = argv[1];
	std::string pass = argv[2];
	int concurrent_games = 4;
	bool ranked = true;
	bool newVersion = false;

	httplib::Client cli(server_url);
	cli.set_basic_auth(user.c_str(), pass.c_str());
	cli.enable_server_certificate_verification(false);
	cli.set_follow_location(true);
	cli.set_connection_timeout(1800, 0);

	// Set ranked
	cout << "Setting ranked flag." << endl;
	json rankedReq;
	rankedReq["ranked"] = ranked;
	if (auto res = cli.Post("/api/users/me/ranked", rankedReq.dump(), "application/json")) {
		auto rep = json::parse(res->body);
		cout << "\t" << rep["username"].get<string>() << " ranked flag: " << (rep["ranked"].get<bool>()? "True" : "False") << endl;
	} else {
		std::cout << "Throwing error " << res.error() << std::endl;
		throw(res.error());
	}

	// Set new version
	if (newVersion)
	{
		cout << "Creating new version." << endl;
		if (auto res = cli.Post("/api/users/me/version")) {
			auto rep = json::parse(res->body);
			cout << "\t" << rep["username"].get<string>() << " now has version number " << rep["version"].get<int>() << endl;
		} else {
			std::cout << "Throwing error " << res.error() << std::endl;
			throw(res.error());
		}
	}

	// Loop and take invites
	bool connected = false;
	unordered_map<int, InviteThread> games;
	while(true)
	{
		try
		{
			
			// If pinging is needed to keep available
			/*
			if (auto res = cli.Post("/api/users/me")) {
			} else {
				std::cout << "Throwing error " << res.error() << std::endl;
				throw(res.error());
			}
			*/
			

			if (!connected)
			{
				json maxG;
				maxG["max_games"] = concurrent_games;
				if (auto res = cli.Post("/api/users/me/max_games", maxG.dump(), "application/json")) {
					cout << "Successfully connected to the server." << endl;
					connected = true;
				} else {
					std::cout << "Throwing error " << res.error() << std::endl;
					throw(res.error());
				}
			}

			bool looped = false;
			while (!looped)
			{
				looped = true;
				for (auto it = games.begin(); it != games.end(); it++)
				{
					if (*it->second.end)
					{
						looped = false;
						it->second.t->join();
						delete it->second.end;
						delete it->second.t;
						games.erase(it->first);

						//cout << "Slot freed" << endl;
						break;
					}	
				}
			}
			
			if (games.size() < concurrent_games)
			{
				// Search for new invites
				if (auto res = cli.Get("/api/invitations")) {
					auto invites = json::parse(res->body);
					//cout << "Available invites: " << invites << endl;

					for (auto it = invites["invitations"].begin(); it != invites["invitations"].end(); it++)
					{
						if (games.size() >= concurrent_games)
							break;

						int inviteID = it->get<int>();
						if (games.count(inviteID) <= 0)
						{
							// New invite to add;
							cout << endl << "Creating thread to accept invite " << inviteID << endl;
							bool* newTracker = new bool;
							(*newTracker) = false;
							thread* newThread = new thread(PlayGame, &cli, server_url, inviteID, user, pass, newTracker);
							InviteThread iT;
							iT.t = newThread;
							iT.end = newTracker;
							games[inviteID] = iT;
						}
					}

				} else {
					std::cout << "Throwing error " << res.error() << std::endl;
					throw(res.error());
				}
			}
		}
		catch(const std::exception &e)
		{
			cout << "Exception: " << e.what() << endl;
			cout << "Error caught. Reconnecting." << endl;
			connected = false;
		}
		catch (...)
		{
			cout << "Error" << endl;
			connected = false;
		}

		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

	return 0;
}
