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
#include "based/graphics/mesh.h"
#include "based/graphics/sprite.h"
#include "based/input/mouse.h"
#include "based/scene/components.h"
#include "based/math/basedmath.h"
#include "based/scene/entity.h"
#include "based/ui/textentity.h"
#include "based/scene/audio.h"
#include "based/graphics/model.h"
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
	scene::Entity* modelEntity;

	bool mouseControl = false;
	float speed = 2.5f;
	float yaw = 0.f;
	float pitch = 0.0f;
	float sensitivity = 100.f;
	glm::vec3 camPos = glm::vec3(0.f, 0.f, 1.5f);
	glm::vec3 camRot = glm::vec3(0.f);

	glm::vec3 cubePos;
	glm::vec3 cubeRot;
	glm::vec3 cubeScale;

	graphics::Mesh* crateMesh;
	graphics::Mesh* skyboxMesh;
	graphics::Model* testModel;
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
		// TODO: Figure out how to capture the mouse properly
		//input::Mouse::SetCursorLocked(true);
		//SDL_SetRelativeMouseMode(SDL_TRUE);
		//SDL_SetWindowGrab(Engine::Instance().GetWindow().GetSDLWindow(), SDL_TRUE);
		//SDL_CaptureMouse(SDL_TRUE);
		Engine::Instance().GetWindow().SetShouldRenderToScreen(false);

		cubePos = glm::vec3(0.f);
		cubeRot = glm::vec3(0.f);
		cubeScale = glm::vec3(1.f);
		startScene->GetActiveCamera()->SetProjection(based::graphics::PERSPECTIVE);

		// Setup text
		text = new ui::TextEntity("Assets/fonts/Arimo-Regular.ttf", "This is a test!", 128, 
			glm::vec3(Engine::Instance().GetWindow().GetSize().x / 2, Engine::Instance().GetWindow().GetSize().y / 2, 0.f),
			{ 0, 0, 0, 255 });
		text->SetActive(false);

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
		testEnt->SetActive(false);

		auto crateTex = std::make_shared<graphics::Texture>("Assets/crate.png");
		auto crateMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/test_vert.vert", "Assets/shaders/test_frag.frag"),
			crateTex);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Crate", crateMat);

		auto skyboxTex = std::make_shared<graphics::Texture>("Assets/skybox_tex.png");
		auto skybox = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/test_vert.vert", "Assets/shaders/test_frag.frag"),
			skyboxTex);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Sky", skybox);

		crateMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"));
		skyboxMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("AtlasTextureCube"));

		auto modelMat = std::make_shared<graphics::Material>(graphics::DefaultLibraries::GetShaderLibrary().Get("Model"), 
			std::make_shared<graphics::Texture>(""));
		modelMat->SetUniformValue(std::string("col"), glm::vec4{ 1.f, 1.f, 1.f, 1.f });
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Model", modelMat);

		testModel = new graphics::Model("Assets/Models/rotate_cylinder.obj");

		modelEntity = new scene::Entity();
		modelEntity->AddComponent<scene::ModelRenderer>(testModel);
		modelEntity->SetPosition({ 2, 0, 0 });

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

		if (input::Keyboard::Key(BASED_INPUT_KEY_W))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position + speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_S))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position - speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_A))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position + speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetRight());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_D))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position - speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetRight());
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_P))
		{
			mouseControl = !mouseControl;
		}

		if (mouseControl)
		{
			pitch += static_cast<float>(-input::Mouse::DX()) * sensitivity * deltaTime;
			yaw += static_cast<float>(input::Mouse::DY()) * sensitivity * deltaTime;

			yaw = based::math::Clamp(yaw, -89.f, 89.f);

			GetCurrentScene()->GetActiveCamera()->SetRotation(glm::vec3(yaw, pitch, GetCurrentScene()->GetActiveCamera()->GetTransform().Rotation.z));
		}

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

		/*if (input::Mouse::ButtonDown(BASED_INPUT_MOUSE_LEFT))
		{
			const auto pos = GetCurrentScene()->GetActiveCamera()->ScreenToWorldPoint(
				static_cast<float>(input::Mouse::X()),
				static_cast<float>(input::Mouse::Y()));
			scene::Entity* square = CreateSquare(pos.x, pos.y, pos.z);
		}*/

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_SPACE))
		{
			scene::Entity::EntityForEach<scene::Transform, FallingObject>([&](scene::Entity* ent)
				{
					ent->AddOrReplaceComponent<scene::Velocity>(0.f, ent->GetComponent<FallingObject>().speed);
				});
		}

		TestFall(deltaTime, "Test!");

		modelEntity->SetTransform(glm::vec3(cubePos.x + 2, cubePos.y, cubePos.z), cubeRot, cubeScale);
	}

	scene::Entity* CreateSquare(float x, float y, float z, float scaleX = 0.3f, float scaleY = 0.3f) const
	{
		const auto sprite = scene::Entity::CreateEntity<graphics::Sprite>(
			glm::vec3(x, y, z),
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
		// TODO: Make meshes render automatically
		crateMesh->Draw(cubePos, cubeRot, cubeScale, graphics::DefaultLibraries::GetMaterialLibrary().Get("Crate"));
		skyboxMesh->Draw({ 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 500.f, 500.f, 500.f }, graphics::DefaultLibraries::GetMaterialLibrary().Get("Sky"));
	}

	void ImguiRender() override
	{
		//return;

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

			ImGui::DragFloat("Sensitivity", &sensitivity, 0.5f);

			float nearPlane = startScene->GetActiveCamera()->GetNear();
			ImGui::DragFloat("Near", &nearPlane, 0.5f);
			startScene->GetActiveCamera()->SetNear(nearPlane);

			float farPlane = startScene->GetActiveCamera()->GetFar();
			ImGui::DragFloat("Far", &farPlane, 0.5f);
			startScene->GetActiveCamera()->SetFar(farPlane);

			glm::vec3 pos = startScene->GetActiveCamera()->GetTransform().Position;
			ImGui::DragFloat3("Camera Pos", glm::value_ptr(pos), 0.01f);
			//startScene->GetActiveCamera()->SetPosition(camPos);//SetViewMatrix(camPos, 0.f);

			glm::vec3 rot = startScene->GetActiveCamera()->GetTransform().Rotation;
			ImGui::DragFloat3("Camera Rot", glm::value_ptr(rot), 0.01f);
			if (!mouseControl) startScene->GetActiveCamera()->SetRotation(rot);

			ImGui::DragFloat3("Cube Position", glm::value_ptr(cubePos), 0.01f);
			ImGui::DragFloat3("Cube Rotation", glm::value_ptr(cubeRot), 0.01f);
			ImGui::DragFloat3("Cube Scale", glm::value_ptr(cubeScale), 0.01f);
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