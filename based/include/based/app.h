#pragma once

#include "engine.h"
#include "core/window.h"
#include "graphics/camera.h"
#include "scene/scene.h"
#include "memory/memoryhelpers.h"
#include "scene/sceneserializer.h"

namespace based
{
	struct GameSettings
	{
		uint32_t gameMemory;

		GameSettings();
	};

	inline GameSettings::GameSettings()
	{
		gameMemory = MEGABYTES_BYTES(100);
	}

	class App
	{
	public:
		App() = default;
		virtual ~App() = default;

		// TODO: should this be public?
		std::shared_ptr<scene::Scene> persistentScene;

		virtual core::WindowProperties GetWindowProperties() { return {}; }
		virtual based::GameSettings GetGameSettings() { return {}; }
		inline std::shared_ptr<scene::Scene> GetCurrentScene() const { return mCurrentScene; }
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

			persistentScene = std::make_shared<scene::Scene>();
			persistentScene->SetActiveCamera(mCamera);
			LoadScene(persistentScene);
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
