#include "game.hpp"

Material::Material(Texture albedo, Texture normal, Texture metallic, Texture roughness)
{
	m_albedo = albedo;
	m_normal = normal;
	m_metallic = metallic;
	m_roughness = roughness;
}

Material::~Material()
{
	m_albedo.Destroy();
	m_normal.Destroy();
	m_metallic.Destroy();
	m_roughness.Destroy();
}

void Material::BindMaterial(Shader * shader, GLuint activeTex)
{
	m_albedo.Bind(shader->Uniform("albedo"), activeTex);
	m_normal.Bind(shader->Uniform("normalMap"), activeTex + 1);
	m_metallic.Bind(shader->Uniform("metallic"), activeTex + 2);
	m_roughness.Bind(shader->Uniform("roughness"), activeTex + 3);
}