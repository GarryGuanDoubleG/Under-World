#include "game.hpp"

static float g_prevTime;

#define FP_MODE 1

float Game::GetElapsedTime()
{
	return SDL_GetTicks();
}

float Game::GetDeltaTime()
{
	return SDL_GetTicks() - g_prevTime;
}

Game::Game() : m_running(true)
{
	m_resManager = new ResManager();
	m_camera = new Camera(glm::vec3(-2.f, 2.f, -2.f), glm::vec3(0.f));

	m_graphics = new Graphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_graphics->SetShaders(m_resManager->LoadShaders());
	m_graphics->SetTextures(m_resManager->LoadTextures());
	m_graphics->SetModel(m_resManager->LoadModels());
	m_graphics->SetCamera(m_camera);

	InitFlags();

	g_prevTime = 0;
}

Game::~Game()
{
	delete m_graphics;
	delete m_resManager;
}

void Game::Draw()
{
	GLfloat bg_color[] = { 0.3f, 0.3f, 0.3f, 1.f };

	m_graphics->RenderBackground(bg_color);
	m_graphics->RenderSkybox();
	m_graphics->RenderCube(glm::mat4(1.0f));
	m_graphics->Render();
}

void Game::Update()
{
	if (!m_running)
	{
		Close();
		exit(0);
	}

	g_prevTime = SDL_GetTicks();

	if (m_flag & FP_MODE) {
		SDL_WarpMouseInWindow(m_graphics->GetWindow(), SCREEN_WIDTH * .5f, SCREEN_HEIGHT * .5f);
	}
}

void Game::Close()
{
	m_graphics->Cleanup();
}

void Game::Input()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		m_camera->HandleInput(event);

		if (event.type == SDL_QUIT)
			m_running = false;

		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				m_running = false;
				break;
			case SDLK_q:
				if (m_flag & FP_MODE)
					m_flag &= ~FP_MODE;
				else
					m_flag |= FP_MODE;
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

void Game::InitFlags()
{
	m_flag = FP_MODE;
}
