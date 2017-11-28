#pragma once
#include <random>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL\freeglut.h>
#include "game.hpp"

#define NUM_FRUSTUM_CORNERS 8

Graphics::Graphics(int winWidth, int winHeight, Camera *camera)
{
	if(!InitGraphics(winWidth, winHeight))
	{
		slog("Graphics Failed to initialize");
		Cleanup();
		exit(1);
	}
	//set player camera
	SetCamera(camera);

	InitShapes();
	InitShadowMaps();
	InitFBOS();
	InitSSAOBuffers();
	InitSSAONoise();

	for (unsigned int i = 0; i < 32; i++)
	{
		float xPos = ((rand() % 1000) / 10.0) * 6.0 - 3.0;
		float yPos = ((rand() % 1000) / 10.0) * 6.0 - 4.0;
		float zPos = ((rand() % 1000) / 10.0) * 6.0 - 3.0;

		lightPositions[i] = glm::vec3(xPos, yPos, zPos);
	}
	glEnable(GL_DEPTH_TEST);
}

Graphics::~Graphics()
{
}

bool Graphics::InitGraphics(int winWidth, int winHeight)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Window Initialization Failed: %s\n", SDL_GetError());
		return false;
	}

	m_window = SDL_CreateWindow("Under World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		winWidth, winHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	
	if(!m_window)
	{
		cout << "Unable to create Window \n";
		CheckSDLError(__LINE__);
		return false;
	}

	m_context = SDL_GL_CreateContext(m_window);

	if(m_context == NULL)
	{
		CheckSDLError();
		return false;
	}

	CheckSDLError();

	SDL_GL_SetSwapInterval(1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	CheckSDLError();

	//intialize glew for opengl calls
	if(glewInit() != GLEW_OK) printf("Glew Initialization Error\n");
	glViewport(0, 0, winWidth, winHeight);

	//intialze SDL2_image
	int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
	if(!(IMG_Init(imgFlags) & imgFlags)) printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LEQUAL);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	

	atexit(SDL_Quit);
	return true;
}

void Graphics::InitShapes()
{
	static const GLfloat cube[] = {
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};

	GLuint cube_vao;
	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);

	GLuint cube_vbo;
	glGenBuffers(1, &cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), &cube[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_vaoMap.insert(pair<string, GLuint>("cube", cube_vao));
	m_vboMap.insert(pair<string, GLuint>("cube", cube_vbo));

	//quad
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	GLuint quad_vao;
	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	GLuint quad_vbo;
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	//uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_vaoMap.insert(pair<string, GLuint>("quad", quad_vao));
	m_vboMap.insert(pair<string, GLuint>("quad", quad_vbo));
}

Skydome * Graphics::InitSkybox()
{
	m_skydome = new Skydome(m_modelMap["skydome"], m_camera);
	return m_skydome;
}

