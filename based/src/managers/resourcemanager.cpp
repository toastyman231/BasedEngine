#include "pch.h"
#include "managers/resourcemanager.h"

#include <ktx.h>

#include "engine.h"
#include "external/stb/stb_image.h"
#include "graphics/texture.h"

namespace based::managers
{
	void ResourceManager::Initialize()
	{
		PROFILE_FUNCTION();
		BASED_TRACE("Generating error texture, ignore any following warnings/errors");
		mErrorTexture = std::make_shared<graphics::Texture>("INVALID");
	}

	void ResourceManager::Shutdown()
	{
		mErrorTexture.reset();
	}

	std::shared_ptr<graphics::Texture> ResourceManager::LoadTextureAsync(const std::string& path, bool overrideFlip)
	{
		PROFILE_FUNCTION();
		auto tex = std::make_shared<graphics::Texture>();
		tex->mId = mErrorTexture->mId;
		tex->mWidth = tex->mHeight = 4;
		tex->mNumChannels = 3;
		tex->mPath = path;
		tex->SetName(std::filesystem::path(path).filename().string());
		mTextureStorage.Load(tex->GetName(), tex);
		Engine::Instance().GetJobManager().Execute([tex, path, overrideFlip]
			{
				PROFILE_SCOPE("Load Texture Async");
				PROFILE_SCOPE_TEXT("Texture: %s", path.c_str());

				if (std::filesystem::path(path).extension() == ".ktx2")
				{
					ktxTexture2* kTexture;
					GLuint texture = 0;

					ktx_error_code_e result = ktxTexture2_CreateFromNamedFile(path.c_str(),
					                                                        KTX_TEXTURE_CREATE_NO_FLAGS,
					                                                        &kTexture);
					
					khr_df_model_e dataFormat = ktxTexture2_GetColorModel_e(kTexture);
					if (dataFormat == KHR_DF_MODEL_BC5)
					{
						tex->mBC5Compressed = true;
					}
					
					if (result != KTX_SUCCESS)
					{
						BASED_FATAL("Fatal error: {}", result);
					}
					
					Engine::Instance().GetJobManager().ExecuteOnMainThread(
						[kTexture, tex, texture]
						{
							tex->LoadKTXTexture(kTexture);
						});
				} else
				{
					int width, height, numChannels;
					if (overrideFlip) stbi_set_flip_vertically_on_load(0);
					else stbi_set_flip_vertically_on_load(1);
					tex->mPixels = stbi_load(path.c_str(), &width, &height, 
						&numChannels, 0);
					if (tex->mPixels)
					{
						tex->mWidth = static_cast<uint32_t>(width);
						tex->mHeight = static_cast<uint32_t>(height);
						tex->mNumChannels = static_cast<uint32_t>(numChannels);

						Engine::Instance().GetJobManager().ExecuteOnMainThread(
							[tex]
							{
								tex->LoadTexture();
							});
					}
				}
			});
		return tex;
	}
}
