#include <iostream>
#include <string>
#include <cstdlib>

#include <thread>
#include <unordered_map>

#include <exception>
#include <typeinfo>
#include <stdexcept>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "http/httplib.h"

#include "reconUtils/LocalGame.h"
#include "chess/board.h"

#include "utilities/json.hpp"
using json = nlohmann::json;

using namespace WRB_Chess;
using namespace std;

#include "reconUtils/BotBase.h"

#include <ctime>

#include <filesystem>

class BotConstructor
{
public:
	virtual BotBase* createBot() = 0;
	virtual void destructBot(BotBase* b) { delete b; };
	virtual string getName() = 0;
};


void PlayGame(httplib::Client* cli, int gameN, BotConstructor* whiteBot, BotConstructor* blackBot, bool* finished)
{
	*finished = false;

	WRB_Chess::LocalGame game(900.0);

	WRB_Chess::BotBase* bots[2];
	bots[0] = whiteBot->createBot();
	string whiteBName = whiteBot->getName();
	bots[1] = blackBot->createBot();
	string blackBName = blackBot->getName();

	string gameHead = "[" + to_string(gameN) + "|" + whiteBName + "|" + blackBName + "]\t";
	bots[0]->setPrefix(gameHead + "\t");
	bots[1]->setPrefix(gameHead + "\t");

	double turnCount = 0;
	short activeTurn = 0;
	try
	{
		game.start();
		bots[0]->handle_game_start(WRB_Chess::Color::White, WRB_Chess::Bitboard(), blackBName);
		activeTurn = 1;
		bots[1]->handle_game_start(WRB_Chess::Color::Black, WRB_Chess::Bitboard(), whiteBName);
		while(!game.is_over())
		{
			activeTurn = (short)game.turn();
			std::chrono::time_point<std::chrono::steady_clock> turnStart = std::chrono::steady_clock::now();
			cout << endl << endl;
			cout << gameHead <<  turnCount << " " << (activeTurn == 0? "White: " + whiteBName : "Black: "  + blackBName) << " turn starts: " << game.get_seconds_left() << " seconds remaining." << endl;
			cout << gameHead << "Board State" << endl;
			cout << WRB_Chess::GetPrintable(game.getBoard());
			cout << "fen: " << game.getBoard().fen() << endl;
			auto senseActions = game.sense_actions();
			auto moveActions = game.move_actions();
	
			short captureSquare = game.opponent_move_results();
			bool wasCaptured = (captureSquare >= 0);
			bots[activeTurn]->handle_opponent_move_result(wasCaptured, captureSquare);
	
			short senseChoice = bots[activeTurn]->choose_sense(senseActions, moveActions, game.get_seconds_left());
			if (senseChoice != -1)
			{
				cout << gameHead << "Sensing square " << WRB_Chess::SquareNames[senseChoice] << endl;
			}
			else
			{
				cout << gameHead <<  "No sense." << endl;
			}
			
			auto senseResult = game.sense(senseChoice);
			bots[activeTurn]->handle_sense_result(senseResult);
			auto mvChoice = bots[activeTurn]->choose_move(moveActions, game.get_seconds_left());
			auto mvResult = game.move(mvChoice);
			cout << gameHead <<  "Attempting move " << WRB_Chess::GetPrintable(mvChoice) << endl;
			bots[activeTurn]->handle_move_result(get<0>(mvResult), get<1>(mvResult), (get<2>(mvResult) >= 0), get<2>(mvResult));
			cout << gameHead <<  "Took move " << WRB_Chess::GetPrintable(get<1>(mvResult));
			if (get<2>(mvResult) == -1)
			{
				cout << endl;
			}
			else
			{
				cout << " captured a piece at " << WRB_Chess::SquareNames[get<2>(mvResult)] << endl;
			}

			cout << ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - turnStart)).count() << " seconds taken." << endl;

			game.end_turn();
			turnCount += 0.5;
		}
	
		cout << gameHead <<  "Game over! ";

		json sendJ;
		sendJ["white"] = whiteBName;
		sendJ["black"] = blackBName;

		if (game.get_winner_color() == WRB_Chess::Color::NoColor)
		{
			cout << "Draw!" << endl;
			sendJ["winner"] = "";
		}
		else if (game.get_winner_color() == WRB_Chess::Color::White)
		{
			cout << "White Wins!" << endl;
			sendJ["winner"] = whiteBName;
		}
		else
		{
			cout << "Black Wins!" << endl;
			sendJ["winner"] = blackBName;
		}
	
		cout << endl <<  gameHead << "Final Board State" << endl;
		cout << WRB_Chess::GetPrintable(game.getBoard()) << endl;

		WRB_Chess::GameHistory hist = game.get_game_history();
		std::string fName = "";
		fName += to_string(std::time(nullptr));
		fName += "-";
		fName += whiteBName;
		fName += "-";
		fName += blackBName;
		fName += ".json";

		// std::fstream outputFile(fName.c_str(), fstream::out);
		// if (!outputFile.is_open())
		// {
		// 	cout << "Error opening file!\n!!!\n\n";
		// }
		// else
		// {
		// 	cout << "Outputting game history to file " << fName << endl;
		// 	outputFile << hist.j.dump() << std::endl;
		// 	outputFile.flush();
		// 	outputFile.close();
		// }		
	
		bots[0]->handle_game_end(game.get_winner_color(), hist);
		bots[1]->handle_game_end(game.get_winner_color(), hist);


		cli->Post("/", sendJ.dump(), "application/json");

		cout << gameHead << "Deleting white bot." << endl;
		whiteBot->destructBot(bots[0]);
		cout << gameHead << "Deleting black bot." << endl;
		blackBot->destructBot(bots[1]);
	}
	catch (...)
	{
		cout << "Error caught" << endl;
		json sendJ;
		sendJ["white"] = whiteBName;
		sendJ["black"] = blackBName;
		if (activeTurn == 0)
		{
			sendJ["winner"] = blackBName;
		}
		else
		{
			sendJ["winner"] = whiteBName;
		}
		cli->Post("/", sendJ.dump(), "application/json");
	}
	*finished = true;
}

