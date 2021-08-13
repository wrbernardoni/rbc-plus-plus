#ifndef WRB_MOVE_PROBABILITY_H_
#define WRB_MOVE_PROBABILITY_H_

#include "../chess/board.h"
#include <vector>
#include <unordered_set>

namespace WRB_Chess
{
	class MoveProbability
	{
	public:
		virtual std::vector<double> moveProb(const WRB_Chess::Bitboard& b, std::vector<WRB_Chess::Move>& mvs)
		{
			std::vector<double> ret;
			std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves;
			double nonNull = 0.0;
			for (int i = 0; i < mvs.size(); i++)
			{
				WRB_Chess::Move recM = b.RectifyMove(mvs[i]);
				if (uniqueMoves.count(recM) > 0)
				{
					ret.push_back(0);
				}
				else
				{
					uniqueMoves.emplace(recM);
					ret.push_back(1);
				}
			}

			if (uniqueMoves.size() > 0)
			{
				for (int i = 0; i < ret.size(); i++)
				{
					ret[i] /= uniqueMoves.size();
				}
			}

			return ret;
		};
	};
};


#endif
