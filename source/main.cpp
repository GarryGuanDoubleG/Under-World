#include <iostream>
#include "game.hpp"


/**
* @brief main game loop
* @return 0 if properly exited
*/
int main(int argc, char **argv)
{
	Game *game = new Game();

	//main game loop

	while (game->IsRunning())
	{
		game->Input();
		game->Update();
		game->Draw();
	}

//	graphics->Cleanup();

	return 0;
}