#include "based/log.h"
#include "based/main.h"

#include <memory>

#include "based/graphics/defaultassetlibraries.h"

#include "based/core/assetlibrary.h"

#include "based/scene/components.h"
#include "based/scene/entity.h"

using namespace based;

class BasedApp : public based::App
{
public:
	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.w = 1280;
		props.h = 720;
		props.imguiProps.IsDockingEnabled = true;

		return props;
	}

	void Initialize() override
	{
		App::Initialize();

		auto texture = std::make_shared<graphics::Texture>("Assets/icon.png");
		auto material = std::make_shared<graphics::Material>(graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect"),
			texture);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("BasedIcon", material);
		scene::Entity* testEnt = scene::Entity::CreateEntity();
		testEnt->AddComponent<scene::SpriteRenderer>(graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect"),
			graphics::DefaultLibraries::GetMaterialLibrary().Get("BasedIcon"));
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update() override
	{
		App::Update();
	}

	void Render() override
	{
		App::Render();
	}
};

based::App* CreateApp()
{
	return new BasedApp();
}