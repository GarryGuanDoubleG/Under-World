#pragma once

struct GBuffer {
	Texture gPosition;
	Texture gNormal;
	Texture gAlbedoSpec;

	//GBuffer(const GBuffer &) = delete;
	//GBuffer & operator =(const GBuffer&) = delete;

	void Bind(GLuint activeTex)
	{
		gPosition.Bind(activeTex);
		gNormal.Bind(activeTex + 1);
		gAlbedoSpec.Bind(activeTex + 2);
	}

	void Unbind()
	{
		gPosition.Unbind();
		gNormal.Unbind();
		gAlbedoSpec.Unbind();
	}
};

class Graphics
{
	SDL_Window *m_window;
	SDL_GLContext m_context;

	Camera *m_camera;
	Skydome *m_skydome;

	GLuint m_depthMapFBO;

	map<string, GLuint> m_vaoMap;
	map<string, GLuint> m_vboMap;
	map<string, Texture*> m_textureMap;
	map<string, Shader*> m_shaderMap;
	map<string, Model*> m_modelMap;
	
	vector<LightSource> m_lightSources;

	//deferred rendering
	GLuint m_deferredFBO;
	GLuint m_sceneFBO;
	GLuint m_depthRBO;
	GBuffer m_GBuffer;
	glm::vec3 lightPositions[32];

	GLuint gBuffer, gPosition, gNormal, gAlbedoSpec, rboDepth;
public:
	GLuint m_flag;

public:

	Graphics(int winWidth, int winHeight);
	~Graphics();
	bool InitGraphics(int winWidth, int winHeight);
	void InitShapes();
	void InitSkybox();

	void InitDepthMap();

	void InitFBOS();

	void RealInitFBO();

	void SetCamera(Camera *camera);
	void SetShaders(map<string, Shader*> &shaders);
	void SetTextures(map<string, Texture*> &textures);
	void SetModel(map<string, Model*>& models);
	void SetFlag(GLuint flag);
	void XORSetFlag(GLuint flag);

	Shader * GetShader(const char *key);

	GLuint GetVAO(const char * name);

	Texture *GetTexture(const char *name);

	void Display();
	void RenderBackground(GLfloat bg_color[4]);
	void RenderSkybox();
	GBuffer DeferredRenderScene();
	void DeferredRenderLighting(Shader *shader);
	void RenderScene();
	void RenderToQuad();
	void RenderShadowMap();
	void RenderCube(glm::mat4 model);
	void DeferredRenderModel(Shader * shader, const string & name, const glm::mat4 & modelMat);
	void RenderModel(const string &name, const glm::mat4 &modelMat);
	void DeferredRenderVoxels(Shader * shader);
	void RenderVoxels();

	void Cleanup();

	SDL_Window *GetWindow();
private:
	void CheckSDLError(int line = -1);
};