void Graphics::InitShadowMaps()
{
	for (int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		Texture shadowMap;
		shadowMap.SetDepthMap(SCREEN_WIDTH, SCREEN_HEIGHT);
		m_shadowMaps.push_back(shadowMap);
	}

	//define range of each shadow map
	m_shadowRange[0] = m_camera->GetNearPlane();
	m_shadowRange[1] = 1000.0f;
	m_shadowRange[2] = 20000.0f;
	m_shadowRange[3] = m_camera->GetFarPlane() * .5f;

	//bind buffers
	glGenFramebuffers(1, &m_shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMaps[0].GetTexID(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::InitFBOS()
{
	//initialize deferred fbo
	InitDeferredFBO();

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "Frame buffer not complete " << endl;

	//texture output of shaded scene
	glGenFramebuffers(1, &m_sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
	Texture *shadedScene = new Texture();
	shadedScene->CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadedScene->GetTexID(), 0);

	m_textureMap["scene"] = shadedScene;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "Frame buffer not complete " << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::InitDeferredFBO()
{
	//TODO: MOVE FBOS TO hash MAP
	//deferred rendering FBOs
	glGenFramebuffers(1, &m_deferredFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_deferredFBO);

	//initializes output textures for deferred renderin
	m_deferredBuffer.Create(SCREEN_WIDTH, SCREEN_HEIGHT);
	//set up gbuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_deferredBuffer.Position.GetTexID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_deferredBuffer.Normal.GetTexID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_deferredBuffer.AlbedoSpec.GetTexID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_deferredBuffer.Metallic.GetTexID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_deferredBuffer.Roughness.GetTexID(), 0);

	unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	//depth map
	glGenRenderbuffers(1, &m_depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
}

//screen space ambient occlusion buffers
void Graphics::InitSSAOBuffers()
{
	//bind main SSAO FBO
	glGenFramebuffers(1, &m_ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);
	
	Texture *ssaoTex = new Texture();
	ssaoTex->CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RED, GL_RGB, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTex->GetTexID(), 0);

	m_textureMap["ssao"] = ssaoTex;

	//BIND the ssao Blur FBO
	glGenFramebuffers(1, &m_ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFBO);

	Texture *ssaoBlurTex = new Texture();
	ssaoBlurTex->CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RED, GL_RGB, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurTex->GetTexID(), 0);

	m_textureMap["ssaoBlur"] = ssaoBlurTex;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::InitSSAONoise()
{
	// generate sample kernel
	// ----------------------
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0;

		// scale samples s.t. they're more aligned to center of kernel
		scale = MathUtil::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_ssaoKernel.push_back(sample);
	}

	// generate noise texture
	// ----------------------
	Texture *ssaoNoiseTex = new Texture();
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	ssaoNoiseTex->CreateTexture2D(4, 4, GL_RGB32F, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	m_textureMap["ssaoNoiseTex"] = ssaoNoiseTex;
}

void Graphics::SetCamera(Camera * camera)
{
	m_camera = camera;
}

void Graphics::SetShaders(map<string, Shader*>& shaders)
{
	m_shaderMap = shaders;
}

void Graphics::SetTextures(map<string, Texture*>& textures)
{
	m_textureMap.insert(textures.begin(), textures.end());
}

void Graphics::SetModel(map<string, Model*> &models)
{
	m_modelMap = models;
}

void Graphics::SetMaterials(map<string, Material *> &materials)
{
	m_materialMap = materials;
}

void Graphics::SetFlag(GLuint flag)
{
	m_flag = flag;
}

void Graphics::XORSetFlag(GLuint flag)
{
	m_flag ^= flag;
}

Shader * Graphics::GetShader(const char * key)
{
	return m_shaderMap[key];
}

GLuint Graphics::GetVAO(const char * name)
{
	return m_vaoMap[name];
}

Texture * Graphics::GetTexture(const char * name)
{
	return m_textureMap[name];
}

void Graphics::RenderBackground(GLfloat bg_color[4])
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, bg_color);
}

void Graphics::RenderSkybox(Shader *shader)
{
	if(~m_flag & SKYBOX_MODE) return;

	shader->Use();

	glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);

	glm::mat4 model(1.0f);
	model = glm::translate(model, m_camera->GetPosition());
	model = glm::scale(model, glm::vec3(20.f));
	shader->SetMat4("model", model);
	shader->SetMat4("view", m_camera->GetViewMat());
	shader->SetMat4("projection", m_camera->GetProj());

	m_skydome->Draw(shader);
	glEnable(GL_DEPTH_TEST);
}

//code based on http://ogldev.atspace.co.uk/www/tutorial49/tutorial49.html
void Graphics::CalculateShadowProj()
{
	glm::mat4 invViewMat = m_camera->GetInverseViewMat();
	glm::mat4 lightSpaceView = glm::lookAt(m_camera->GetPosition(), 
											m_camera->GetPosition() + m_skydome->m_sunDirection, 
											glm::vec3(0.0, 1.0, 0.0));

	float verticalFOV = SCREEN_HEIGHT / SCREEN_WIDTH;
	float TanHorizontalFOV = glm::tan(glm::radians(m_camera->GetFOV() * .5f));
	float TanVerticalFOV = glm::tan(glm::radians((m_camera->GetFOV() * verticalFOV) * .5f));

	for (int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		float xn = m_shadowRange[i] * TanHorizontalFOV;
		float xf = m_shadowRange[i + 1] * TanHorizontalFOV;
		float yn = m_shadowRange[i] * TanVerticalFOV;
		float yf = m_shadowRange[i + 1] * TanVerticalFOV;

		glm::vec4 frustumCorners[8] = {
			// near face
			glm::vec4(xn, yn, m_shadowRange[i], 1.0),
			glm::vec4(-xn, yn, m_shadowRange[i], 1.0),
			glm::vec4(xn, -yn, m_shadowRange[i], 1.0),
			glm::vec4(-xn, -yn, m_shadowRange[i], 1.0),
			// far face
			glm::vec4(xf, yf, m_shadowRange[i + 1], 1.0),
			glm::vec4(-xf, yf, m_shadowRange[i + 1], 1.0),
			glm::vec4(xf, -yf, m_shadowRange[i + 1], 1.0),
			glm::vec4(-xf, -yf, m_shadowRange[i + 1], 1.0)
		};

		glm::vec4 frustumCornersL[NUM_FRUSTUM_CORNERS];
		float minX = FLT_MAX;
		float maxX = FLT_MIN;
		float minY = FLT_MAX;
		float maxY = FLT_MIN;
		float minZ = FLT_MAX;
		float maxZ = FLT_MIN;

		for (int j = 0; j < NUM_FRUSTUM_CORNERS; j++) 
		{
			// Transform the frustum coordinate from world to light space
			frustumCornersL[j] = lightSpaceView /** invViewMat */* frustumCorners[j];

			minX = min(minX, frustumCornersL[j].x);
			maxX = max(maxX, frustumCornersL[j].x);
			minY = min(minY, frustumCornersL[j].y);
			maxY = max(maxY, frustumCornersL[j].y);
			minZ = min(minZ, frustumCornersL[j].z);
			maxZ = max(maxZ, frustumCornersL[j].z);
		}

		m_lightProjViewMats[i] = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ) * lightSpaceView;
	}
}

