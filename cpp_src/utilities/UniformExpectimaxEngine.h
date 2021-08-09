#ifndef WRB_ENGINES_UNIFORM_EXPECTIMAX_H_
#define WRB_ENGINES_UNIFORM_EXPECTIMAX_H_

#include "PureMonteEvaluator.h"

namespace WRB_Chess
{
	class UniformExpectimax : public PureMonte_Evaluator
	{
	protected:
		std::unordered_map<WRB_Chess::Bitboard, double, WRB_Chess::BoardHash> scores;
	public:
		UniformExpectimax(unsigned int playouts) : PureMonte_Evaluator(playouts) {};
		virtual short RecommendScan(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color, double&);
		virtual WRB_Chess::Move RecommendMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color, double&);
		virtual std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> RecommendScanMovePolicy(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>&, WRB_Chess::Color);
	};
};

#endif
