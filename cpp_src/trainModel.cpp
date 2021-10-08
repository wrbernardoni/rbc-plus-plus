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
		if (argc < 4)
		{
			cout << "Missing parameters, must call it as train new (path to train/test/validate data) (path to store model)\n";
			return 1;
		}

		string dataPath = argv[2];
		string modelPath = argv[3];

		string trainingPath = dataPath + "/train/";
		string testPath = dataPath + "/test/";
		string validatePath = dataPath + "/validate/";

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

		Eigen::MatrixXd m(2,2);
		m(0,0) = 1;
		m(0,1) = 1;
		m(1,0) = 0;
		m(1,1) = 1;

		Eigen::MatrixXd v(1,2);
		v(0,0) = 1;
		v(0,1) = 3;

		Eigen::MatrixXd vT(2,1);
		vT(0,0) = 1;
		vT(1,0) = 3;

		cout << m << endl;
		cout << v << endl;
		cout << vT << endl;
		cout << v * m << endl;
		cout << m * vT << endl;
	}
	else
	{
		cout << "Operation type unknown try:\n";
		cout << "\tpartition|P \t Partition training and test data\n";
		cout << "\tunpartition|U \tUnpartitions data from training and test to a general pool\n";
		cout << "\tnew|N \t Create a new model\n";
		return 1;
	}

	return 0;
}
