#include "EngineBase.h"

void AsyncScan(std::promise<short> promise, WRB_Chess::EngineBase* engine, const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	promise.set_value(engine->RecommendScan(brds, c));
};

void AsyncMove(std::promise<WRB_Chess::Move> promise, WRB_Chess::EngineBase* engine, const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	promise.set_value(engine->RecommendMove(brds, c));
};

void AsyncScanMove(std::promise<std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>> promise, WRB_Chess::EngineBase* engine, const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	promise.set_value(engine->RecommendScanMovePolicy(brds, c));
};


std::future<short> WRB_Chess::EngineBase::RecommendScanAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	std::promise<short> prom;
	std::future<short> ret = prom.get_future();

	std::thread worker(AsyncScan, std::move(prom), this, brds, c);

	return ret;
};

std::future<WRB_Chess::Move> WRB_Chess::EngineBase::RecommendMoveAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	std::promise<WRB_Chess::Move> prom;
	std::future<WRB_Chess::Move> ret = prom.get_future();

	std::thread worker(AsyncMove, std::move(prom), this, brds, c);

	return ret;
};

std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> WRB_Chess::EngineBase::RecommendScanMovePolicy(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	short scan = RecommendScan(brds, c);

	std::unordered_map<WRB_Chess::Bitboard, std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>, WRB_Chess::BoardHash> partition;
	for (auto it = brds.begin(); it != brds.end(); it++)
	{
		WRB_Chess::Bitboard masked = (*it).senseMask(scan);
		partition[masked].emplace((*it));
	}

	std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash> policy;

	for (auto it = partition.cbegin(); it != partition.cend(); it++)
	{
		policy[it->first] = RecommendMove(it->second, c);
	}

	return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>(scan, policy);
}

std::future<std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>> WRB_Chess::EngineBase::RecommendScanMovePolicyAsync(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>& brds, WRB_Chess::Color c)
{
	std::promise<std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>> prom;
	std::future<std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>> ret = prom.get_future();

	std::thread worker(AsyncScanMove, std::move(prom), this, brds, c);

	return ret;
};
