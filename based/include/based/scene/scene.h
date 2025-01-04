#pragma once

#include <external/entt/entt.hpp>
#include "based/core/assetlibrary.h"

namespace based::graphics
{
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
		void UpdateScene(float deltaTime) const;
		void ShutdownScene() const;

		inline entt::registry& GetRegistry() { return mRegistry; }
		inline std::shared_ptr<graphics::Camera> GetActiveCamera() const { return mActiveCamera; }
		inline core::AssetLibrary<Entity> GetEntityStorage() const { return mAssetStorage; }

		void SetActiveCamera(std::shared_ptr<graphics::Camera> cam);
	private:
		entt::registry mRegistry;
		std::shared_ptr<graphics::Camera> mActiveCamera;
		core::AssetLibrary<Entity> mAssetStorage;

		friend class SceneSerializer;
	};
}
