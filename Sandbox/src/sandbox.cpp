#include "based/engine.h"
#include "based/log.h"
#include "based/main.h"

#include "based/graphics/camera.h"
#include "based/graphics/framebuffer.h"
#include "based/input/keyboard.h"

#include <memory>

#include "based/graphics/defaultassetlibraries.h"

#include <external/glm/gtx/string_cast.hpp>

#include "TestEntity.h"
#include "based/core/assetlibrary.h"
#include "based/input/mouse.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "based/ui/textentity.h"
#include "external/entt/entt.hpp"
#include "external/imgui/imgui.h"

using namespace based;

class Sandbox : public based::App
{
private:
	std::shared_ptr<scene::Scene> secondScene;
	ui::TextEntity* text;
	scene::Entity* testEnt;
	TestEntity* anotherEntity;
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
		text = new ui::TextEntity("Assets/fonts/Arimo-Regular.ttf", "This is a test!", 128, 
			glm::vec3(Engine::Instance().GetWindow().GetSize().x / 2, Engine::Instance().GetWindow().GetSize().y / 2, 0.f),
			{ 0, 0, 0, 255 });

		secondScene = std::make_shared<scene::Scene>();
		secondScene->SetActiveCamera(GetCurrentScene()->GetActiveCamera());
		BASED_TRACE("Created entity in second scene");
		const auto entity = secondScene->GetRegistry().create();
		secondScene->GetRegistry().emplace<scene::Transform>(entity, glm::vec3(0.f));
		secondScene->GetRegistry().emplace<scene::SpriteRenderer>(entity,
			graphics::DefaultLibraries::GetVALibrary().Get("Rect"),
			graphics::DefaultLibraries::GetMaterialLibrary().Get("RectGreen"));
		auto texture = std::make_shared<graphics::Texture>("Assets/icon.png");
		auto material = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/test_vert.vert", "Assets/shaders/test_frag.frag"),
			texture);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Test", material);
		testEnt = scene::Entity::CreateEntity();
		testEnt->AddComponent<scene::SpriteRenderer>(graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect"), 
			graphics::DefaultLibraries::GetMaterialLibrary().Get("Test"));

		BASED_TRACE("Done initializing");

		// TODO: Add sprite class for easy sprite creation, and sort order for layering sprites

	}

	void Shutdown() override
	{
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

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_B))
		{
			//text->SetAlignment(ui::TopLeft); //This works but causes an unnecessary warning
			//text->MoveText({ 0, 0, 0 }); This works
			//text->SetColor({ 255, 0, 0, 255 }); //This works but causes the same warning and looks kinda weird
			//text->SetText("New text!"); //This works but causes the same warning
			//text->SetSize(16); // This works but causes the same warning
			testEnt->SetActive(!testEnt->IsActive());
			scene::Entity::DestroyEntity(anotherEntity);
			anotherEntity = new TestEntity();
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_H))
		{
			//text->SetActive(!text->IsActive());
			if (anotherEntity) anotherEntity->SetActive(!anotherEntity->IsActive());
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
		GetCurrentScene()->GetRegistry().emplace<scene::Enabled>(entity);

		return entity;
	}

	void Render() override
	{
	}
};

based::App* CreateApp()
{
	return new Sandbox();
}