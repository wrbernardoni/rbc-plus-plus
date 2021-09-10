#include "ExpectimaxEngineMT.h"
#include <iostream>
#include <unordered_map>

void WRB_Chess::EMWorker__(bool* end, EngineBase* engine, std::queue<EMTask>* q, std::mutex* qMutex)
{
	while (!(*end))
	{
		while (q->size() != 0)
		{
			EMTask tsk;
			bool taskGrabbed = false;
			qMutex->lock();
			if (q->size() > 0)
			{
				tsk = q->front();
				q->pop();
				taskGrabbed = true;
			}
			qMutex->unlock();

			if (taskGrabbed)
			{
				tsk.sp->set_value(engine->EvaluatePosition(tsk.b, OPPOSITE_COLOR(tsk.c)));
				delete tsk.sp;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
};

std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> WRB_Chess::ExpectimaxMT::RecommendScanMovePolicy(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c)
{
	if (brds.boards.size() == 0)
		return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>();

	WRB_Chess::Bitboard firstB = *brds.boards.cbegin();
	std::vector<WRB_Chess::Move> mvs = ((firstB)).AvailableMoves(c);

	std::unordered_map<WRB_Chess::Bitboard, std::unordered_map<WRB_Chess::Move, WRB_Chess::Bitboard, WRB_Chess::MoveHash>, WRB_Chess::BoardHash> evals;
	std::unordered_map<WRB_Chess::Bitboard, std::unordered_map<WRB_Chess::Move, double, WRB_Chess::MoveHash>, WRB_Chess::BoardHash> mvScore;
	std::unordered_map<WRB_Chess::Bitboard, std::future<double>, WRB_Chess::BoardHash> futures;

	qMutex.lock();
	for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
	{
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard eBrd = (*it);
			eBrd.ApplyMove(mvs[i]);
			evals[(*it)][mvs[i]] = eBrd;

			if (!scores.contains(eBrd) && (futures.count(eBrd) == 0))
			{
				EMTask tsk;
				tsk.b = eBrd;
				tsk.sp = new std::promise<double>;
				tsk.c = c;
				taskQueue.push(tsk);
				futures[eBrd] = tsk.sp->get_future();
			}
			else if (futures.count(eBrd) == 0)
			{
				std::promise<double> p;
				futures[eBrd] = p.get_future();
				p.set_value(scores[eBrd]);
			}
		}
	}
	qMutex.unlock();

	int nT = futures.size();
	int nF = 0;
	std::unordered_map<WRB_Chess::Bitboard, double, WRB_Chess::BoardHash> scrs;
	for (auto it = futures.begin(); it != futures.end(); it++)
	{
		std::cout << "\r" << nF << "/" << nT << " boards evaluated" << std::flush;
		scrs[it->first] = it->second.get();
		nF++;
	}
	std::cout << "\r" << nF << "/" << nT << " boards evaluated" << std::flush;

	
	for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
	{
		for (int i = 0; i < mvs.size(); i++)
		{
			double bP = 0.0;
			if (brds.probability.count((*it)) > 0)
			{
				bP = brds.probability.at(*it).p;
			}

			mvScore[(*it)][mvs[i]] = -1.0 * scrs[evals[(*it)][mvs[i]]] * bP;
		}
	}
	std::cout<<std::endl;

	std::vector<short> bestSense;
	double minN = -1;
	double bestES = 0.0;

	std::unordered_map<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> policies;
	for (int j = 1; j < 7; j++)
	{
		for (int i = 1; i < 7; i++)
		{
			// This is a temporary time saving measure. We should consider scans of our own pieces, but not now.
			// if (firstB.PieceAt(i + 8*j).color == c)
			// 	continue;

			std::unordered_map<WRB_Chess::Bitboard, std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>, WRB_Chess::BoardHash> partition;
			for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
			{
				WRB_Chess::Bitboard masked = (*it).senseMask(i + 8*j);
				partition[masked].emplace((*it));
			}

			// No need to evaluate partition if it doesn't discern anything 
			if ((partition.size() < 2) && bestSense.size() != 0)
				continue;
	
			double ES = 0.0;
			double maxN = 0;
			
			for (auto it = partition.begin(); it != partition.end(); it++)
			{
				double sc = 0.0;
				std::vector<int> bestMvs;

				for (int k = 0; k < mvs.size(); k++)
				{
					double lms = 0.0;
					for (auto bd = it->second.cbegin(); bd != it->second.cend(); bd++)
					{
						// double bP = 0.0;
						// if (brds.probability.count((*bd)) > 0)
						// {
						// 	bP = brds.probability.at(*bd).p;
						// }

						//std::cout << bP << std::endl;

						lms += mvScore[(*bd)][mvs[k]];// * bP;
					}

					if ((bestMvs.size() == 0) || (lms > sc))
					{
						bestMvs.clear();
						bestMvs.push_back(k);
						sc = lms;
					}
					else if (lms == sc)
					{
						bestMvs.push_back(k);
					}
				}
				if (bestMvs.size() != 0)
					policies[i + 8*j][it->first] = mvs[bestMvs[rand() % bestMvs.size()]];
				else
					policies[i + 8*j][it->first] = WRB_Chess::Move();

				ES += sc;
				for (auto bd = it->second.cbegin(); bd != it->second.cend(); bd++)
				{
					double bP = 0.0;
					if (brds.probability.count((*bd)) > 0)
					{
						bP = brds.probability.at(*bd).p;
					}

					//std::cout << bP << std::endl;

					maxN += it->second.size() * bP;
				}
			}
	
			if ((bestSense.size() == 0) || (bestES < ES))
			{
				bestES = ES;
				bestSense.clear();
				bestSense.push_back(i + 8 * j);
				minN = maxN;
			}
			else if ((bestES == ES) && (maxN < minN))
			{
				bestSense.clear();
				bestSense.push_back(i + 8 * j);
				minN = maxN;
			}
			else if ((bestES == ES) && (maxN == minN))
			{
				bestSense.push_back(i + 8 * j);
			}
		}
	}
	if (bestSense.size() == 0)
		bestSense.push_back(-1);
	
	std::cout << "\t\t\tExpected score from scan: " << bestES << std::endl;
	std::cout << "\t\t\tExpected number of boards: " << minN << std::endl;
	short senseChoice = bestSense[rand() % bestSense.size()];
	return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>(senseChoice,policies[senseChoice]);
}
