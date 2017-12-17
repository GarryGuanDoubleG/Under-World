#include "game.hpp"
#include "texture.hpp"

Texture::Texture() : m_type(Inactive)
{

}

Texture::Texture(string filepath) : m_type(Inactive)
{
	LoadTexture(filepath);
}

Texture::Texture(const char * filename, const char * directory) : m_type(Inactive)
{
	std::string name = directory;
	name += filename;
}

Texture::Texture(aiTexture * texture) : m_type(Inactive)
{
	if(!texture) return;

	//now generate texture with data
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);

	if(texture->mHeight == 0)
	{
		SDL_RWops *rw = SDL_RWFromMem(texture->pcData, texture->mWidth);
		SDL_Surface *surface = IMG_Load_RW(rw, 1);
		int mode = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

		SDL_FreeSurface(surface);
	}
	else
	{
		//texture compressed. uncompress.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->mWidth, texture->mHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture->pcData);
	}

	TriFiltering();
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{

}

void Texture::LoadTexture(string filepath)
{
	SDL_Surface *texture = IMG_Load(filepath.c_str());
	m_type = Tex2D;

	if(!texture)
	{
		cout << "Could not load image " << filepath << endl;
		cout << IMG_GetError() << endl;
		
		m_type = Inactive;
		return;
	}

	int mode = GL_RGB;

	if (texture->format->BytesPerPixel == 1)
		mode = GL_RED;
	else if(texture->format->BytesPerPixel == 4)
		mode = GL_RGBA;

	//now generate texture with data
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	//use GL_BGR because thats how bmp files store color
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, mode, GL_UNSIGNED_BYTE, texture->pixels);

	//trilinear filtering
	TriFiltering();

	//free image & unbind texture
	SDL_FreeSurface(texture);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::LoadTexture3D(string filepath, int w, int h, int d, GLuint internalFormat, GLuint format, GLuint wrapmode)
{
	m_type = Tex3D;

	ifstream infile;

	infile.open(filepath, ios::in | ios::binary | ios::ate);

	if (!infile.is_open())
	{
		slog("Could not open file %s", filepath);
		return;
	}

	int fileSize = infile.tellg();
	infile.seekg(0, ios::beg);

	char *data = new char[fileSize];
	infile.read(data, fileSize);

	infile.close();

	//load data into a 3D texture
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapmode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapmode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapmode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, w, h, d, 0, format, GL_UNSIGNED_BYTE, data);
	delete[] data;
}

