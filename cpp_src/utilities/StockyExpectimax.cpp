#include "StockyExpectimax.h"
#include <iostream>
#include <unordered_map>
#include "UCIInterface.h"
#include <cmath>

void WRB_Chess::SEMWorker__(bool* end, EngineBase* engine, std::queue<EMTask>* q, std::mutex* qMutex, std::string eL)
{
	WRB_Chess::UCI_Interface stocky(eL);
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
				tsk.b.setToMove(OPPOSITE_COLOR(tsk.c));
				double score = stocky.ScorePosition(tsk.b, 100);
				if (tsk.c != WRB_Chess::Color::Black)
					score *= -1;
				tsk.sp->set_value(score);
				delete tsk.sp;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
};

std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> WRB_Chess::StockyExpectimax::RecommendScanMovePolicy(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c)
{
	if (brds.boards.size() == 0)
		return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>();


	WRB_Chess::Bitboard firstB = *brds.boards.cbegin();
	std::vector<WRB_Chess::Move> mvs = ((firstB)).AvailableMoves(c);

	std::unordered_map<WRB_Chess::Bitboard, std::unordered_map<WRB_Chess::Move, WRB_Chess::Bitboard, WRB_Chess::MoveHash>, WRB_Chess::BoardHash> evals;
	std::unordered_map<WRB_Chess::Bitboard, std::unordered_map<WRB_Chess::Move, double, WRB_Chess::MoveHash>, WRB_Chess::BoardHash> mvScore;
	std::unordered_map<WRB_Chess::Bitboard, std::future<double>, WRB_Chess::BoardHash> futures;
	std::unordered_map<WRB_Chess::Bitboard, double, WRB_Chess::BoardHash> scrs;

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
				futures[eBrd] = tsk.sp->get_future();
				taskQueue.push(tsk);
			}
			else if (futures.count(eBrd) == 0)
			{
				scrs[eBrd] = scores[eBrd];
			}
		}
	}
	qMutex.unlock();

	int nT = futures.size();
	int nF = 0;
	std::cout << nT << " boards to evaluate." << std::endl;

	// while (taskQueue.size() > 0)
	// {
	// 	std::cout << "\r" << taskQueue.size() << " tasks remaining               " << std::flush;
	// 	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	// }
	// std::cout << "\r" << 0 << " tasks remaining               " << std::flush;
	// std::cout << std::endl;
	for (auto it = futures.begin(); it != futures.end(); it++)
	{
		// std::cout << "\r" << nF << "/" << nT << " boards evaluated" << std::flush;
		double s = it->second.get();
		if (std::isfinite(s))
		{
			scrs[it->first] = s;
			scores[it->first] = scrs[it->first];
			nF++;
		}
	}
	std::cout << "\r" << nF << "/" << nT << " boards evaluated" << std::flush;

	double lostProb = 0.0;
	for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
	{
		for (int i = 0; i < mvs.size(); i++)
		{
			double bP = 0.0;
			if (brds.probability.count((*it)) > 0)
			{
				bP = brds.probability.at(*it).p;
			}

			if (scrs.count(evals[(*it)][mvs[i]]) != 0)
				mvScore[(*it)][mvs[i]] = -1.0 * scrs[evals[(*it)][mvs[i]]] * bP;
			else
				lostProb += bP;
		}
	}

	if (lostProb == 1.0)
		lostProb = 0.0;
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
			if ((partition.size() < 2) && (bestSense.size() != 0))
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
						if (mvScore[(*bd)].count(mvs[k]) != 0)
							lms += mvScore[(*bd)][mvs[k]] / (1.0 - lostProb);// * bP;
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
