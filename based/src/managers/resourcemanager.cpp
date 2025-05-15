#include "pch.h"
#include "managers/resourcemanager.h"

#include "engine.h"
#include "external/stb/stb_image.h"
#include "graphics/texture.h"

namespace based::managers
{
	void ResourceManager::Initialize()
	{
		BASED_TRACE("Generating error texture, ignore any following warnings/errors");
		mErrorTexture = std::make_shared<graphics::Texture>("INVALID");
	}

	void ResourceManager::Shutdown()
	{
		mErrorTexture.reset();
	}

	std::shared_ptr<graphics::Texture> ResourceManager::LoadTextureAsync(const std::string& path, bool overrideFlip)
	{
		auto tex = std::make_shared<graphics::Texture>();
		tex->mId = mErrorTexture->mId;
		tex->mWidth = tex->mHeight = 4;
		tex->mNumChannels = 3;
		tex->mPath = path;
		tex->SetName(tex->mPath);
		mTextureStorage.Load(tex->GetName(), tex);
		Engine::Instance().GetJobManager().Execute([tex, path, overrideFlip]
			{
				int width, height, numChannels;
				if (overrideFlip) stbi_set_flip_vertically_on_load(0);
				else stbi_set_flip_vertically_on_load(1);
				tex->mPixels = stbi_load(path.c_str(), &width, &height, 
					&numChannels, 0);
				if (tex->mPixels)
				{
					tex->mWidth = (uint32_t)width;
					tex->mHeight = (uint32_t)height;
					tex->mNumChannels = (uint32_t)numChannels;

					Engine::Instance().GetJobManager().ExecuteOnMainThread(
						[tex]
						{
							tex->LoadTexture();
						});
				}
			});
		return tex;
	}
}
