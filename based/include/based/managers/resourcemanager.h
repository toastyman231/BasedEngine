#pragma once
#include "core/assetlibrary.h"

namespace based::animation
{
	class Animation;
	class Animator;
}

namespace based::graphics
{
	class Model;
	class Mesh;
	class Material;
	class Texture;
}

namespace based::managers
{
	class ResourceManager
	{
	public:
		void Initialize();
		void Shutdown();

		core::AssetLibrary<graphics::Texture>& GetTextureStorage() { return mTextureStorage; }
		core::AssetLibrary<graphics::Material>& GetMaterialStorage() { return mMaterialStorage; }
		core::AssetLibrary<graphics::Mesh>& GetMeshStorage() { return mMeshStorage; }
		core::AssetLibrary<graphics::Model>& GetModelStorage() { return mModelStorage; }
		core::AssetLibrary<animation::Animation>& GetAnimationStorage() { return mAnimationStorage; }
		core::AssetLibrary<animation::Animator>& GetAnimatorStorage() { return mAnimatorStorage; }

		std::shared_ptr<graphics::Texture> LoadTextureAsync(const std::string& path, bool overrideFlip = false);
		std::shared_ptr<graphics::Texture> GetErrorTexture() { return mErrorTexture; }
	private:
		std::shared_ptr<graphics::Texture> mErrorTexture;

		core::AssetLibrary<graphics::Texture> mTextureStorage;
		core::AssetLibrary<graphics::Material> mMaterialStorage;
		core::AssetLibrary<graphics::Mesh> mMeshStorage;
		core::AssetLibrary<graphics::Model> mModelStorage;
		core::AssetLibrary<animation::Animation> mAnimationStorage;
		core::AssetLibrary<animation::Animator> mAnimatorStorage;
	};
}
