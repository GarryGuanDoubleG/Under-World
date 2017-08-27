#pragma once

class Shader
{
public:
	GLuint m_shaderID;

	Shader(string & vs_shader, string fs_shader);
	~Shader();

	void Use();
	void SetUniform1f(string uniform, GLfloat value);
	void SetUniform1i(string uniform, GLint value);

	GLuint Uniform(string uniformName);
};