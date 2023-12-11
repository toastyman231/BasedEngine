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
#include "external/glm/gtc/type_ptr.hpp"
#include "external/imgui/imgui.h"
#include <RmlUi/Core.h>

#include "based/core/basedtime.h"
#include "based/math/random.h"
#include "Models-Surfaces/Generators.h"

using namespace based;

struct ApplicationData {
	bool show_text = true;
	Rml::String animal = "dog";
	float my_value = 0.f;
} my_data;

class MyListener : public Rml::EventListener
{
public:
	MyListener(Rml::String value) : val(std::move(value)) {}
	void ProcessEvent(Rml::Event& event) override
	{
		if (val == "button")
		{
			BASED_TRACE("CLICKED!");
		}

		if (val == "radio")
		{
			BASED_TRACE("RADIO!");
		}
	}
private:
	std::string val;
};

class Sandbox : public based::App
{
private:
	std::shared_ptr<scene::Scene> secondScene;
	scene::Entity* modelEntity;
	scene::Entity* skyEntity;
	scene::Entity* planeEntity;
	scene::Entity* crateEntity;
	scene::Entity* lightPlaceholder;
	scene::Entity* grassInstance;

	bool mouseControl = false;
	float speed = 2.5f;
	float yaw = 0.f;
	float pitch = 0.0f;
	float sensitivity = 100.f;
	float ambientStrength = 0.1f;
	float R = 100.f;
	float heightCoef = 1.f;
	bool useLight = true;
	glm::vec3 camPos = glm::vec3(0.f, 0.f, 1.5f);
	glm::vec3 camRot = glm::vec3(0.f);

	glm::vec3 cubePos;
	glm::vec3 cubeRot;
	glm::vec3 cubeScale;
	glm::vec3 lightCol;
	glm::vec3 lightPosition;
	glm::ivec2 initialPos;

	graphics::Mesh* planeMesh;
	graphics::Mesh* skyboxMesh;
	graphics::Mesh* crateMesh;
	std::shared_ptr<graphics::Texture> crateTex;

	Rml::ElementDocument* document;

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
		Engine::Instance().GetWindow().SetShouldRenderToScreen(false);

		// UI Setup, not related to PS05
		Rml::Context* context = Engine::Instance().GetUiManager().CreateContext("main", 
			Engine::Instance().GetWindow().GetSize());

		if (Rml::DataModelConstructor constructor = context->CreateDataModel("animals"))
		{
			constructor.Bind("show_text", &my_data.show_text);
			constructor.Bind("animal", &my_data.animal);
			constructor.Bind("my_value", &my_data.my_value);
		}

		Engine::Instance().GetUiManager().SetPathPrefix("Assets/ui/");

		document = Engine::Instance().GetUiManager().LoadWindow("my_document", context);
		document->Hide();

		Rml::Element* element = document->GetElementById("world");
		element->SetInnerRML("WORLD");
		element->SetProperty("font-size", "1.5em");

		element = document->GetElementById("button");
		element->AddEventListener("mousedown", new MyListener("button"));
		// NOTE: For radio buttons to automatically uncheck, they must be wrapped in a <form>

		// Old stuff, plus setting camera to perspective mode
		cubePos = glm::vec3(2.7f, 1.f, 1.7f);
		cubeRot = glm::vec3(0.f);
		cubeScale = glm::vec3(1.f);
		startScene->GetActiveCamera()->SetProjection(based::graphics::PERSPECTIVE);

		// TODO: Confirm local transforms work in 2D, scene loading

