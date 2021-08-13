#include "PureMonteEvaluator.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

WRB_Chess::PureMonte_Evaluator::PureMonte_Evaluator(unsigned int playouts)
{
	playoutsPerEval = playouts;
	//srand(time(0));
}

double WRB_Chess::PureMonte_Evaluator::EvaluatePosition(const WRB_Chess::Bitboard& brd, WRB_Chess::Color toMove)
{
	double wins = 0;
	double losses = 0;
	double draws = 0;
	double count = 0;

	for (int i = 0; i < playoutsPerEval; i++)
	{
		WRB_Chess::Bitboard gameBrd = brd;

		WRB_Chess::Color activeColor = toMove;
		int it = 1;
		while (gameBrd.KingsAlive() && it < 1000)
		{
			it++;
			std::vector<WRB_Chess::Move> mvs = gameBrd.AvailableMoves(activeColor);
			short kingSq = (*WRB_Chess::MaskToSquares(gameBrd.Pieces(OPPOSITE_COLOR(activeColor), WRB_Chess::Piece::King)).begin());
			for (int i = 0; i < mvs.size(); i++)
			{
				if (mvs[i].toSquare == kingSq)
				{
					WRB_Chess::Bitboard testB = gameBrd;
					testB.ApplyMove(mvs[i]);
					if (!testB.KingsAlive())
					{
						gameBrd = testB;
						kingSq = -1;
						break;
					}
				}
			}

			if (kingSq != -1)
			{
				WRB_Chess::Move choice = mvs[rand() % mvs.size()];
				gameBrd.ApplyMove(choice);
			}
			
			activeColor = OPPOSITE_COLOR(activeColor);
		}

		if (it == 0)
			it = 1;

		if (gameBrd.Pieces(toMove, WRB_Chess::Piece::King) == 0)
			losses += 1.0/(double)it;
		else if (gameBrd.Pieces(OPPOSITE_COLOR(toMove), WRB_Chess::Piece::King) == 0)
			wins += 1.0/(double)it;
		else
			draws += 1.0/(double)it;

		count += 1.0;
	}

	return ((long double)(wins - losses))/((long double)(count));
}
