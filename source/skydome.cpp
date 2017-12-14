#include "game.hpp"


const glm::vec3 Skydome::sun_dawn = { .8f, .3f, 0.2f };
const glm::vec3 Skydome::sun_noon = { .5f, .9f, 0.2f };
const glm::vec3 Skydome::sun_dusk = { 0.9f, 0.3f, 0.1f };
const glm::vec3 Skydome::sun_midnight = { 0.f, 0.f, 0.f };

const glm::vec3 Skydome::zenith_dawn = { 0.1f, 0.1f, 0.65f };
const glm::vec3 Skydome::horizon_dawn = { 0.5f, 0.15f, 0.4f };

const glm::vec3 Skydome::zenith_noon = { 0.1f, 1.0f, .4f };
const glm::vec3 Skydome::horizon_noon = { 0.34f, 0.88f, 0.54f };

const glm::vec3 Skydome::zenith_dusk = { 0.5f, 0.4f, 0.3f };
const glm::vec3 Skydome::horizon_dusk = { 0.9f, 0.4f, 0.1f };

const glm::vec3 Skydome::zenith_midnight = { 0.f, 0.0f, 0.01f };
const glm::vec3 Skydome::horizon_midnight = { 0.f, 0.01f, 0.05f };

const float Skydome::altitude_margin = -0.12f;

Skydome::Skydome(Model * model, Camera *camera)
{
	m_sphere = model;


	m_timeScale= .003f;
	m_timeOfDay = 12.f;
	m_sunDirection = -1.f * glm::vec3(.7f, .3f, 0);
	m_sunIntensity = 10.0f;
	//float orthoSize = 50000.0f;
	////glm::mat4 orthoLightProj = glm::ortho(-camera->GetFarPlane(), camera->GetFarPlane(), -camera->GetFarPlane(), camera->GetFarPlane(), 1.0f, camera->GetFarPlane());
	//m_orthoLightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, -orthoSize, 2.0f * orthoSize);
	//glm::mat4 lightViewMat = glm::lookAt(camera->GetPosition(), camera->GetPosition() - m_sunDirection, glm::vec3(0.0, 1.0, 0.0));
	//m_lightSpaceMatrix = m_orthoLightProj * lightViewMat;
}

Skydome::~Skydome()
{
}

void Skydome::Update(Camera *camera)
{
	propagate_time(g_game->GetDeltaTime());
	CalculateSun(camera);
}

void Skydome::Draw(Shader *shader)
{	
	m_sphere->Draw(shader);
}

void Skydome::upload_sun(const GLuint shader, const Camera & camera)
{
}

void Skydome::propagate_time(const float delta_time)
{
	m_timeOfDay += delta_time * m_timeScale;
	if (m_timeOfDay >= 24.f)
		m_timeOfDay -= 24.f;
	else if (m_timeOfDay < 0.f)
		m_timeOfDay += 24.f;
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
// Converts a normalized spherical coordinate (r = 1) to cartesian coordinates
glm::vec3 spherical_to_vector(float theta, float phi) {
	float sin_theta = sin(theta);
	return glm::normalize(glm::vec3(
		sin(phi) * cos(theta),
		cos(phi),
		sin_theta * sin(phi)
	));
}

void Skydome::CalculateSun(Camera *camera)
{
	//// Assuming declination = 0
	float latitude = 45 * M_PI / 180.f; // lat 40 deg in rad
	float solar_hour_angle = (m_timeOfDay - 12.f) * 15 * M_PI / 180.f;
	//int sha_sign = (solar_hour_angle > 0) - (solar_hour_angle < 0);
	this->m_altitude = asin(cos(latitude) * cos(solar_hour_angle));

	if (solar_hour_angle > 0)
		this->m_azimuth = acos(sin(m_altitude) * sin(latitude) / (cos(m_altitude) * cos(latitude)));
	else
		m_azimuth = 2 * M_PI - acos(sin(m_altitude) * sin(latitude) / (cos(m_altitude) * cos(latitude)));

	//m_sunDirection = glm::normalize(glm::vec3(sin(m_azimuth), sin(m_altitude), -cos(m_azimuth)));
	//m_sunDirection = glm::vec3(.95, .05, 0);
	//m_sunDirection = glm::vec3(0, 1.0, 0);
	//m_sunDirection = spherical_to_vector(m_altitude, m_azimuth);
	m_sunDirection = glm::normalize(m_sunDirection);
	//m_lightSpaceMatrix = m_orthoLightProj * glm::lookAt(camera->GetPosition(), camera->GetPosition() -m_sunDirection, glm::vec3(0.0, 1.0, 0.0));
	m_sunIntensity = 10.0f;
}

float Skydome::GetAzimuth()
{
	return m_azimuth;
}

float Skydome::GetAltitude()
{
	return m_altitude;
}

glm::vec3 Skydome::GetSunColor()
{
	return m_sunColor;
}

glm::vec3 Skydome::GetSunDirection()
{
	return m_sunDirection;
}