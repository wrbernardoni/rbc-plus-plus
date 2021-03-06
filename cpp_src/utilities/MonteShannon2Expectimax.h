#ifndef WRB_ENGINE_MONTE_SHANNON_2_EXPECTI_H_
#define WRB_ENGINE_MONTE_SHANNON_2_EXPECTI_H_

#include "ExpectimaxEngineMT.h"

#include <iostream>

namespace WRB_Chess
{
	class MonteShannon2Expectimax : public ExpectimaxMT
	{
	private:
		//unsigned int playoutsPerEval;
		unsigned int depth;
	public:
		MonteShannon2Expectimax(unsigned int ppE, unsigned int d, size_t ns, int nT) : ExpectimaxMT(ppE, ns, nT) 
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
					auto mvs = gameBrd.AvailableMoves(activeColor);
					std::vector<short> kinSqs = WRB_Chess::MaskToSquares(gameBrd.Pieces(OPPOSITE_COLOR(activeColor), WRB_Chess::Piece::King));
					if (kinSqs.size() == 0)
					{
						break;
					}
					short kingSq = kinSqs[0];
					auto kingAtt = gameBrd.Attacks(activeColor, kingSq);
					for (int j = 0; j < kingAtt.size(); j++)
					{
						if (kingAtt[j].toSquare == kingSq)
						{
							WRB_Chess::Bitboard testB = gameBrd;
							testB.ApplyMove(kingAtt[j]);
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
				score += 200.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::King).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King).count());
				score += 9.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Queen).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Queen).count());
				score += 5.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Rook).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Rook).count());
				score += 3.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Bishop).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Bishop).count());
				score += 3.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Knight).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Knight).count());
				score += 1.0 * ((int)gameBrd.Pieces(c, WRB_Chess::Piece::Pawn).count() - (int)gameBrd.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Pawn).count());
				auto mob1 = gameBrd.AvailableMoves(c);
				int tM1 = 0;

				std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves1;
				for (int j = 0; j < mob1.size(); j++)
				{
					WRB_Chess::Move taken = gameBrd.RectifyMove(mob1[j]);
					if (uniqueMoves1.count(taken) == 0)
					{
						uniqueMoves1.emplace(taken);
						WRB_Chess::Bitboard tB = gameBrd;
						tB.ApplyMove(taken);
						if (tB.Pieces(c, WRB_Chess::Piece::King) != 0)
						{
							bool good = true;
							short kingSq = (*WRB_Chess::MaskToSquares(tB.Pieces(c, WRB_Chess::Piece::King)).begin());
							auto aM = tB.Attacks(OPPOSITE_COLOR(c), kingSq);
							if (aM.size() == 0)
							{
								tM1++;
							}
						}
					}
				}

				auto mob2 = gameBrd.AvailableMoves(OPPOSITE_COLOR(c));
				int tM2 = 0;

				std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves2;
				for (int j = 0; j < mob2.size(); j++)
				{
					WRB_Chess::Move taken = gameBrd.RectifyMove(mob2[j]);
					if (uniqueMoves2.count(taken) == 0)
					{
						uniqueMoves2.emplace(taken);
						WRB_Chess::Bitboard tB = gameBrd;
						tB.ApplyMove(taken);
						if (tB.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King) != 0)
						{
							bool good = true;
							short kingSq = (*WRB_Chess::MaskToSquares(tB.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King)).begin());
							auto aM = tB.Attacks(c, kingSq);
							if (aM.size() == 0)
							{
								tM2++;
							}
						}
					}
				}

				score += 0.1 * (tM1 - tM2);

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
