#ifndef WRB_ENGINES_EXPECTIMAX_MT_H_
#define WRB_ENGINES_EXPECTIMAX_MT_H_

#include "PureMonteEvaluator.h"
#include "RollingMap.h"
#include "ExpectimaxEngine.h"

#include <thread>
#include <vector>
#include <future>
#include <tuple>
#include <queue>
#include <mutex>

namespace WRB_Chess
{
	struct EMTask
	{
		WRB_Chess::Bitboard b;
		std::promise<double>* sp;
		WRB_Chess::Color c;

		EMTask()
		{
		};

		EMTask(const EMTask& t)
		{
			b = t.b;
			c = t.c;
			sp = t.sp;
		};

		EMTask& operator=(const EMTask& t)
		{
			b = t.b;
			c = t.c;
			sp = t.sp;

			return *this;
		};
	};

	extern void EMWorker__(bool* end, EngineBase* engine, std::queue<EMTask>* q, std::mutex* qMutex);

	class ExpectimaxMT : public Expectimax
	{
	protected:
		std::vector<std::thread*> threads;
		std::queue<EMTask> taskQueue;
		std::mutex qMutex;
		bool end;
	public:
		ExpectimaxMT(unsigned int playouts, size_t n_scores, int nThreads) : Expectimax(playouts, n_scores) 
		{
			end = false;

			for (int i = 0; i < nThreads; i++)
			{
				threads.push_back(new std::thread(EMWorker__, &end, this, &taskQueue, &qMutex));
			}
		};

		~ExpectimaxMT()
		{
			end = true;

			for (int i = 0; i < threads.size(); i++)
			{
				threads[i]->join();
				delete threads[i];
			}
		};

		virtual std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> RecommendScanMovePolicy(const WRB_Chess::InformationSet&, WRB_Chess::Color);
	};
};

#endif
