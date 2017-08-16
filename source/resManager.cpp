#include <map>
#include <fstream>
#include "ResManager.hpp"
#include "json.hpp"
#include "shader.hpp"

#define RESOURCE_PATH "Resources\\resources.json"

static ResManager *g_resManager;
static std::map<std::string, Shader *> g_shader_map;

using Json = nlohmann::json;

ResManager::ResManager()
{
	if (g_resManager)
	{
		std::cout << "Error: Resource Manager has already been initialized\n";
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
	std::ifstream in(RESOURCE_PATH);
	if (!in.is_open()) return;

	Json resources;
	in >> resources;
	in.close();

	std::string filepath;

	filepath = resources["shaders"].get<std::string>();
	LoadShaders(filepath);
}

void DeleteShaders()
{
	for (auto &kv : g_shader_map)
	{
		glDeleteProgram(kv.second->m_shaderID);
	}

	g_shader_map.clear();
}

void ResManager::LoadShaders(std::string filepath)
{
	std::ifstream i(filepath);
	
	if (!i.is_open()) {
		std::cout << "Incorrect filename: " << filepath << std::endl;
		return;
	}

	Json shaders;
	i >> shaders;

	for (Json::iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		Json obj = it.value();
		std::string key = it.key();

		std::string vs_filename = obj["vs"];
		std::string fs_filename = obj["fs"];

		g_shader_map.insert(std::pair<std::string, Shader*>(key, new Shader(vs_filename, fs_filename)));
	}

	atexit(DeleteShaders);
}

