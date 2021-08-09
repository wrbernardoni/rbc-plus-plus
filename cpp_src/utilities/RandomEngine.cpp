#include "RandomEngine.h"
#include <cstdlib> 
#include <ctime>


WRB_Chess::RandomEngine::RandomEngine()
{
	srand(time(0));
}

double WRB_Chess::RandomEngine::EvaluatePosition(const WRB_Chess::Bitboard&, WRB_Chess::Color)
{
	return 0;
}

short WRB_Chess::RandomEngine::RecommendScan(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c, double& d)
{
	d = 0;
	return rand() % 64;
}

WRB_Chess::Move WRB_Chess::RandomEngine::RecommendMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c, double& d)
{
	d = 0;
	if (brds.size() == 0)
		return WRB_Chess::Move();

	std::vector<WRB_Chess::Move> mvs = (*(brds.cbegin())).AvailableMoves(c);

	return mvs[rand() % mvs.size()];
}

