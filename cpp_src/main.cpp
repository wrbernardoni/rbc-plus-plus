#include "chess/board.h"
#include <iostream>
#include <bitset>

using namespace std;

int main()
{
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
}