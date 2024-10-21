#pragma once

#include <external/entt/entt.hpp>

namespace based::scene
{
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

		inline void SetActiveCamera(std::shared_ptr<graphics::Camera> cam) { mActiveCamera = std::move(cam); }
	private:
		entt::registry mRegistry;
		std::shared_ptr<graphics::Camera> mActiveCamera;
	};
}