DeferredBuffer Graphics::DeferredRenderScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get depthMap for voxels and entites
	DeferredShadowMap(m_shaderMap["depthMap"]);

	glBindFramebuffer(GL_FRAMEBUFFER, m_deferredFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render skydome
	RenderSkybox(m_shaderMap["deferred"]);

	//render voxels
	DeferredRenderVoxels(m_shaderMap["voxel_deferred"]);
	
	//render model
	DeferredRenderModel(m_shaderMap["deferredPBR"], "a", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 300.f, 0.0f)));
	
	//SSAO
	DeferredSSAO(m_shaderMap["ssao"]);

	//SSAO Blur
	DeferredSSAOBlur(m_shaderMap["ssaoBlur"]);

	//Deferred Lighting
	DeferredRenderLighting(m_shaderMap["deferredLighting"]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
											   // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
											   // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
											   // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
	glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Shader *quad = m_shaderMap["quad"];
	quad->Use();
	//m_deferredBuffer.Normal.Bind(quad->Uniform("tex"), 0);
	m_textureMap["scene"]->Bind(quad->Uniform("tex"), 0);
	RenderToQuad();

	return m_deferredBuffer;
}

void Graphics::DeferredSSAO(Shader *shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shader->Use();
	for (unsigned int i = 0; i < 64; i++)
	{
		shader->SetUniform3fv("samples[" + std::to_string(i) + "]", m_ssaoKernel[i]);
	}
	shader->SetMat4("projection", m_camera->GetProj());
	m_deferredBuffer.Position.Bind(shader->Uniform("gPosition"), 0);
	m_deferredBuffer.Normal.Bind(shader->Uniform("gNormal"), 1);
	m_textureMap["ssaoNoiseTex"]->Bind(shader->Uniform("texNoise"), 2);

	RenderToQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::DeferredSSAOBlur(Shader *shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shader->Use();
	m_textureMap["ssao"]->Bind(shader->Uniform("ssaoInput"), 0);

	RenderToQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::DeferredShadowMap(Shader *shader)
{
	//get orthographic projections for each cascading shadow map
	CalculateShadowProj();

	//bind frame buffers
	shader->Use();

	//generate depth textures
	for (int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMaps[i].GetTexID(), 0);
		glClear(GL_DEPTH_BUFFER_BIT);

		shader->SetMat4("lightSpaceMat", m_lightProjViewMats[i]);

		//render entities
		shader->SetMat4("model", glm::mat4(1.0f));
		//render voxels
		g_game->m_voxelManager->Render();
		shader->SetMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 300.f, 0.0f)));
		m_modelMap["arissa"]->DrawVertices();

		glm::mat4 sphereMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1000, 4000, 1000)), glm::vec3(10.0));
		shader->SetMat4("model", sphereMatrix);
		m_modelMap["sphere"]->DrawVertices();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::DeferredRenderLighting(Shader *shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	m_deferredBuffer.Bind(shader, 0);//binds 0-4 inclusive
	m_textureMap["ssaoBlur"]->Bind(shader->Uniform("gSSAO"), 5); //ssao tex
	m_shadowMaps[0].Bind(shader->Uniform("g_shadowMap[0]"), 6);
	m_shadowMaps[1].Bind(shader->Uniform("g_shadowMap[1]"), 7);
	m_shadowMaps[2].Bind(shader->Uniform("g_shadowMap[2]"), 8);
	
	//shadow info
	//glUniform1iv(shader->Uniform("g_shadowMap"), NUM_SHADOW_MAPS, &shadowLoc[0]);
	glUniformMatrix4fv(shader->Uniform("g_lightSpaceMatrix"), NUM_SHADOW_MAPS, GL_FALSE, (const GLfloat *)&m_lightProjViewMats[0][0]);
	glUniform1fv(shader->Uniform("g_shadowRanges"), NUM_SHADOW_MAPS, &m_shadowRange[1]);

	shader->SetUniform3fv("sunDir", m_skydome->m_sunDirection);
	shader->SetMat4("InvViewMat", m_camera->GetInverseViewMat());
	shader->SetUniform3fv("viewPos", m_camera->m_pos);


	RenderToQuad();
	m_deferredBuffer.Unbind();
	m_shadowMaps[0].Unbind();
	m_shadowMaps[1].Unbind();
	m_shadowMaps[2].Unbind();
	m_textureMap["ssaoBlur"]->Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::DeferredRenderModel(Shader *shader, const string &name, const glm::mat4 &modelMat)
{
	shader->Use();

	//Model * model = m_modelMap["arissa"];
	//shader->SetMat4("model", modelMat);
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	//model->Draw(shader);

	//draw sphere
	Material *ironMat = m_materialMap["iron"];
	ironMat->BindMaterial(shader, 0);

	glm::mat4 sphereMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1000, 4000, 1000)), glm::vec3(10.0));
	shader->SetMat4("model", sphereMatrix);
	m_modelMap["sphere"]->DrawVertices();
}

