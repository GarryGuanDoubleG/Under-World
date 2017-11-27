#pragma once

class Material
{
public:
	Texture m_albedo;
	Texture m_normal;
	//Texture m_heightMap;
	Texture m_metallic;
	Texture m_roughness;
public:
	Material(Texture albedo, Texture normal, Texture metallic, Texture roughness);
	~Material();

	void BindMaterial(Shader *shader, GLuint activeTex);
	
};