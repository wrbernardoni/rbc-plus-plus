#include "chess/board.h"
#include <iostream>
#include <bitset>

#include <cstdlib> 
#include <ctime>

#include <fstream>
#include <string>

#include "utilities/RollingMap.h"

using namespace std;

#include "../utilities/json.hpp"
using json = nlohmann::json;

#include <Eigen/Dense>

#include <filesystem>
#include <ctime>

#include "../utilities/NeuralModel.h"

#include <list>
#include <algorithm>

#include "reconUtils/BotBase.h"

struct GameData
{
	vector<WRB_Chess::Bitboard> boards; // Boards by half move
	bool victor;
};

GameData extractGameData(json hist)
{
	GameData gData;

	if (hist.contains("game_history"))
		hist = hist["game_history"];

	if (!hist.contains("winner_color"))
		return gData;
	if (!hist.contains("fens_after_move"))
		return gData;


	gData.boards.push_back(WRB_Chess::Bitboard());

	gData.victor = hist["winner_color"];

	int wc = 0;
	int bc = 0;
	bool turn = true;
	while(true)
	{
		string fen;
		if (turn)
		{
			if (wc < hist["fens_after_move"]["true"].size())
			{
				fen = hist["fens_after_move"]["true"][wc];
				wc++;
			}
		}
		else
		{
			if (bc < hist["fens_after_move"]["false"].size())
			{
				fen = hist["fens_after_move"]["false"][bc];
				bc++;
			}
		}

		gData.boards.push_back(WRB_Chess::Bitboard(fen));

		turn = !turn;

		if ((wc >= hist["fens_after_move"]["true"].size()) && (bc >= hist["fens_after_move"]["false"].size()))
		{
			break;
		}
	}

	return gData;
};

// Gamma is the decay of reward parameter
#define GAMMA 0.8

double trainingGoal(WRB_Chess::NeuralModel& model, WRB_Chess::Bitboard& b, bool toMove)
{
	if (!b.KingsAlive())
	{
		if (b.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King) == 0)
			return -1;
		else if (b.Pieces(WRB_Chess::Color::Black, WRB_Chess::Piece::King) == 0)
			return 1;
	}

	if (toMove)
	{
		// White = Maximize
		double bestSuccessor = 0.0;
		std::vector<WRB_Chess::Move> mvs = b.AvailableMoves((WRB_Chess::Color)toMove);
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard testB = b;
			testB.ApplyMove(mvs[i]);
			double tempScore = model.runForward(testB, !toMove);

			if ((tempScore > bestSuccessor) || (i == 0))
			{
				bestSuccessor = tempScore;
			}
		}

		return GAMMA * bestSuccessor;
	}
	else
	{
		// Black = Minimize
		double bestSuccessor = 0.0;
		std::vector<WRB_Chess::Move> mvs = b.AvailableMoves((WRB_Chess::Color)toMove);
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard testB = b;
			testB.ApplyMove(mvs[i]);
			double tempScore = model.runForward(testB, !toMove);

			if ((tempScore < bestSuccessor) || (i == 0))
			{
				bestSuccessor = tempScore;
			}
		}

		return GAMMA * bestSuccessor;
	}
}

using namespace WRB_Chess;

class BotConstructor
{
public:
	virtual BotBase* createBot() = 0;
	virtual void destructBot(BotBase* b) { delete b; };
	virtual string getName() = 0;
};

#include "bots/inference.h"
#include "utilities/NeuralExpectimax.h"
#include "reconUtils/LocalGame.h"
#include "chess/board.h"

class NeuralConst : public BotConstructor
{
public:
	WRB_Chess::NeuralModel* m;
	string id;
	int wins;

	bool operator>(const NeuralConst& c)
	{
		return (wins > c.wins);
	}

	NeuralConst(string i, WRB_Chess::NeuralModel* md)
	{
		m = md;
		id = "Neural-";
		id += i;
	}

	~NeuralConst()
	{
		delete m;
	}

	BotBase* createBot() 
	{ 
		WRB_Chess::NeuralExpectimax* eng = new WRB_Chess::NeuralExpectimax(m, 100000,4);
		return new WRB_Bot::Inference(eng); 
	};
	string getName() { return id; };
	void destructBot(BotBase* b) 
	{ 
		delete ((WRB_Bot::Inference*)b)->engine; 
		delete b; 
	};
};

