#include <memory>
#include <external/glm/gtx/string_cast.hpp>

#include "based/engine.h"
#include "based/log.h"
#include "based/main.h"
#include "based/core/assetlibrary.h"
#include "based/graphics/camera.h"
#include "based/graphics/defaultassetlibraries.h"
#include "based/graphics/framebuffer.h"
#include "based/graphics/mesh.h"
#include "based/graphics/model.h"
#include "based/input/keyboard.h"
#include "based/input/mouse.h"
#include "based/math/basedmath.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "based/ui/linearbox.h"
#include "external/glm/gtc/type_ptr.hpp"
#include "external/imgui/imgui.h"
#include "based/ui/uielement.h"

using namespace based;

class Sandbox : public based::App
{
private:
	std::shared_ptr<scene::Scene> secondScene;
	scene::Entity* modelEntity;
	scene::Entity* skyEntity;
	scene::Entity* crateEntity;

	bool mouseControl = false;
	bool useTexture = false;
	float speed = 2.5f;
	float yaw = 0.f;
	float pitch = 0.0f;
	float sensitivity = 100.f;
	float padding = 0.f;
	glm::vec3 camPos = glm::vec3(0.f, 0.f, 1.5f);
	glm::vec3 camRot = glm::vec3(0.f);

	glm::vec3 cubePos;
	glm::vec3 cubeRot;
	glm::vec3 cubeScale;
	glm::vec3 uiScale = glm::vec3{ 100.f, 100.f, 0.f };
	glm::vec3 uiPos;
	glm::vec2 alignment;
	glm::vec2 anchor;
	glm::mat4 orthoMatrix;

	graphics::Mesh* crateMesh;
	graphics::Mesh* skyboxMesh;
	graphics::Model* testModel;
	std::shared_ptr<graphics::Texture> crateTex;
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
		uiPos = glm::vec3(0.f);
		uiScale = glm::vec3(100.f);
		cubeScale = glm::vec3(1.f);
		startScene->GetActiveCamera()->SetProjection(based::graphics::PERSPECTIVE);

		// TODO: Confirm local transforms work in 2D, scene loading, better UI/Text

