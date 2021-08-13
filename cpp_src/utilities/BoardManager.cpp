#include "BoardManager.h"

#include "../reconUtils/BotBase.h"

#include <iostream>

WRB_Chess::BoardManager::BoardManager()
{
	mP = new WRB_Chess::MoveProbability();
}

WRB_Chess::BoardManager::BoardManager(WRB_Chess::MoveProbability* p)
{
	mP = p;
}

WRB_Chess::BoardManager::~BoardManager()
{
	delete mP;
}

void WRB_Chess::BoardManager::Initialize(WRB_Chess::Color color, WRB_Chess::Bitboard b)
{
	infoSet.boards.emplace(b);
	infoSet.probability[b].p = 1;
	c = color;
}

WRB_Chess::InformationSet WRB_Chess::BoardManager::AdvanceOpponentMove(const WRB_Chess::InformationSet &brds, short capture_square, WRB_Chess::Color c)
{
	WRB_Chess::InformationSet iS;
	double totalMass = 0.0;

	for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
	{
		std::vector<WRB_Chess::Move> mvs = (*it).AvailableMoves(OPPOSITE_COLOR(c));
		std::vector<double> mProb = mP->moveProb((*it), mvs);
		for (int i = 0; i < mvs.size(); i++)
		{
			if (mProb[i] == 0)
				continue;
			
			WRB_Chess::Bitboard b = (*it);
			bool cap = false;
			short cS = -1;
			b.ApplyMove(mvs[i], cap, cS);

			if (cS == capture_square && b.KingsAlive())
			{
				iS.boards.emplace(b);
				if (brds.probability.count((*it)) > 0)
				{
					double p = brds.probability.at((*it)).p * mProb[i];
					iS.probability[b].p += p;
					totalMass += p;
				}
				else
				{
					//std::cout << "bprob missing" << std::endl;
				}
			}
		}
	}

	if (totalMass != 0)
	{
		for (auto it = iS.boards.begin(); it != iS.boards.end(); it++)
		{
			iS.probability[(*it)].p = iS.probability[(*it)].p / (totalMass);
			//std::cout << "@AOM " << iS.probability[(*it)].p << std::endl;
		}
	}

	return iS;
}

void WRB_Chess::BoardManager::OpponentMove(short captureSquare)
{
	infoSet = WRB_Chess::BoardManager::AdvanceOpponentMove(infoSet, captureSquare, c);
}

void WRB_Chess::BoardManager::SenseResult(std::vector<std::pair<short, WRB_Chess::ColorPiece>> &sR)
{
	InformationSet iS;
	double totalMass = 0.0;

	for (auto it = infoSet.boards.begin(); it != infoSet.boards.end(); it++)
	{
		bool good = true;
		for (int i = 0; i < sR.size(); i++)
		{
			WRB_Chess::ColorPiece pAt = (*it).PieceAt(sR[i].first);
			if (pAt != sR[i].second)
			{
				good = false;
				break;
			}
		}

		if (good)
		{
			WRB_Chess::Bitboard b = (*it);
			iS.boards.emplace(b);
			iS.probability[b].p = infoSet.probability[b].p;
			totalMass += infoSet.probability[b].p;
		}
	}

	if (totalMass != 0)
	{
		for (auto it = iS.boards.begin(); it != iS.boards.end(); it++)
		{
			iS.probability[(*it)].p = iS.probability[(*it)].p / (totalMass);
			//std::cout << "@SR " << iS.probability[(*it)].p << std::endl;
		}
	}
	
	infoSet = iS;
}

void WRB_Chess::BoardManager::TakenMove(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, short capture_square)
{
	InformationSet iS;
	double totalMass = 0.0;

	for (auto it = infoSet.boards.begin(); it != infoSet.boards.end(); it++)
	{
		WRB_Chess::Bitboard b = (*it);
		bool cap = false;
		short cS = -1;
		WRB_Chess::Move taken = b.ApplyMove(requested_move, cap, cS);

		if ((cS == capture_square) && (taken == taken_move) && b.KingsAlive())
		{
			iS.boards.emplace(b);
			iS.probability[b].p = infoSet.probability[*it].p;
			totalMass += infoSet.probability[*it].p;
		}
	}

	if (totalMass != 0)
	{
		for (auto it = iS.boards.begin(); it != iS.boards.end(); it++)
		{
			iS.probability[(*it)].p = iS.probability[(*it)].p / (totalMass);
			//std::cout << "@TM " << iS.probability[(*it)].p << std::endl;
		}
	}

	infoSet = iS;
}
