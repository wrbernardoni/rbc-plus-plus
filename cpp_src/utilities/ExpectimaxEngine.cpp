#include "ExpectimaxEngine.h"
#include <iostream>
#include <unordered_map>

short WRB_Chess::Expectimax::RecommendScan(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c, double& d)
{
	d = 0;
	return RecommendScanMovePolicy(brds, c).first;
}

WRB_Chess::Move WRB_Chess::Expectimax::RecommendMove(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c, double& d)
{
	if (brds.boards.size() == 0)
		return WRB_Chess::Move();

	std::vector<WRB_Chess::Move> mvs = (*(brds.boards.cbegin())).AvailableMoves(c);

	std::vector<int> bestMove;
	long double bestScore = 0;

	for (int i = 0; i < mvs.size(); i++)
	{
		long double mvScore = 0;

		for (auto iter = brds.boards.cbegin(); iter != brds.boards.cend(); iter++)
		{
			WRB_Chess::Bitboard eBrd = (*iter);
			eBrd.ApplyMove(mvs[i]);

			if (!scores.contains(eBrd))
			{
				scores[eBrd] = -1.0 * EvaluatePosition(eBrd, OPPOSITE_COLOR(c));
			}

			double bP = 0.0;
			if (brds.probability.count((*iter)) > 0)
			{
				bP = brds.probability.at(*iter).p;
			}
			mvScore += scores[eBrd] * bP;
		}

		mvScore = mvScore;

		if ((mvScore > bestScore) || (bestMove.size() == 0))
		{
			bestMove.clear();
			bestMove.push_back(i);
			bestScore = mvScore;
		}
		else if (mvScore == bestScore)
		{
			bestMove.push_back(i);
		}
	}

	if (bestMove.size() == 0)
		return WRB_Chess::Move();

	WRB_Chess::Move choice = mvs[bestMove[rand() % bestMove.size()]];
	d = bestScore;

	//std::cout << "\tChoosing move " << WRB_Chess::GetPrintable(choice) << " from " << bestMove.size() << " candidates with an expected score of " << bestScore << std::endl;

	return choice;
}

std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> WRB_Chess::Expectimax::RecommendScanMovePolicy(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c)
{
	if (brds.boards.size() == 0)
		return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>();

	WRB_Chess::Bitboard firstB = *brds.boards.cbegin();
	std::vector<WRB_Chess::Move> mvs = ((firstB)).AvailableMoves(c);

	std::unordered_map<WRB_Chess::Bitboard, std::unordered_map<WRB_Chess::Move, double, WRB_Chess::MoveHash>, WRB_Chess::BoardHash> mvScore;

	for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
	{
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard eBrd = (*it);
			eBrd.ApplyMove(mvs[i]);
			if (!scores.contains(eBrd))
			{
				scores[eBrd] = -1.0 * EvaluatePosition(eBrd, OPPOSITE_COLOR(c));
			}

			double bP = 0.0;
			if (brds.probability.count((*it)) > 0)
			{
				bP = brds.probability.at(*it).p;
			}

			mvScore[(*it)][mvs[i]] = scores[eBrd] * bP;
		}
	}

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
