#pragma once

#define NUM_SHADOW_MAPS 3

struct DeferredBuffer
{
	Texture Position;
	Texture Normal;
	Texture AlbedoSpec;
	Texture Metallic;
	Texture Roughness;

	void Create(int width, int height)
	{
		//set up gbuffer
		Position.CreateTexture2D(width, height, GL_RGB32F, GL_RGB);
		Normal.CreateTexture2D(width, height, GL_RGB16F, GL_RGB);
		AlbedoSpec.CreateTexture2D(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		Metallic.CreateTexture2D(width, height, GL_RED, GL_RGB, GL_FLOAT);
		Roughness.CreateTexture2D(width, height, GL_RED, GL_RGB, GL_FLOAT);
	}

	void BindGBuffer(GLuint activeTex)
	{
		Position.Bind(activeTex);
		Normal.Bind(activeTex + 1);
		AlbedoSpec.Bind(activeTex + 2);
	}

	void Bind(GLuint activeTex)
	{
		Position.Bind(activeTex);
		Normal.Bind(activeTex + 1);
		AlbedoSpec.Bind(activeTex + 2);
		Metallic.Bind(activeTex + 3);
		Roughness.Bind(activeTex + 4);
	}

	void UnbindGBuffer()
	{
		Position.Unbind();
		Normal.Unbind();
		AlbedoSpec.Unbind();
	}

	void Unbind()
	{
		UnbindGBuffer();
		Metallic.Unbind();
		Roughness.Unbind();
	}
};

class Graphics
{
	SDL_Window *m_window;
	SDL_GLContext m_context;

	Camera *m_camera;
	Skydome *m_skydome;

	GLuint m_shadowMapFBO;

	//maps for rendering
	map<string, GLuint> m_vaoMap;
	map<string, GLuint> m_vboMap;
	map<string, Texture*> m_textureMap;
	map<string, Shader*> m_shaderMap;
	map<string, Model*> m_modelMap;
	map<string, Material*> m_materialMap;
	
	//Cascading Shadow Maps
	vector<Texture> m_shadowMaps;
	glm::mat4 m_lightProjViewMats[NUM_SHADOW_MAPS];
	float m_shadowRange[NUM_SHADOW_MAPS + 1]; /* < float how far each bounding box extends for shadow maps*/

	vector<LightSource> m_lightSources;

	//deferred rendering
	GLuint m_deferredFBO;
	GLuint m_sceneFBO;
	GLuint m_depthRBO;

	//buffers
	DeferredBuffer m_deferredBuffer;

	//SSAO
	GLuint m_ssaoFBO;
	GLuint m_ssaoBlurFBO;
	std::vector<glm::vec3> m_ssaoKernel;

	glm::vec3 lightPositions[32];
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

	void InitDeferredFBO();

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

	//deferred rendering pipeline
	DeferredBuffer DeferredRenderScene();
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