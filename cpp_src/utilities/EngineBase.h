#ifndef WRB_CHESS_ENGINE_BASE_H
#define WRB_CHESS_ENGINE_BASE_H

#include "../chess/board.h"
#include <unordered_set>
#include <unordered_map>
#include <future>
#include <thread>

namespace WRB_Chess
{
	class EngineBase
	{
	public:
		virtual double EvaluatePosition(const WRB_Chess::Bitboard&, WRB_Chess::Color) = 0;

		virtual short RecommendScan(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
		{
			double d = 0;
			return RecommendScan(brds, c, d);
		};

		virtual short RecommendScan(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color, double&) = 0;
		virtual std::future<short> RecommendScanAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c);

		virtual WRB_Chess::Move RecommendMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
		{
			double d = 0;
			return RecommendMove(brds, c, d);
		};
		virtual WRB_Chess::Move RecommendMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color, double&) = 0;
		
		virtual std::future<WRB_Chess::Move> RecommendMoveAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c);

		virtual std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> RecommendScanMovePolicy(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color);

		virtual std::future<std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>> RecommendScanMovePolicyAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color);
	};
}

#endif
