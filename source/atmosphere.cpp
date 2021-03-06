#include "game.hpp"

Atmosphere::Atmosphere() : m_skyW(256), m_skyH(64), m_transW(1024), m_transH(256), m_inscatterW(256), m_inscatterH(128), m_inscatterD(32)
{
}

Atmosphere::Atmosphere(GLuint quadVao) : m_skyW(256), m_skyH(64), m_transW(1024), m_transH(256), m_inscatterW(256), m_inscatterH(128), m_inscatterD(32)
{
	m_HR = 8.0;
	m_betaR = glm::vec3(5.8e-3, 1.35e-2, 3.31e-2);
	//mie factors
	m_HM = 1.2;
	m_betaMSca = glm::vec3(20e-3);
	m_mieG = 0.8;

	m_showImGUI = false;

	this->quadVao = quadVao;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	//bind texture
	m_outputTex.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA16F, GL_RGBA);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outputTex.GetTexID(), 0);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Atmosphere::SetUniforms(Shader *shader)
{
	shader->SetUniform1f("HR", m_HR);
	shader->SetUniform3fv("betaR", m_betaR);

	shader->SetUniform1f("HM", m_HM);
	shader->SetUniform3fv("betaMSca", m_betaMSca);
	shader->SetUniform1f("mieG", m_mieG);
}

void Atmosphere::Precompute()
{
	//better to load textures & shaders locally and delete them after precomputation
	//2D
	Texture deltaE;
	transmittance = new Texture();
	irradiance = new Texture();
	
	transmittance->CreateImage2D(m_transW, m_transH, false);
	irradiance->CreateImage2D(m_skyW, m_skyH, false);
	deltaE.CreateImage2D(m_skyW, m_skyH, false);
	
	//3D
	Texture deltaSM, deltaSR, deltaJ;
	inscatter = new Texture();
	inscatter->CreateImage3D(m_inscatterW, m_inscatterH, m_inscatterD, false);
	
	deltaSM.CreateImage3D(m_inscatterW, m_inscatterH, m_inscatterD, false);
	deltaSR.CreateImage3D(m_inscatterW, m_inscatterH, m_inscatterD, false);
	deltaJ.CreateImage3D(m_inscatterW, m_inscatterH, m_inscatterD, false);

	//Start precomputing
	PrecomputeTransmittance();
	PrecomputeDeltaE(deltaE);
	PrecomputeDeltaSMSR(deltaSR, deltaSM);
	//copy the values into irradiance & inscatter textures
	
	CopyIrradiance(deltaE);
	CopyInscatter(deltaSM, deltaSR);
	
	bool first = true;
	//calculate multiple scattering
	for (int i = 0; i < 3; i++)
	{
		PrecomputeDeltaJ(deltaJ, deltaSR, deltaSM, deltaE, first);
		PrecomputeIrradiance_N(deltaJ, deltaSR, deltaSM, deltaE, first);
		PrecomputeDeltaSR(deltaSR, deltaJ);
		AddDeltas(deltaE, deltaSR);
		
		first = false;
	}

	deltaJ.Destroy();
	deltaSR.Destroy();
	deltaE.Destroy();
	deltaSM.Destroy();
}

