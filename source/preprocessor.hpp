
class Preprocessor
{
	GLuint m_globalCubeMapSize;

	GLuint m_frameBuffer;
	GLuint m_renderBuffer;

	map<string, Texture*> m_textureMap;
public:
	Preprocessor();
	~Preprocessor();

	void Init();

	Texture * ComputeBRDFLUT(int w, int h);

	Texture * ComputeEnvironMap(Atmosphere * atmosphere, Camera * camera, Skydome * skydome, GLuint cubeVAO);

	Texture * PrefilterEnvironMap(int w, int h);

	Texture * ConvoluteCubeMap(Texture * cubeMap, Camera * camera, GLuint cubeVAO, int w, int h);

	void RenderToQuad(GLuint quadVAO);

	void RenderCube(GLuint cubeVAO);

	void RenderSkybox(Camera * camera, GLuint cubeVAO);

	void RenderTexture(const char * key);

};