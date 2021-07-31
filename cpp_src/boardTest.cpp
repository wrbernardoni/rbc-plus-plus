#include "chess/board.h"
#include <iostream>
#include <bitset>

#include <cstdlib> 
#include <ctime> 

using namespace std;

int main()
{
	WRB_Chess::BoardHash hash;
	WRB_Chess::BoardHash::Init();

	WRB_Chess::Bitboard board;

	cout << "Hello world!" << endl;
	cout << "All: " << bitset<64>(board.Pieces()) << endl;

	bitset<64> z = 0;
	for (int i = 0; i < 2; i++)
	{
		for(int j = 0; j < 6; j++)
		{
			z = z | board.Pieces((WRB_Chess::Color)i,(WRB_Chess::Piece)j);
		}
	}
	cout << "Test: " << bitset<64>(z) << endl;
	cout << "White: " << bitset<64>(board.Pieces(WRB_Chess::Color::White)) << endl;
	cout << "Black: " << bitset<64>(board.Pieces(WRB_Chess::Color::Black)) << endl;
	cout << "White Bishops: " << bitset<64>(board.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::Pawn)) << endl;
	WRB_Chess::ColorPiece pc = board.PieceAt(4);
	cout << "Piece at E1: " << WRB_Chess::GetPieceName(pc.piece) << "| Color: " << WRB_Chess::GetColorName(pc.color) << endl;

	cout << "E3 is square number " << WRB_Chess::RankAndFileToSquare(3, WRB_Chess::File::E) << ", arrcheck " << WRB_Chess::SquareNames[WRB_Chess::RankAndFileToSquare(3, WRB_Chess::File::E)] << endl;

	cout << "The manhattan distance from E3 to F2 is: " << WRB_Chess::ManhattanDistance(WRB_Chess::RankAndFileToSquare(2, WRB_Chess::File::F), WRB_Chess::RankAndFileToSquare(3, WRB_Chess::File::E)) << endl;

	cout << endl << "---Board---" << endl << WRB_Chess::GetPrintable(board) << endl;

	cout << "\nAvailable white moves:\n";
	vector<WRB_Chess::Move> mvs = board.AvailableMoves(WRB_Chess::Color::White);
	for (int i = 0; i < mvs.size(); i++)
	{
		cout << WRB_Chess::GetPrintable(mvs[i]) << " \t|\t " << WRB_Chess::GetPrintable(board.RectifyMove(mvs[i])) << endl;
	}

	WRB_Chess::Bitboard brdCpy = board;
	cout << "Board copy:" << endl;

	srand(time(0));
	cout << "Taking 10 random turns!" << endl;
	for (int i = 0; i < 30; i++)
	{
		cout << endl << endl << "Turn: " << i << endl;
		WRB_Chess::Color c = (WRB_Chess::Color)(i % 2);
		if (c == WRB_Chess::Color::White)
			cout << "White Moves available:" << endl;
		else
			cout << "Black Moves available:" << endl;

		vector<WRB_Chess::Move> mvs = board.AvailableMoves(c);
		for (int i = 0; i < mvs.size(); i++)
		{
			cout << WRB_Chess::GetPrintable(mvs[i]) << " \t|\t " << WRB_Chess::GetPrintable(board.RectifyMove(mvs[i])) << endl;
		}

		WRB_Chess::Move attemptedMove = mvs[rand() % mvs.size()];
		bool capture = false;
		short captureSquare = -1;
		cout << "Attempting " << WRB_Chess::GetPrintable(attemptedMove) << endl;
		WRB_Chess::Move taken = board.ApplyMove(attemptedMove, capture, captureSquare);
		cout << "Took: " << WRB_Chess::GetPrintable(taken) << " Capture? " << capture << " at: " << captureSquare << endl;
		cout << WRB_Chess::GetPrintable(board) << endl;
	}

	cout << "Original board: " << endl;
	cout << WRB_Chess::GetPrintable(brdCpy) << endl;
	cout << "Board Hash: " << hash(brdCpy) << endl;

	int it = 10000;
	cout << "Let's zoom " << it << " iterations!" << endl;
	for (int i = 0; i < it; i++)
	{
		WRB_Chess::Bitboard pB = brdCpy;
		WRB_Chess::Color c = (WRB_Chess::Color)(i % 2);

		vector<WRB_Chess::Move> mvs = brdCpy.AvailableMoves(c);
		WRB_Chess::Move attemptedMove = mvs[rand() % mvs.size()];
		bool capture = false;
		short captureSquare = -1;
		WRB_Chess::Move taken = brdCpy.ApplyMove(attemptedMove, capture, captureSquare);

		bitset<64> auditA = (brdCpy.Pieces(WRB_Chess::Color::White) ^ brdCpy.Pieces(WRB_Chess::Color::Black));
		bitset<64> auditB = (brdCpy.Pawns() ^ brdCpy.Bishops() ^ brdCpy.Rooks() ^ brdCpy.Knights() ^ brdCpy.Queens() ^ brdCpy.Kings());

		if (auditA != auditB)
		{
			cout << "ERROR: Audit failed on iteration " << i << endl;
			cout << "Attempting " << WRB_Chess::GetPrintable(attemptedMove) << endl;
			cout << "Took: " << WRB_Chess::GetPrintable(taken) << " Capture? " << capture << " at: " << captureSquare << endl;
			cout << WRB_Chess::GetPrintable(pB) << endl;
			cout << "===>" << endl;
			cout << WRB_Chess::GetPrintable(brdCpy) << endl;

			bitset<64> dif = auditA ^ auditB;
			for( int k = 0; k < 64; k++)
			{
				if (dif[k])
				{
					cout << "Difference at " << WRB_Chess::SquareNames[k] << endl;
				}
			}
			break;
		}
	}

	cout << "After " << it << " iterations:" << endl;
	cout << WRB_Chess::GetPrintable(brdCpy) << endl;
	cout << (brdCpy.Pieces(WRB_Chess::Color::White) ^ brdCpy.Pieces(WRB_Chess::Color::Black)) << endl;
	cout << (brdCpy.Pawns() ^ brdCpy.Bishops() ^ brdCpy.Rooks() ^ brdCpy.Knights() ^ brdCpy.Queens() ^ brdCpy.Kings()) << endl;
	cout << hash(brdCpy) << endl;
}
