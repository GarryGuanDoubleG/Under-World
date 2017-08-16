#pragma once
#include <GL\glew.h>
#include <string>

class Shader
{
public:
	GLuint m_shaderID;

	Shader(std::string & vs_shader, std::string fs_shader);
};