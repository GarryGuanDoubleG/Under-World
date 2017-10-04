#pragma once

class Skydome
{
	Model *m_sphere;
	// Constant color values:
	static const glm::vec3 sun_dawn;
	static const glm::vec3 sun_noon;
	static const glm::vec3 sun_dusk;
	static const glm::vec3 sun_midnight;

	static const glm::vec3 zenith_dawn;
	static const glm::vec3 horizon_dawn;

	static const glm::vec3 zenith_noon;
	static const glm::vec3 horizon_noon;

	static const glm::vec3 zenith_dusk;
	static const glm::vec3 horizon_dusk;

	static const glm::vec3 zenith_midnight;
	static const glm::vec3 horizon_midnight;

	static const float altitude_margin;

public:
	float m_timeOfDay, m_timeScale;
	float m_altitude, m_azimuth, m_interp, m_interp_night;
	float m_sunIntensity;

	glm::vec3 m_sunDirection;
	glm::vec3 m_sunColor;

public:
	Skydome(Model *model);
	~Skydome();

	void Update();
	void Draw(Shader * shader);
	void upload_sun(const GLuint shader, const Camera &camera);
	void update_light_space(const Camera &camera);
	void reset_time();
	glm::mat4 get_light_space_matrix();
	void update_sun_frustum(const glm::vec3 sun_pos, const glm::vec3 sun_front, const glm::vec3 sun_right);
	bool sphere_in_sun_frustum(glm::vec3 center, float radius);

	//update per frame

	void CalculateSun();
	float GetAzimuth();
	float GetAltitude();
	glm::vec3 GetSunColor();
	glm::vec3 GetSunDirection();
	void propagate_time(const float elapsed_time);
};
