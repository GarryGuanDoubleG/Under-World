#include "game.hpp"

Weather::Weather()
{
}
Weather::Weather(GLuint quadVao)
{
	this->quadVAO = quadVao;

	glGenFramebuffers(1, &m_cloudFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_cloudFBO);

	//bind texture
	m_cloudTex.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA16F, GL_RGBA);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_cloudTex.GetTexID(), 0);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Weather::~Weather()
{
}

void Weather::PrecomputeNoise()
{
	const int weatherDataRes = 256;
	const int highRes = 256;
	const int lowRes = 64;
	const int curlRes = 128;

	const glm::ivec3 workGroupSize(8, 8, 4);

	//noise textures
	m_cloudNoiseTexHigh.CreateImage3D(highRes, highRes, highRes, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	m_cloudNoiseTexLow.CreateImage3D(lowRes, lowRes, lowRes, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	m_cloudNoiseCurl.CreateImage2D(curlRes, curlRes, GL_MIRRORED_REPEAT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	//weather data
	m_weatherDataTex.CreateImage2D(weatherDataRes, weatherDataRes, GL_MIRRORED_REPEAT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);


	//high res cloud noise
	Shader * shader = g_game->GetShader("cloudHighFreqNoise");
	shader->Use();
	glBindImageTexture(0, m_cloudNoiseTexHigh.GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchCompute(highRes / workGroupSize.x, highRes / workGroupSize.y, highRes / workGroupSize.z);

	//low res
	shader = g_game->GetShader("cloudLowFreqNoise");
	shader->Use();
	glBindImageTexture(0, m_cloudNoiseTexLow.GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchCompute(lowRes / workGroupSize.x, lowRes / workGroupSize.y, lowRes / workGroupSize.z);

	//curl noise
	shader = g_game->GetShader("cloudCurlNoise");
	shader->Use();
	glBindImageTexture(0, m_cloudNoiseCurl.GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchCompute(curlRes / workGroupSize.x, curlRes / workGroupSize.y, 1);

	//Weather data
	shader = g_game->GetShader("GenerateWeatherData");
	shader->Use();
	shader->SetUniform1f("time", 1.0f);

	glBindImageTexture(0, m_weatherDataTex.GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchCompute(weatherDataRes / workGroupSize.x, weatherDataRes / workGroupSize.y, 1);
	SlogCheckGLError();

	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Weather::RenderToQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);	
}

void Weather::Render(GBuffer gBuffer, Texture *shadedScene)
{
	//bind frmaebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_cloudFBO);

	Camera *camera = g_game->GetCamera();
	Shader *shader = g_game->GetShader("raymarchClouds");
	shader->Use();

	gBuffer.gPosition.Bind(0);

	m_cloudNoiseTexHigh.Bind(1);
	m_cloudNoiseTexLow.Bind(2);
	m_cloudNoiseCurl.Bind(3);
	m_weatherDataTex.Bind(4);

	shader->SetUniform3fv("sunDir", g_game->m_skydome->GetSunDirection());
	shader->SetUniform3fv("cameraPos", camera->GetPosition());
	shader->SetUniform1f("frame_time", 1.0f);

	RenderToQuad();

	m_cloudNoiseTexHigh.Unbind();
	m_weatherDataTex.Unbind();
	m_cloudNoiseCurl.Unbind();
	m_cloudNoiseTexLow.Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader = g_game->GetShader("applyCloud");
	shader->Use();
	m_cloudTex.Bind(0);
	shadedScene->Bind(1);
	RenderToQuad();
	m_cloudTex.Unbind();
	shadedScene->Unbind();
}
