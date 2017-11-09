#pragma once

struct CloudParams
{
	CloudParams() {};
	CloudParams(Texture *weather, Texture * base, Texture *detail, Texture *curl) : weatherTex(weather), baseNoise3D(base), detailNoise3D(detail), curlNoise(curlNoise) {};

	void LoadParams(Shader *shader) {

		detailNoise3D->Bind(1);
		baseNoise3D->Bind(2);
		curlNoise->Bind(3);
		weatherTex->Bind(4);

		shader->SetUniform1f("_CloudBottomFade", CloudBottomFade);

		shader->SetUniform1f("_StartHeight", atmosphereStartHeight);
		shader->SetUniform1f("_AtmosphereThickness", atmosphereThickness);

		shader->SetUniform1f("_MaxIterations", MaxIterations);
		shader->SetUniform1f("_SampleScalar", SampleScalar);

		shader->SetUniform1f("_RayMinimumY", RayMinimumY);
		shader->SetUniform1f("_DetailScale", DetailScale);
		shader->SetUniform1f("_HorizonFadeScalar", HorizonFadeScalar);
		shader->SetUniform1f("_HorizonFadeStartAlpha", HorizonFadeStartAlpha);
		shader->SetUniform1f("_OneMinusHorizonFadeStartAlpha", 1.0f - HorizonFadeStartAlpha);
		shader->SetUniform1f("_BaseScale", BaseScale);
		shader->SetUniform1f("_LightScalar", LightScalar);
		shader->SetUniform1f("_AmbientScalar", AmbientScalar);

		shader->SetUniform3fv("_LightColor", LightColor);
		shader->SetUniform3fv("_CloudBaseColor", CloudBaseColor);
		shader->SetUniform3fv("_CloudTopColor", CloudTopColor);

		shader->SetUniform1f("_HorizonCoverageStart", HorizonCoverageStart);
		shader->SetUniform1f("_HorizonCoverageEnd", HorizonCoverageEnd);
		shader->SetUniform1f("_BaseFBMScale", BaseFBMScale);
		shader->SetUniform1f("_DetailFBMScale", DetailFBMScale);

		shader->SetUniform1f("_MaxDistance", maxDistance);

		shader->SetUniform1f("_Density", Density);
		shader->SetUniform1f("_ForwardScatteringG", ForwardScatteringG);
		shader->SetUniform1f("_BackwardScatteringG", BackwardScatteringG);
		shader->SetUniform1f("_DarkOutlineScalar", DarkOutlineScalar);

		shader->SetUniform1f("_SunRayLength", SunRayLength * atmosphereThickness);
		shader->SetUniform1f("_ConeRadius", ConeRadius * atmosphereThickness);
		shader->SetUniform1f("_RayStepLength", RayStepLength);


		shader->SetUniform1f("_CoverageScale", CoverageScale);
		shader->SetUniform3fv("_BaseOffset", BaseOffset);
		shader->SetUniform3fv("_DetailOffset", DetailOffset);
		shader->SetUniform2fv("_CoverageOffset", CoverageOffset);

	}
	void UnbindTextures()
	{
		detailNoise3D->Unbind();
		baseNoise3D->Unbind();
		curlNoise->Unbind();
		weatherTex->Unbind();
	}
	Texture *weatherTex;
	Texture *baseNoise3D;
	Texture *detailNoise3D;
	Texture *curlNoise;

	float atmosphereEndHeight = 80000;
	float atmosphereStartHeight = 50000;
	float atmosphereThickness = atmosphereEndHeight - atmosphereStartHeight;
	float horizonDistance = 35000;

	float maxDistance = 160000;
	//float maxRayDistance = 22242.0f;
	float BaseFBMScale = 2;
	float DetailScale = 4;
	float DetailFBMScale = .22f;

	float SunRayLength = 0.08f;
	float ConeRadius = 0.08f;

	float CloudBottomFade = 0.33f;

	//animations
	float animationScale = .01f;
	float timeScale = .01f;
	glm::vec2 coverageOffsetPerFrame = glm::vec2(.01f, .01f);
	glm::vec3 cloudDetailOffsetPerFrame = glm::vec3(.1f, .1f, 0);
	glm::vec3 cloudBaseOffsetPerFrame = glm::vec3(.01, -.01f, 0.0f);

	glm::vec3 BaseOffset = glm::vec3(0.0f);
	glm::vec3 DetailOffset = glm::vec3(0.0f);
	glm::vec2 CoverageOffset = glm::vec2(0.0f);


	float BaseScale = 1.f / (atmosphereEndHeight);
	float CoverageScale = 1.0f / (maxDistance * 10);
	float HorizonFadeStartAlpha = .5f;
	float OneMinusHorizonFadeStartAlpha = 1.0f - HorizonFadeStartAlpha;
	float HorizonFadeScalar = .1f;					// Fades clouds on horizon, 1.0 -> 10.0 (1.0 = smooth fade, 10 = no fade)

	glm::vec3 LightColor = glm::vec3(1.0f);
	float LightScalar = 1.f;
	float AmbientScalar = 1.0f;

	//181 157 10
	glm::vec3 CloudTopColor = glm::vec3(1.0f);
	glm::vec3 CloudBaseColor = glm::vec3(169, 198, 255) / 255.0f;

	float MaxIterations = 128;
	float RayStepLength = (atmosphereThickness / (MaxIterations * .5f));
	float SampleScalar = 1.f;

	float ErosionEdgeSize = .5f;
	float CloudDistortion = .45f;
	float CloudDistortionScale = .5f;
	float Density = 1.0f;
	float ForwardScatteringG = 0.79f;
	float BackwardScatteringG = -0.39f;
	float DarkOutlineScalar = 1.f;

	float HorizonCoverageStart = .4f;
	float HorizonCoverageEnd = .7f;

	float LODDistance = .313;
	float RayMinimumY = 0.0;
};


class Weather
{
	int quadVAO;
	//post processing
	GLuint m_cloudFBO;
	Texture m_cloudTex;

	CloudParams m_cloudParams;
public:
	bool m_showCloudParams;

public:
	Weather();
	Weather(GLuint quadVao);
	~Weather();

	void LoadCloudData();

	void Update();

	void RenderToQuad();

	void Render(GBuffer gBuffer, Texture *shadedScene);
	void RenderImGui();
};