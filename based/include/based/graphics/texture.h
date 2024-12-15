#pragma once

#include <string>
#include <memory>

#include "assetlibrary.h"

#define DEFAULT_TEX_LIB based::graphics::DefaultLibraries::GetTextureLibrary()

struct SDL_Surface;

namespace based::graphics
{
	enum class TextureFilter
	{
		Nearest,
		Linear
	};

	enum class TextureAccessLevel
	{
		ReadOnly = 0x88B8,
		WriteOnly = 0x88B9,
		ReadWrite = 0x88BA
	};

	class Texture
	{
	public:
		Texture(const std::string& path, bool overrideFlip = false);
		Texture(const SDL_Surface* surface, unsigned int id);
		Texture(const Texture& other) = default;
		Texture(unsigned int id, uint32_t width, uint32_t height);
		Texture(uint32_t width, uint32_t height);
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

		static std::shared_ptr<Texture> CreateImageTexture(const std::string& name,
			uint32_t width, uint32_t height, TextureAccessLevel accessLevel, core::AssetLibrary<Texture>& library);
		static uint32_t GetNextImageTextureUnit() { return mNextId++; }
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

		inline static uint32_t mNextId = 0;
	};
}
