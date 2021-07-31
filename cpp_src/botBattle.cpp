#include "reconUtils/LocalGame.h"
#include "chess/board.h"
#include "bots/inference.h"

#include <iostream>

using namespace std;

int main()
{
	WRB_Chess::BoardHash::Init();

	WRB_Chess::LocalGame game(900.0);
	WRB_Bot::Inference whiteBot;
	WRB_Bot::Inference blackBot;

	cout << "Beginning bot match. 900 seconds per bot." << endl;
	whiteBot.handle_game_start(WRB_Chess::Color::White, WRB_Chess::Bitboard(), "");
	blackBot.handle_game_start(WRB_Chess::Color::Black, WRB_Chess::Bitboard(), "");
	game.start();

	WRB_Chess::BotBase* bots[2];
	bots[0] = &whiteBot;
	bots[1] = &blackBot;

	double turnCount = 0;
	while(!game.is_over())
	{
		short activeTurn = (short)game.turn();
		cout << turnCount << " " << (activeTurn == 0? "White" : "Black") << " turn starts: " << game.get_seconds_left() << " seconds remaining." << endl;
		cout << WRB_Chess::GetPrintable(game.getBoard()) << endl;
		auto senseActions = game.sense_actions();
		auto moveActions = game.move_actions();

		short captureSquare = game.opponent_move_results();
		bool wasCaptured = (captureSquare >= 0);
		bots[activeTurn]->handle_opponent_move_result(wasCaptured, captureSquare);

		short senseChoice = bots[activeTurn]->choose_sense(senseActions, moveActions, game.get_seconds_left());
		if (senseChoice != -1)
		{
			cout << "Sensing square " << WRB_Chess::SquareNames[senseChoice] << endl;
		}
		else
		{
			cout << "No sense." << endl;
		}
		
		auto senseResult = game.sense(senseChoice);
		bots[activeTurn]->handle_sense_result(senseResult);
		auto mvChoice = bots[activeTurn]->choose_move(moveActions, game.get_seconds_left());
		auto mvResult = game.move(mvChoice);
		cout << "Attempting move " << WRB_Chess::GetPrintable(mvChoice) << endl;
		bots[activeTurn]->handle_move_result(get<0>(mvResult), get<1>(mvResult), (get<2>(mvResult) >= 0), get<2>(mvResult));
		cout << "Took move " << WRB_Chess::GetPrintable(get<1>(mvResult));
		if (get<2>(mvResult) == -1)
		{
			cout << endl;
		}
		else
		{
			cout << " captured a piece at " << WRB_Chess::SquareNames[get<2>(mvResult)] << endl;
		}

		cout << WRB_Chess::GetPrintable(game.getBoard()) << endl;
		game.end_turn();
		turnCount += 0.5;
	}

	cout << "Game over! ";
	if (game.get_winner_color() == WRB_Chess::Color::NoColor)
	{
		cout << "Draw!" << endl;
	}
	else if (game.get_winner_color() == WRB_Chess::Color::White)
	{
		cout << "White Wins!" << endl;
	}
	else
	{
		cout << "Black Wins!" << endl;
	}

	cout << WRB_Chess::GetPrintable(game.getBoard()) << endl;

	bots[0]->handle_game_end(game.get_winner_color(), game.get_game_history());
	bots[1]->handle_game_end(game.get_winner_color(), game.get_game_history());
	
}