#ifndef WRB_MOVE_PROBABILITY_OLD_H_
#define WRB_MOVE_PROBABILITY_OLD_H_

#include "../chess/board.h"
#include <vector>
#include "MoveProbability.h"

namespace WRB_Chess
{
	class OldMoveProbability : public MoveProbability
	{
	public:
		virtual std::vector<double> moveProb(const WRB_Chess::Bitboard& b, std::vector<WRB_Chess::Move>& mvs)
		{
			std::vector<double> ret;
			double nonNull = 0.0;
			for (int i = 0; i < mvs.size(); i++)
			{
				ret.push_back(1.0 / (double)mvs.size());
			}

			return ret;
		};
	};
};


#endif
