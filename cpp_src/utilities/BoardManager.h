#ifndef WRB_BOARD_MANAGER_H_
#define WRB_BOARD_MANAGER_H_

#include "../chess/board.h"
#include <unordered_set>

namespace WRB_Chess
{
	class BoardManager
	{
	private:
		std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash> boards;
	public:
		BoardManager();

		void Initialize(WRB_Chess::Color c, WRB_Chess::Bitboard b);

		inline unsigned int size() { return boards.size(); };
	};
};

#endif
