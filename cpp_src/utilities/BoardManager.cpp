#include "BoardManager.h"

#include "../reconUtils/BotBase.h"

WRB_Chess::BoardManager::BoardManager()
{

}

void WRB_Chess::BoardManager::Initialize(WRB_Chess::Color color, WRB_Chess::Bitboard b)
{
	boards.emplace(b);
	c = color;
}

void WRB_Chess::BoardManager::OpponentMove(short captureSquare)
{
	std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> newBoards;

	bool trueBoardGen = false;
	for (auto it = boards.begin(); it != boards.end(); it++)
	{
		std::vector<WRB_Chess::Move> mvs = (*it).AvailableMoves(OPPOSITE_COLOR(c));
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard b = (*it);
			bool cap = false;
			short cS = -1;
			b.ApplyMove(mvs[i], cap, cS);

			if (b == trueBoard)
			{
				trueBoardGen = true;
			}

			if (cS == captureSquare && b.KingsAlive())
			{
				newBoards.emplace(b);
			}
			else if (b == trueBoard)
			{
				std::cout << "True board discarded at opponent inference" << std::endl;
				std::cout << "TCS:" << captureSquare << " FCS:" << cS << std::endl;
				exit(1);
			}
		}
	}

	if (!trueBoardGen)
	{
		std::cout << "True board never generated -- opponent inference" << std::endl;
		exit(1);
	}

	boards = newBoards;
}

void WRB_Chess::BoardManager::SenseResult(std::vector<std::pair<short, WRB_Chess::ColorPiece>> sR)
{
	std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> newBoards;

	bool trueBoardGen = false;
	for (auto it = boards.begin(); it != boards.end(); it++)
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

		if ((*it) == trueBoard)
			trueBoardGen = true;

		if (good)
		{
			WRB_Chess::Bitboard b = (*it);
			newBoards.emplace(b);
		}
		else if ((*it) == trueBoard)
		{
			std::cout << "True board discarded at sense" << std::endl;
			exit(1);
		}
	}

	if (!trueBoardGen)
	{
		std::cout << "True board never generated -- sense inference" << std::endl;
		exit(1);
	}

	boards = newBoards;
}

void WRB_Chess::BoardManager::TakenMove(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, short capture_square)
{
	std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> newBoards;

	bool trueBoardGen = false;
	for (auto it = boards.begin(); it != boards.end(); it++)
	{
		WRB_Chess::Bitboard b = (*it);
		bool cap = false;
		short cS = -1;
		WRB_Chess::Move taken = b.ApplyMove(requested_move, cap, cS);

		if (b == trueBoard)
		{
			trueBoardGen = true;
		}

		if ((cS == capture_square) && (taken == taken_move) && b.KingsAlive())
		{
			newBoards.emplace(b);
		}
		else if ((b == trueBoard) && (b.KingsAlive()))
		{
			std::cout << "True board discarded at move inference" << std::endl;
			std::cout << "RM:" << WRB_Chess::GetPrintable(requested_move) << " TM:" << WRB_Chess::GetPrintable(taken_move) << " OM:" << WRB_Chess::GetPrintable(taken) << " ACS:" << capture_square << " OCS: " << cS << std::endl;
			exit(1);
		}
	}

	if (!trueBoardGen)
	{
		std::cout << "True board never generated -- move inference" << std::endl;
		exit(1);
	}

	boards = newBoards;
}