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

	Texture::Texture(const SDL_Surface* surface)
		: mPath(nullptr)
		, mHeight(surface->h)
		, mWidth(surface->w)
		, mNumChannels(4)
		, mPixels((unsigned char*)surface->pixels)
		, mFilter(TextureFilter::Linear)
	{
		LoadTexture();
	}

	Texture::~Texture()
	{
		stbi_image_free(mPixels);
		mPixels = nullptr;
	}

	void Texture::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;
	}

	void Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
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
		if (mNumChannels == 4)
		{
			dataFormat = GL_RGBA;
		}
		else if (mNumChannels == 3)
		{
			dataFormat = GL_RGB;
		}

		if (mPixels && dataFormat == 0) BASED_ERROR("Texture format not supported - num channnels: {}", mNumChannels);

		if (mPixels && dataFormat != 0)
		{
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
