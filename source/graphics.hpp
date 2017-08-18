#pragma once

class Graphics
{
	SDL_Window *m_window;
	SDL_GLContext m_context;

	Camera *m_camera;

	map<string, GLuint> m_vaoMap;
	map<string, GLuint> m_vboMap;
	map<string, Texture*> m_textureMap;
	map<string, Shader*> m_shaderMap;
	map<string, Model*> m_models;

public:
	Graphics(int winWidth, int winHeight);

	bool InitGraphics(int winWidth, int winHeight);
	void InitShapes();
	void InitSkybox();

	void SetCamera(Camera *camera);
	void SetShaders(map<string, Shader*> &shaders);
	void SetTextures(map<string, Texture*> &textures);
	void SetModel(map<string, Model*>& models);

	void Render();
	void RenderBackground(GLfloat bg_color[4]);
	void RenderSkybox();
	void RenderCube(glm::mat4 model, glm::mat4 proj, glm::mat4 view);
	//void RenderModel(Shader *)

	void Cleanup();

	SDL_Window *GetWindow();
private:
	void CheckSDLError(int line = -1);
};