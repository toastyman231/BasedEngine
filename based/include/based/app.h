#pragma once

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
		inline void LoadScene(std::shared_ptr<scene::Scene> newScene) { mCurrentScene = std::move(newScene); }

		virtual void Initialize()
		{
			auto& window = Engine::Instance().GetWindow();
			window.SetShouldRenderToScreen(true);

			mCamera = std::make_shared<graphics::Camera>();
			mCamera->SetHeight(2.f);
			mCamera->SetViewMatrix(glm::vec3(0.f), 0.f);

			startScene = std::make_shared<scene::Scene>();
			startScene->SetActiveCamera(mCamera);
			LoadScene(startScene);
		}
		virtual void Shutdown() {}

		virtual void Update() {}
		virtual void Render() {}
		virtual void ImguiRender() {}
	private:
		//std::shared_ptr<scene::Scene> startScene;
		std::shared_ptr<graphics::Camera> mCamera;
		std::shared_ptr<scene::Scene> mCurrentScene;
	};
}
