#pragma once

#include "engine.h"
#include "core/window.h"
#include "graphics/camera.h"
#include "scene/scene.h"

namespace based
{
	class App
	{
	public:
		App() {}
		~App() {}

		// TODO: should this be public?
		std::shared_ptr<scene::Scene> startScene;

		virtual core::WindowProperties GetWindowProperties() { return core::WindowProperties(); }
		inline const std::shared_ptr<scene::Scene> GetCurrentScene() const { return mCurrentScene; }
		inline void LoadScene(std::shared_ptr<scene::Scene> newScene) 
		{
			mCurrentScene = std::move(newScene);
			mCurrentScene->InitializeScene();
		}

		virtual void Initialize()
		{
			auto& window = Engine::Instance().GetWindow();
			window.SetShouldRenderToScreen(true);

			mCamera = std::make_shared<graphics::Camera>();
			mCamera->SetHeight(2.f);

			//mCameraEntity = new scene::Entity();
			/*mCameraEntity = scene::Entity::CreateEntity<scene::Entity>(
				glm::vec3(0.f, 0.f, 1.5f), 
				glm::vec3(0.f), 
				glm::vec3(1.f));*/
			//mCameraEntity->AddComponent<scene::Camera>(mCamera);
			//mCamera->SetViewMatrix(glm::vec3(0.f), 0.f)

			startScene = std::make_shared<scene::Scene>();
			startScene->SetActiveCamera(mCamera);
			LoadScene(startScene);
		}
		virtual void Shutdown() {}

		virtual void Update(float deltaTime) {}
		virtual void Render() {}
		virtual void ImguiRender() {}
	private:
		std::shared_ptr<graphics::Camera> mCamera;
		std::shared_ptr<scene::Scene> mCurrentScene;
	};
}
