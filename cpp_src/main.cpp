#include "chess/board.h"
#include <iostream>
#include <bitset>

using namespace std;

int main()
{
	WRB_Chess::Bitboard board;

	cout << "Hello world!" << endl;
	cout << "All: " << bitset<64>(board.Pieces()) << endl;

	uint64_t z = 0;
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
}