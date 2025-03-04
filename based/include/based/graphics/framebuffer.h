#pragma once

#include "texture.h"
#include <memory>
#include <external/glm/vec2.hpp>
#include <external/glm/vec4.hpp>

namespace based::graphics
{
	class Framebuffer
	{
	public:
		Framebuffer();
		Framebuffer(uint32_t width, uint32_t height);
		Framebuffer(uint32_t width, uint32_t height, int format, int type,
			graphics::TextureFilter filter, int attachment, bool colorData);
		~Framebuffer();

		inline uint32_t GetFbo() const { return mFbo; }
		inline uint32_t GetTextureId() const { return mTextureIDs[0]; }
		inline uint32_t GetTexture(int index)
		{
			BASED_ASSERT(index >= 0 && index < mTextureIDs.size(), "Invalid texture index passed to framebuffer!");
			return mTextureIDs[index];
		}
		inline const glm::ivec2& GetSize() { return mSize; }
		inline void SetClearColor(const glm::vec4& cc) { mClearColor = cc; }
		inline const glm::vec4& GetClearColor() { return mClearColor; }

		Framebuffer& AddTexture(int attachment, int type, int target, int format, TextureFilter filter = TextureFilter::Linear);
		Framebuffer& AddTexture(uint32_t id, int attachment, int target);
	private:
		uint32_t mFbo;
		std::vector<uint32_t> mTextureIDs;

		glm::ivec2 mSize;
		glm::vec4 mClearColor;
	};
}