#include "game.hpp"
#include "texture.hpp"

Texture::Texture()
{
}

Texture::Texture(string filepath)
{
	LoadTexture(filepath);
}

Texture::Texture(const char * filename, const char * directory)
{
	std::string name = directory;
	name += filename;
}

Texture::Texture(aiTexture * texture)
{
	m_type = Tex2D;

	if (!texture) return;

	//now generate texture with data
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_2D, m_texID);

	if (texture->mHeight == 0)
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

	if (!texture)
	{
		cout << "Could not load image " << filepath << endl;
		return;
	}

	int mode = GL_RGB;

	if (texture->format->BytesPerPixel == 4) {
		mode = GL_RGBA;
	}
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

void Texture::LoadSkybox(string filepath)
{
	const std::string faces[6] =
	{
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"back.jpg",
		"front.jpg"
	};


	//now generate texture with data
	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
	//use GL_BGR because thats how bmp files store color
	//give image to opengl

	m_type = Skybox;

	for (GLuint i = 0; i < 6; i++)
	{
		string filename = filepath + faces[i];
		SDL_Surface *texture = IMG_Load(filename.c_str());

		if (!texture)
		{
			cout << "Could not load image " << filename << endl;
			cout << IMG_GetError() << endl;
			return;
		}

		int mode = GL_RGB;

		if (texture->format->BytesPerPixel == 4) {
			mode = GL_RGBA;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
			texture->w, texture->h, 0, mode,
			GL_UNSIGNED_BYTE, texture->pixels);

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

void Texture::Bind(GLuint activeTex)
{
	m_activeTex = activeTex;
	glActiveTexture(activeTex); // Active proper texture unit before binding

	switch (m_type)
	{
	case Tex2D:
		glBindTexture(GL_TEXTURE_2D, m_texID);
		break;
	case Skybox:
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
		break;
	default:
		glBindTexture(GL_TEXTURE_2D, m_texID);
		break;
	}
}

void Texture::Unbind()
{
	glActiveTexture(m_activeTex); // Active proper texture unit before binding									  
	glBindTexture(GL_TEXTURE_2D, 0);
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
	default:
		break;
	}
}

GLuint Texture::GetTexID()
{
	return m_texID;
}

TextureType Texture::GetTexType()
{
	return m_type;
}