		crateTex = std::make_shared<graphics::Texture>("Assets/crate.png");
		auto crateMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/test_vert.vert", "Assets/shaders/test_frag.frag"),
			crateTex);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Crate", crateMat);

		auto ui = ui::UiElement::CreateUiElement<ui::LinearBox>(0, 0, 100, 100, ui::LinearBox::VERTICAL);//new ui::LinearBox(0, 0, 100, 100, ui::LinearBox::HORIZONTAL);
		uiScale = ui->GetTransform()->GetSize();
		ui::UiElement::ShowElement(ui);

		auto skyboxTex = std::make_shared<graphics::Texture>("Assets/skybox_tex.png", true);
		auto skybox = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/test_vert.vert", "Assets/shaders/test_frag.frag"),
			skyboxTex);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Sky", skybox);

		crateMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), crateMat);
		skyboxMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("AtlasTextureCube"), skybox);

		skyEntity = scene::Entity::CreateEntity<scene::Entity>();
		skyEntity->AddComponent<scene::MeshRenderer>(skyboxMesh);
		skyEntity->SetScale(glm::vec3(500.f));
		crateEntity = scene::Entity::CreateEntity<scene::Entity>();
		crateEntity->AddComponent<scene::MeshRenderer>(crateMesh);

		modelEntity = graphics::Model::CreateModelEntity("Assets/Models/rotate_cylinder.obj");
		modelEntity->SetPosition({ 2, 0, 0 });

		auto axe = graphics::Model::CreateModelEntity("Assets/Models/axe.obj");
		axe->SetPosition({ -2, 0, 0 });	

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
			pitch += static_cast<float>(input::Mouse::DX()) * sensitivity * deltaTime;
			yaw += static_cast<float>(input::Mouse::DY()) * sensitivity * deltaTime;

			yaw = based::math::Clamp(yaw, -89.f, 89.f);

			GetCurrentScene()->GetActiveCamera()->SetRotation(glm::vec3(yaw, pitch, GetCurrentScene()->GetActiveCamera()->GetTransform().Rotation.z));
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_G))
		{
			//LoadScene(secondScene);
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_L))
		{
			//LoadScene(startScene);
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_B))
		{
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_H))
		{
		}

		crateEntity->SetTransform(cubePos, cubeRot, cubeScale);
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->x = uiPos.x;
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->y = uiPos.y;
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->width = uiScale.x;
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->height = uiScale.y;
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->alignment = alignment;
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->anchorPoint = anchor;
		based::ui::UiElement::GetAllUiElements()[0]->GetTransform()->SetPadding(padding);

		/*if (useTexture) dynamic_cast<ui::Image*>(ui::UiElement::GetAllUiElements()[0])->SetTexture(crateTex);
		else dynamic_cast<ui::Image*>(ui::UiElement::GetAllUiElements()[0])->SetTexture(nullptr);*/
	}

	void Render() override
	{
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

			glm::vec3 uiPosition = uiPos;
			ImGui::DragFloat2("UI Position", glm::value_ptr(uiPosition), 0.5f);
			uiPos = uiPosition;

			glm::vec3 uiSize = uiScale;
			ImGui::DragFloat2("UI Scale", glm::value_ptr(uiSize), 0.5f);
			uiScale = uiSize;

			ImGui::DragFloat("UI Padding", &padding, 0.5f);

			glm::vec2 align = alignment;
			ImGui::DragFloat2("UI Alignment", glm::value_ptr(align), 0.1f);
			alignment = align;

			glm::vec2 anchorPoint = anchor;
			ImGui::DragFloat2("UI Anchor", glm::value_ptr(anchorPoint), 0.1f);
			anchor = anchorPoint;

			ImGui::Checkbox("Use Texture", &useTexture);

			ImGui::DragFloat3("Cube Position", glm::value_ptr(cubePos), 0.01f);
			ImGui::DragFloat3("Cube Rotation", glm::value_ptr(cubeRot), 0.01f);
			ImGui::DragFloat3("Cube Scale", glm::value_ptr(cubeScale), 0.01f);

			glm::vec3 rootPos = modelEntity->GetComponent<scene::Transform>().Position;
			bool changed = ImGui::DragFloat3("Root Entity Pos", glm::value_ptr(rootPos));
			if (changed) modelEntity->SetPosition(rootPos);
			glm::vec3 rootLocalPos = modelEntity->GetComponent<scene::Transform>().LocalPosition;
			changed = ImGui::DragFloat3("Root Entity Local Pos", glm::value_ptr(rootLocalPos));
			if (changed) modelEntity->SetLocalPosition(rootLocalPos);

			glm::vec3 cylinderPos = modelEntity->Children[0]->GetComponent<scene::Transform>().Position;
			changed = ImGui::DragFloat3("Cylinder Pos", glm::value_ptr(cylinderPos));
			if (changed) modelEntity->Children[0]->SetPosition(cylinderPos);
			glm::vec3 cylinderLocalPos = modelEntity->Children[0]->GetComponent<scene::Transform>().LocalPosition;
			changed = ImGui::DragFloat3("Cylinder Local Pos", glm::value_ptr(cylinderLocalPos));
			if (changed) modelEntity->Children[0]->SetLocalPosition(cylinderLocalPos);

			glm::vec3 rootRot = modelEntity->GetComponent<scene::Transform>().Rotation;
			changed = ImGui::DragFloat3("Root Entity Rot", glm::value_ptr(rootRot));
			if (changed) modelEntity->SetRotation(rootRot);
			glm::vec3 rootLocalRot = modelEntity->GetComponent<scene::Transform>().LocalRotation;
			changed = ImGui::DragFloat3("Root Entity Local Rot", glm::value_ptr(rootLocalRot));
			if (changed) modelEntity->SetLocalRotation(rootLocalRot);

			glm::vec3 cylinderRot = modelEntity->Children[0]->GetComponent<scene::Transform>().Rotation;
			changed = ImGui::DragFloat3("Cylinder Rot", glm::value_ptr(cylinderRot));
			if (changed) modelEntity->Children[0]->SetRotation(cylinderRot);
			glm::vec3 cylinderLocalRot = modelEntity->Children[0]->GetComponent<scene::Transform>().LocalRotation;
			changed = ImGui::DragFloat3("Cylinder Local Rot", glm::value_ptr(cylinderLocalRot));
			if (changed) modelEntity->Children[0]->SetLocalRotation(cylinderLocalRot);

			glm::vec3 rootScale = modelEntity->GetComponent<scene::Transform>().Scale;
			changed = ImGui::DragFloat3("Root Entity Scale", glm::value_ptr(rootScale));
			if (changed) modelEntity->SetScale(rootScale);
			glm::vec3 rootLocalScale = modelEntity->GetComponent<scene::Transform>().LocalScale;
			changed = ImGui::DragFloat3("Root Entity Local Scale", glm::value_ptr(rootLocalScale));
			if (changed) modelEntity->SetLocalScale(rootLocalScale);

			glm::vec3 cylinderScale = modelEntity->Children[0]->GetComponent<scene::Transform>().Scale;
			changed = ImGui::DragFloat3("Cylinder Scale", glm::value_ptr(cylinderScale));
			if (changed) modelEntity->Children[0]->SetScale(cylinderScale);
			glm::vec3 cylinderLocalScale = modelEntity->Children[0]->GetComponent<scene::Transform>().LocalScale;
			changed = ImGui::DragFloat3("Cylinder Local Scale", glm::value_ptr(cylinderLocalScale));
			if (changed) modelEntity->Children[0]->SetLocalScale(cylinderLocalScale);
		}
		ImGui::End();
	}
};

based::App* CreateApp()
{
	return new Sandbox();
}