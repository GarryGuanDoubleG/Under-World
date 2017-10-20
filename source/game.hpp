#pragma once
#include <json.hpp>
using Json = nlohmann::json;
using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <unordered_map>
#include <GL\glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <omp.h>

#include <ImGui\imgui.h>
#include <ImGui\imgui_impl_sdl_gl3.h>

#include <FastNoiseSIMD.h>
#include <FastNoise.h>
#include "simple_logger.h"
#include "mathUtil.hpp"

#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "animController.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "entity.hpp"
#include "LightSource.hpp"
#include "resManager.hpp"
#include "skydome.hpp"
#include "tables.hpp"
#include "SVD.h"
#include "QEFSolver.h"
#include "VoxelVertex.hpp"
#include "density.hpp"
#include "octree.hpp"
#include "chunk.hpp"
#include "voxelManager.hpp"
#include "graphics.hpp"
#include "atmosphere.hpp"
#include "weather.hpp"

#define SCREEN_WIDTH 1680.0f
#define SCREEN_HEIGHT 1080.0f

#define FP_MODE 1
#define WIRE_FRAME_MODE 2
#define SKYBOX_MODE 4
#define VOXEL_MODE 8
#define MODEL_MODE 16
#define SHADOW_MODE 32
#define DEFERRED_MODE 64
class Game;
extern Game *g_game;

class Game
{

private:
	bool m_running;
	int m_flag;

	Camera *m_camera;
	Graphics *m_graphics;
	ResManager *m_resManager;
	Entity *m_entitiesList;

	Atmosphere *m_atmosphere;
	Weather *m_weather;
public:
	Skydome *m_skydome;
	VoxelManager *m_voxelManager;

public:

	//get time in milliseconds since SDL initiation
	static float GetElapsedTime();
	//get time since last game update
	static float GetDeltaTime();

	Game();
	~Game();
	//loop
	void Draw();
	void Update();
	void Close();
	void Input();

	glm::vec3 GetPlayerPosition();

	Shader * GetShader(const char * name);

	Texture * GetTexture(const char * name);

	Camera * GetCamera();

	bool IsRunning();
};