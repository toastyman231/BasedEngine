#pragma once
#include "core/assetlibrary.h"

namespace based::graphics
{
	class Texture;
}

namespace based::managers
{
	class ResourceManager
	{
	public:
		void Initialize();
		void Shutdown();

		std::shared_ptr<graphics::Texture> LoadTextureAsync(const std::string& path, bool overrideFlip = false);
	private:
		std::shared_ptr<graphics::Texture> mErrorTexture;

		core::AssetLibrary<graphics::Texture> mTextureStorage;
	};
}
