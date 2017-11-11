#include "game.hpp"


float CalculateHorizonDistance(float innerRadius, float outerRadius)
{
	return sqrt((outerRadius * outerRadius) - (innerRadius * innerRadius));
}

float CalculateMaxDistance(float earthRadius, float atmosphereEndHeight)
{
	return CalculateHorizonDistance(earthRadius, earthRadius + atmosphereEndHeight);
}

float CalculateMaxRayDistance(float earthRadius, float atmosphereStartHeight, float atmosphereEndHeight)
{
	float cloudInnerDistance = CalculateHorizonDistance(earthRadius, earthRadius + atmosphereStartHeight);
	float cloudOuterDistance = CalculateHorizonDistance(earthRadius, earthRadius + atmosphereEndHeight);
	return cloudOuterDistance - cloudInnerDistance;
}

Weather::Weather() : m_showCloudParams(false)
{

}
Weather::Weather(GLuint quadVao)
{
	this->quadVAO = quadVao;
	glGenFramebuffers(1, &m_cloudFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_cloudFBO);

	//bind texture
	m_cloudTex.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA32F, GL_RGBA);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_cloudTex.GetTexID(), 0);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_showCloudParams = false;
}

Weather::~Weather()
{
}

void Weather::LoadCloudData()
{
	Texture *baseNoise3D = new Texture();
	Texture *detailNoise3D = new Texture();

	baseNoise3D->LoadTexture3D("Resources\\textures\\noise.bytes", 128, 128, 128, GL_RGBA, GL_RGBA, GL_REPEAT);
	detailNoise3D->LoadTexture3D("Resources\\textures\\noise_detail.bytes", 32, 32, 32, GL_RGB, GL_RGB, GL_REPEAT);


	m_cloudParams.weatherTex = g_game->GetTexture("weather");
	m_cloudParams.curlNoise = g_game->GetTexture("CurlNoise");
	m_cloudParams.baseNoise3D = baseNoise3D;
	m_cloudParams.detailNoise3D = detailNoise3D;
}

void Weather::Update()
{
	m_cloudParams.CoverageOffset += m_cloudParams.animationScale * m_cloudParams.coverageOffsetPerFrame * g_game->GetDeltaTime() * m_cloudParams.timeScale;
	m_cloudParams.BaseOffset += m_cloudParams.animationScale * m_cloudParams.cloudBaseOffsetPerFrame * g_game->GetDeltaTime() * m_cloudParams.timeScale;
	m_cloudParams.DetailOffset += m_cloudParams.animationScale * m_cloudParams.cloudDetailOffsetPerFrame * g_game->GetDeltaTime() * m_cloudParams.timeScale;
}

void Weather::RenderToQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);	
}



void Weather::Render(GBuffer gBuffer, Texture *shadedScene)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_cloudFBO);

	Camera *camera = g_game->GetCamera();
	Shader *shader = g_game->GetShader("raymarchClouds");
	shader->Use();
	//render clouds

	gBuffer.gPosition.Bind(0);
	shader->SetUniform3fv("sunDir", g_game->m_skydome->GetSunDirection());
	shader->SetUniform3fv("cameraPos", camera->GetPosition());
	shader->SetMat4("invViewMat", camera->GetInverseViewMat());

	m_cloudParams.LoadParams(shader);
	RenderToQuad();
	m_cloudParams.UnbindTextures();

	//render to final scene
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader = g_game->GetShader("applyCloud");
	shader->Use();
	m_cloudTex.Bind(0);
	shadedScene->Bind(1);
	RenderToQuad();
	m_cloudTex.Unbind();
	shadedScene->Unbind();
}


