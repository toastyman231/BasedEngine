#include "based/engine.h"
#include "based/log.h"
#include "based/main.h"

#include "based/graphics/camera.h"
#include "based/graphics/framebuffer.h"
#include "based/input/keyboard.h"
#include "based/ui/textentity.h"

#include <memory>

#include "based/graphics/defaultassetlibraries.h"

#include <external/glm/gtx/string_cast.hpp>
#include "based/core/assetlibrary.h"
#include "based/input/mouse.h"
#include "external/entt/entt.hpp"
#include "external/glm/ext/matrix_transform.hpp"
#include "external/imgui/imgui.h"
#include "based/scene/components.h"

using namespace based;

class Sandbox : public based::App
{
private:
	std::shared_ptr<scene::Scene> secondScene;
	ui::TextEntity text;
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
		text = ui::TextEntity("Test");
		//ui::TextEntity::TestRender();
		secondScene = std::make_shared<scene::Scene>();
		secondScene->SetActiveCamera(GetCurrentScene()->GetActiveCamera());
		BASED_TRACE("Created entity in second scene");
		const auto entity = secondScene->GetRegistry().create();
		secondScene->GetRegistry().emplace<scene::Transform>(entity, glm::vec3(0.f));
		secondScene->GetRegistry().emplace<scene::SpriteRenderer>(entity,
			graphics::DefaultLibraries::GetVALibrary().Get("Rect"),
			graphics::DefaultLibraries::GetMaterialLibrary().Get("RectGreen"));
		BASED_TRACE("Done initializing");
		//text = ui::TextEntity("This is a test!");
		//text.SetText("This is a test!");
		//text.RenderText(glm::vec3(0.f), glm::vec4(1.f), 1.f);
	}

	void Shutdown() override
	{
		//text.DeleteText();
	}

	void Update() override
	{
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_G))
		{
			LoadScene(secondScene);
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_L))
		{
			LoadScene(startScene);
		}

		if (input::Mouse::ButtonDown(BASED_INPUT_MOUSE_LEFT))
		{
			const auto pos = GetCurrentScene()->GetActiveCamera()->ScreenToWorldPoint(
				static_cast<float>(input::Mouse::X()),
				static_cast<float>(input::Mouse::Y()));
			CreateSquare(pos.x, pos.y);
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_SPACE))
		{
			const auto view = GetCurrentScene()->GetRegistry().view<scene::Transform>();
			
			for (const auto entity : view)
			{
				// TODO: figure out why registry.has doesn't exist
				GetCurrentScene()->GetRegistry().emplace_or_replace<scene::Velocity>(entity, 0.f, -0.002f);
			}
		}

		const auto view = GetCurrentScene()->GetRegistry().view<scene::Transform, scene::Velocity>();

		for (const auto entity : view)
		{
			const scene::Transform& trans = GetCurrentScene()->GetRegistry().get<scene::Transform>(entity);
			const float dx = GetCurrentScene()->GetRegistry().get<scene::Velocity>(entity).dx;
			const float dy = GetCurrentScene()->GetRegistry().get<scene::Velocity>(entity).dy;
			GetCurrentScene()->GetRegistry().replace<scene::Transform>(entity, 
				glm::vec3(trans.Position.x + dx, trans.Position.y + dy, 0.f),
				glm::vec3(0.f), trans.Scale);
		}
	}

	entt::entity CreateSquare(float x, float y, float scaleX = 0.3f, float scaleY = 0.3f)
	{
		const auto entity = GetCurrentScene()->GetRegistry().create();

		GetCurrentScene()->GetRegistry().emplace<scene::Transform>(entity,
			glm::vec3( x, y, 0.f ), glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( scaleX, scaleY, 0.f ));
		BASED_TRACE("Entity created at ({}, {})", x, y);
		GetCurrentScene()->GetRegistry().emplace<scene::SpriteRenderer>(entity,
			graphics::DefaultLibraries::GetVALibrary().Get("Rect"),
			graphics::DefaultLibraries::GetMaterialLibrary().Get("RectGreen"));

		return entity;
	}

	void Render() override
	{
		//BASED_TRACE("Rendering");
		text.RenderText("This is a test!", 640.f, 360.f, glm::vec3(1.f), 1.f);
	}
};

based::App* CreateApp()
{
	return new Sandbox();
}