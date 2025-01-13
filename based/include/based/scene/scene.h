#pragma once

#include <external/entt/entt.hpp>
#include "based/core/assetlibrary.h"

namespace based::animation
{
	class Animator;
	class Animation;
}

namespace based::graphics
{
	class Texture;
	class Material;
	class Model;
	class Mesh;
	class Camera;
}

namespace based::scene
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		void InitializeScene() const;
		void AnimateScene() const;
		void RenderScene() const;
		void UpdateScene(float deltaTime);
		void ShutdownScene() const;

		inline entt::registry& GetRegistry() { return mRegistry; }
		inline std::shared_ptr<graphics::Camera> GetActiveCamera() const { return mActiveCamera; }

		inline core::AssetLibrary<graphics::Texture>& GetTextureStorage() { return mTextureStorage; }
		inline core::AssetLibrary<graphics::Material>& GetMaterialStorage() { return mMaterialStorage; }
		inline core::AssetLibrary<graphics::Mesh>& GetMeshStorage() { return mMeshStorage; }
		inline core::AssetLibrary<graphics::Model>& GetModelStorage() { return mModelStorage; }
		inline core::AssetLibrary<Entity>& GetEntityStorage() { return mEntityStorage; }
		inline core::AssetLibrary<animation::Animation>& GetAnimationStorage() { return mAnimationStorage; }
		inline core::AssetLibrary<animation::Animator>& GetAnimatorStorage() { return mAnimatorStorage; }
		inline core::AssetLibrary<graphics::Camera>& GetCameraStorage() { return mCameraStorage; }

		void SetActiveCamera(std::shared_ptr<graphics::Camera> cam);
	private:
		entt::registry mRegistry;
		std::shared_ptr<graphics::Camera> mActiveCamera;

		core::AssetLibrary<graphics::Texture> mTextureStorage;
		core::AssetLibrary<graphics::Material> mMaterialStorage;
		core::AssetLibrary<graphics::Mesh> mMeshStorage;
		core::AssetLibrary<graphics::Model> mModelStorage;
		core::AssetLibrary<Entity> mEntityStorage;
		core::AssetLibrary<animation::Animation> mAnimationStorage;
		core::AssetLibrary<animation::Animator> mAnimatorStorage;
		core::AssetLibrary<graphics::Camera> mCameraStorage;

		friend class SceneSerializer;
	};
}
