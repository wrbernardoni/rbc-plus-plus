#ifndef WRB_BOARD_MANAGER_H_
#define WRB_BOARD_MANAGER_H_

#include "../chess/board.h"
#include <unordered_set>

namespace WRB_Chess
{
	class BoardManager
	{
	private:
		WRB_Chess::Color c;
	public:
		std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> boards;
		
		BoardManager();

		void Initialize(WRB_Chess::Color c, WRB_Chess::Bitboard b);
		void OpponentMove(short captureSquare);
		void SenseResult(std::vector<std::pair<short, WRB_Chess::ColorPiece>>&);
		void TakenMove(WRB_Chess::Move requested_move, WRB_Chess::Move taken_move, short capture_square);

		static std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> AdvanceOpponentMove(const std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> &brds, short capture_square, WRB_Chess::Color);

		inline unsigned int size() { return boards.size(); };
	};
};

#endif
