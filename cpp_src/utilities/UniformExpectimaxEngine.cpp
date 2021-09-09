#include "UniformExpectimaxEngine.h"
#include <iostream>
#include <unordered_map>

short WRB_Chess::UniformExpectimax::RecommendScan(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c, double& d)
{
	d = 0;
	return RecommendScanMovePolicy(brds, c).first;
}

WRB_Chess::Move WRB_Chess::UniformExpectimax::RecommendMove(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c, double& d)
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

			mvScore += scores[eBrd];
		}

		mvScore = mvScore / ((long double)brds.boards.size());

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

std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> WRB_Chess::UniformExpectimax::RecommendScanMovePolicy(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c)
{
	if (brds.boards.size() == 0)
		return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>();

	std::vector<WRB_Chess::Move> mvs = (*(brds.boards.cbegin())).AvailableMoves(c);

	std::unordered_map<WRB_Chess::Bitboard, std::unordered_map<WRB_Chess::Move, double, WRB_Chess::MoveHash>, WRB_Chess::BoardHash> mvScore;

	for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
	{
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard eBrd = (*it);
			eBrd.ApplyMove(mvs[i]);
			if (scores.contains(eBrd))
			{
				scores[eBrd] = -1.0 * EvaluatePosition(eBrd, OPPOSITE_COLOR(c));
			}

			mvScore[(*it)][mvs[i]] = scores[eBrd];
		}
	}

	short bestSense = -1;
	int minN = -1;
	double bestES = 0.0;
	std::unordered_map<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>> policies;
	for (int j = 1; j < 7; j++)
	{
		for (int i = 1; i < 7; i++)
		{
			std::unordered_map<WRB_Chess::Bitboard, std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>, WRB_Chess::BoardHash> partition;
			for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
			{
				WRB_Chess::Bitboard masked = (*it).senseMask(i + 8*j);
				partition[masked].emplace((*it));
			}
	
			double ES = 0.0;
			int maxN = 0;
			
			for (auto it = partition.begin(); it != partition.end(); it++)
			{
				int bestMv = -1;
				double sc = 0.0;

				for (int k = 0; k < mvs.size(); k++)
				{
					double lms = 0.0;
					for (auto bd = it->second.cbegin(); bd != it->second.cend(); bd++)
					{
						lms += mvScore[(*bd)][mvs[k]];
					}

					if ((bestMv == -1) || (lms > sc))
					{
						bestMv = k;
						sc = lms;
					}
				}
				policies[i + 8*j][it->first] = mvs[bestMv];
				ES += sc;
				if (it->second.size() > maxN)
				{
					maxN = it->second.size();
				}
			}
	
			if ((bestSense == -1) || (bestES < ES))
			{
				bestES = ES;
				bestSense = i + 8 * j;
				minN = maxN;
			}
			else if ((bestES == ES) && (maxN < minN))
			{
				bestES = ES;
				bestSense = i + 8 * j;
				minN = maxN;
			}
		}
	}
	
	std::cout << "\t\t\tExpected score from scan: " << bestES / ((double) brds.boards.size()) << std::endl;
	
	return std::pair<short, std::unordered_map<WRB_Chess::Bitboard, WRB_Chess::Move, WRB_Chess::BoardHash>>(bestSense,policies[bestSense]);
}
