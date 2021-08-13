#ifndef WRB_BOARD_MANAGER_H_
#define WRB_BOARD_MANAGER_H_

#include "../chess/board.h"
#include "MoveProbability.h"
#include <unordered_set>
#include <unordered_map>

namespace WRB_Chess
{

	struct DoubleDefaulted
	{
		double p = 0;
	};

	struct InformationSet
	{
		std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> boards;
		std::unordered_map<WRB_Chess::Bitboard, DoubleDefaulted, WRB_Chess::BoardHash> probability;

		InformationSet& operator= (const InformationSet& other)
		{
			this->boards = other.boards;
			this->probability = other.probability;
			return *this;
		};
	};

	class BoardManager
	{
	private:
		WRB_Chess::Color c;
		WRB_Chess::MoveProbability* mP;
	public:
		InformationSet infoSet;
		
		BoardManager(WRB_Chess::MoveProbability*);
		BoardManager();
		~BoardManager();

		void Initialize(WRB_Chess::Color c, WRB_Chess::Bitboard b);
		void OpponentMove(short captureSquare);
		void SenseResult(std::vector<std::pair<short, WRB_Chess::ColorPiece>>&);
		void TakenMove(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, short capture_square);

		InformationSet AdvanceOpponentMove(const InformationSet &brds, short capture_square, WRB_Chess::Color);

		inline unsigned int size() { return infoSet.boards.size(); };
	};
};

#endif
