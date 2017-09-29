#include "game.hpp"


Skydome::Skydome(Model * model, float outerRadius, float innerRadius)
{
	m_sphere = model;
	this->outerRadius = outerRadius;
	this->innerRadius = innerRadius;
}

Skydome::~Skydome()
{
}

void Skydome::draw(Camera * camera, Shader * shader)
{	
	//shader->SetMat4("projection", camera->GetProj());
	//shader->SetMat4("view", camera->GetViewMat());

	glm::vec3 light_dir(-.2f, -1.f, -0.3f);

	m_sphere->Draw(shader);
}

void Skydome::upload_sun(const GLuint shader, const Camera & camera)
{
}

void Skydome::propagate_time(const float elapsed_time)
{
}

void Skydome::update_light_space(const Camera & camera)
{
}

void Skydome::reset_time()
{
}

glm::mat4 Skydome::get_light_space_matrix()
{
	return glm::mat4();
}

void Skydome::update_sun_frustum(const glm::vec3 sun_pos, const glm::vec3 sun_front, const glm::vec3 sun_right)
{
}

bool Skydome::sphere_in_sun_frustum(glm::vec3 center, float radius)
{
	return false;
}
