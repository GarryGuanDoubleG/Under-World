#pragma once
#include <iostream>
#include <GL\glew.h>
#include <SDL.h>

#include "graphics.hpp"
#include "shader.hpp"
#include "ResManager.hpp"


class Game
{
private:
	bool m_running;

	Graphics *m_graphics;
	ResManager *m_resManager;
public:
	Game();
	~Game();

	void Draw();
	void Update();
	void Input();

	bool IsRunning();



};