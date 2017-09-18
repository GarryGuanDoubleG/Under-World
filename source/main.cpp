#include "game.hpp"

/**
* @brief main game loop
* @return 0 if properly exited
*/
int main(int argc, char **argv)
{
	g_game = new Game();

	//main game loop
	while (g_game->IsRunning())
	{
		g_game->Input();
		g_game->Update();
		g_game->Draw();
	}

	g_game->Close();
	delete g_game;
	return 0;
}