void Weather::RenderImGui()
{
	if (!m_showCloudParams) return;
	ImGui::Begin("Another Window", &m_showCloudParams);
	ImGui::Text("Cloud Params!");

	ImGui::SliderFloat("Start Height", &m_cloudParams.atmosphereStartHeight, 0.0f, 100000.0f);
	ImGui::SliderFloat("End Height", &m_cloudParams.atmosphereEndHeight, 0.0f, 100000.0f);
	ImGui::SliderFloat("Max Distance", &m_cloudParams.maxDistance, 0.0f, 300000.0f);

	//update dependant values
	m_cloudParams.atmosphereThickness = m_cloudParams.atmosphereEndHeight - m_cloudParams.atmosphereStartHeight;
	m_cloudParams.BaseScale = 1.0f / m_cloudParams.atmosphereEndHeight;
	m_cloudParams.CoverageScale = 1.0f / (m_cloudParams.maxDistance * 10.0f);

	ImGui::SliderFloat("Horizon Fade Scalar", &m_cloudParams.HorizonFadeScalar, 0, 1.0f);
	ImGui::SliderFloat("Horizon Fade Start Alpha", &m_cloudParams.HorizonFadeStartAlpha, 0, 1.0f);

	m_cloudParams.OneMinusHorizonFadeStartAlpha = 1.0f - m_cloudParams.HorizonFadeStartAlpha;
	
	ImGui::SliderFloat("LightScalar", &m_cloudParams.LightScalar, 0.0f, 1.0f);
	ImGui::SliderFloat("AmbientScalar", &m_cloudParams.AmbientScalar, 0.0f, 1.0f);
	ImGui::SliderFloat("SunRayLength", &m_cloudParams.SunRayLength, 0.0f, 1.0f);
	ImGui::SliderFloat("Cone Radius", &m_cloudParams.ConeRadius, 0.0f, 1.0f);
	ImGui::SliderFloat("Cloud Bottom Fade", &m_cloudParams.CloudBottomFade, 0.0f, 1.0f);

	ImGui::SliderFloat3("Cloud Top Color", &m_cloudParams.CloudTopColor[0], 0.0f, 1.0f);
	ImGui::SliderFloat3("Cloud Bottom COlor", &m_cloudParams.CloudBaseColor[0], 0.0f, 1.0f);

	ImGui::InputFloat("Max Iterations", &m_cloudParams.MaxIterations, 8, 64.0f, 1);
	m_cloudParams.RayStepLength = m_cloudParams.atmosphereThickness / (m_cloudParams.MaxIterations * .5f);

	//animations
	ImGui::SliderFloat("Animation Scale", &m_cloudParams.animationScale, 0.0f, 1.0f);
	ImGui::SliderFloat2("CoverageOffset Per Frame", &m_cloudParams.coverageOffsetPerFrame[0], 0.0f, 1.0f);
	ImGui::SliderFloat3("BaseOffset Per Frame", &m_cloudParams.cloudBaseOffsetPerFrame[0], 0, 1);
	ImGui::SliderFloat3("DetailOffset Per Frame", &m_cloudParams.cloudDetailOffsetPerFrame[0], 0, 1);

	ImGui::SliderFloat("Base FBM Scale", &m_cloudParams.BaseFBMScale, 0.0f, 20.0f);
	ImGui::SliderFloat("Detail Scale", &m_cloudParams.DetailScale, 0.0f, 20.0f);
	ImGui::SliderFloat("Detail FBM SCALE", &m_cloudParams.DetailFBMScale, 0.0f, 1.0f);

	ImGui::SliderFloat("Density", &m_cloudParams.LightScalar, 0.0f, 8.0f);
	ImGui::SliderFloat("Forward Scattering", &m_cloudParams.ForwardScatteringG, 0.0f, 1.0f);
	ImGui::SliderFloat("Backward Scattering", &m_cloudParams.BackwardScatteringG, 0.0f, -1.0f);
	ImGui::SliderFloat("Dark Outline Scalar", &m_cloudParams.DarkOutlineScalar, 0.0f, 1.0f);
	ImGui::SliderFloat("Horizon Coverage Start", &m_cloudParams.HorizonCoverageStart, 0.0f, 1.0f);
	ImGui::SliderFloat("Horizon Coverage End", &m_cloudParams.HorizonCoverageEnd, 0.0f, 1.0f);

	ImGui::SliderFloat("Ray Minimum Y", &m_cloudParams.RayMinimumY, 0, 1.0f);

	ImGui::End();
}