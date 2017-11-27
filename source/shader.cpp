#include "game.hpp"

#define SHADER_PATH "Shaders/" //default shader path

/**
*@brief prints out any errors with compiling a particular shader
*@param name the name of the shader
*@param shader the compiled shader to check
*/
GLuint check_shader_err(char *name, GLuint shader)
{
	GLchar infoLog[4096];

	GLint result = GL_FALSE;
	GLint InfoLogLength;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if(!result)
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

	if(!in.is_open())
	{
		cout << "Read_File: Failed to open file " << name << endl;
		return false;
	}

	while (getline(in, input))
	{
		out.append(input + "\n");

		string word = "";
		istringstream iss(input);

		iss >> word;

		if (word == "#pragma")
		{
			string command;
			iss >> command;

			if (command == "include")
			{
				string currentPath(name);
				currentPath.erase(currentPath.rfind('/') + 1);

				string file;
				iss >> file;

				if (file.front() == '"' && file.back() == '"')
				{
					file.erase(0, 1);
					file.erase(file.size() - 1);
				}

				string filesource = "";
				if(read_file((char*)(currentPath + file).c_str(), filesource))
					out.append(filesource + "\n");
				else if(read_file((char*)(SHADER_PATH + file).c_str(), filesource))
					out.append(filesource + "\n");
			}
		}
	}

	return true;
}

bool compile_attach_shader(string filename, GLuint type, GLuint program)
{
	GLuint shader;

	string shader_str;
	const char * shader_src;

	if(!read_file((char *)filename.c_str(), shader_str)) return false;

	shader_src = shader_str.c_str();

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &shader_src, nullptr);
	glCompileShader(shader);

	//check if compiled successfully
	if(check_shader_err((char *)filename.c_str(), shader))
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

	if(!result)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		glGetProgramInfoLog(program, InfoLogLength, &InfoLogLength, infoLog);

		printf("Program Log: %s\n", infoLog);
	}
	return program;
}

Shader::Shader( string &vs_shader, string &fs_shader)
{
	m_shaderID = compile_shaders(vs_shader, fs_shader);	
}

Shader::Shader(string & compute)
{
	GLchar infoLog[512];
	GLint result = GL_FALSE;
	GLint InfoLogLength;

	//Create program, attach shaders to it, and link it
	m_shaderID = glCreateProgram();

	compile_attach_shader(compute, GL_COMPUTE_SHADER, m_shaderID);

	glLinkProgram(m_shaderID);
	glGetProgramiv(m_shaderID, GL_LINK_STATUS, &result);

	if (!result)
	{
		glGetProgramiv(m_shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		glGetProgramInfoLog(m_shaderID, InfoLogLength, &InfoLogLength, infoLog);

		printf("Program Log: %s\n", infoLog);
	}
}

Shader::~Shader()
{
	glDeleteProgram(m_shaderID);
}

void Shader::Use()
{
	if(m_shaderID)
	{
		glUseProgram(m_shaderID);
	}
}

void Shader::SetMat4(string uniform, glm::mat4 mat4)
{
	glUniformMatrix4fv(Uniform(uniform), 1, GL_FALSE, &mat4[0][0]);
}

void Shader::SetUniform1f(string uniform, GLfloat value)
{
	glUniform1f(Uniform(uniform), value);
}

void Shader::SetUniform1i(string uniform, GLint value)
{
	glUniform1i(Uniform(uniform), value);
}

void Shader::SetUniform2fv(const string &uniform, const glm::vec2 &value)
{
	glUniform2fv(Uniform(uniform), 1, &value[0]);
}

void Shader::SetUniform3fv(const string & uniform, const glm::vec3 & value)
{
	glUniform3fv(Uniform(uniform), 1, &value[0]);
}

GLuint Shader::Uniform(string uniformName)
{
	if(glGetUniformLocation(m_shaderID, uniformName.c_str()) < 0)
		cout << "Shader could not find " << uniformName << endl;

	return glGetUniformLocation(m_shaderID, uniformName.c_str());
}
