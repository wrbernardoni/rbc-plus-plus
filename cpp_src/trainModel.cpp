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

struct GameData
{
	vector<WRB_Chess::Bitboard> boards; // Boards by half move
	bool victor;
};

GameData extractGameData(string fn)
{
	GameData gData;
	ifstream file(fn);
	json hist;
	file >> hist;
	file.close();

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

int main(int argc, char* argv[])
{
	WRB_Chess::BoardHash::Init();

	if (argc < 2)
	{
		cout << "At least 1 arguments needed: call it like: train operation(new|partition)" << endl;
		return 1;
	}

	string operation = argv[1];

	if (operation == "partition" || operation == "P")
	{
		if (argc < 3)
		{
			cout << "train partition requires an additional argument, input the folder location of training data." << endl;
			return 1;
		}

		string dataFolder = argv[2];

		string trainPath = dataFolder + "/train/";
		if (!filesystem::exists(trainPath))
		{
			filesystem::create_directory(trainPath);
		}
		string testPath = dataFolder + "/test/";
		if (!filesystem::exists(testPath))
		{
			filesystem::create_directory(testPath);
		}
		string validatePath = dataFolder + "/validate/";
		if (!filesystem::exists(validatePath))
		{
			filesystem::create_directory(validatePath);
		}

		string ext = ".json";
		for (auto &p : filesystem::directory_iterator(dataFolder))
	    {
	        if (p.path().extension() == ext)
	        {
	        	int diversion = rand() % 7;
	        	if (diversion == 0)
	        	{
	        		std::cout << "TEST:\t" << p.path().string() << '\n';
	        		filesystem::rename(p.path(), testPath + p.path().filename().string());
	        	}
	        	else if (diversion == 1)
	        	{
	        		std::cout << "VALID:\t" << p.path().string() << '\n';
	        		filesystem::rename(p.path(), validatePath + p.path().filename().string());
	        	}
	        	else
	        	{
	        		std::cout << "TRAIN:\t" << p.path().string() << '\n';
	        		filesystem::rename(p.path(), trainPath + p.path().filename().string());
	        	}
	            
	        }
	    }
	}
	else if (operation == "unpartition" || operation == "U")
	{
		if (argc < 3)
		{
			cout << "train unpartition requires an additional argument, input the folder location of training data." << endl;
			return 1;
		}

		string dataFolder = argv[2];

		string trainPath = dataFolder + "/train/";
		if (!filesystem::exists(trainPath))
		{
			filesystem::create_directory(trainPath);
		}
		string testPath = dataFolder + "/test/";
		if (!filesystem::exists(testPath))
		{
			filesystem::create_directory(testPath);
		}
		string validatePath = dataFolder + "/validate/";
		if (!filesystem::exists(validatePath))
		{
			filesystem::create_directory(validatePath);
		}

		for (auto &p : filesystem::directory_iterator(trainPath))
	    {
	        if (p.path().extension() == ".json")
	        {
				filesystem::rename(p.path(), dataFolder + p.path().filename().string());
	        }
	    }

	    for (auto &p : filesystem::directory_iterator(testPath))
	    {
	        if (p.path().extension() == ".json")
	        {
				filesystem::rename(p.path(), dataFolder + p.path().filename().string());
	        }
	    }

	    for (auto &p : filesystem::directory_iterator(validatePath))
	    {
	        if (p.path().extension() == ".json")
	        {
				filesystem::rename(p.path(), dataFolder + p.path().filename().string());
	        }
	    }
	}
	else if (operation == "new" || operation == "N")
	{
		if (argc < 3)
		{
			cout << "Missing parameters, must call it as train new (path to store model)\n";
			return 1;
		}

		string modelPath = argv[2];

		cout << "Generating new model" << endl;
	    WRB_Chess::NeuralModel model;
	    cout << "Model generated!" << endl << endl;

	    string startTime = to_string(std::time(nullptr));

	    string fn = modelPath;
		fn += "/";
		fn += startTime;
		fn += ".model";
		model.save(fn);

		cout << "Saved model as " << fn << endl;
	}
	else if (operation == "train" || operation == "T")
	{
		if (argc < 6)
		{
			cout << "Missing parameters, must call it as train train (path to train/test/validate data) (path to store model) (path to model to train) (model name)\n";
			return 1;
		}

		string dataPath = argv[2];
		string modelPath = argv[3];

		int batchN = 5; // TODO: Make this a command line arg
		int saveEvery = 100; // TODO: Make this a command line

		string trainingPath = dataPath + "/train/";
		string testPath = dataPath + "/test/";
		string validatePath = dataPath + "/validate/";

		cout << "Loading model" << endl;
	    WRB_Chess::NeuralModel model(argv[4]);
	    cout << "Model loaded!" << endl << endl;

	    string stemName = argv[5];

		// Load training data
		cout << "Loading training data" << endl;
		vector<GameData> trainingData;
		for (auto &p : filesystem::directory_iterator(trainingPath))
	    {
	        if (p.path().extension() == ".json")
	        {
				trainingData.push_back(extractGameData(p.path().string()));
	        }
	    }
	    cout << "Training data loaded" << endl;
	    cout << trainingData.size() << " games in dataset" << endl << endl;

	    cout << "Loading test data" << endl;
	    vector<GameData> testData;
	    for (auto &p : filesystem::directory_iterator(testPath))
	    {
	        if (p.path().extension() == ".json")
	        {
				testData.push_back(extractGameData(p.path().string()));
	        }
	    }
	    cout << "Test data loaded" << endl;
	    cout << testData.size() << " games in dataset" << endl << endl;

	    cout << "Loading validation data" << endl;
	    vector<GameData> validData;
	    for (auto &p : filesystem::directory_iterator(validatePath))
	    {
	        if (p.path().extension() == ".json")
	        {
				validData.push_back(extractGameData(p.path().string()));
	        }
	    }
	    cout << "Validation data loaded" << endl;
	    cout << validData.size() << " games in dataset" << endl << endl;


		cout << "Training model!" << endl;

		double lr = 0.0001;
		double gamma = 0.8;

		double cesaroMSE = 0.0;
		double cesaroCount = 0.0;
		int epoch = -1;

		int trainIt = 0;
		while(true)
		{
			double mseSum = 0.0;
			double count = 0;
			cout << "\nValidating, mse on test score: ";
			cout.flush();
			for (int i = 0; i < 10; i++)
			{
				int gC = rand() % testData.size();

				if (testData[gC].boards.size() == 0)
					continue;

				double finalScore = 0.0;
				if (testData[gC].victor)
					finalScore = 1.0;
				else
					finalScore = -1.0;

				int j = testData[gC].boards.size() - 1;
				mseSum += std::pow(finalScore - model.runForward(testData[gC].boards[j], (j + 1) % 2),2);

				for (j = testData[gC].boards.size() - 2; j >= 0; j--)
				{
					double score = model.runForward(testData[gC].boards[j], (j + 1) % 2);
					mseSum += std::pow(trainingGoal(model, testData[gC].boards[j], (j + 1) % 2) - score,2);
					count += 1;
				}
			}

			if ((trainIt % saveEvery) == 0)
			{
				epoch++;
				string fn = modelPath;
				fn += "/";
				fn += stemName;
				fn += "-e";
				fn += to_string(epoch);
				fn += ".model";
				model.save(fn);
			}

			string fn = modelPath;
			fn += "/";
			fn += stemName;
			fn += "-eLatest.model";
			model.save(fn);
			

			double oldCesaro = 100.0;

			if (cesaroCount >= 1)
				oldCesaro = cesaroMSE / cesaroCount;

			cesaroCount += 1.0;
			cesaroMSE += mseSum/count;

			cout <<  cesaroMSE/cesaroCount << " (" << mseSum/count << ")" << endl;

			if ((cesaroMSE/cesaroCount) > oldCesaro)
			{
				cout << "Reducing LR: " << lr;
				lr = lr * 0.99;
				cout << "->" << lr << endl;
			}
			

			for (int i = 0; i < 10; i ++)
			{
				cout << i << "/10 : ";
				cout.flush();
				vector<WRB_Chess::Bitboard> brds;
				vector<bool> toMove;
				vector<double> scores;
				
				for (int j = 0; j < batchN; j++)
				{
					int gameC = rand() % trainingData.size();

					if (trainingData[gameC].boards.size() == 0)
						continue;

					double finalScore = 0.0;
					if (trainingData[gameC].victor)
						finalScore = 1.0;
					else
						finalScore = -1.0;

					int k = trainingData[gameC].boards.size() - 1;
					brds.push_back(trainingData[gameC].boards[k]);
					toMove.push_back((k + 1) % 2);
					scores.push_back(finalScore);
					
					for (k = trainingData[gameC].boards.size() - 2; k >= 0; k--)
					{
						brds.push_back(trainingData[gameC].boards[k]);
						toMove.push_back((k + 1) % 2);
						scores.push_back(trainingGoal(model, trainingData[gameC].boards[k], (k + 1) % 2));
					}
				}
				
				cout << brds.size() << " boards in batch : ";
				cout.flush();

				double delta = model.trainBatch(brds, toMove, scores, lr);
				
				cout << " batch delta: " << delta << endl;
			}

			trainIt++;
		}


	}
	else
	{
		cout << "Operation type unknown try:\n";
		cout << "\tpartition|P \t Partition training and test data\n";
		cout << "\tunpartition|U \tUnpartitions data from training and test to a general pool\n";
		cout << "\tnew|N \t Create a new model\n";
		cout << "\ttrain|T \t Train a model\n";
		return 1;
	}

	return 0;
}
