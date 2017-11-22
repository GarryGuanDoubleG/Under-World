#pragma once

#define NUM_SHADOW_MAPS 3

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

	GLuint m_shadowMapFBO;

	map<string, GLuint> m_vaoMap;
	map<string, GLuint> m_vboMap;
	map<string, Texture*> m_textureMap;
	map<string, Shader*> m_shaderMap;
	map<string, Model*> m_modelMap;
	
	//Cascading Shadow Maps
	vector<Texture> m_shadowMaps;
	glm::mat4 m_lightProjViewMats[NUM_SHADOW_MAPS];
	float m_shadowRange[NUM_SHADOW_MAPS + 1]; /* < float how far each bounding box extends for shadow maps*/

	vector<LightSource> m_lightSources;

	//deferred rendering
	GLuint m_deferredFBO;
	GLuint m_sceneFBO;
	GLuint m_depthRBO;
	GBuffer m_GBuffer;
	//SSAO
	GLuint m_ssaoFBO;
	GLuint m_ssaoBlurFBO;
	std::vector<glm::vec3> m_ssaoKernel;

	glm::vec3 lightPositions[32];

	GLuint gBuffer, rboDepth;
public:
	GLuint m_flag;

public:

	Graphics(int winWidth, int winHeight, Camera * camera);
	~Graphics();
	bool InitGraphics(int winWidth, int winHeight);
	void InitShapes();
	Skydome* InitSkybox();

	void InitShadowMaps();//Uses Cascading shadow maps

	void InitFBOS();

	void InitGBufferFBO();

	void InitSSAOBuffers();

	void InitSSAONoise();

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
	void RenderSkybox(Shader * shader);
	void CalculateShadowProj();
	GBuffer DeferredRenderScene();
	void DeferredSSAO(Shader * shader);
	void DeferredSSAOBlur(Shader * shader);
	void DeferredShadowMap(Shader * shader);
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