#include "game.hpp"
#include <array>

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
	m_skydome = m_graphics->InitSkybox();

	ImGui_ImplSdlGL3_Init(m_graphics->GetWindow());
	ImGui_ImplSdlGL3_NewFrame(m_graphics->GetWindow());

	m_atmosphere = new Atmosphere(m_graphics->GetVAO("quad"));
	m_atmosphere->Precompute();

	m_weather = new Weather(m_graphics->GetVAO("quad"));
	m_weather->PrecomputeNoise();

	m_entitiesList = new Entity[MAX_ENTITIES];
	for (int i = 0; i < MAX_ENTITIES - 1; i++)
		m_entitiesList[i].m_nextFree = &m_entitiesList[i + 1];

	m_voxelManager = new VoxelManager();
	m_voxelManager->Init();

	SlogCheckGLError();
	m_graphics->SetFlag(FP_MODE | SKYBOX_MODE | VOXEL_MODE | MODEL_MODE | SHADOW_MODE);
	g_prevTime = 0;

	SlogCheckGLError();
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GBuffer gBuffer;
	if (m_flag & DEFERRED_MODE)
	{
		m_graphics->RenderScene();
	}
	else
	{
		gBuffer = m_graphics->DeferredRenderScene();
		Texture postProcessing = m_atmosphere->Render(gBuffer, GetTexture("scene"));
		m_weather->Render(gBuffer, &postProcessing);
	}
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_test_window = true;
	bool show_another_window = false;
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	ImGui::Render();
	m_graphics->Display();
}

void Game::Update()
{
	ImGui_ImplSdlGL3_NewFrame(m_graphics->GetWindow());

	m_voxelManager->Update();
	m_skydome->Update();
	m_weather->Update();

	if(m_flag & FP_MODE) {
		SDL_WarpMouseInWindow(m_graphics->GetWindow(), SCREEN_WIDTH * .5f, SCREEN_HEIGHT * .5f);
	}

	//slog("FPS %5f", 1000.0f / GetDeltaTime());
	g_prevTime = SDL_GetTicks();
}
 
void Game::Close()
{
	ImGui_ImplSdlGL3_Shutdown();
	m_voxelManager->Close();
	m_graphics->Cleanup();
}

void Game::Input()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		m_camera->HandleInput(event);
		ImGui_ImplSdlGL3_ProcessEvent(&event);
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
			case SDLK_4:
				m_flag ^= DEFERRED_MODE;
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

Shader * Game::GetShader(const char * name)
{
	return m_graphics->GetShader(name);
}

Texture * Game::GetTexture(const char *name)
{
	return m_graphics->GetTexture(name);
}

Camera * Game::GetCamera()
{
	return m_camera;
}

bool Game::IsRunning()
{
	return m_running;
}

