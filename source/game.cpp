#include "game.hpp"

Game::Game()
{
	m_graphics = new Graphics(1680, 1080);
	m_resManager = new ResManager();

	m_running = true;
}

Game::~Game()
{
	delete m_graphics;
	delete m_resManager;
}

void Game::Draw()
{
	m_graphics->Render();
}

void Game::Update()
{
	if (!m_running)
	{
		m_graphics->Cleanup();
	}

}

void Game::Input()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			m_running = false;

		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				m_running = false;
				break;
			default:
				break;
			}
		}
	}

}

bool Game::IsRunning()
{
	return m_running;
}