#pragma once

#include <string>
#include <memory>

#include "based/core/assetlibrary.h"
#include "based/core/uuid.h"

#define DEFAULT_TEX_LIB based::graphics::DefaultLibraries::GetTextureLibrary()

namespace based::managers
{
	class ResourceManager;
}

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
		Texture(uint32_t width, uint32_t height, int numChannels, unsigned char* data);
		Texture() = default;
		~Texture();

		inline uint32_t GetId() const { return mId; }
		inline uint64_t GetUUID() const { return mUUID; }
		inline std::string GetName() const { return mName; }
		inline uint32_t GetWidth() const { return mWidth; }
		inline uint32_t GetHeight() const { return mHeight; }
		inline uint32_t GetNumChannels() const { return mNumChannels; }
		inline const std::string& GetPath() const { return mPath; }
		inline TextureFilter GetTextureFilter() const { return mFilter; }

		void Bind();
		void Unbind();

		void SetTextureFilter(TextureFilter filter);
		void SetName(const std::string& name) { mName = name; }

		static std::shared_ptr<Texture> CreateImageTexture(const std::string& name,
			uint32_t width, uint32_t height, TextureAccessLevel accessLevel, core::AssetLibrary<Texture>& library);
		static uint32_t GetNextImageTextureUnit() { return mNextId++; }

		friend class managers::ResourceManager;
	private:
		TextureFilter mFilter;

		std::string mPath = "";
		std::string mName = "New Texture";
		uint32_t mId;
		core::UUID mUUID;
		uint32_t mWidth, mHeight;
		uint32_t mNumChannels;
		uint32_t mRmask;

		bool mStbiTex;
		bool mSdlSurface;

		unsigned char* mPixels;

		void LoadTexture();
		void ResetUUID(uint64_t id);

		inline static uint32_t mNextId = 0;

		friend class SceneSerializer;
	};
}
