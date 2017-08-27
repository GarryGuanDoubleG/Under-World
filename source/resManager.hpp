#pragma once

typedef struct MemBlock_S
{
	int start;
	int end;
}MemBlock;

class ResManager
{
	Json m_resources;

	void *m_memory;
public:
	ResManager();
	~ResManager();

	void LoadResources();
	map<string, Texture*> LoadTextures();
	map<string, Shader *> LoadShaders();
	map<string, Model *> LoadModels();

	template <typename T> T *Allocate(int size);
	void ReserveMemory();
};

