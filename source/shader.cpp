#include <fstream>
#include <iostream>
#include <string>

#include"shader.hpp"

/**
*@brief prints out any errors with compiling a particular shader
*@param name the name of the shader
*@param shader the compiled shader to check
*/
GLuint check_shader_err(char *name, GLuint shader)
{
	GLchar infoLog[512];

	GLint result = GL_FALSE;
	GLint InfoLogLength;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
		glGetShaderInfoLog(shader, InfoLogLength, &InfoLogLength, infoLog);

		std::cout << "Shader Error: " << name << " " << infoLog << std::endl;

		return GL_FALSE;
	}
	return GL_TRUE;
}
/*
* @brief reads content of shader and stores it to a string
* @param name of the file
* @param reference to output string
* @return false if file could not be opened, else true
*/
bool read_file(char *name, std::string &out)
{
	std::ifstream shader_if(name, std::ifstream::in);
	std::string input;

	if (!shader_if.is_open())
	{
		std::cout << "Read_File: Failed to open file " << name << std::endl;
		return false;
	}

	while (std::getline(shader_if, input))
	{
		out.append(input + "\n");
	}

	return true;
}

bool compile_attach_shader(std::string filename, GLuint type, GLuint program)
{
	GLuint shader;

	std::string shader_str;
	const char * shader_src;

	if (!read_file((char *)filename.c_str(), shader_str)) return false;

	shader_src = shader_str.c_str();

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &shader_src, nullptr);
	glCompileShader(shader);

	//check if compiled successfully
	if (check_shader_err((char *)filename.c_str(), shader))
	{
		glAttachShader(program, shader);
		glDeleteShader(shader);
	}
	else
	{
		return false;//error
	}
}

/**
* @brief compiles a vertex and fragment shader and attaches it into a program
* @return id of compiled program
*/
GLuint compile_shaders(std::string &vs_shader, std::string &fs_shader)
{
	GLuint program;
	GLchar infoLog[512];
	GLint result = GL_FALSE;
	GLint InfoLogLength;

	//Create program, attach shaders to it, and link it
	program = glCreateProgram();

	compile_attach_shader(vs_shader, GL_VERTEX_SHADER, program);
	compile_attach_shader(fs_shader, GL_FRAGMENT_SHADER, program);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);

	if (!result)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		glGetProgramInfoLog(program, InfoLogLength, &InfoLogLength, infoLog);

		printf("Program Log: %s\n", infoLog);
	}
	return program;
}


Shader::Shader( std::string &vs_shader, std::string fs_shader)
{
	m_shaderID = compile_shaders(vs_shader, fs_shader);	
}