void Graphics::DeferredRenderVoxels(Shader *shader)
{
	int maxTextures = 15;

	shader->Use();

	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("model", glm::mat4(1.0f));
	shader->SetMat4("view", m_camera->GetViewMat());

	//max 15 diffuse textures
	m_textureMap["grass"]->Bind(shader->Uniform("voxelTexture[0]"), 5);
	//m_textureMap["brick"]->Bind(shader->Uniform("voxelTexture[1]"), 6);

	Material * brickMat = m_materialMap["iron"];
	brickMat->m_albedo.Bind(shader->Uniform("voxelTexture[1]"), 6);
	brickMat->m_normal.Bind(shader->Uniform("normalMap[1]"),  maxTextures + 1);
	brickMat->m_metallic.Bind(shader->Uniform("metallicTexture[1]"), 2 * maxTextures + 1);
	brickMat->m_roughness.Bind(shader->Uniform("roughnessTexture[1]"), 3 * maxTextures + 1);

	//max 15 normal maps
	m_textureMap["grassNormal"]->Bind(shader->Uniform("normalMap[0]"), maxTextures);
	//m_textureMap["brickNormal"]->Bind(shader->Uniform("normalMap[1]"), maxTextures + 1);


	//GLint samplers[] = { 3, 4 };
	//GLint samplersNormalMap[] = { 15,16 };
	//glUniform1iv(shader->Uniform("voxelTexture"), 2, &samplers[0]);
	//glUniform1iv(shader->Uniform("normalMap"), 2, &samplersNormalMap[0]);
	////glUniform3fv(shader->Uniform("sunDir"), 1, &m_skydome->m_sunDirection[0]);

	g_game->m_voxelManager->Render();

	brickMat->Unbind();
	/*m_textureMap["brick"]->Unbind();
	m_textureMap["brickNormal"]->Unbind();*/

	m_textureMap["grass"]->Unbind();
	m_textureMap["grassNormal"]->Unbind();
}

//////FORWARD RENDERING
void Graphics::RenderScene()
{
	if (m_flag & SKYBOX_MODE)
	{
		m_textureMap["grass"]->Bind(1, 0);
		RenderSkybox(m_shaderMap["object"]);
		m_textureMap["grass"]->Unbind();

	}

	if(m_flag & SHADOW_MODE) RenderShadowMap();
	if(m_flag & VOXEL_MODE) RenderVoxels();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(400.f, 150, 4000.0f));
	model = glm::scale(model, glm::vec3(1.5f));

	for (unsigned int i = 0; i < 32; i++)
	{
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, glm::vec3(30.125f));
		RenderCube(model);
	}


	if(m_flag & MODEL_MODE) RenderModel("arissa", model);
}

