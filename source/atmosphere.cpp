#include "game.hpp"

Atmosphere::Atmosphere() : m_skyW(256), m_skyH(64), m_transW(1024), m_transH(256), m_inscatterW(256), m_inscatterH(128), m_inscatterD(32)
{

}

Atmosphere::Atmosphere(GLuint quadVao) : m_skyW(256), m_skyH(64), m_transW(1024), m_transH(256), m_inscatterW(256), m_inscatterH(128), m_inscatterD(32)
{
	this->quadVao = quadVao;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	//bind texture
	m_outputTex.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA16F, GL_RGBA);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outputTex.GetTexID(), 0);

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	shader->SetUniform1i("transmittanceSampler", 0);
	transmittance->Bind(0);

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
	shader->SetUniform1i("transmittanceSampler", 0);
	transmittance->Bind(0);
	
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
	copy_irradiance->SetUniform1f("k", 0.0f);
	copy_irradiance->SetUniform1i("deltaESampler", 0);

	deltaE.Bind(0);
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
	shader->SetUniform1i("deltaSRSampler", 0);
	shader->SetUniform1i("deltaSMSampler", 1);

	deltaSR.Bind(0);
	deltaSM.Bind(1);

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
	shader->SetUniform1i("deltaSRSampler", 0);
	deltaSR.Bind(0);
	shader->SetUniform1i("deltaSMSampler", 1);
	deltaSM.Bind(1);
	shader->SetUniform1i("deltaESampler", 2);
	deltaE.Bind(2);
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
	shader->SetUniform1i("deltaSRSampler", 0);
	deltaSR.Bind(0);
	shader->SetUniform1i("deltaSMSampler", 1);
	deltaSM.Bind(1);
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
	shader->SetUniform1i("deltaJSampler", 0);
	deltaJ.Bind(0);

	shader->SetUniform1i("transmittanceSampler", 1);
	transmittance->Bind(1);
	
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
	shader->SetUniform1i("deltaESampler", 0);
	deltaE.Bind(0);

	//3D textures must use layers
	glBindImageTexture(0, irradiance->GetTexID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(m_skyW / workGroupSize, m_skyH / workGroupSize, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	deltaE.Unbind();

	workGroupSize = 8;

	// add delta SR
	shader = g_game->GetShader("add_delta_sr");

	shader->Use();

	shader->SetUniform1i("deltaSSampler", 0);
	deltaSR.Bind(0);

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

Texture Atmosphere::Render(DeferredBuffer &gbuffer, Texture *scene)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	Shader *shader = g_game->GetShader("atmosphere");
	Camera *camera = g_game->GetCamera();

	shader->Use();
	shader->SetUniform3fv("cameraPos", camera->GetPosition());
	shader->SetMat4("invViewMat", camera->GetInverseViewMat());
	//shader->SetUniform1f("sun_azimuth", g_game->m_skydome->m_azimuth);
	//shader->SetUniform1f("sun_altitude", g_game->m_skydome->m_altitude);
	shader->SetUniform3fv("sunDir", g_game->m_skydome->m_sunDirection);
	shader->SetUniform1f("sunIntensity", g_game->m_skydome->m_sunIntensity);

	irradiance->Bind(0);
	transmittance->Bind(1);
	inscatter->Bind(2);
	gbuffer.BindGBuffer(3);//binds textures 3-5
	scene->Bind(6);

	RenderToQuad();

	gbuffer.Unbind();
	irradiance->Unbind();
	transmittance->Unbind();
	inscatter->Unbind();
	scene->Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return m_outputTex;
}
