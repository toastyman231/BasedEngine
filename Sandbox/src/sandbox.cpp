#include "based/engine.h"
#include "based/log.h"
#include "based/main.h"

#include "based/graphics/camera.h"
#include "based/graphics/framebuffer.h"
#include "based/input/keyboard.h"

#include <memory>

#include "based/graphics/defaultassetlibraries.h"

#include <external/glm/gtx/string_cast.hpp>
#include "external/glm/gtc/type_ptr.hpp"

#include "TestEntity.h"
#include "based/core/basedtime.h"
#include "based/core/assetlibrary.h"
#include "based/graphics/sprite.h"
#include "based/input/mouse.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "based/ui/textentity.h"
#include "based/scene/audio.h"
#include "external/entt/entt.hpp"
#include "external/imgui/imgui.h"

using namespace based;

class Sandbox : public based::App
{
private:
	std::shared_ptr<scene::Scene> secondScene;
	ui::TextEntity* text;
	graphics::Sprite* testEnt;
	TestEntity* anotherEntity;
	glm::vec3 camPos;
	glm::vec3 spriteRot;
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
		Engine::Instance().GetWindow().SetShouldRenderToScreen(false);
		camPos = glm::vec3(0.f, 0.f, 1.5f);
		startScene->GetActiveCamera()->SetProjection(based::graphics::PERSPECTIVE);
		startScene->GetActiveCamera()->SetViewMatrix(camPos, 0.f);

		// Setup text
		text = new ui::TextEntity("Assets/fonts/Arimo-Regular.ttf", "This is a test!", 128, 
			glm::vec3(Engine::Instance().GetWindow().GetSize().x / 2, Engine::Instance().GetWindow().GetSize().y / 2, 0.f),
			{ 0, 0, 0, 255 });

		// Create second scene
		secondScene = std::make_shared<scene::Scene>();
		secondScene->SetActiveCamera(GetCurrentScene()->GetActiveCamera());

		// Setup gigachad sprite
		auto texture = std::make_shared<graphics::Texture>("Assets/icon.png");
		auto material = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/test_vert.vert", "Assets/shaders/test_frag.frag"),
			texture);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Test", material);
		testEnt = new graphics::Sprite(graphics::DefaultLibraries::GetVALibrary().Get("TexturedRect"),
			graphics::DefaultLibraries::GetMaterialLibrary().Get("Test"));

		BASED_TRACE("Done initializing");

		// TODO: Fix text rendering behind sprites even when handled last
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);
		//BASED_TRACE("Time:{}, DeltaTime:{}, GetDelta:{}", core::Time::GetTime(), deltaTime, core::Time::DeltaTime());
		//testEnt->SetRotation(glm::vec3(testEnt->GetTransform().Rotation.x + 5.f * deltaTime, testEnt->GetTransform().Rotation.y, testEnt->GetTransform().Rotation.z ));

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
			//if (anotherEntity) anotherEntity->SetActive(!anotherEntity->IsActive());
			//core::Time::SetTimeScale(1.f - core::Time::TimeScale());
			//testEnt->SetSprite(std::make_shared<graphics::Texture>("Assets/tex_test.png"));
			testEnt->SetSortOrder(2);
		}

		if (input::Mouse::ButtonDown(BASED_INPUT_MOUSE_LEFT))
		{
			const auto pos = GetCurrentScene()->GetActiveCamera()->ScreenToWorldPoint(
				static_cast<float>(input::Mouse::X()),
				static_cast<float>(input::Mouse::Y()));
			scene::Entity* square = CreateSquare(pos.x, pos.y);
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_SPACE))
		{
			scene::Entity::EntityForEach<scene::Transform, FallingObject>([&](scene::Entity* ent)
				{
					ent->AddOrReplaceComponent<scene::Velocity>(0.f, ent->GetComponent<FallingObject>().speed);
				});
		}

		TestFall(deltaTime, "Test!");
	}

	scene::Entity* CreateSquare(float x, float y, float scaleX = 0.3f, float scaleY = 0.3f) const
	{
		const auto sprite = scene::Entity::CreateEntity<graphics::Sprite>(
			glm::vec3(x, y, 0.f),
			glm::vec3(0.f), glm::vec3(scaleX, scaleY, 1.f), 
			glm::vec4(0.f, 1.f, 0.f, 1.f));
		sprite->AddComponent<FallingObject>(-0.8f);
		sprite->GetComponent<scene::SpriteRenderer>().sprite->SetSortOrder(1);
		scene::Audio::PlayAudio(std::string("Assets/sounds/TestSound.wav"));

		return sprite;
	}

	void TestFall(float deltaTime, const std::string& str) const
	{
		auto lambda = [deltaTime, str](scene::Entity* ent)
		{
			const auto trans = ent->GetComponent<scene::Transform>();
			const auto dx = ent->GetComponent<scene::Velocity>().dx * deltaTime;
			const auto dy = ent->GetComponent<scene::Velocity>().dy * deltaTime;
			ent->AddOrReplaceComponent<scene::Transform>(
				glm::vec3(trans.Position.x + dx, trans.Position.y + dy, 0.f),
				glm::vec3(0.f), trans.Scale);
			BASED_TRACE(str);
		};
		scene::Entity::EntityForEach<scene::Transform, scene::Velocity>(lambda);
	}

	void Render() override
	{
		App::Render();
	}

	void ImguiRender() override
	{
		if (ImGui::Begin("GameView"))
		{
			if (ImGui::IsWindowHovered())
			{
				ImGui::CaptureMouseFromApp(false);
			}

			auto& window = Engine::Instance().GetWindow();

			ImVec2 winsize = ImGui::GetWindowSize();
			glm::ivec2 arsize = window.GetSizeInAspectRatio(static_cast<int>(winsize.x) - 15,
				static_cast<int>(winsize.y) - 35);
			ImVec2 size = { static_cast<float>(arsize.x), static_cast<float>(arsize.y) };
			ImVec2 pos = {
				(winsize.x - size.x) * 0.5f,
				((winsize.y - size.y) * 0.5f) + 10
			};
			ImVec2 uv0 = { 0, 1 };
			ImVec2 uv1 = { 1, 0 };
			ImGui::SetCursorPos(pos);
			ImGui::Image((void*)static_cast<intptr_t>(window.GetFramebuffer()->GetTextureId()), size, uv0, uv1);
		}
		ImGui::End();

		if (ImGui::Begin("Settings"))
		{
			float fov = startScene->GetActiveCamera()->GetFOV();
			ImGui::DragFloat("FOV", &fov, 0.5f);
			startScene->GetActiveCamera()->SetFOV(fov);

			ImGui::DragFloat3("Camera Pos", glm::value_ptr(camPos), 0.01f);
			startScene->GetActiveCamera()->SetViewMatrix(camPos, 0.f);

			spriteRot = testEnt->GetTransform().Rotation;
			ImGui::DragFloat3("Sprite Rotation", glm::value_ptr(spriteRot), 0.01f);
			testEnt->SetRotation(spriteRot);
		}
		ImGui::End();
	}

	struct FallingObject
	{
		float speed;

		FallingObject() = default;
		FallingObject(float spd) : speed(spd) {}
	};
};

struct Arguments
{
	std::string str;
};

based::App* CreateApp()
{
	return new Sandbox();
}