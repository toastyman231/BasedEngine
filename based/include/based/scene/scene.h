#pragma once

#include <external/entt/entt.hpp>
#include "based/core/assetlibrary.h"
#include "based/engine.h"

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
		virtual ~Scene() = default;

		virtual void InitializeScene();
		void AnimateScene() const;
		virtual void RenderScene() const;
		virtual void UpdateScene(float deltaTime);
		virtual void ShutdownScene();

		entt::registry& GetRegistry() { return mRegistry; }
		std::shared_ptr<graphics::Camera> GetActiveCamera() const { return mActiveCamera; }
		std::string GetSceneName() const { return mSceneName; }

		static core::AssetLibrary<graphics::Texture>& GetTextureStorage() { return Engine::Instance().GetResourceManager().GetTextureStorage(); }
		static core::AssetLibrary<graphics::Material>& GetMaterialStorage() { return Engine::Instance().GetResourceManager().GetMaterialStorage(); }
		static core::AssetLibrary<graphics::Mesh>& GetMeshStorage() { return Engine::Instance().GetResourceManager().GetMeshStorage(); }
		static core::AssetLibrary<graphics::Model>& GetModelStorage() { return Engine::Instance().GetResourceManager().GetModelStorage(); }
		static core::AssetLibrary<animation::Animation>& GetAnimationStorage() { return Engine::Instance().GetResourceManager().GetAnimationStorage(); }
		static core::AssetLibrary<animation::Animator>& GetAnimatorStorage() { return Engine::Instance().GetResourceManager().GetAnimatorStorage(); }

		core::AssetLibrary<Entity>& GetEntityStorage() { return mEntityStorage; }
		core::AssetLibrary<graphics::Camera>& GetCameraStorage() { return mCameraStorage; }

		void SetActiveCamera(std::shared_ptr<graphics::Camera> cam);
		void SetSceneName(const std::string& name) { mSceneName = name; }

		static bool LoadScene(const std::string& path, const std::string& prefix = "", bool keepLoadedAssets = true);
		static bool LoadSceneAdditive(const std::string& path, const std::string& prefix = "");
	private:
		entt::registry mRegistry;
		std::shared_ptr<graphics::Camera> mActiveCamera;
		std::string mSceneName = "Unnamed Scene";

		core::AssetLibrary<Entity> mEntityStorage;
		core::AssetLibrary<graphics::Camera> mCameraStorage;

		friend class SceneSerializer;
	};
}
