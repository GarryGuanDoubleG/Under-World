#include "game.hpp"

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

		cout << "Shader Error: " << name << " " << infoLog << endl;

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
bool read_file(char *name, string &out)
{
	ifstream in(name, ifstream::in);
	string input;

	if (!in.is_open())
	{
		cout << "Read_File: Failed to open file " << name << endl;
		return false;
	}

	while (getline(in, input))
	{
		out.append(input + "\n");
	}

	return true;
}

bool compile_attach_shader(string filename, GLuint type, GLuint program)
{
	GLuint shader;

	string shader_str;
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
GLuint compile_shaders(string &vs_shader, string &fs_shader)
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


Shader::Shader( string &vs_shader, string fs_shader)
{
	m_shaderID = compile_shaders(vs_shader, fs_shader);	
}

void Shader::Use()
{
	if (m_shaderID)
	{
		glUseProgram(m_shaderID);
	}
}

GLuint Shader::Uniform(string uniformName)
{
	if (glGetUniformLocation(m_shaderID, uniformName.c_str()) < 0)
		cout << "Shader could not find " << uniformName << endl;

	return glGetUniformLocation(m_shaderID, uniformName.c_str());
}
