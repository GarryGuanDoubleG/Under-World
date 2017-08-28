#pragma once
#include <json.hpp>
using Json = nlohmann::json;
using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <thread>
#include <GL\glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <glm/gtc/noise.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Game;
extern Game *g_game;
#include "mathUtil.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "animController.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "resManager.hpp"
#include "density.hpp"
#include "octree.hpp"
#include "chunk.hpp"
#include "voxelManager.hpp"
#include "graphics.hpp"

#define SCREEN_WIDTH 1680.0f
#define SCREEN_HEIGHT 1080.0f

class Game
{
	bool m_running;
	int m_flag;

	Camera *m_camera;
	Graphics *m_graphics;
	ResManager *m_resManager;
	VoxelManager *m_voxelManager;
public:
	//get time in milliseconds since SDL initiation
	static float GetElapsedTime();
	//get time since last game update
	static float GetDeltaTime();

	Game();
	~Game();

	void Draw();
	void Update();
	void Close();
	void Input();

	glm::vec3 GetPlayerPosition();

	bool IsRunning();
private:
	void InitFlags();
};