		// Set up crate object and material
		crateTex = std::make_shared<graphics::Texture>("Assets/crate.png");
		const auto crateMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/basic_lit.vert", "Assets/shaders/basic_lit.frag"));
		crateMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		crateMat->SetUniformValue("material.shininessMat.color", glm::vec4(128.f));
		crateMat->SetUniformValue("material.diffuseMat.useSampler", 1);
		crateMat->AddTexture(crateTex);
		crateMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), crateMat);
		crateEntity = scene::Entity::CreateEntity<scene::Entity>();
		crateEntity->AddComponent<scene::MeshRenderer>(crateMesh);
		crateEntity->SetPosition(glm::vec3(-1.f, 0.f, 0.f));

		lightCol = glm::vec3(1, 1, 1);
		lightPosition = glm::vec3(1, 1.2f, 0.3f);

		// Skybox material setup
		const auto skyboxTex = std::make_shared<graphics::Texture>("Assets/skybox_tex.png", true);
		const auto skybox = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/basic_lit.vert", "Assets/shaders/basic_unlit.frag"));
		skybox->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		skybox->SetUniformValue("material.diffuseMat.useSampler", 1);
		skybox->AddTexture(skyboxTex);
		graphics::DefaultLibraries::GetMaterialLibrary().Load("Sky", skybox);

		// Generate plane mesh and skybox cube
		planeMesh = GeneratePlane(100, 100);
		skyboxMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("AtlasTextureCube"), skybox);

		// Skybox setup
		skyEntity = scene::Entity::CreateEntity<scene::Entity>();
		skyEntity->AddComponent<scene::MeshRenderer>(skyboxMesh);
		skyEntity->SetScale(glm::vec3(500.f));

		// Set up plane material
		planeEntity = scene::Entity::CreateEntity<scene::Entity>();
		planeEntity->SetPosition({-50.f, 0.f, -50.f});
		planeMesh->material = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/ps05/heightmap.vert", "Assets/shaders/basic_lit.frag"));
		planeMesh->material->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		planeMesh->material->SetUniformValue("material.shininessMat.color", glm::vec4(32.f));
		planeMesh->material->SetUniformValue("material.diffuseMat.useSampler", 0);
		const auto heightMap = std::make_shared<graphics::Texture>("Assets/heightmap.png");
		planeMesh->material->AddTexture(heightMap);
		planeEntity->AddComponent<scene::MeshRenderer>(planeMesh);

		// Load grass mesh and set up material
		const auto grassMesh = graphics::Model::LoadSingleMesh("Assets/Models/grass_highPoly.obj");//GenerateGrassBlade(glm::vec3(0.1f, 1.f, 1.f));
		const auto grassMatBase = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/ps05/grass.vert", "Assets/shaders/ps05/grass.frag"));
		grassMesh->material = grassMatBase;
		grassMesh->material->SetUniformValue("material.shininessMat.color", glm::vec4(12.f));
		grassMesh->material->SetUniformValue("material.diffuseMat.useSampler", 0);
		grassMatBase->AddTexture(heightMap);

		// Set up grass instancing
		auto grassInstanceMesh = new graphics::InstancedMesh(grassMesh->vertices, grassMesh->indices, grassMesh->textures);
		grassInstanceMesh->material = grassMatBase;
		grassInstance = scene::Entity::CreateEntity<scene::Entity>();
		grassInstance->AddComponent<scene::MeshRenderer>(grassInstanceMesh);

		// Instance a bunch of grass blades in an offset grid
		constexpr int GRASS_BLADES = 100000;
		const float GRASS_X = based::math::Sqrt(GRASS_BLADES);

		for (int i = 0; i < GRASS_X; i++)
		{
			const float x = (static_cast<float>(i) / GRASS_X) - 0.5f;
			for (int j = 0; j < GRASS_X; j++)
			{
				const float y = (static_cast<float>(j) / GRASS_X) - 0.5f;
				glm::vec3 pos = { x * 20 + based::math::RandomRange(-0.2f, 0.2f), 0,
					y * 20 + based::math::RandomRange(-0.2f, 0.2f) };
				glm::vec3 rot = { 0, based::math::RandomRange(0, 90), 0 };
				grassInstanceMesh->AddInstance(scene::Transform(pos, rot, glm::vec3(1)));
			}
		}

		// Set up light placeholder
		const auto cubeMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/basic_lit.vert", "Assets/shaders/basic_unlit.frag"));
		cubeMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		cubeMat->SetUniformValue("material.diffuseMat.useSampler", 0);
		auto cubeMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), cubeMat);
		lightPlaceholder = scene::Entity::CreateEntity<scene::Entity>();
		lightPlaceholder->AddComponent<scene::MeshRenderer>(cubeMesh);
		lightPlaceholder->SetPosition(lightPosition);
		lightPlaceholder->SetScale(glm::vec3(0.1f));

		GetCurrentScene()->GetActiveCamera()->SetPosition(glm::vec3(-1, 1, 4));
		GetCurrentScene()->GetActiveCamera()->SetRotation(glm::vec3(6, 53, 0));

		BASED_TRACE("Done initializing");

		// TODO: Fix text rendering behind sprites even when handled last
		// TODO: Optimize UI to not regenerate VAs every single frame

		// TODO: Implement proper lighting using upgraded materials
	}

	void Shutdown() override
	{
		App::Shutdown();
	}

	void Update(float deltaTime) override
	{
		App::Update(deltaTime);

		// Movement input
		if (input::Keyboard::Key(BASED_INPUT_KEY_W))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position - speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_S))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position + speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_A))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position - speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetRight());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_D))
		{
			scene::Transform transform = GetCurrentScene()->GetActiveCamera()->GetTransform();
			GetCurrentScene()->GetActiveCamera()->SetPosition(transform.Position + speed * deltaTime * GetCurrentScene()->GetActiveCamera()->GetRight());
		}

		// Enable/disable mouse control
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_P))
		{
			mouseControl = !mouseControl;
			dynamic_cast<graphics::InstancedMesh*>(grassInstance->GetComponent<scene::MeshRenderer>().mesh)->ClearInstances();
		}

		// Mouse input
		if (mouseControl)
		{
			pitch += static_cast<float>(input::Mouse::DX()) * sensitivity * deltaTime;
			yaw += static_cast<float>(input::Mouse::DY()) * sensitivity * deltaTime;

			yaw = based::math::Clamp(yaw, -89.f, 89.f);

			GetCurrentScene()->GetActiveCamera()->SetRotation(glm::vec3(yaw, pitch, GetCurrentScene()->GetActiveCamera()->GetTransform().Rotation.z));
		}

		if (input::Mouse::ButtonDown(BASED_INPUT_MOUSE_LEFT))
		{
			initialPos = input::Mouse::GetMousePosition();
		}

		if (input::Mouse::Button(BASED_INPUT_MOUSE_LEFT))
		{
			// Rolling ball algorithm
			glm::ivec2 current = input::Mouse::GetMousePosition();

			float dx = static_cast<float>(current.x) - static_cast<float>(initialPos.x);
			float dy = static_cast<float>(current.y) - static_cast<float>(initialPos.y);

			float dr = glm::length(glm::vec2(dx, dy));
			glm::vec3 n = glm::vec3(-dy / dr, dx / dr, 0.f);
			float theta = dr / R;

			glm::vec3 rot = AngleAxisToEuler(glm::normalize(n), theta);
			if (isnan(rot.x) || isnan(rot.y) || isnan(rot.z)) return;

			cubeRot += rot;
		}

		// Show/Hide UI (not related to ps05)
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_B))
		{
			if (document->IsVisible()) document->Hide();
			else document->Show();
		}

		// Set light position and pass info to shaders
		//lightPosition = glm::vec3(based::math::Sin(based::core::Time::GetTime()) * 4, 
		//	lightPosition.y, lightPosition.z);

		lightPlaceholder->SetPosition(lightPosition);

		crateEntity->SetTransform(cubePos, cubeRot, cubeScale);

		planeMesh->material->SetUniformValue("lightColor", lightCol);
		crateMesh->material->SetUniformValue("lightColor", lightCol);

		planeMesh->material->SetUniformValue("lightPos", lightPosition);
		crateMesh->material->SetUniformValue("lightPos", lightPosition);

		planeMesh->material->SetUniformValue("ambientStrength", ambientStrength);
		crateMesh->material->SetUniformValue("ambientStrength", ambientStrength);

		planeMesh->material->SetUniformValue("heightCoef", heightCoef);


		if (!useLight)
		{
			planeMesh->material->SetUniformValue("ambientStrength", 1.f);
			crateMesh->material->SetUniformValue("ambientStrength", 1.f);
		}

		UpdateShaders(grassInstance->GetComponent<scene::MeshRenderer>().mesh, 
			lightCol, lightPosition, useLight, ambientStrength, heightCoef);
	}

	void Render() override
	{
	}

	void ImguiRender() override
	{
		if (Engine::Instance().GetWindow().GetShouldRenderToScreen()) return;

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
			startScene->GetActiveCamera()->SetPosition(pos);

			glm::vec3 rot = startScene->GetActiveCamera()->GetTransform().Rotation;
			ImGui::DragFloat3("Camera Rot", glm::value_ptr(rot), 0.01f);
			if (!mouseControl) startScene->GetActiveCamera()->SetRotation(rot);

			ImGui::DragFloat3("Cube Position", glm::value_ptr(cubePos), 0.01f);
			ImGui::DragFloat3("Cube Rotation", glm::value_ptr(cubeRot), 0.01f);
			ImGui::DragFloat3("Cube Scale", glm::value_ptr(cubeScale), 0.01f);

			ImGui::DragFloat3("Light Color", glm::value_ptr(lightCol), 0.01f);
			ImGui::DragFloat3("Light Position", glm::value_ptr(lightPosition), 0.01f);

			ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f);
			ImGui::Checkbox("Use Light", &useLight);

			ImGui::DragFloat("Rolling Ball Scale", &R, 0.01f);
			ImGui::DragFloat("Height Coefficient", &heightCoef, 0.01f);
		}
		ImGui::End();
	}

	static glm::vec3 AngleAxisToEuler(glm::vec3 axis, float angle)
	{
		const float s = based::math::Sin(angle);
		const float c = based::math::Cos(angle);
		const float t = 1 - c;

		float heading;
		float attitude;
		float bank;

		if ((axis.x * axis.y * t + axis.z * s) > 0.998)
		{
			heading = 2.f * based::math::Atan2(axis.x * based::math::Sin(angle / 2), based::math::Cos(angle / 2));
			attitude = based::math::PI / 2.f;
			bank = 0;
			return {attitude, heading, bank};
		}

		if ((axis.x * axis.y * t + axis.z * s) < -0.998)
		{
			heading = -2.f * based::math::Atan2(axis.x * based::math::Sin(angle / 2), based::math::Cos(angle / 2));
			attitude = -based::math::PI / 2.f;
			bank = 0;
			return {attitude, heading, bank};
		}

		heading = based::math::Atan2(axis.y * s - axis.x * axis.z * t, 1 - (axis.y * axis.y + axis.z * axis.z) * t);
		attitude = based::math::Asin(axis.x * axis.y * t + axis.z * s);
		bank = based::math::Atan2(axis.x * s - axis.y * axis.z * t, 1 - (axis.x * axis.x + axis.z * axis.z) * t);
		return {attitude, heading, bank};
	}

	void UpdateShaders(graphics::Mesh* mesh, 
		glm::vec3 lightColor, glm::vec3 lightPosition, bool useLight, float ambientStrength, float height)
	{
		mesh->material->SetUniformValue("lightColor", lightColor);

		mesh->material->SetUniformValue("lightPos", lightPosition);

		mesh->material->SetUniformValue("ambientStrength", ambientStrength);

		mesh->material->SetUniformValue("heightCoef", height);

		if (!useLight)
		{
			mesh->material->SetUniformValue("ambientStrength", 1.f);
		}
	}
};

based::App* CreateApp()
{
	return new Sandbox();
}