#include "graphics/texture.h"
#include "graphics/helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#include "glad/glad.h"
#include "log.h"

namespace based::graphics
{
	Texture::Texture(const std::string& path)
		: mPath(path)
		, mHeight(0)
		, mWidth(0)
		, mNumChannels(0)
		, mPixels(nullptr)
		, mStbiTex(true)
		, mFilter(TextureFilter::Linear)
	{
		int width, height, numChannels;
		stbi_set_flip_vertically_on_load(1);
		mPixels = stbi_load(path.c_str(), &width, &height, &numChannels, 0);
		if (mPixels)
		{
			mWidth = (uint32_t)width;
			mHeight = (uint32_t)height;
			mNumChannels = (uint32_t)numChannels;
		}

		LoadTexture();
	}

	//Texture::Texture(unsigned char* pixels, int width, int height)
	//	: mHeight(height)
	//	, mWidth(width)
	//	, mNumChannels(4)
	//	, mPixels(pixels)
	//	, mFilter(TextureFilter::Linear)
	//{
	//	LoadTexture(pixels, width, height);
	//}

	Texture::Texture(unsigned int id, uint32_t height, uint32_t width)
		: mId((GLuint) id)
		, mHeight(height)
		, mWidth(width)
		, mPixels(nullptr)
		, mNumChannels(4)
		, mStbiTex(false)
		, mFilter(TextureFilter::Linear)
	{
		//TODO: remove
	}

	Texture::Texture(const SDL_Surface* surface, unsigned int id)
		: mPath("")
		, mId(id)
		, mHeight(surface->h)
		, mWidth(surface->w)
		, mNumChannels(surface->format->BytesPerPixel)
		, mPixels((unsigned char*)surface->pixels)
		, mStbiTex(false)
		, mFilter(TextureFilter::Nearest)
		, mRmask(surface->format->Rmask)
	{
		LoadTexture();
	}

	Texture::~Texture()
	{
		if (mStbiTex)
		{
			stbi_image_free(mPixels);
			mPixels = nullptr;
		}
	}

	void Texture::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;
	}

	void Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	void Texture::FreeTexture()
	{
		glDeleteTextures(1, &mId);
		//delete(this);
	}

	void Texture::SetTextureFilter(TextureFilter filter)
	{
		mFilter = filter;

		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;
		switch (mFilter)
		{
		case TextureFilter::Linear:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
			break;
		case TextureFilter::Nearest:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); BASED_CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); BASED_CHECK_GL_ERROR;
			break;
		}
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	void Texture::LoadTexture()
	{
		glGenTextures(1, &mId); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;

		GLenum dataFormat = 0;

		switch (mNumChannels)
		{
		case 1:
			dataFormat = GL_ALPHA;
			break;
		case 3:     // no alpha channel
			if (mRmask == 0x000000ff)
				dataFormat = GL_RGB;
			else
				dataFormat = GL_BGR;
			break;
		case 4:     // contains an alpha channel
			if (mRmask == 0x000000ff)
				dataFormat = GL_RGBA;
			else
				dataFormat = GL_BGRA;
			break;
		default:
			break;
		}

		if (mPixels && dataFormat == 0) BASED_ERROR("Texture format not supported - num channnels: {}", mNumChannels);

		/*if (pixels != nullptr)
		{
			mWidth = width;
			mHeight = height;
			mNumChannels = 4;

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, pixels); BASED_CHECK_GL_ERROR;
			SetTextureFilter(TextureFilter::Nearest);
			glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
			return;
		}*/

		if (mPixels && dataFormat != 0)
		{
			// TODO: Make this use the right enum and not crash
			BASED_TRACE(dataFormat);
			glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, mWidth, mHeight, 0, dataFormat, GL_UNSIGNED_BYTE, mPixels); BASED_CHECK_GL_ERROR;
			SetTextureFilter(mFilter);
			BASED_TRACE("Loaded {}-channel texture: {}", mNumChannels, mPath.c_str());
		}
		else
		{
			float pixels[] = {
				1.f, 0.f, 1.f,	1.f, 1.f, 1.f,	1.f, 0.f, 1.f,	1.f, 1.f, 1.f,
				1.f, 1.f, 1.f,	1.f, 0.f, 1.f,	1.f, 1.f, 1.f,	1.f, 0.f, 1.f,
				1.f, 0.f, 1.f,	1.f, 1.f, 1.f,	1.f, 0.f, 1.f,	1.f, 1.f, 1.f,
				1.f, 1.f, 1.f,	1.f, 0.f, 1.f,	1.f, 1.f, 1.f,	1.f, 0.f, 1.f,
			};

			mWidth = 4;
			mHeight = 4;
			mNumChannels = 3;

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, pixels); BASED_CHECK_GL_ERROR;
			SetTextureFilter(TextureFilter::Nearest);
			BASED_WARN("Unable to load texture: {} - defaulting to checkerboard", mPath.c_str());
		}
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}
}
