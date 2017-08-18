#pragma once

enum TextureType
{
	Tex2D = 0,
	Skybox = 1
};

class Texture
{
	TextureType m_type;
	GLuint m_texID;
	GLuint m_activeTex;
public:
	Texture();
	Texture(string filepath);
	Texture(const char * filename, const char * directory);
	Texture(aiTexture *texture);

	void LoadTexture(string filepath);
	void LoadSkybox(string filepath);
	void TriFiltering();
	
	void Bind(GLuint activeTex);
	void Unbind();


	GLuint GetTexID();
};