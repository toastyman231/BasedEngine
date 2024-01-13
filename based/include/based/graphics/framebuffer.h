#pragma once
#include <cstdint>

#include "texture.h"
#include "external/glm/glm.hpp"

namespace based::graphics
{
	class Framebuffer
	{
	public:
		Framebuffer(uint32_t width, uint32_t height);
		Framebuffer::Framebuffer(uint32_t width, uint32_t height, int format, int type,
			graphics::TextureFilter filter, int attachment, bool colorData);
		~Framebuffer();

		inline uint32_t GetFbo() const { return mFbo; }
		inline uint32_t GetTextureId() const { return mTextureId; }
		inline uint32_t GetRenderbufferId() const { return mRenderbufferId; }
		inline const glm::ivec2& GetSize() { return mSize; }
		inline void SetClearColor(const glm::vec4& cc) { mClearColor = cc; }
		inline const glm::vec4& GetClearColor() { return mClearColor; }
	private:
		uint32_t mFbo;
		uint32_t mTextureId;
		uint32_t mRenderbufferId;

		glm::ivec2 mSize;
		glm::vec4 mClearColor;
	};
}