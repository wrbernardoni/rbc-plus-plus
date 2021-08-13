#ifndef WRB_ENGINE_SHANNON2_EXPECTI_H_
#define WRB_ENGINE_SHANNON2_EXPECTI_H_

#include "ExpectimaxEngine.h"

namespace WRB_Chess
{
	class Shannon2Expectimax : public Expectimax
	{
	private:
		unsigned int playoutsPerEval;
	public:
		Shannon2Expectimax() : Expectimax(0) {};
		virtual double EvaluatePosition(const WRB_Chess::Bitboard& b, WRB_Chess::Color c)
		{
			double score = 0.0;
			score += 1000.0 * ((int)b.Pieces(c, WRB_Chess::Piece::King).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King).count());
			score += 9.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Queen).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Queen).count());
			score += 5.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Rook).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Rook).count());
			score += 3.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Bishop).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Bishop).count());
			score += 3.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Knight).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Knight).count());
			score += 1.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Pawn).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Pawn).count());
			
			auto mob1 = b.AvailableMoves(c);
			int tM1 = 0;

			std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves1;
			for (int i = 0; i < mob1.size(); i++)
			{
				WRB_Chess::Move taken = b.RectifyMove(mob1[i]);
				if (uniqueMoves1.count(taken) == 0)
				{
					uniqueMoves1.emplace(taken);
					WRB_Chess::Bitboard tB = b;
					tB.ApplyMove(taken);
					if (tB.Pieces(c, WRB_Chess::Piece::King) != 0)
					{
						bool good = true;
						short kingSq = (*WRB_Chess::MaskToSquares(tB.Pieces(c, WRB_Chess::Piece::King)).begin());
						auto aM = tB.AvailableMoves(OPPOSITE_COLOR(c));
						for (int j = 0; j < aM.size(); j++)
						{
							if (aM[j].toSquare == kingSq)
							{
								WRB_Chess::Bitboard testB = tB;
								testB.ApplyMove(aM[j]);
								if (!testB.KingsAlive())
								{
									good = false;
									break;
								}
							}
						}

						if (good)
						{
							tM1++;
						}
					}
				}
			}

			auto mob2 = b.AvailableMoves(OPPOSITE_COLOR(c));
			int tM2 = 0;

			std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves2;
			for (int i = 0; i < mob2.size(); i++)
			{
				WRB_Chess::Move taken = b.RectifyMove(mob2[i]);
				if (uniqueMoves2.count(taken) == 0)
				{
					uniqueMoves2.emplace(taken);
					WRB_Chess::Bitboard tB = b;
					tB.ApplyMove(taken);
					if (tB.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King) != 0)
					{
						bool good = true;
						short kingSq = (*WRB_Chess::MaskToSquares(tB.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King)).begin());
						auto aM = tB.AvailableMoves(c);
						for (int j = 0; j < aM.size(); j++)
						{
							if (aM[j].toSquare == kingSq)
							{
								WRB_Chess::Bitboard testB = tB;
								testB.ApplyMove(aM[j]);
								if (!testB.KingsAlive())
								{
									good = false;
									break;
								}
							}
						}

						if (good)
						{
							tM2++;
						}
					}
				}
			}

			score += 0.1 * (tM1 - tM2);

			return score;
		};
	};
};

#endif
