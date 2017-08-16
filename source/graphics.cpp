#pragma once
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL\freeglut.h>
#include "graphics.hpp"


SDL_Window *g_window;
SDL_GLContext g_context;

Graphics::Graphics(int winWidth, int winHeight)
{
	if (!InitializeGraphics(winWidth, winHeight))
	{
		Cleanup();
		exit(1);
	}
}

bool Graphics::InitializeGraphics(int winWidth, int winHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Window Initialization Failed: %s\n", SDL_GetError());
		return false;
	}

	g_window = SDL_CreateWindow("Under World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		winWidth, winHeight, SDL_WINDOW_OPENGL);
	if (!g_window)
	{
		std::cout << "Unable to create Window \n";
		CheckSDLError(__LINE__);
		return false;
	}

	g_context = SDL_GL_CreateContext(g_window);
	SDL_GL_SetSwapInterval(1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	glewInit();

	atexit(SDL_Quit);

	return true;
}

void Graphics::Render()
{
	GLfloat bg_color[] = { 0.3f, 0.3f, 0.3f, 1.f };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, bg_color);

	SDL_GL_SwapWindow(g_window);
}

void Graphics::CheckSDLError(int line)
{
	std::string error = SDL_GetError();

	if (error != "")
	{
		std::cout << "SLD Error : " << error << std::endl;

		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;

		SDL_ClearError();
	}
}

void Graphics::Cleanup()
{
	SDL_GL_DeleteContext(g_context);
	SDL_DestroyWindow(g_window);
}
