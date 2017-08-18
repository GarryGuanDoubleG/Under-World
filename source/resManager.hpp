#pragma once

class ResManager
{
private:
	Json m_resources;

public:
	ResManager();
	~ResManager();

	void LoadResources();

	map<string, Texture*> LoadTextures();
	map<string, Shader *> LoadShaders();
	map<string, Model *> LoadModels();
};