void Graphics::RenderShadowMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	 
	Shader *shader = m_shaderMap["depthMap"];
	shader->Use();

	float near_plane = 1.0f,
		far_plane = 2000.f;

	glm::vec3 lightPos(700.0f, 700, 700.f);

	glm::mat4 lightProjection = glm::ortho(-840.0f, 840.0f, -540.0f, 540.0f, -far_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

	glCullFace(GL_FRONT);
	//render voxel shadows

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(400.f, 150, 400.0f));
	model = glm::scale(model, glm::vec3(1.5f));
	shader->SetMat4("model", model);
	m_modelMap["arissa"]->DrawVertices();

	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::RenderToQuad()
{
	glBindVertexArray(m_vaoMap["quad"]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

}

void Graphics::RenderCube(glm::mat4 model)
{
	Shader *shader = m_shaderMap["deferred"];
	shader->Use();

	Texture *tex = m_textureMap["grass"];
	tex->Bind(shader->Uniform("texture_diffuse1"), 0);

	glBindVertexArray(m_vaoMap["cube"]);

	shader->SetMat4("model", model);
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	glm::vec3 light_pos(10.f,10.f, 10.f);
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);

	//glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	//glUniform3fv(shader->Uniform("lightPos"), 1, &light_pos[0]);
	//glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	tex->Unbind();
	glBindVertexArray(0);
}

void Graphics::RenderModel(const string &name, const glm::mat4 &modelMat)
{
	if (~m_flag & MODEL_MODE) return;

	Model * model = m_modelMap["arissa"];
	Shader *shader = m_shaderMap["object"];
	shader->Use();

	shader->SetMat4("model", modelMat);
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	glm::vec3 light_pos(50.f, 200.f, 50.f);
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);

	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightPos"), 1, &light_pos[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);

	model->Draw(shader);
}

void Graphics::RenderVoxels()
{
	if(~m_flag & VOXEL_MODE) return;

	int maxTextures = 15;

	Shader *shader = m_shaderMap["voxelTex"];
	shader->Use();

	shader->SetMat4("model", glm::mat4(1.0f));
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	glm::vec3 light_color(1.0f, 1.0f, 1.0f);
	glm::vec3 light_dir = g_game->m_skydome->m_sunDirection;

	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);
	glUniform3fv(shader->Uniform("lightDirection"), 1, &light_dir[0]);

	m_textureMap["grass"]->Bind(shader->Uniform("voxelTexture[0]"), 3);
	m_textureMap["brick"]->Bind(shader->Uniform("voxelTexture[1]"), 4);

	//max 15 normal maps
	m_textureMap["grassNormal"]->Bind(shader->Uniform("normalMap[0]"), maxTextures);
	m_textureMap["brickNormal"]->Bind(shader->Uniform("normalMap[1]"), maxTextures + 1);

	
	//m_textureMap["brickHeight"]->Bind(6);
	//GLint samplers[] = { 0, 1, 2, 3, 4 };
	//GLint samplersNormalMap[] = { 15,16,17,19 };
	//glUniform1iv(shader->Uniform("voxelTexture"), 5, &samplers[0]);
	//glUniform1iv(shader->Uniform("normalMap"), 5, &samplersNormalMap[0]);
	g_game->m_voxelManager->Render();

	m_textureMap["brick"]->Unbind();
	m_textureMap["brickNormal"]->Unbind();

	m_textureMap["grass"]->Unbind();
	m_textureMap["grassNormal"]->Unbind();
}

void Graphics::Display()
{
	SDL_GL_SwapWindow(m_window);
}

void Graphics::CheckSDLError(int line)
{
	string error = SDL_GetError();

	if(error != "")
	{
		cout << "SLD Error : " << error << endl;

		if(line != -1)
			cout << "\nLine : " << line << endl;

		SDL_ClearError();
	}
}

void Graphics::Cleanup()
{
	for (auto &kv : m_vaoMap) glDeleteVertexArrays(1, &kv.second);
	for (auto &kv : m_vboMap) glDeleteBuffers(1, &kv.second);
	for (auto &tex : m_textureMap) tex.second->Destroy();

	m_shaderMap.clear();
	m_textureMap.clear();
	m_modelMap.clear();
	
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
}

SDL_Window * Graphics::GetWindow()
{
	return m_window;
}

