#ifndef WRB_ENGINES_STOCKY_EXPECTIMAX_MT_H_
#define WRB_ENGINES_STOCKY_EXPECTIMAX_MT_H_

#include "PureMonteEvaluator.h"
#include "RollingMap.h"
#include "ExpectimaxEngineMT.h"

#include <thread>
#include <vector>
#include <future>
#include <tuple>
#include <queue>
#include <mutex>
#include <string>

namespace WRB_Chess
{
	extern void SEMWorker__(bool* end, EngineBase* engine, std::queue<EMTask>* q, std::mutex* qMutex, std::string eL);

	class StockyExpectimax : public Expectimax
	{
	protected:
		std::vector<std::thread*> threads;
		std::queue<EMTask> taskQueue;
		std::mutex qMutex;
		std::string engineLoc;
		bool end;
	public:
		StockyExpectimax(unsigned int playouts, size_t n_scores, int nThreads, std::string engineLoc) : Expectimax(playouts, n_scores) 
		{
			end = false;

			for (int i = 0; i < nThreads; i++)
			{
				threads.push_back(new std::thread(SEMWorker__, &end, this, &taskQueue, &qMutex, engineLoc));
			}
		};

		~StockyExpectimax()
		{
			end = true;
			qMutex.lock();
			while (taskQueue.size() > 0)
			{
				auto tsk = taskQueue.front();
				taskQueue.pop();
				delete tsk.sp;
			};
			qMutex.unlock();

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
