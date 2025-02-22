#pragma once

#include "engine.h"
#include "core/window.h"
#include "graphics/camera.h"
#include "scene/scene.h"
#include "memory/memoryhelpers.h"
#include "scene/sceneserializer.h"

#define REFLECT_SIMPLE_TYPE(T) entt::meta<T>().type(entt::type_hash<T>());

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
			mCurrentScene.reset();
			mCurrentScene = std::move(newScene);
			mCurrentScene->InitializeScene();
		}

		virtual void Initialize()
		{
			using namespace entt::literals;
			REFLECT_SIMPLE_TYPE(std::string)
			REFLECT_SIMPLE_TYPE(int)
			REFLECT_SIMPLE_TYPE(unsigned int)
			REFLECT_SIMPLE_TYPE(float)
			REFLECT_SIMPLE_TYPE(char)
			REFLECT_SIMPLE_TYPE(unsigned char)
			REFLECT_SIMPLE_TYPE(short)
			REFLECT_SIMPLE_TYPE(unsigned short)
			REFLECT_SIMPLE_TYPE(double)
			REFLECT_SIMPLE_TYPE(bool)
			REFLECT_SIMPLE_TYPE(long)
			REFLECT_SIMPLE_TYPE(unsigned long)
			REFLECT_SIMPLE_TYPE(long long)
			REFLECT_SIMPLE_TYPE(unsigned long long)
			entt::meta<graphics::Projection>()
				.type(entt::type_hash<graphics::Projection>())
				.data<graphics::Projection::ORTHOGRAPHIC>("Orthographic"_hs)
				.data<graphics::Projection::PERSPECTIVE>("Perspective"_hs);
			entt::meta<scene::ScriptComponent>()
				.type(entt::type_hash<scene::ScriptComponent>())
				.data<&scene::ScriptComponent::Enabled>("Enabled"_hs);

			auto& window = Engine::Instance().GetWindow();
			window.SetShouldRenderToScreen(true);

			persistentScene = std::make_shared<scene::Scene>();
			LoadScene(persistentScene);
		}
		virtual void Shutdown() {}

		virtual bool ValidateShutdown() { return true; }

		virtual void Update(float deltaTime) {}
		virtual void Render() {}
		virtual void ImguiRender() {}
	private:
		std::shared_ptr<graphics::Camera> mCamera;
		std::shared_ptr<scene::Scene> mCurrentScene;
	};
}
