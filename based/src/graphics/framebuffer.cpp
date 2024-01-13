#include "graphics/framebuffer.h"
#include "log.h"
#include "graphics/helpers.h"

#include "glad/glad.h"
#include "core/profiler.h"

namespace based::graphics
{
	Framebuffer::Framebuffer(uint32_t width, uint32_t height)
		: mFbo(0)
		, mTextureId(0)
		, mRenderbufferId(0)
		, mSize({width, height})
		, mClearColor(1.f)
	{
		PROFILE_FUNCTION();
		glGenFramebuffers(1, &mFbo); BASED_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo); BASED_CHECK_GL_ERROR;

		// Create color texture
		glGenTextures(1, &mTextureId); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, mTextureId); BASED_CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSize.x, mSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0); BASED_CHECK_GL_ERROR;

		// Create depth/stencil renderbuffer
		glGenRenderbuffers(1, &mRenderbufferId); BASED_CHECK_GL_ERROR;
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferId); BASED_CHECK_GL_ERROR;
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mSize.x, mSize.y); BASED_CHECK_GL_ERROR;
		glBindRenderbuffer(GL_RENDERBUFFER, 0); BASED_CHECK_GL_ERROR;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRenderbufferId); BASED_CHECK_GL_ERROR;

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
		, mTextureId(0)
		, mRenderbufferId(0)
		, mSize({ width, height })
		, mClearColor(1.f)
	{
		PROFILE_FUNCTION();
		glGenFramebuffers(1, &mFbo); BASED_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo); BASED_CHECK_GL_ERROR;

		// Create color texture
		glGenTextures(1, &mTextureId); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, mTextureId); BASED_CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, format, mSize.x, mSize.y, 0, format, type, nullptr); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter == graphics::TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR); BASED_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filter == graphics::TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR); BASED_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, mTextureId, 0); BASED_CHECK_GL_ERROR;

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
		mTextureId = 0;
		mRenderbufferId = 0;
	}
}
