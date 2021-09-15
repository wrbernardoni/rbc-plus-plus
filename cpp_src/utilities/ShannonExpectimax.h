#ifndef WRB_ENGINE_SHANNON_EXPECTI_H_
#define WRB_ENGINE_SHANNON_EXPECTI_H_

#include "ExpectimaxEngineMT.h"

namespace WRB_Chess
{
	class ShannonExpectimax : public ExpectimaxMT
	{
	private:
		unsigned int playoutsPerEval;
	public:
		ShannonExpectimax(size_t ns, int nT) : ExpectimaxMT(0, ns, nT) {};
		virtual double EvaluatePosition(const WRB_Chess::Bitboard& b, WRB_Chess::Color c)
		{
			double score = 0.0;
			score += 1000.0 * ((int)b.Pieces(c, WRB_Chess::Piece::King).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King).count());
			score += 9.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Queen).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Queen).count());
			score += 5.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Rook).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Rook).count());
			score += 3.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Bishop).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Bishop).count());
			score += 3.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Knight).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Knight).count());
			score += 1.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Pawn).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Pawn).count());
			score += 0.1 * ((int)b.AvailableMoves(c).size() - (int)b.AvailableMoves(OPPOSITE_COLOR(c)).size());
			return score;
		};
	};
};

#endif