struct GameThread
{
	thread* t;
	bool* end;
};


#include "bots/fullRandom.h"

class RandomBotConst : public BotConstructor
{
	BotBase* createBot() { return new WRB_Bot::FullRandom(); };
	string getName() { return "Random"; };
};

#include "bots/inference.h"
#include "utilities/UniformExpectimaxEngine.h"

class UniformExpectimaxConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::UniformExpectimax* eng = new WRB_Chess::UniformExpectimax(10, 100000);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "UniformExpectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/ExpectimaxEngine.h"
#include "utilities/ExpectimaxEngineMT.h"

class ExpectimaxConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::Expectimax* eng = new WRB_Chess::Expectimax(10, 100000);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "Expectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

class ExpectimaxMTConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::ExpectimaxMT* eng = new WRB_Chess::ExpectimaxMT(10, 100000, 4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "Expectimax_MT"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};


#include "utilities/ShannonExpectimax.h"

class ShannonExpectimaxConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::ShannonExpectimax* eng = new WRB_Chess::ShannonExpectimax(100000,4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "S_Expectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/MonteShannonExpectimax.h"

class MonteShannonExpectimaxConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::MonteShannonExpectimax* eng = new WRB_Chess::MonteShannonExpectimax(10,10, 100000,4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "MS_Expectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/MoveProbabilityOld.h"

class ExpectimaxMPOldConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::Expectimax* eng = new WRB_Chess::Expectimax(10, 100000);
		return new WRB_Bot::Inference(eng, new WRB_Chess::OldMoveProbability()); 
	};
	string getName() { return "Expectimax_MPO"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/MoveProbabilityOld.h"

class ExpectimaxMPOldMTConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::ExpectimaxMT* eng = new WRB_Chess::ExpectimaxMT(10, 100000,4);
		return new WRB_Bot::Inference(eng, new WRB_Chess::OldMoveProbability()); 
	};
	string getName() { return "Expectimax_MPO_MT"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};


class ShannonExpectimaxMPOldConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::ShannonExpectimax* eng = new WRB_Chess::ShannonExpectimax(100000,4);
		return new WRB_Bot::Inference(eng, new WRB_Chess::OldMoveProbability()); 
	};
	string getName() { return "S_Expectimax_MPO"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

class MonteShannonExpectimaxMPOldConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::MonteShannonExpectimax* eng = new WRB_Chess::MonteShannonExpectimax(10,10, 100000,4);
		return new WRB_Bot::Inference(eng, new WRB_Chess::OldMoveProbability()); 
	};
	string getName() { return "MS_Expectimax_MPO"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};


#include "utilities/MonteShannon2Expectimax.h"

