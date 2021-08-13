#ifndef WRB_ENGINE_PUREMONTE_H_
#define WRB_ENGINE_PUREMONTE_H_

#include "EngineBase.h"

namespace WRB_Chess
{
	class PureMonte_Evaluator : public EngineBase
	{
	protected:
		unsigned int playoutsPerEval;
	public:
		PureMonte_Evaluator(unsigned int playouts);
		virtual double EvaluatePosition(const WRB_Chess::Bitboard&, WRB_Chess::Color);
	};
};

#endif
