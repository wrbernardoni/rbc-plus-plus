#ifndef WRB_CHESS_ENGINE_BASE_H
#define WRB_CHESS_ENGINE_BASE_H

#include "../chess/board.h"
#include <unordered_set>

namespace WRB_Chess
{
	class EngineBase
	{
	protected:
		short scanRec;
		WRB_Chess::Move moveRec;
	public:
		virtual double EvaluatePosition(const WRB_Chess::Bitboard&, WRB_Chess::Color) = 0;

		virtual short RecommendScan(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color) = 0;
		virtual void RecommendScanAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
		{
			scanRec = RecommendScan(brds, c);
		}
		virtual short GetScanRecommendation()
		{
			return scanRec;
		}

		virtual WRB_Chess::Move RecommendMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color) = 0;
		virtual void RecommendMoveAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
		{
			moveRec = RecommendMove(brds, c);
		}
		virtual WRB_Chess::Move GetMoveRecommendation()
		{
			return moveRec;
		}
	};
}

#endif
