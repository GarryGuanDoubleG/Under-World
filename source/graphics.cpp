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
	if (!InitGraphics(winWidth, winHeight))
	{
		Cleanup();
		exit(1);
	}

	InitShapes();
	InitSkybox();
	InitDepthMap();
}

Graphics::~Graphics()
{
}

bool Graphics::InitGraphics(int winWidth, int winHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Window Initialization Failed: %s\n", SDL_GetError());
		return false;
	}

	m_window = SDL_CreateWindow("Under World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		winWidth, winHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	
	if (!m_window)
	{
		cout << "Unable to create Window \n";
		CheckSDLError(__LINE__);
		return false;
	}

	m_context = SDL_GL_CreateContext(m_window);

	if (m_context == NULL)
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
	if (!(IMG_Init(imgFlags) & imgFlags)) printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	

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
	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
								 // Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	GLuint quad_vao;
	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	GLuint quad_vbo;
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	//uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_vaoMap.insert(pair<string, GLuint>("quad", quad_vao));
	m_vboMap.insert(pair<string, GLuint>("quad", quad_vbo));
}

void Graphics::InitSkybox()
{
	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	GLuint skyboxVAO, skyboxVBO;

	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	m_vaoMap["skybox"] = skyboxVAO;
	m_vboMap["skybox"] = skyboxVBO;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

void Graphics::RenderBackground(GLfloat bg_color[4])
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, bg_color);
}

void Graphics::RenderSkybox()
{
	if (~m_flag & SKYBOX_MODE) return;

	Shader *shader = m_shaderMap["skybox"];
	shader->Use();

	glm::mat4 view = glm::mat4(glm::mat3(m_camera->GetViewMat()));

	GLint depthMode;
	glGetIntegerv(GL_DEPTH_FUNC, &depthMode);
	glDepthFunc(GL_LEQUAL);

	glBindVertexArray(m_vaoMap["skybox"]);
	glUniformMatrix4fv(shader->Uniform("view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(shader->Uniform("projection"), 1, GL_FALSE, &m_camera->GetProj()[0][0]);

	Texture  *texture = m_textureMap["skybox"];
	texture->Bind(0);

	glUniform1i(shader->Uniform("skybox"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	texture->Unbind();

	glBindVertexArray(0);

	glDepthFunc(depthMode);
}

void Graphics::RenderScene()
{
	if (m_flag & SHADOW_MODE)
	{
		RenderShadowMap();
		//RenderToQuad();
	}

	RenderVoxels();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(400.f, 150, 400.0f));
	model = glm::scale(model, glm::vec3(1.5f));
	RenderModel("arissa", model);
	
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
	m_modelMap["arissa"]->DrawVertices(shader);

	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::RenderToQuad()
{
	Shader *shader = m_shaderMap["quad"];
	shader->Use();

	float near_plane = 1.0f,
		far_plane = 1000.f;
	shader->SetUniform1i("depthMap", 0);

	glBindVertexArray(m_vaoMap["quad"]);
	m_textureMap["brickNormal"]->Bind(0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_textureMap["brickNormal"]->Unbind();
	glBindVertexArray(0);
}

void Graphics::RenderCube(glm::mat4 model)
{
	Shader *shader = m_shaderMap["object"];
	shader->Use();

	Texture *tex = m_textureMap["grass"];
	tex->Bind(0);

	glBindVertexArray(m_vaoMap["cube"]);

	shader->SetMat4("model", model);
	shader->SetMat4("projection", m_camera->GetProj());
	shader->SetMat4("view", m_camera->GetViewMat());

	glm::vec3 light_pos(10.f,10.f, 10.f);
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);

	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightPos"), 1, &light_pos[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);

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
	if (~m_flag & VOXEL_MODE) return;

	Shader *shader = m_shaderMap["voxelTex"];
	shader->Use();

	if (m_flag & SHADOW_MODE)
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
	glm::vec3 light_dir(-.2f, -1.f, -0.3f);


	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);
	glUniform3fv(shader->Uniform("lightDirection"), 1, &light_dir[0]);
	
	m_textureMap["brick"]->Bind(0);
	m_textureMap["brickNormal"]->Bind(17);
	shader->SetUniform1i("normalMap", 17);
	//m_textureMap["brickHeight"]->Bind(6);
	GLint samplers[] = { 0, 1, 2, 3, 4 };
	glUniform1iv(shader->Uniform("voxelTexture"), 5, &samplers[0]);
	g_game->m_voxelManager->Render();

	m_textureMap["brick"]->Unbind();
	m_textureMap["brickNormal"]->Unbind();
	if (m_flag & SHADOW_MODE) m_textureMap["depthMap"]->Unbind();
}

void Graphics::Display()
{
	SDL_GL_SwapWindow(m_window);
}

void Graphics::CheckSDLError(int line)
{
	string error = SDL_GetError();

	if (error != "")
	{
		cout << "SLD Error : " << error << endl;

		if (line != -1)
			cout << "\nLine : " << line << endl;

		SDL_ClearError();
	}
}

void Graphics::Cleanup()
{
	for (auto &kv : m_vaoMap) glDeleteVertexArrays(1, &kv.second);
	for (auto &kv : m_vboMap) glDeleteBuffers(1, &kv.second);

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