void Atmosphere::PrecomputeTransmittance()
{
	int workGroupSize = 16;
	Shader * shader = g_game->GetShader("transmittance");
	shader->Use();
	//SetUniforms(shader);
	GLuint id = transmittance->GetTexID();
	glBindImageTexture(0, id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glDispatchCompute(m_transW / workGroupSize, m_transH / workGroupSize, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Atmosphere::PrecomputeDeltaE(Texture &deltaE)
{
	int workGroupSize = 16;
	Shader *shader = g_game->GetShader("delta_e");
	shader->Use();

	//SetUniforms(shader);
	transmittance->Bind(shader->Uniform("transmittanceSampler"), 0);

	glBindImageTexture(0, deltaE.GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_skyW / workGroupSize, m_skyH / workGroupSize, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	transmittance->Unbind();
}

void Atmosphere::PrecomputeDeltaSMSR(Texture & deltaSR, Texture &deltaSM)
{
	int workGroupSize = 8;
	Shader *shader = g_game->GetShader("delta_sm_sr");
	shader->Use();

	//SetUniforms(shader);
	transmittance->Bind(shader->Uniform("transmittanceSampler"), 0);
	
	//3D textures must use layers
	glBindImageTexture(0, deltaSR.GetTexID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, deltaSM.GetTexID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	
	glDispatchCompute(m_inscatterW / workGroupSize, m_inscatterH / workGroupSize, m_inscatterD / workGroupSize);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	transmittance->Unbind();
}

void Atmosphere::CopyIrradiance(Texture & deltaE)
{
	int workGroupSize = 16;
	Shader *copy_irradiance = g_game->GetShader("copy_irradiance");
	
	copy_irradiance->Use();

	SetUniforms(copy_irradiance);
	copy_irradiance->SetUniform1f("k", 0.0f);

	deltaE.Bind(copy_irradiance->Uniform("deltaESampler"), 0);
	glBindImageTexture(0, irradiance->GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_skyW / workGroupSize, m_skyH / workGroupSize, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	deltaE.Unbind();
}

void Atmosphere::CopyInscatter(Texture & deltaSM, Texture &deltaSR)
{
	int workGroupSize = 8;
	Shader *shader = g_game->GetShader("copy_inscatter");
	
	shader->Use();
	//SetUniforms(shader);
	deltaSR.Bind(shader->Uniform("deltaSRSampler"), 0);
	deltaSM.Bind(shader->Uniform("deltaSMSampler"), 1);

	glBindImageTexture(0, inscatter->GetTexID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_inscatterW / workGroupSize, m_inscatterH / workGroupSize, m_inscatterD / workGroupSize);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	deltaSR.Unbind();
	deltaSM.Unbind();
}

void Atmosphere::PrecomputeDeltaJ(Texture &deltaJ, Texture &deltaSR, Texture& deltaSM, Texture& deltaE, GLboolean first)
{
	int workGroupSize = 8;
	Shader *shader = g_game->GetShader("delta_j");

	shader->Use();
	//SetUniforms(shader);
	deltaSR.Bind(shader->Uniform("deltaSRSampler"), 0);
	deltaSM.Bind(shader->Uniform("deltaSMSampler"), 1);
	deltaE.Bind(shader->Uniform("deltaESampler"), 2);
	shader->SetUniform1i("first", first);

	//3D textures must use layers
	glBindImageTexture(0, deltaJ.GetTexID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_inscatterW / workGroupSize, m_inscatterH / workGroupSize, m_inscatterD / workGroupSize);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	deltaSR.Unbind();
	deltaSM.Unbind();
	deltaE.Unbind();
}

void Atmosphere::PrecomputeIrradiance_N(Texture &deltaJ, Texture &deltaSR, Texture& deltaSM, Texture& deltaE, GLboolean first)
{
	int workGroupSize = 16;
	Shader *shader = g_game->GetShader("irradiance_n");

	shader->Use();
	//SetUniforms(shader);
	deltaSR.Bind(shader->Uniform("deltaSRSampler"), 0);
	deltaSM.Bind(shader->Uniform("deltaSMSampler"), 1);
	shader->SetUniform1i("first", first);

	//3D textures must use layers
	glBindImageTexture(0, deltaE.GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_skyW / workGroupSize, m_skyW / workGroupSize, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	deltaSR.Unbind();
	deltaSM.Unbind();
}

void Atmosphere::PrecomputeDeltaSR(Texture &deltaSR, Texture& deltaJ)
{
	int workGroupSize = 8;
	Shader *shader = g_game->GetShader("delta_sr");

	shader->Use();

	//bind textures
	//SetUniforms(shader);
	deltaJ.Bind(shader->Uniform("deltaJSampler"), 0);
	transmittance->Bind(shader->Uniform("transmittanceSampler"), 1);
	
	//3D textures must use layers
	glBindImageTexture(0, deltaSR.GetTexID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_inscatterW / workGroupSize, m_inscatterH / workGroupSize, m_inscatterD / workGroupSize);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	deltaJ.Unbind();
	transmittance->Unbind();
}

void Atmosphere::AddDeltas(Texture& deltaE, Texture& deltaSR)
{
	int workGroupSize = 16;
	
	Shader *shader = g_game->GetShader("add_delta_e");

	shader->Use();
	//SetUniforms(shader);
	shader->SetUniform1i("deltaESampler", 0);
	deltaE.Bind(shader->Uniform("deltaESampler"), 0);

	//3D textures must use layers
	glBindImageTexture(0, irradiance->GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_skyW / workGroupSize, m_skyH / workGroupSize, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	deltaE.Unbind();

	workGroupSize = 8;

	// add delta SR
	shader = g_game->GetShader("add_delta_sr");

	shader->Use();
	//SetUniforms(shader);

	deltaSR.Bind(shader->Uniform("deltaSSampler"), 0);

	glBindImageTexture(0, inscatter->GetTexID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_inscatterW / workGroupSize, m_inscatterH / workGroupSize, m_inscatterD / workGroupSize);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	deltaSR.Unbind();
}

void Atmosphere::RenderToQuad()
{
	glBindVertexArray(quadVao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Atmosphere::RenderImGUI()
{
	if (!m_showImGUI) return;

	ImGui::Begin("Atmosphere Window", &m_showImGUI);
	ImGui::Text("Atmosphere Params!");

	ImGui::SliderFloat("Rayleigh Factor", &m_HR, 0.0f, 100.0f);
	ImGui::SliderFloat("Mie Factor", &m_HM, 0.0f, 100.0f);

	ImGui::InputFloat3("BetaR", &m_betaR[0], 5);

	ImGui::InputFloat3("BetaM", &m_betaMSca[0], 5);

	ImGui::SliderFloat("mie G", &m_mieG, 0.0f, 10.0f);
	ImGui::End();
}

void Atmosphere::BindScatteringTextures(Shader *shader)
{
	//irradiance->Bind(shader->Uniform("texIrradiance"), 0);
	//transmittance->Bind(shader->Uniform("texTransmittance"), 1);
	inscatter->Bind(shader->Uniform("texInscatter"), 2);
}

Texture Atmosphere::Render(DeferredBuffer &gbuffer, Texture *scene)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	Shader *shader = g_game->GetShader("atmosphere");
	Camera *camera = g_game->GetCamera();

	shader->Use();
	
	//set uniforms
	//SetUniforms(shader);
	shader->SetUniform3fv("cameraPos", camera->GetPosition());
	shader->SetMat4("invViewMat", camera->GetInverseViewMat());
	//shader->SetUniform1f("sun_azimuth", g_game->m_skydome->m_azimuth);
	//shader->SetUniform1f("sun_altitude", g_game->m_skydome->m_altitude);
	shader->SetUniform3fv("sunDir", g_game->m_skydome->m_sunDirection);
	shader->SetUniform1f("sunIntensity", g_game->m_skydome->m_sunIntensity);

	//set uniforms
	inscatter->Bind(shader->Uniform("texInscatter"), 2);
	gbuffer.Position.Bind(shader->Uniform("gPosition"), 3);
	scene->Bind(shader->Uniform("ShadedScene"), 4);

	RenderToQuad();

	gbuffer.Unbind();
	irradiance->Unbind();
	transmittance->Unbind();
	inscatter->Unbind();
	scene->Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return m_outputTex;
}
