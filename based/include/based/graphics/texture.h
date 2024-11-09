#pragma once

#include <string>

struct SDL_Surface;

namespace based::graphics
{
	enum class TextureFilter
	{
		Nearest,
		Linear
	};

	class Texture
	{
	public:
		Texture(const std::string& path, bool overrideFlip = false);
		Texture(const SDL_Surface* surface, unsigned int id);
		Texture(const Texture& other) = default;
		Texture(unsigned int id, uint32_t width, uint32_t height);
		Texture() = default;
		~Texture();

		inline uint32_t GetId() const { return mId; }
		inline uint32_t GetWidth() const { return mWidth; }
		inline uint32_t GetHeight() const { return mHeight; }
		inline uint32_t GetNumChannels() const { return mNumChannels; }
		inline const std::string& GetPath() const { return mPath; }
		inline TextureFilter GetTextureFilter() const { return mFilter; }

		void Bind();
		void Unbind();

		void SetTextureFilter(TextureFilter filter);
	private:
		TextureFilter mFilter;

		std::string mPath = "";
		uint32_t mId;
		uint32_t mWidth, mHeight;
		uint32_t mNumChannels;
		uint32_t mRmask;

		bool mStbiTex;
		bool mSdlSurface;

		unsigned char* mPixels;

		void LoadTexture();
	};
}
