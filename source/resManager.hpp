#pragma once
#include <GL\glew.h>
#include <SDL.h>

class ResManager
{
public:
	ResManager();
	~ResManager();

	void LoadResources();
	void LoadShaders(std::string filepath);
};