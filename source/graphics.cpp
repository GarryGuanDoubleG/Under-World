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
}

bool Graphics::InitGraphics(int winWidth, int winHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Window Initialization Failed: %s\n", SDL_GetError());
		return false;
	}

	m_window = SDL_CreateWindow("Under World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		winWidth, winHeight, SDL_WINDOW_OPENGL);
	if (!m_window)
	{
		cout << "Unable to create Window \n";
		CheckSDLError(__LINE__);
		return false;
	}

	m_context = SDL_GL_CreateContext(m_window);
	SDL_GL_SetSwapInterval(1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//intialize glew for opengl calls
	glewInit();

	//intialze SDL2_image
	int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}

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
	m_textureMap = textures;
}

void Graphics::SetModel(map<string, Model*> &models)
{
	m_modelMap = models;
}

void Graphics::RenderBackground(GLfloat bg_color[4])
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, bg_color);
}

void Graphics::RenderSkybox()
{
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

void Graphics::RenderCube(glm::mat4 model)
{
	Shader *shader = m_shaderMap["object"];
	shader->Use();

	Texture *tex = m_textureMap["grass"];
	tex->Bind(0);

	glBindVertexArray(m_vaoMap["cube"]);

	glUniformMatrix4fv(shader->Uniform("model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(shader->Uniform("projection"), 1, GL_FALSE, &m_camera->GetProj()[0][0]);
	glUniformMatrix4fv(shader->Uniform("view"), 1, GL_FALSE, &m_camera->GetViewMat()[0][0]);

	glm::vec3 light_pos(10.f,10.f, 10.f);
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);

	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightPos"), 1, &light_pos[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	tex->Unbind();
	glBindVertexArray(0);
}

void Graphics::RenderModel(string name, glm::mat4 modelMat)
{
	Model * model = m_modelMap["arissa"];
	Shader *shader = m_shaderMap["object"];
	shader->Use();

	glUniformMatrix4fv(shader->Uniform("model"), 1, GL_FALSE, &modelMat[0][0]);
	glUniformMatrix4fv(shader->Uniform("projection"), 1, GL_FALSE, &m_camera->GetProj()[0][0]);
	glUniformMatrix4fv(shader->Uniform("view"), 1, GL_FALSE, &m_camera->GetViewMat()[0][0]);

	glm::vec3 light_pos(200.f, 200.f, 200.f);
	glm::vec3 light_color(1.0f, 1.0f, 1.0f);

	glUniform3fv(shader->Uniform("viewPos"), 1, &m_camera->GetPosition()[0]);
	glUniform3fv(shader->Uniform("lightPos"), 1, &light_pos[0]);
	glUniform3fv(shader->Uniform("lightColor"), 1, &light_color[0]);

	model->Draw(shader);
}

void Graphics::Render()
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
	for (auto &kv : m_shaderMap)
	{
		glDeleteProgram(kv.second->m_shaderID);
	}

	m_shaderMap.clear();

	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
}

SDL_Window * Graphics::GetWindow()
{
	return m_window;
}

