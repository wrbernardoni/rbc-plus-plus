#ifndef WRB_ENGINE_NEURAL_EXPECTI_H_
#define WRB_ENGINE_NEURAL_EXPECTI_H_

#include "ExpectimaxEngineMT.h"
#include "NeuralModel.h"

#include <iostream>
#include <string>

namespace WRB_Chess
{
	class NeuralExpectimax : public ExpectimaxMT
	{
	private:
		
	public:
		WRB_Chess::NeuralModel* model;

		NeuralExpectimax(NeuralModel* m, size_t ns, int nT) : ExpectimaxMT(10, ns, nT)
		{
			model = m;
		};
		virtual double EvaluatePosition(const WRB_Chess::Bitboard& b, WRB_Chess::Color c)
		{
			if (!b.KingsAlive())
			{
				if (b.Pieces(c, WRB_Chess::Piece::King) == 0)
					return -1;
				else if (b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King) == 0)
					return 1;
			}

			double sc = model->runForward(b, c);
			if (c == WRB_Chess::Color::Black)
			{
				return -sc;
			}
			else
			{
				return sc;
			}
		};
	};
};

#endif
