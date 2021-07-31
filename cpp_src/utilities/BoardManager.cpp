#include "BoardManager.h"

WRB_Chess::BoardManager::BoardManager()
{

}

void WRB_Chess::BoardManager::Initialize(WRB_Chess::Color c, WRB_Chess::Bitboard b)
{
	boards.emplace(b);
}