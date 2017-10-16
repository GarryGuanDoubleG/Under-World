#pragma once

class Weather
{
	float m_animationScale;
	float m_timeScale;
	glm::vec2 m_coverageOffsetPerFrame;
	glm::vec3 m_cloudBaseOffsetPerFrame;
	glm::vec3 m_cloudDetailOffsetPerFrame;

	glm::vec2 m_coverageOffset;
	glm::vec3 m_cloudBaseOffset;
	glm::vec3 m_cloudDetailOffset;

	int quadVAO;

	Texture m_cloudNoiseTexHigh;
	Texture m_cloudNoiseTexLow;
	Texture m_cloudNoiseCurl;

	Texture m_weatherDataTex;

	//post processing
	GLuint m_cloudFBO;
	Texture m_cloudTex;
public:
	Weather();
	Weather(GLuint quadVao);
	~Weather();

	void PrecomputeNoise();

	void Update();

	void RenderToQuad();

	void Render(GBuffer gBuffer, Texture *shadedScene);
};