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
		Scene(const std::string& name) : Scene()
		{
			mSceneName = name;
		}
		Scene(const std::string& name, entt::registry& registry)
			: Scene(name)
		{
			for (auto [id, storage] : registry.storage())
			{
				if (storage.type().name().find("DontDestroyOnLoad") != std::string_view::npos)
				{
					for (auto [newId, newStorage] : mRegistry.storage())
					{
						if (newStorage.type().name().find("DontDestroyOnLoad") != std::string_view::npos)
						{
							// TODO: Load scenes properly
							//newStorage.push(storage, );
						}
					}
				}
			}
		}
		~Scene() = default;

		void InitializeScene() const;
		void AnimateScene() const;
		void RenderScene() const;
		void UpdateScene(float deltaTime);
		void ShutdownScene() const;

		inline entt::registry& GetRegistry() { return mRegistry; }
		inline std::shared_ptr<graphics::Camera> GetActiveCamera() const { return mActiveCamera; }
		inline std::string GetSceneName() const { return mSceneName; }

		inline core::AssetLibrary<graphics::Texture>& GetTextureStorage() { return mTextureStorage; }
		inline core::AssetLibrary<graphics::Material>& GetMaterialStorage() { return mMaterialStorage; }
		inline core::AssetLibrary<graphics::Mesh>& GetMeshStorage() { return mMeshStorage; }
		inline core::AssetLibrary<graphics::Model>& GetModelStorage() { return mModelStorage; }
		inline core::AssetLibrary<Entity>& GetEntityStorage() { return mEntityStorage; }
		inline core::AssetLibrary<animation::Animation>& GetAnimationStorage() { return mAnimationStorage; }
		inline core::AssetLibrary<animation::Animator>& GetAnimatorStorage() { return mAnimatorStorage; }
		inline core::AssetLibrary<graphics::Camera>& GetCameraStorage() { return mCameraStorage; }

		void SetActiveCamera(std::shared_ptr<graphics::Camera> cam);
		void SetSceneName(const std::string& name) { mSceneName = name; }

		static bool LoadScene(const std::string& path);
		static bool LoadSceneAdditive(const std::string& path);
	private:
		entt::registry mRegistry;
		std::shared_ptr<graphics::Camera> mActiveCamera;
		std::string mSceneName = "Unnamed Scene";

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
