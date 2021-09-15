#ifndef WRB_ENGINE_MONTE_SHANNON_EXPECTI_H_
#define WRB_ENGINE_MONTE_SHANNON_EXPECTI_H_

#include "ExpectimaxEngineMT.h"

#include <iostream>

namespace WRB_Chess
{
	class MonteShannonExpectimax : public ExpectimaxMT
	{
	private:
		unsigned int depth;
	public:
		MonteShannonExpectimax(unsigned int ppE, unsigned int d, size_t ns, int nT) : ExpectimaxMT(ppE, ns, nT) 
		{
			depth = d;
		};
		virtual double EvaluatePosition(const WRB_Chess::Bitboard& b, WRB_Chess::Color c)
		{
			double scoreTot = 0;
			double count = 0;

			for (int po = 0; po < playoutsPerEval; po++)
			{
				WRB_Chess::Bitboard gameBrd = b;

				WRB_Chess::Color activeColor = c;
				int it = 1;
				while (gameBrd.KingsAlive() && it < depth)
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

				double score = 0.0;
				score += 1000.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::King).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King).count());
				score += 9.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Queen).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Queen).count());
				score += 5.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Rook).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Rook).count());
				score += 3.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Bishop).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Bishop).count());
				score += 3.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Knight).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Knight).count());
				score += 1.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Pawn).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Pawn).count());
				int mob1 = gameBrd.AvailableMoves(c).size();
				int mob2 = gameBrd.AvailableMoves(OPPOSITE_COLOR(c)).size();
				score += 0.1 * (mob1 - mob2);

				scoreTot += score/it;
				count += 1.0;
			}
			
			if (count != 0)
				return scoreTot / count;
			else
				return 0;
		};
	};
};

#endif