class MonteShannon2ExpectimaxConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::MonteShannon2Expectimax* eng = new WRB_Chess::MonteShannon2Expectimax(10,10, 100000,4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "MS2_Expectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

class MonteShannon2ExpectimaxMPOldConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::MonteShannon2Expectimax* eng = new WRB_Chess::MonteShannon2Expectimax(10,10, 100000,4);
		return new WRB_Bot::Inference(eng, new WRB_Chess::OldMoveProbability()); 
	};
	string getName() { return "MS2_Expectimax_MPO"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/Shannon2Expectimax.h"

class Shannon2ExpectimaxConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::Shannon2Expectimax* eng = new WRB_Chess::Shannon2Expectimax(100000,4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "S2_Expectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

class Shannon2ExpectimaxMPOldConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::Shannon2Expectimax* eng = new WRB_Chess::Shannon2Expectimax(100000,4);
		return new WRB_Bot::Inference(eng, new WRB_Chess::OldMoveProbability()); 
	};
	string getName() { return "S2_Expectimax_MPO"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};


#include "utilities/MinBoardsScanEngine.h"
class MinScanConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::MinBoardsScanEngine* eng = new WRB_Chess::MinBoardsScanEngine();
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "MinBoardsScan"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/AlphaBetaExpectimax.h"
class AlphaBetaConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::AlphaBetaExpectimax* eng = new WRB_Chess::AlphaBetaExpectimax(5, 2, 1, 10000000, 6);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "AlphaBetaExpectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

#include "utilities/StockyExpectimax.h"
class StockyConst : public BotConstructor
{
	BotBase* createBot() 
	{ 
		WRB_Chess::StockyExpectimax* eng = new WRB_Chess::StockyExpectimax(5, 10000000, 8, "../../Stockfish/src/stockfish.exe");
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return "StockyExpectimax"; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};


#include "utilities/NeuralExpectimax.h"
class NeuralConst : public BotConstructor
{
public:
	string mP;
	string id;

	NeuralConst(string s, string i)
	{
		mP = s;
		id = "Neural-";
		id += i;
	}

	BotBase* createBot() 
	{ 
		WRB_Chess::NeuralModel* mdl = new WRB_Chess::NeuralModel(mP);
		WRB_Chess::NeuralExpectimax* eng = new WRB_Chess::NeuralExpectimax(mdl, 100000,4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return id; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Chess::NeuralExpectimax*)(((WRB_Bot::Inference*)b)->engine))->model;
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Need at least one argument: LocalTournament #concurrentgames" << endl;
		return 1;
	}

	vector<BotConstructor*> bots;
	bots.push_back(new RandomBotConst());
	bots.push_back(new UniformExpectimaxConst());
	//bots.push_back(new MinScanConst());
	// bots.push_back(new ExpectimaxConst());
	// bots.push_back(new ExpectimaxMPOldConst());
	bots.push_back(new ExpectimaxMTConst());
	bots.push_back(new ExpectimaxMPOldMTConst());
	bots.push_back(new ShannonExpectimaxConst());
	bots.push_back(new MonteShannonExpectimaxConst());
	bots.push_back(new MonteShannonExpectimaxMPOldConst());
	bots.push_back(new ShannonExpectimaxMPOldConst());
	bots.push_back(new MonteShannon2ExpectimaxConst());
	bots.push_back(new MonteShannon2ExpectimaxMPOldConst());
	bots.push_back(new Shannon2ExpectimaxConst());
	bots.push_back(new Shannon2ExpectimaxMPOldConst());
	// bots.push_back(new AlphaBetaConst());
	bots.push_back(new StockyConst());

	WRB_Chess::BoardHash::Init();
	// TODO Make these cmd line args
	int concurrent_games = stoi(argv[1]);

	string server = "localhost:3000";
	if (argc > 2)
	{
		server = argv[2];
	}

	if (argc > 3)
	{
		string modelPath = argv[3];
		for (auto &p : filesystem::directory_iterator(modelPath))
	    {
			if (p.path().extension() == ".model")
	        {
	        	bots.push_back(new NeuralConst(p.path().string(), p.path().stem().string()));
	        }
	    }
	}

	httplib::Client cli(server.c_str());
	cli.enable_server_certificate_verification(false);
	cli.set_follow_location(true);
	cli.set_connection_timeout(1800, 0);

	unordered_map<int, GameThread> games;
	int gameN = 0;

	vector<int> gamesInRound;
	while(true)
	{
		try
		{
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
			
			while (games.size() < concurrent_games)
			{
				if (gamesInRound.size() == 0)
				{
					cout << "New round begun." << endl;
					for (int i = 0; i < (bots.size() * (bots.size() - 1)); i++)
					{
						gamesInRound.push_back(i);
					}
				}

				cout << gamesInRound.size() << " games remaining in round." << endl;

				int choice = rand() % gamesInRound.size();
				int gameSet = gamesInRound[choice];
				gamesInRound.erase(gamesInRound.begin() + choice);
				// Create new games
				//void PlayGame(httplib::Client* cli, int gameN, BotConstructor* whiteBot, BotConstructor* blackBot, bool* finished)
				int botW = gameSet / (bots.size() - 1);
				int botB = gameSet % (bots.size() - 1);
				if (botB >= botW)
				{
					botB += 1;
				}

				gameSet += 1;
				gameSet = gameSet % (bots.size() * (bots.size() - 1));

				cout << endl << "Beginning game " << gameN << endl;
				bool* newTracker = new bool;
				(*newTracker) = false;
				thread* newThread = new thread(PlayGame, &cli, gameN, bots[botW], bots[botB], newTracker);
				GameThread iT;
				iT.t = newThread;
				iT.end = newTracker;
				games[gameN] = iT;
				gameN += 1;
			}
		}
		catch(const std::exception &e)
		{
			cout << "Exception: " << e.what() << endl;
		}
		catch (...)
		{
			cout << "Error" << endl;
		}

		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	return 0;
}
