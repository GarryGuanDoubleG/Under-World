#pragma once
#include <json.hpp>
using Json = nlohmann::json;

#include <iostream>
#include <fstream>
#include <vector>
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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string.h>

using namespace std;

#include "mathUtil.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "animController.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "graphics.hpp"
#include "ResManager.hpp"

#define SCREEN_WIDTH 1680.0f
#define SCREEN_HEIGHT 1080.0f

class Game
{
	bool m_running;
	int m_flag;

	Camera *m_camera;
	Graphics *m_graphics;
	ResManager *m_resManager;


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

	bool IsRunning();

private:
	void InitFlags();
};