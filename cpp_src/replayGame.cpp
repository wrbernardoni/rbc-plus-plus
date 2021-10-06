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

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "At least 1 arguments needed: call it like: replay hist.json" << endl;
		return 1;
	}

	string fname = argv[1];

	ifstream file(fname.c_str());

	if (!file.is_open())
	{
		cout << "Error opening file" << endl;
		return 1;
	}

	json hist;
	file >> hist;

	file.close();

	WRB_Chess::Bitboard brd;

	cout << "White to move 0-" << endl;
	cout <<  WRB_Chess::GetPrintable(brd) << endl;

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

			cout << "Post white move " << wc << "-" << endl;
		}
		else
		{
			if (bc < hist["fens_after_move"]["false"].size())
			{
				fen = hist["fens_after_move"]["false"][bc];
				bc++;
			}

			cout << "Post black move " << bc << "-" << endl;
		}

		cout << WRB_Chess::GetPrintable(WRB_Chess::Bitboard(fen)) << endl;

		cin.get();

		turn = !turn;

		if ((wc >= hist["fens_after_move"]["true"].size()) && (bc >= hist["fens_after_move"]["false"].size()))
		{
			break;
		}
	}

	if (hist["winner_color"])
		cout << "White wins" << endl;
	else
		cout << "Black wins" << endl;

	return 0;
}
