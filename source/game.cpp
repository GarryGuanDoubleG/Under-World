#include "game.hpp"

Game * g_game;
static float g_prevTime;

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
	g_game = this;

	m_resManager = new ResManager();
	m_camera = new Camera(glm::vec3(20, 150, 20), glm::vec3(0.f));

	m_graphics = new Graphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_graphics->SetShaders(m_resManager->LoadShaders());
	m_graphics->SetTextures(m_resManager->LoadTextures());
	m_graphics->SetModel(m_resManager->LoadModels());
	m_graphics->SetCamera(m_camera);

	m_entitiesList = new Entity[MAX_ENTITIES];
	for (int i = 0; i < MAX_ENTITIES - 1; i++)
		m_entitiesList[i].m_nextFree = &m_entitiesList[i + 1];

	m_voxelManager = new VoxelManager();
	m_voxelManager->Init();

	m_graphics->SetFlag(FP_MODE | SKYBOX_MODE | VOXEL_MODE | MODEL_MODE | SHADOW_MODE);
	g_prevTime = 0;
}

Game::~Game()
{
	delete m_graphics;
	delete m_resManager;
	delete m_voxelManager;
}

void Game::Draw()
{
	GLfloat bg_color[] = { 0.3f, 0.3f, 0.3f, 1.f };

	m_graphics->RenderBackground(bg_color);

	if(m_flag & WIRE_FRAME_MODE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	

	m_graphics->RenderScene();
	m_graphics->Display();
}

void Game::Update()
{
	g_prevTime = SDL_GetTicks();

	m_voxelManager->Update();

	if(m_flag & FP_MODE) {
		SDL_WarpMouseInWindow(m_graphics->GetWindow(), SCREEN_WIDTH * .5f, SCREEN_HEIGHT * .5f);
	}
}

void Game::Close()
{
	m_voxelManager->Close();
	m_graphics->Cleanup();
}

void Game::Input()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		m_camera->HandleInput(event);

		if(event.type == SDL_QUIT)
			m_running = false;

		if(event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				m_running = false;
				break;
			case SDLK_q:				
				m_flag ^=FP_MODE;
				break;
			case SDLK_1:
				m_flag ^=WIRE_FRAME_MODE;
				break;
			case SDLK_2:
				m_graphics->m_flag ^=SKYBOX_MODE;
				break;
			case SDLK_3:
				m_graphics->m_flag ^=VOXEL_MODE;
				break;
			default:
				break;
			}
		}
	}
}

glm::vec3 Game::GetPlayerPosition()
{
	return m_camera->GetPosition();
}

bool Game::IsRunning()
{
	return m_running;
}

