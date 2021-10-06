#ifndef WRB_GAME_HISTORY_H_
#define WRB_GAME_HISTORY_H_

#include "../utilities/json.hpp"
using json = nlohmann::json;

namespace WRB_Chess
{
	class GameHistory
	{
	public:
		json j;

		GameHistory& operator=(const GameHistory& other)
		{
			j = other.j;
			return *this;
		}
	};
}


#endif