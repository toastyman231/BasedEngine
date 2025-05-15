#include "pch.h"
#include "graphics/framebuffer.h"

#include "engine.h"
#include "glad/glad.h"

namespace based::graphics
{
	Framebuffer::Framebuffer()
	{
		PROFILE_FUNCTION();
		glGenFramebuffers(1, &mFbo); BASED_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo); BASED_CHECK_GL_ERROR;

		mSize = Engine::Instance().GetWindow().GetSize();
		mClearColor = glm::vec4(1.f);

		AddTexture(Engine::Instance().GetWindow().GetFramebuffer()->GetTexture(0),
			GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);

		// Check for completeness
		int32_t completeStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); BASED_CHECK_GL_ERROR;
		if (completeStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			BASED_ERROR("Failure to create framebuffer. Complete status: {}", completeStatus);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); BASED_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, 0); BASED_CHECK_GL_ERROR;
	}

	Framebuffer::Framebuffer(uint32_t width, uint32_t height)
		: mFbo(0)
		, mSize({width, height})
		, mClearColor(1.f)
	{
		PROFILE_FUNCTION();
		glGenFramebuffers(1, &mFbo); BASED_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo); BASED_CHECK_GL_ERROR;

		// Create color texture
		AddTexture(GL_COLOR_ATTACHMENT0, GL_UNSIGNED_BYTE, GL_TEXTURE_2D, GL_RGBA);

		// Create depth/stencil texture
		AddTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_UNSIGNED_INT_24_8, GL_TEXTURE_2D, GL_DEPTH_STENCIL);

		// Check for completeness
		int32_t completeStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); BASED_CHECK_GL_ERROR;
		if (completeStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			BASED_ERROR("Failure to create framebuffer. Complete status: {}", completeStatus);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); BASED_CHECK_GL_ERROR;
	}

	Framebuffer::Framebuffer(uint32_t width, uint32_t height, int format, int type,
		graphics::TextureFilter filter, int attachment, bool colorData)
		: mFbo(0)
		, mSize({ width, height })
		, mClearColor(1.f)
	{
		PROFILE_FUNCTION();
		glGenFramebuffers(1, &mFbo); BASED_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo); BASED_CHECK_GL_ERROR;

		// Create color texture
		AddTexture(attachment, type, GL_TEXTURE_2D, format, filter);

		if (!colorData)
		{
			glDrawBuffer(GL_NONE); BASED_CHECK_GL_ERROR;
			glReadBuffer(GL_NONE); BASED_CHECK_GL_ERROR;
		}

		// Check for completeness
		int32_t completeStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); BASED_CHECK_GL_ERROR;
		if (completeStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			BASED_ERROR("Failure to create framebuffer. Complete status: {}", completeStatus);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); BASED_CHECK_GL_ERROR;
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &mFbo); BASED_CHECK_GL_ERROR;
		mFbo = 0;
		mTextureIDs.clear();
	}

	Framebuffer& Framebuffer::AddTexture(int attachment, int type, int target, int format, TextureFilter filter)
	{
		uint32_t textureId;
		glGenTextures(1, &textureId); BASED_CHECK_GL_ERROR;
		glBindTexture(target, textureId); BASED_CHECK_GL_ERROR;
		glTexImage2D(target, 0, format, mSize.x, mSize.y, 0,
			format, type, nullptr); BASED_CHECK_GL_ERROR;
		if (filter == TextureFilter::Linear)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
		} else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST); BASED_CHECK_GL_ERROR;
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST); BASED_CHECK_GL_ERROR;
		}
		glBindTexture(target, 0); BASED_CHECK_GL_ERROR;
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, textureId, 0); BASED_CHECK_GL_ERROR;

		mTextureIDs.emplace_back(textureId);
		return *this;
	}

	Framebuffer& Framebuffer::AddTexture(uint32_t id, int attachment, int target)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, id, 0); BASED_CHECK_GL_ERROR;

		mTextureIDs.emplace_back(id);
		return *this;
	}
}
