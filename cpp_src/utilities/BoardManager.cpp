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

std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> WRB_Chess::BoardManager::AdvanceOpponentMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> &brds, short capture_square, WRB_Chess::Color c)
{
	std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> newBoards;

	for (auto it = brds.begin(); it != brds.end(); it++)
	{
		std::vector<WRB_Chess::Move> mvs = (*it).AvailableMoves(OPPOSITE_COLOR(c));
		for (int i = 0; i < mvs.size(); i++)
		{
			WRB_Chess::Bitboard b = (*it);
			bool cap = false;
			short cS = -1;
			b.ApplyMove(mvs[i], cap, cS);

			if (cS == capture_square && b.KingsAlive())
			{
				newBoards.emplace(b);
			}
		}
	}

	return newBoards;
}

void WRB_Chess::BoardManager::OpponentMove(short captureSquare)
{
	boards = WRB_Chess::BoardManager::AdvanceOpponentMove(boards, captureSquare, c);
}

void WRB_Chess::BoardManager::SenseResult(std::vector<std::pair<short, WRB_Chess::ColorPiece>> &sR)
{
	std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> newBoards;

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

		if (good)
		{
			WRB_Chess::Bitboard b = (*it);
			newBoards.emplace(b);
		}
	}

	boards = newBoards;
}

void WRB_Chess::BoardManager::TakenMove(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, short capture_square)
{
	std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> newBoards;

	for (auto it = boards.begin(); it != boards.end(); it++)
	{
		WRB_Chess::Bitboard b = (*it);
		bool cap = false;
		short cS = -1;
		WRB_Chess::Move taken = b.ApplyMove(requested_move, cap, cS);

		if ((cS == capture_square) && (taken == taken_move) && b.KingsAlive())
		{
			newBoards.emplace(b);
		}
	}

	boards = newBoards;
}