void Texture::LoadSkybox(string filepath)
{
	const std::string faces[6] =
	{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"back.png",
		"front.png"
	};


	//now generate texture with data
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
	//use GL_BGR because thats how bmp files store color
	//give image to opengl

	m_type = Skybox;

	for (GLuint i = 0; i < 6; i++)
	{
		string filename = filepath + faces[i];
		SDL_Surface *texture = IMG_Load(filename.c_str());

		if(!texture)
		{
			cout << "Could not load image " << filename << endl;
			cout << IMG_GetError() << endl;
			return;
		}

		int mode = GL_RGB;

		if(texture->format->BytesPerPixel == 4) {
			mode = GL_RGBA;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
			texture->w, texture->h, 0, mode,
			GL_UNSIGNED_BYTE, texture->pixels);

		SlogCheckGLError("Skybox");

		SDL_FreeSurface(texture);
	}


	//trilinear filtering
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//unbind texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::Destroy()
{
	glDeleteTextures(1, &m_texID);
}

void Texture::TriFiltering()
{
	//trilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Bind(GLuint shaderLoc, GLuint activeTex)
{
	if (m_type == Inactive)
	{
		cout << "Warning: Binding an inactive texture" << endl;
		return;
	}

	m_activeTex = activeTex;
	glActiveTexture(GL_TEXTURE0 + activeTex); // Active proper texture unit before binding
	glUniform1i(shaderLoc, activeTex);

	switch (m_type)
	{
	case Tex2D:
		glBindTexture(GL_TEXTURE_2D, m_texID);
		break;
	case Tex3D:
		glBindTexture(GL_TEXTURE_3D, m_texID);
		break;
	case Skybox:
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
		break;
	case DepthMap:
		glBindTexture(GL_TEXTURE_2D, m_texID);
		break;
	default:
		glBindTexture(GL_TEXTURE_2D, m_texID);
		break;
	}
}

void Texture::Unbind()
{
	glActiveTexture(GL_TEXTURE0 + m_activeTex); // Active proper texture unit before binding									  
	switch (m_type)
	{
	case Tex2D:
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	case Tex3D:
		glBindTexture(GL_TEXTURE_3D, 0);
		break;
	case Skybox:
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		break;
	case DepthMap:
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	default:
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	}
}

void Texture::SetTexType(TextureType type)
{
	m_type = type;
}

void Texture::SetTexType(aiTextureType type)
{
	switch (type)
	{
	case aiTextureType_DIFFUSE:
		m_type = Diffuse;
		break;
	case aiTextureType_SPECULAR:
		m_type = Specular;
		break;
	case aiTextureType_AMBIENT:
		m_type = Ambient;
		break;
	case aiTextureType_NORMALS:
		m_type = Normal;
		break;
	default:
		break;
	}
}

void Texture::SetTexID(GLuint id)
{
	m_texID = id;
}

void Texture::SetDepthMap(GLuint width, GLuint height)
{
	m_type = DepthMap;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindTexture(GL_TEXTURE_2D, 0);
}

//creates an empty 1 x 1 texture with a value of 255 ubyte
Texture Texture::CreateEmpty2D(int format)
{
	m_type = Tex2D;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//setup format & pixel data for texture based input format
	switch (format) {
	case GL_RED:{
			GLubyte data[] = { 255 };
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, format, GL_UNSIGNED_BYTE, data);
			break;
		}
	case GL_RGB: {
		GLubyte data[] = { 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, format, GL_UNSIGNED_BYTE, data);
		break;
	}
	case GL_RGBA: {
		GLubyte data[] = { 255, 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, format, GL_UNSIGNED_BYTE, data);
		break;
	}
	default:
		break;
	}

	return *this;
}

void Texture::CreateTexture2D(int w, int h, GLuint internalFormat, GLuint format, GLuint type, const GLvoid *value)
{
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_type = Tex2D;
}

void Texture::CreateCubeMap(int w, int h, GLuint format, GLuint type, GLuint filter, const GLvoid * value)
{
	m_type = Skybox;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
	for (GLuint i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, w, h, 0, GL_RGB, type, value);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter);
}

void Texture::CreateImage2D(int w, int h, bool float32)
{
	m_type = Tex2D;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::CreateImage2D(int w, int h, GLuint texRepeat, GLuint internalFormat, GLuint format, GLuint type)
{
	m_type = Tex2D;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texRepeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texRepeat);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::CreateImage2D(int w, int h, GLuint internalFormat, GLuint format, GLuint type, GLuint WrapMode, GLuint Filter, const GLvoid * value)
{
	m_type = Tex2D;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, value);

	glBindTexture(GL_TEXTURE_2D, 0); 
}

void Texture::CreateImage3D(int w, int h, int d, bool float32)
{
	m_type = Tex3D;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, w, h, d, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture::CreateImage3D(int w, int h, int d, GLuint internalFormat, GLuint format, GLuint type, GLuint WrapMode, GLuint Filter)
{
	m_type = Tex3D;

	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, WrapMode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, WrapMode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, w, h, d, 0, format, type, NULL);

	glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture::BindImage2D()
{
	glBindImageTexture(0, m_texID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
}

GLuint Texture::GetTexID()
{
	return m_texID;
}

TextureType Texture::GetTexType()
{
	return m_type;
}