WRB_Chess::GameHistory PlayGame(int gameN, NeuralConst* whiteBot, NeuralConst* blackBot)
{
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


	if (game.get_winner_color() == WRB_Chess::Color::NoColor)
	{
		cout << "Draw!" << endl;
	}
	else if (game.get_winner_color() == WRB_Chess::Color::White)
	{
		cout << "White Wins!" << endl;
		whiteBot->wins++;
	}
	else
	{
		cout << "Black Wins!" << endl;
		blackBot->wins++;
	}

	cout << endl <<  gameHead << "Final Board State" << endl;
	cout << WRB_Chess::GetPrintable(game.getBoard()) << endl;

	WRB_Chess::GameHistory hist = game.get_game_history();

	bots[0]->handle_game_end(game.get_winner_color(), hist);
	bots[1]->handle_game_end(game.get_winner_color(), hist);

	cout << gameHead << "Deleting white bot." << endl;
	whiteBot->destructBot(bots[0]);
	cout << gameHead << "Deleting black bot." << endl;
	blackBot->destructBot(bots[1]);

	return hist;
}

bool compare (NeuralConst* struct1, NeuralConst* struct2)
{
    return (struct1->wins > struct2->wins);
}

int main(int argc, char* argv[])
{
	WRB_Chess::BoardHash::Init();

	if (argc < 2)
	{
		cout << "At least 1 arguments needed: call it like: evolve (path to models)" << endl;
		return 1;
	}

	vector<NeuralConst*> genes;

	string modelPath = argv[1];
	for (auto &p : filesystem::directory_iterator(modelPath))
    {
		if (p.path().extension() == ".model")
        {
        	cout << "Loading model " << p.path().stem().string() << endl;
        	WRB_Chess::NeuralModel* m = new WRB_Chess::NeuralModel(p.path().string());
        	genes.push_back(new NeuralConst(p.path().stem().string() + "_0", m));
        }
    }

    int keepTop = 10;
    list<vector<GameData>> tDat;
    int experienceReplay = 5;
    double lr = 0.001;
    while (true)
    {
    	cout << "Starting new round!" << endl << endl;
    	vector<GameData> games;
    	int gN = 1;
    	for (int i = 0; i < genes.size(); i++)
    	{
    		for (int j = i + 1; j < genes.size(); j++)
    		{
    			gN++;
    			cout << "Game " << gN << "/" << genes.size() * (genes.size()-1)/2 << endl;
    			WRB_Chess::GameHistory h = PlayGame(gN, genes[i], genes[j]);
    			games.push_back(extractGameData(h.j));
    		}
    	}
    	tDat.emplace_back(games);
    	if (tDat.size() > experienceReplay)
    		tDat.pop_front();

    	cout << "Pruning to top " << keepTop << endl;
    	std::sort(genes.begin(), genes.end(), compare);

    	for (int k = genes.size() - 1; k >= keepTop; k--)
    	{
    		delete genes[k];
    		genes.pop_back();
    	}

    	for (int i = 0; i < genes.size(); i++)
    	{
    		string fn = modelPath;
			fn += "/";
			fn += "GeneticPlayground-r";
			fn += to_string(i);
			fn += ".model";
			genes[i]->m->save(fn);
			genes[i]->wins = 0;
			cout << "Saving " << genes[i]->id << " as " << fn << endl;
    	}

    	cout << "Pruning complete." << endl;

    	cout << "Spawning children." << endl;

    	int top = genes.size();
    	for (int l = 0; l < top; l++)
    	{
    		WRB_Chess::NeuralModel* child = new WRB_Chess::NeuralModel(1,1);
    		(*child) = *(genes[l]->m);

    		cout << "Spawning child " << l << " / " << top << endl;
    		int trainN;
    		for (auto it = tDat.begin(); it != tDat.end(); it++)
    		{
    			for (int i = 0; i < it->size(); i++)
    			{
    				trainN++;
    				if ((rand() % 5) != 1)
    					continue;
    				cout << "Training on game " << trainN << "/" << experienceReplay * keepTop * (2 * keepTop - 1) << endl;

    				vector<WRB_Chess::Bitboard> brds;
					vector<bool> toMove;
					vector<double> scores;
    				if ((*it)[i].boards.size() == 0)
						continue;

					double finalScore = 0.0;
					if ((*it)[i].victor)
						finalScore = 1.0;
					else
						finalScore = -1.0;

					int k = (*it)[i].boards.size() - 1;
					brds.push_back((*it)[i].boards[k]);
					toMove.push_back((k + 1) % 2);
					scores.push_back(finalScore);
					double d = child->trainBatch(brds, toMove, scores, lr);
					double s = 1.0;

					for (k = (*it)[i].boards.size() - 2; k >= 0; k--)
					{
						brds.clear();
						toMove.clear();
						scores.clear();
						brds.push_back((*it)[i].boards[k]);
						toMove.push_back((k + 1) % 2);
						scores.push_back(trainingGoal((*child), (*it)[i].boards[k], (k + 1) % 2));
						d += child->trainBatch(brds, toMove, scores, lr);
						s += 1;
					}

					//double delta = child->trainBatch(brds, toMove, scores, lr);
					cout << "Child delta: " << d / s << endl;
    			}
    		}
    		char c = (char)('a' + rand() % 28);
    		string s = "";
    		s += c;
    		genes.push_back(new NeuralConst(genes[l]->id + s, child));
    	}
    };
}
