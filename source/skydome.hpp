#pragma once

class Skydome
{
	Model *m_sphere;
	glm::vec3 m_sunLightDir;
	glm::vec3 m_waveLength;

	float outerRadius;
	float outerRadiusSQ;
	float innerRadius;
	float innerRadiusSQ;

	float fKrESun;
	float fKmESun;
	float fKr4Pi;
	float fKm4Pi;
	float fScale;
	float fScaleDepth;
	float fScaleOverScaleDepth;
public:
	Skydome(Model *model, float outerRadius = 100, float innerRadius = 100);
	~Skydome();
	float time_of_day, time_scale;

	void draw(Camera *camera, Shader * shader);
	void upload_sun(const GLuint shader, const Camera &camera);
	void propagate_time(const float elapsed_time);
	void update_light_space(const Camera &camera);
	void reset_time();
	glm::mat4 get_light_space_matrix();
	void update_sun_frustum(const glm::vec3 sun_pos, const glm::vec3 sun_front, const glm::vec3 sun_right);
	bool sphere_in_sun_frustum(glm::vec3 center, float radius);
};
