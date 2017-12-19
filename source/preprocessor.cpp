#include "game.hpp"

Preprocessor::Preprocessor()
{
	Init();
}

Preprocessor::~Preprocessor()
{
}

void Preprocessor::Init()
{
	m_globalCubeMapSize = 512;

	//generate framebuffer
	glGenFramebuffers(1, &m_frameBuffer);
	glBindBuffer(GL_FRAMEBUFFER, m_frameBuffer);

	//generate render buffer
	glGenRenderbuffers(1, &m_renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_globalCubeMapSize, m_globalCubeMapSize);

	//attach render buffer to framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//precomputes the BRDF look up texture
Texture *Preprocessor::ComputeBRDFLUT(int w, int h)
{
	//could probably just load the brdf texture rather than precompute
	Texture *brdfLUT = new Texture();
	m_textureMap["brdfLUT"] = brdfLUT;
	brdfLUT->CreateImage2D(w, h, GL_RG16F, GL_RG, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT->GetTexID(), 0);

	Shader *shader = g_game->GetShader("brdfLUT");
	shader->Use();
	
	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderToQuad(g_game->GetVAO("quad"));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	return brdfLUT;
}

//compute global environment map for atmospheric scattering
Texture * Preprocessor::ComputeEnvironMap(Atmosphere *atmosphere, Camera * camera, Skydome *skydome, GLuint cubeVAO)
{
	Texture *environMap = new Texture();
	environMap->CreateCubeMap(m_globalCubeMapSize, m_globalCubeMapSize, GL_RGB16F, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	m_textureMap["globalEnv"] = environMap;

	Shader *shader = g_game->GetShader("atmosphereEnvMap");
	shader->Use();

	//view matrixes to form a cube map
	glm::mat4 cubeMapViewMatrixes [] = 
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	glViewport(0, 0, m_globalCubeMapSize, m_globalCubeMapSize);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	//set world and projection matrixes
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	shader->SetMat4("projection", projection);
	shader->SetUniform3fv("sunDir", skydome->m_sunDirection);
	shader->SetUniform1f("sunIntensity", skydome->m_sunIntensity);

	atmosphere->BindScatteringTextures(shader);  
	for (GLuint i = 0; i < 6; i++)
	{
		shader->SetMat4("view", cubeMapViewMatrixes[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environMap->GetTexID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube(cubeVAO);
	}
	//store the irradiance map
	m_textureMap["global"] = ConvoluteCubeMap(environMap, camera, cubeVAO, m_globalCubeMapSize, m_globalCubeMapSize);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	return m_textureMap["global"];
}

//creates an irradiance map by convoluting the input texture
Texture *Preprocessor::ConvoluteCubeMap(Texture *cubeMap, Camera *camera, GLuint cubeVAO, int w, int h)
{
	glm::mat4 cubeMapViewMatrixes[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	Texture *irradianceMap = new Texture();
	irradianceMap->CreateCubeMap(32, 32, GL_RGB16F, GL_FLOAT, GL_LINEAR, nullptr);

	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glBindRenderbuffer(GL_FRAMEBUFFER, m_renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	Shader *irradianceShader = g_game->GetShader("convolution");
	irradianceShader->Use();

	cubeMap->Bind(irradianceShader->Uniform("envMap"), 0);
	glViewport(0, 0, 32, 32);
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	irradianceShader->SetMat4("projection", projection);
	for (int i = 0; i < 6; i++)	
	{ 
		irradianceShader->SetMat4("view", cubeMapViewMatrixes[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap->GetTexID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderCube(cubeVAO);
	}

	return irradianceMap;
}

Texture *Preprocessor::PrefilterEnvironMap(int w, int h)
{
	Texture *prefilterEnvMap = new Texture();
	prefilterEnvMap->CreateCubeMap(w, h, GL_RGB16F, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR); //enable trilinear filtering
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	m_textureMap["prefilterGlobalMap"] = prefilterEnvMap;

	Shader * shader = g_game->GetShader("prefilter");
	shader->Use();

	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	shader->SetMat4("projection", projection);

	Texture *environmentMap = m_textureMap["globalEnv"];
	environmentMap->Bind(shader->Uniform("environmentMap"), 0);

	glm::mat4 cubeMapViewMatrixes[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	GLuint maxMipLevels = 5;
	GLuint cubeVAO = g_game->GetVAO("cube");
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	for (GLuint mip = 0; mip < maxMipLevels; mip++)
	{
		//resize framebuffer according to mip-level size
		GLuint mipWidth = w * std::pow(0.5, mip);
		GLuint mipHeight = h * std::pow(0.5, mip);

		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		shader->SetUniform1f("roughness", roughness);

		for (GLuint i = 0; i < 6; i++)
		{
			shader->SetMat4("view", cubeMapViewMatrixes[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterEnvMap->GetTexID(), mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCube(cubeVAO);
		}
	}
	//environmentMap->Destroy();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	return prefilterEnvMap;
}

void Preprocessor::RenderToQuad(GLuint quadVAO)
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Preprocessor::RenderCube(GLuint cubeVAO)
{
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Preprocessor::RenderSkybox(Camera *camera, GLuint cubeVAO)
{
	Shader *shader = g_game->GetShader("skybox");
	shader->Use();

	glm::mat4 view = glm::mat4(glm::mat3(camera->GetViewMat()));
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 10.0f);
	shader->SetMat4("projection", projection);
	shader->SetMat4("view", view);
	
	m_textureMap["global"]->Bind(shader->Uniform("environmentMap"), 0);
	RenderCube(cubeVAO);
	m_textureMap["global"]->Unbind();
}

void Preprocessor::RenderTexture(const char *key)
{
	Shader *shader = g_game->GetShader("quad");
	shader->Use();

	m_textureMap[key]->Bind(shader->Uniform("tex"), 0);
	RenderToQuad(g_game->GetVAO("quad"));
	m_textureMap[key]->Unbind();
}