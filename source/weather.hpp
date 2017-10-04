#pragma once

class Weather
{
	int m_cloudCoverage;

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

	void RenderToQuad();

	void Render(GBuffer gBuffer, Texture *shadedScene);
};