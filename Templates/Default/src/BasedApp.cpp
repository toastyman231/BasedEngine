#include "based/log.h"
#include "based/main.h"

#include <memory>

#include "based/graphics/defaultassetlibraries.h"

#include "based/core/assetlibrary.h"

#include "based/scene/components.h"
#include "based/input/mouse.h"
#include "based/scene/entity.h"
#include "based/graphics/sprite.h"

using namespace based;

class BasedApp : public based::App
{
private:
	std::shared_ptr<graphics::Sprite> iconSprite;
public:
	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.w = 1280;
		props.h = 720;
		props.imguiProps.IsDockingEnabled = true;

		return props;
	}

	based::GameSettings GetGameSettings() override
	{
		based::GameSettings settings;
		settings.gameMemory = 1;

		return settings;
	}

	void Initialize() override
	{
		App::Initialize();
		Engine::Instance().GetWindow().SetShouldRenderToScreen(false);
		input::Mouse::SetCursorVisible(!Engine::Instance().GetWindow().GetShouldRenderToScreen());
		input::Mouse::SetCursorMode(Engine::Instance().GetWindow().GetShouldRenderToScreen() ?
			input::CursorMode::Confined : input::CursorMode::Free);

		auto texture = std::make_shared<graphics::Texture>("Assets/icon.png");
		auto material = graphics::Material::CreateMaterial(
			graphics::DefaultLibraries::GetShaderLibrary().Get("TexturedRect"),
			DEFAULT_MAT_LIB, "Icon");
		auto iconSprite = scene::Entity::CreateEntity<graphics::Sprite>("Icon",
			{ 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f },
			{ 1.f, 1.f, 1.f },
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect"), material);
		iconSprite->AddComponent<scene::SpriteRenderer>(iconSprite);
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);
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