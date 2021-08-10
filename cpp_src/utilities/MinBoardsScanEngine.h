#ifndef WRB_ENGINES_MINBOARDSSCAN_H_
#define WRB_ENGINES_MINBOARDSSCAN_H_

#include "RandomEngine.h"

namespace WRB_Chess
{
	class MinBoardsScanEngine : public RandomEngine
	{
	public:
		virtual short RecommendScan(const WRB_Chess::InformationSet&, WRB_Chess::Color, double&);
	};
};

#endif
