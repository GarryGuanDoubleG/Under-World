#pragma once

class Shader
{
public:
	GLuint m_shaderID;

	Shader(string & vs_shader, string fs_shader);

	void Use();
	GLuint Uniform(string uniformName);
};