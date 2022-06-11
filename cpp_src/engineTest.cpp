#include "chess/board.h"
#include <iostream>
#include <bitset>

#include <cstdlib> 
#include <ctime>

#include "utilities/UCIInterface.h"


using namespace std;

int main(int argc, char* argv[])
{
	WRB_Chess::BoardHash hash;
	WRB_Chess::BoardHash::Init();

	WRB_Chess::Bitboard board;

	if (argc < 2)
	{
		cout << "Need path to uci engine" << endl;
		return 1;
	}
	string uciPath = argv[1];

	WRB_Chess::UCI_Interface* eng = new WRB_Chess::UCI_Interface(uciPath);

	cout << "Initial Board" << endl;
	cout << WRB_Chess::GetPrintable(board) << endl;
	// cout << board.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King);
	int i = 0;
	cout << "Score: " << eng->ScorePosition(board, 100) << endl;
	while (board.KingsAlive() && board.getHalfmoveClock() < 100)
	{
		cout << endl << endl;
		WRB_Chess::Color c = (WRB_Chess::Color)(i % 2);
		vector<WRB_Chess::Move> mvs = board.AvailableMoves(c);
		WRB_Chess::Move attemptedMove = mvs[rand() % mvs.size()];
		board.ApplyMove(attemptedMove);
		cout << "Turn " << i << endl;
		cout << WRB_Chess::GetPrintable(board);
		cout << board.fen() << endl;
		// cout << board.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King);
		cout << "Score: " << eng->ScorePosition(board, 50) << endl;
		i++;
	}

	delete eng;

	cout << "Test done!!!" << endl;
}
