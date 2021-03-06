#pragma once

enum TextureType
{
	Tex2D = 0,
	Tex3D = 1,
	Skybox = 2,
	Ambient = 3,
	Diffuse = 4,
	Specular = 5,
	Normal = 6,
	Metallic = 7,
	Roughness =8,
	DepthMap = 9,
	Inactive = 10
};

class Texture
{
	TextureType m_type;
	GLuint m_texID;
	GLuint m_activeTex;
public:
	Texture();
	Texture(string filepath);
	Texture(const char * filename, const char * directory);
	Texture(aiTexture *texture);

	~Texture();

	void LoadTexture(string filepath);
	void LoadTexture3D(string filepath, int w, int h, int d, GLuint internalFormat, GLuint format, GLuint wrapmode);
	void LoadSkybox(string filepath);
	void Destroy();
	void TriFiltering();
	
	void Bind(GLuint shaderLoc, GLuint activeTex);
	void Unbind();

	void SetTexType(TextureType type);
	void SetTexType(aiTextureType type);
	void SetTexID(GLuint id);

	void SetDepthMap(GLuint width, GLuint height);

	Texture CreateEmpty2D(int format);

	void CreateTexture2D(int w, int h, GLuint internalFormat, GLuint format, GLuint type = GL_FLOAT, const GLvoid * value = nullptr);

	void CreateCubeMap(int w, int h, GLuint format, GLuint type, GLuint filter = GL_LINEAR, const GLvoid * value = nullptr);

	void CreateImage2D(int w, int h, bool float32);
	void CreateImage2D(int w, int h, GLuint texRepeat, GLuint internalFormat, GLuint format, GLuint type);
	void CreateImage2D(int w, int h, GLuint internalFormat, GLuint format, GLuint type, GLuint WrapMode, GLuint Filter, const GLvoid * value = nullptr);
	void CreateImage3D(int w, int h, int d, bool float32);
	void CreateImage3D(int w, int h, int d, GLuint internalFormat, GLuint format, GLuint type, GLuint WrapMode = GL_CLAMP_TO_EDGE, GLuint Filter = GL_LINEAR);
	void BindImage2D();

	GLuint GetTexID();
	TextureType GetTexType();
};