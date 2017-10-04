#pragma once
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL\freeglut.h>
#include "game.hpp"


Graphics::Graphics(int winWidth, int winHeight)
{
	if(!InitGraphics(winWidth, winHeight))
	{
		slog("Graphics Failed to initialize");
		Cleanup();
		exit(1);
	}

	InitShapes();
	InitDepthMap();
	InitFBOS();
	//RealInitFBO();

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
	m_skydome = new Skydome(m_modelMap["skydome"]);
	return m_skydome;
}

void Graphics::InitDepthMap()
{
	Texture *depthMap = new Texture();
	depthMap->SetDepthMap(1680, 1080);
	m_textureMap.insert(pair<string, Texture*>("depthMap", depthMap));
	
	glBindTexture(GL_TEXTURE_2D, depthMap->GetTexID());

	glGenFramebuffers(1, &m_depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap->GetTexID(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Graphics::InitFBOS()
{
	//deferred rendering FBOs
	glGenFramebuffers(1, &m_deferredFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_deferredFBO);

	m_GBuffer.gPosition.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB16F, GL_RGB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBuffer.gPosition.GetTexID(), 0);

	m_GBuffer.gNormal.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB16F, GL_RGB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBuffer.gNormal.GetTexID(), 0);

	m_GBuffer.gAlbedoSpec.CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBuffer.gAlbedoSpec.GetTexID(), 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//depth map
	glGenRenderbuffers(1, &m_depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Frame buffer not complete " << endl;
	}

	//post processing FBO
	glGenFramebuffers(1, &m_sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
	Texture *shadedScene = new Texture();
	shadedScene->CreateTexture2D(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadedScene->GetTexID(), 0);

	m_textureMap["scene"] = shadedScene;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Frame buffer not complete " << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	glDepthFunc(GL_LEQUAL);

	glm::mat4 model(1.0f);
	model = glm::translate(model, m_camera->GetPosition());
	model = glm::scale(model, glm::vec3(1.f));
	shader->SetMat4("model", model);
	shader->SetMat4("view", m_camera->GetViewMat());
	shader->SetMat4("projection", m_camera->GetProj());

	m_skydome->Draw(shader);
}

GBuffer Graphics::DeferredRenderScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glBindFramebuffer(GL_FRAMEBUFFER, m_deferredFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render skydome
	RenderSkybox(m_shaderMap["deferred"]);

	//render voxels
	DeferredRenderVoxels(m_shaderMap["voxel_deferred"]);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render lighitng
	DeferredRenderLighting(m_shaderMap["deferredLighting"]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader *quad = m_shaderMap["quad"];
	quad->Use();
	m_textureMap["scene"]->Bind(0);
	RenderToQuad();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
											   // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
											   // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
											   // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
	glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	return m_GBuffer;
}

void Graphics::DeferredRenderLighting(Shader *shader)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	m_GBuffer.Bind(0);
	shader->SetUniform1i("gPosition", 0);
	shader->SetUniform1i("gNormal", 1);
	shader->SetUniform1i("gAlbedoSpec", 2);

	//shader->SetUniform1i("lightCount", 0);
	shader->SetUniform3fv("viewPos", m_camera->GetPosition());
	shader->SetUniform3fv("sunDir", m_skydome->m_sunDirection);

	RenderToQuad();
	m_GBuffer.Unbind();
}

void Graphics::RenderScene()
{
	if (m_flag & SKYBOX_MODE)
	{
		m_textureMap["grass"]->Bind(0);
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
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	 
	Shader *shader = m_shaderMap["depthMap"];
	shader->Use();

	float near_plane = 1.0f,
		far_plane = 2000.f;

	glm::vec3 lightPos(700.0f, 700, 700.f);

	glm::mat4 lightProjection = glm::ortho(-840.0f, 840.0f, -540.0f, 540.0f, near_plane, far_plane);
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
	tex->Bind(0);

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

void Graphics::DeferredRenderModel(Shader *shader, const string &name, const glm::mat4 &modelMat)
{
	Model * model = m_modelMap["arissa"];
	shader->Use();

	shader->SetMat4("model", modelMat);
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	model->Draw(shader);
}

void Graphics::DeferredRenderVoxels(Shader *shader)
{
	shader = m_shaderMap["voxel_deferred"];
	shader->Use();
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("model", glm::mat4(1.0f));
	shader->SetMat4("view", m_camera->GetViewMat());

	m_textureMap["grass"]->Bind(3);
	m_textureMap["brick"]->Bind(4);

	m_textureMap["grassNormal"]->Bind(15);
	m_textureMap["brickNormal"]->Bind(16);

	GLint samplers[] = { 3, 4 };
	GLint samplersNormalMap[] = { 15,16 };
	glUniform1iv(shader->Uniform("voxelTexture"), 2, &samplers[0]);
	glUniform1iv(shader->Uniform("normalMap"), 2, &samplersNormalMap[0]);

	g_game->m_voxelManager->Render();

	m_textureMap["brick"]->Unbind();
	m_textureMap["brickNormal"]->Unbind();

	m_textureMap["grass"]->Unbind();
	m_textureMap["grassNormal"]->Unbind();
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

	Shader *shader = m_shaderMap["voxelTex"];
	shader->Use();

	if(m_flag & SHADOW_MODE)
	{
		float near_plane = 1.0f;
		float far_plane = 1000.f;

		glm::vec3 lightPos(700.0f, 700, 700.f);

		glm::mat4 lightProjection = glm::ortho(-840.0f, 840.0f, -540.0f, 540.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		m_textureMap["depthMap"]->Bind(16);
		shader->SetUniform1i("shadowMap", 16);
	}

	shader->SetMat4("model", glm::mat4(1.0f));
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	glm::vec3 light_color(1.0f, 1.0f, 1.0f);
	glm::vec3 light_dir = g_game->m_skydome->m_sunDirection;


	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);
	glUniform3fv(shader->Uniform("lightDirection"), 1, &light_dir[0]);

	m_textureMap["grass"]->Bind(0);
	m_textureMap["brick"]->Bind(1);

	m_textureMap["grassNormal"]->Bind(15);
	m_textureMap["brickNormal"]->Bind(16);

	
	//m_textureMap["brickHeight"]->Bind(6);
	GLint samplers[] = { 0, 1, 2, 3, 4 };
	GLint samplersNormalMap[] = { 15,16,17,19 };
	glUniform1iv(shader->Uniform("voxelTexture"), 5, &samplers[0]);
	glUniform1iv(shader->Uniform("normalMap"), 5, &samplersNormalMap[0]);
	g_game->m_voxelManager->Render();

	m_textureMap["brick"]->Unbind();
	m_textureMap["brickNormal"]->Unbind();

	m_textureMap["grass"]->Unbind();
	m_textureMap["grassNormal"]->Unbind();

	if(m_flag & SHADOW_MODE) m_textureMap["depthMap"]->Unbind();
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

