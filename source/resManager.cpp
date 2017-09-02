#include <map>
#include "game.hpp"

#define MAX_MEMORY 4294967296 //4gigs
#define RESOURCE_PATH "Resources\\resources.json"

static ResManager *g_resManager;

ResManager::ResManager()
{
	if (g_resManager)
	{
		cout << "Error: Resource Manager has already been initialized\n";
		return;
	}

	g_resManager = this;

	LoadResources();
	ReserveMemory();
}

ResManager::~ResManager()
{
}

void ResManager::LoadResources()
{
	//parse json file into json obj
	//loads the location of resource files
	ifstream in(RESOURCE_PATH);
	if (!in.is_open()) return;

	in >> m_resources;
	in.close();
}

template <typename T> T *ResManager::Allocate(int size)
{

}

void ResManager::ReserveMemory()
{
	//m_memory = malloc(MAX_MEMORY);
}


map<string, Texture*> ResManager::LoadTextures()
{
	map<string, Texture*> texture_map;

	ifstream in(m_resources["textures"].get<string>());
	if (!in.is_open()) {
		cout << "Incorrect filename: " << m_resources["textures"].get<string>() << endl;
		return texture_map;
	}

	Json textures;
	in >> textures;

	for (Json::iterator it = textures.begin(); it != textures.end(); ++it)
	{
		Json obj = it.value();
		string key = it.key();

		if (key == "skybox") {
			Texture *skybox = new Texture();
			skybox->LoadSkybox(obj["path"].get<string>());
			texture_map.insert(pair<string, Texture*>(key,skybox));
		}
		else {
			if(obj.find("normal") != obj.end())
				texture_map.insert(pair<string, Texture*>(key + "Normal", new Texture(obj["normal"].get<string>())));
			if(obj.find("height") !=obj.end())
				texture_map.insert(pair<string, Texture*>(key + "Height", new Texture(obj["height"].get<string>())));

			texture_map.insert(pair<string, Texture*>(key, new Texture(obj["path"].get<string>())));
		}
	}

	in.close();

	return texture_map;
	
}

map<string, Shader *> ResManager::LoadShaders()
{
	map<string, Shader*> shader_map;
	ifstream in(m_resources["shaders"].get<string>());
	
	if (!in.is_open()) {
		cout << "Incorrect filename: " << m_resources["shaders"].get<string>() << endl;
		return shader_map;
	}

	Json shaders;
	in >> shaders;

	for (Json::iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		Json obj = it.value();
		string key = it.key();

		shader_map.insert(pair<string, Shader*>(key, new Shader(obj["vs"].get<string>(), obj["fs"].get<string>())));
	}

	in.close();

	return shader_map;
}

map<string, Model*> ResManager::LoadModels()
{
	map<string, Model*> model_map;
	ifstream i(m_resources["models"].get<string>());

	if (!i.is_open()) {
		cout << "Incorrect filename: " << m_resources["shaders"].get<string>() << endl;
		return model_map;
	}

	Json shaders;
	i >> shaders;

	for (Json::iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		Json obj = it.value();
		string key = it.key();

		model_map.insert(pair<string, Model*>(key, new Model(obj["path"].get<string>())));
	}

	return model_map;
}
