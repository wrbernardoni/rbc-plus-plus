#ifndef WRB_ENGINES_RANDOM_H_
#define WRB_ENGINES_RANDOM_H_

#include "EngineBase.h"

namespace WRB_Chess
{
	class RandomEngine : public EngineBase
	{
	public:
		RandomEngine();
		virtual double EvaluatePosition(const WRB_Chess::Bitboard&, WRB_Chess::Color);
		virtual short RecommendScan(const WRB_Chess::InformationSet&, WRB_Chess::Color, double&);
		virtual WRB_Chess::Move RecommendMove(const WRB_Chess::InformationSet&, WRB_Chess::Color, double&);
	};
};

#endif
