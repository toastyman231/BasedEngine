#include "pch.h"
#include "graphics/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#include "glad/glad.h"

#include <SDL2/SDL_surface.h>

#include "app.h"
#include "engine.h"

namespace based::graphics
{
	Texture::Texture(const std::string& path, bool overrideFlip)
		: mFilter(TextureFilter::Linear)
		, mPath(path)
		, mWidth(0)
		, mHeight(0)
		, mNumChannels(0)
		, mStbiTex(true)
		, mSdlSurface(false)
		, mPixels(nullptr)
		, mUUID(core::UUID())
	{
		PROFILE_FUNCTION();

		int width, height, numChannels;
		if (overrideFlip) stbi_set_flip_vertically_on_load(0);
		else stbi_set_flip_vertically_on_load(1);
		mPixels = stbi_load(path.c_str(), &width, &height, &numChannels, 0);
		if (mPixels)
		{
			mWidth = (uint32_t)width;
			mHeight = (uint32_t)height;
			mNumChannels = (uint32_t)numChannels;
		}

		LoadTexture();
	}

	Texture::Texture(const SDL_Surface* surface, unsigned int id)
		: mFilter(TextureFilter::Nearest)
		, mPath("")
		, mId(id)
		, mWidth(surface->w)
		, mHeight(surface->h)
		, mNumChannels(surface->format->BytesPerPixel)
		, mRmask(surface->format->Rmask)
		, mStbiTex(false)
		, mSdlSurface(true)
		, mPixels((unsigned char*)surface->pixels)
		, mUUID(core::UUID())
	{
		LoadTexture();
	}

	Texture::Texture(unsigned id, uint32_t width, uint32_t height)
		: mFilter(TextureFilter::Nearest)
		, mId(id)
		, mWidth(width)
		, mHeight(height)
		, mNumChannels(4)
		, mPixels(nullptr)
		, mUUID(core::UUID())
	{
	}

	Texture::Texture(uint32_t width, uint32_t height)
		: mFilter(TextureFilter::Nearest)
		, mWidth(width)
		, mHeight(height)
		, mPixels(nullptr)
		, mUUID(core::UUID())
	{
		glGenTextures(1, &mId); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); BASED_CHECK_GL_ERROR;
		SetTextureFilter(mFilter); 
		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mWidth, mHeight, 0, GL_RGBA,
			GL_FLOAT, mPixels); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	Texture::Texture(uint32_t width, uint32_t height, int numChannels, unsigned char* data)
		: mFilter(TextureFilter::Nearest),
		mWidth(width),
		mHeight(height),
		mNumChannels(numChannels),
		mPixels(data),
		mUUID(core::UUID())
	{
		glGenTextures(1, &mId); BASED_CHECK_GL_ERROR;
		SetWrapMode(TextureWrapMode::ClampEdge);
		SetTextureFilter(mFilter);
		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mWidth, mHeight, 0, GL_RGBA,
			GL_FLOAT, mPixels); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	Texture::~Texture()
	{
		if (mStbiTex)
		{
			stbi_image_free(mPixels);
			mPixels = nullptr;
		} else if (mSdlSurface)
		{
			return;
		} else if (mPixels)
		{
			free(mPixels);
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

	void Texture::SetTextureFilter(TextureFilter filter)
	{
		mFilter = filter;

		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;
		switch (mFilter)
		{
		case TextureFilter::Linear:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); BASED_CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
			break;
		case TextureFilter::Nearest:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); BASED_CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); BASED_CHECK_GL_ERROR;
			break;
		}
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	void Texture::SetWrapMode(TextureWrapMode mode)
	{
		glBindTexture(GL_TEXTURE_2D, mId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)mode); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)mode); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::shared_ptr<Texture> Texture::CreateImageTexture(const std::string& name, 
	                                                     uint32_t width, uint32_t height, TextureAccessLevel accessLevel, core::AssetLibrary<Texture>& library)
	{
		auto tex = std::make_shared<Texture>(width, height);
		library.Load(name, tex);

		glBindImageTexture(GetNextImageTextureUnit(), tex->GetId(), 0, GL_FALSE, 0, 
			static_cast<GLenum>(accessLevel), GL_RGBA32F); BASED_CHECK_GL_ERROR;

		return tex;
	}

	void Texture::LoadTexture()
	{
		PROFILE_FUNCTION();
		glGenTextures(1, &mId); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, mId); BASED_CHECK_GL_ERROR;

		GLenum dataFormat = 0;

		switch (mNumChannels)
		{
		case 1:
			dataFormat = GL_RED;
			break;
		case 3:     // no alpha channel
			// TODO: Figure out a better way to determine RGB vs BGR
			/*if (mRmask == 0x000000ff)
				dataFormat = GL_RGB;
			else
				dataFormat = GL_BGR;*/
			dataFormat = GL_RGB;
			break;
		case 4:     // contains an alpha channel
			/*if (mRmask == 0x000000ff)
				dataFormat = GL_RGBA;
			else
				dataFormat = GL_BGRA;*/
			dataFormat = GL_RGBA;
			break;
		default:
			break;
		}

		if (mPixels && dataFormat == 0) BASED_ERROR("Texture format not supported - num channnels: {}", mNumChannels);

		if (mPixels && dataFormat != 0)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, mWidth, mHeight, 
				0, dataFormat, GL_UNSIGNED_BYTE, mPixels); BASED_CHECK_GL_ERROR;
			glGenerateMipmap(GL_TEXTURE_2D); BASED_CHECK_GL_ERROR;
			SetTextureFilter(mFilter);
			//BASED_TRACE("Loaded {}-channel texture: {}", mNumChannels, mPath.c_str());
		}
		else
		{
			if (auto errTex = Engine::Instance().GetResourceManager().GetErrorTexture())
			{
				mId = errTex->mId;
				mWidth = errTex->mWidth;
				mHeight = errTex->mHeight;
				mNumChannels = errTex->mNumChannels;
			} else
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

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight,
					0, GL_RGB, GL_FLOAT, pixels); BASED_CHECK_GL_ERROR;
				glGenerateMipmap(GL_TEXTURE_2D); BASED_CHECK_GL_ERROR;
				SetTextureFilter(TextureFilter::Nearest);
				//BASED_WARN("Unable to load texture: {} - defaulting to checkerboard", mPath.c_str());
			}
		}
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
	}

	void Texture::ResetUUID(uint64_t id)
	{
		mUUID = id;
	}
}
