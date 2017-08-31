#pragma once

class Shader
{
public:
	GLuint m_shaderID;

	Shader(string & vs_shader, string fs_shader);
	~Shader();

	void Use();
	void SetMat4(string uniform, glm::mat4 mat4);
	void SetUniform1f(string uniform, GLfloat value);
	void SetUniform1i(string uniform, GLint value);

	void SetUniform2fv(string uniform, glm::vec2 value);

	GLuint Uniform(string uniformName);
};