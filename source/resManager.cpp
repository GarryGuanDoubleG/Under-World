#include <map>
#include "game.hpp"

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


map<string, Texture*> ResManager::LoadTextures()
{
	map<string, Texture*> texture_map;

	ifstream i(m_resources["textures"].get<string>());
	if (!i.is_open()) {
		cout << "Incorrect filename: " << m_resources["textures"].get<string>() << endl;
		return texture_map;
	}

	Json textures;
	i >> textures;

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
			texture_map.insert(pair<string, Texture*>(key, new Texture(obj["path"].get<string>())));
		}
	}

	return texture_map;
	
}

map<string, Shader *> ResManager::LoadShaders()
{
	map<string, Shader*> shader_map;
	ifstream i(m_resources["shaders"].get<string>());
	
	if (!i.is_open()) {
		cout << "Incorrect filename: " << m_resources["shaders"].get<string>() << endl;
		return shader_map;
	}

	Json shaders;
	i >> shaders;

	for (Json::iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		Json obj = it.value();
		string key = it.key();

		shader_map.insert(pair<string, Shader*>(key, new Shader(obj["vs"].get<string>(), obj["fs"].get<string>())));
	}

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
