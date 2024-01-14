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
	scene::Entity* sunLight;

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

	glm::vec3 cubeRot;
	glm::vec3 lightPosition;
	glm::ivec2 initialPos;
	glm::vec3 sunDirection;

	graphics::Mesh* planeMesh;
	graphics::Mesh* skyboxMesh;
	graphics::Mesh* crateMesh;
	graphics::Mesh* boxMesh;
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

		// UI Setup
		Rml::Context* context = Engine::Instance().GetUiManager().CreateContext("main", 
			Engine::Instance().GetWindow().GetSize());

		if (Rml::DataModelConstructor constructor = context->CreateDataModel("animals"))
		{
			constructor.Bind("show_text", &my_data.show_text);
			constructor.Bind("animal", &my_data.animal);
			constructor.Bind("my_value", &my_data.my_value);
		}

		// Load UI
		Engine::Instance().GetUiManager().SetPathPrefix("Assets/ui/");

		document = Engine::Instance().GetUiManager().LoadWindow("help_screen", context);

		// Old stuff, plus setting camera to perspective mode
		cubeRot = glm::vec3(0.f);
		sunDirection = glm::vec3(60.f, -60.f, 0.f);
		startScene->GetActiveCamera()->SetProjection(based::graphics::PERSPECTIVE);

		// TODO: Confirm local transforms work in 2D, scene loading
		
		// Set up crate object and material
		crateTex = std::make_shared<graphics::Texture>("Assets/crate.png");
		const auto crateMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/basic_lit.vert", "Assets/shaders/basic_lit.frag"));
		crateMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		crateMat->SetUniformValue("material.shininessMat.color", glm::vec4(128.f));
		crateMat->SetUniformValue("material.diffuseMat.useSampler", 1);
		crateMat->AddTexture(crateTex, "material.diffuseMat.tex");
		crateMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), crateMat);
		crateEntity = scene::Entity::CreateEntity<scene::Entity>();
		crateEntity->AddComponent<scene::MeshRenderer>(crateMesh);
		crateEntity->SetPosition(glm::vec3(2.7f, 1.f, 1.7f));
		crateEntity->SetEntityName("Crate");

		// Set up second cube object
		const auto distanceMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/basic_lit.vert", "Assets/shaders/custom/cube_distance.frag"));
		distanceMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		distanceMat->SetUniformValue("material.shininessMat.color", glm::vec4(32.f));
		boxMesh = new graphics::Mesh(
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), distanceMat);
		const auto boxEntity = scene::Entity::CreateEntity<scene::Entity>();
		boxEntity->AddComponent<scene::MeshRenderer>(boxMesh);
		boxEntity->SetPosition(glm::vec3(0.f, 2.f, 0.f));
		boxEntity->SetEntityName("Box");

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
		skyEntity->SetEntityName("Skybox");

		// Set up plane material
		planeEntity = scene::Entity::CreateEntity<scene::Entity>();
		planeEntity->SetPosition({-50.f, 0.f, -50.f});
		planeMesh->material = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/custom/heightmap.vert", "Assets/shaders/custom/terrain.frag"));
		planeMesh->material->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		planeMesh->material->SetUniformValue("material.shininessMat.color", glm::vec4(32.f));
		planeMesh->material->SetUniformValue("material.diffuseMat.useSampler", 0);
		const auto heightMap = std::make_shared<graphics::Texture>("Assets/heightmap.png");
		planeMesh->material->AddTexture(heightMap);
		planeEntity->AddComponent<scene::MeshRenderer>(planeMesh);
		planeEntity->SetEntityName("Ground");

		// Load grass mesh and set up material
		const auto grassMesh = graphics::Model::LoadSingleMesh("Assets/Models/grass_highPoly.obj");
		const auto grassMatBase = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/custom/grass.vert", "Assets/shaders/custom/grass.frag"));
		grassMesh->material = grassMatBase;
		grassMesh->material->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		grassMesh->material->SetUniformValue("material.shininessMat.color", glm::vec4(12.f));
		grassMesh->material->SetUniformValue("material.diffuseMat.useSampler", 0);
		grassMesh->material->SetUniformValue("castShadows", 0);
		grassMatBase->AddTexture(heightMap);

		// Set up grass instancing
		auto grassInstanceMesh = new graphics::InstancedMesh(grassMesh->vertices, grassMesh->indices, grassMesh->textures);
		grassInstanceMesh->material = grassMatBase;
		grassInstance = scene::Entity::CreateEntity<scene::Entity>();
		grassInstance->AddComponent<scene::MeshRenderer>(grassInstanceMesh);
		grassInstance->SetEntityName("Grass");

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
				glm::vec3 rot = { 0, based::math::RandomRange(0, 45), 0 };
				grassInstanceMesh->AddInstance(scene::Transform(pos, rot, glm::vec3(1)));
			}
		}
		//grassInstance->SetActive(false);

		// Set up light placeholder
		const auto cubeMat = std::make_shared<graphics::Material>(
			LOAD_SHADER("Assets/shaders/basic_lit.vert", "Assets/shaders/basic_unlit.frag"));
		cubeMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		cubeMat->SetUniformValue("material.diffuseMat.useSampler", 0);
		cubeMat->SetUniformValue("castShadows", 0);
		auto cubeMesh = new graphics::Mesh(graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), cubeMat);
		lightPlaceholder = scene::Entity::CreateEntity<scene::Entity>();
		lightPlaceholder->AddComponent<scene::MeshRenderer>(cubeMesh);
		lightPlaceholder->AddComponent<scene::PointLight>(1.0f, 0.0014f, 0.0007f, glm::vec3(1.f));
		lightPlaceholder->SetPosition(lightPosition);
		lightPlaceholder->SetScale(glm::vec3(0.1f));
		lightPlaceholder->SetEntityName("LIGHT 1");

		// Set up second light
		const auto otherLight = scene::Entity::CreateEntity<scene::Entity>();
		otherLight->AddComponent<scene::MeshRenderer>(cubeMesh);
		otherLight->AddComponent<scene::PointLight>(1.0f, 0.09f, 0.032f, glm::vec3(1.f));
		otherLight->SetPosition(glm::vec3(1.8f, 2.4f, 2.2f));
		otherLight->SetScale(glm::vec3(0.1f));
		otherLight->SetEntityName("LIGHT 2");

		// Add sun light
		sunLight = scene::Entity::CreateEntity<scene::Entity>();
		sunLight->AddComponent<scene::DirectionalLight>(glm::vec3(1.f));
		sunLight->SetEntityName("Sun");

		GetCurrentScene()->GetActiveCamera()->SetPosition(glm::vec3(-1, 2, 4));
		GetCurrentScene()->GetActiveCamera()->SetRotation(glm::vec3(6, 53, 0));

		BASED_TRACE("Done initializing");

		// TODO: Fix text rendering behind sprites even when handled last
		// TODO: Optimize UI to not regenerate VAs every single frame
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
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_R))
		{
			mouseControl = !mouseControl;
		}

		// Mouse input
		if (mouseControl)
		{
			pitch += static_cast<float>(input::Mouse::DX()) * sensitivity * deltaTime;
			yaw += static_cast<float>(input::Mouse::DY()) * sensitivity * deltaTime;

			yaw = based::math::Clamp(yaw, -89.f, 89.f);

			GetCurrentScene()->GetActiveCamera()->SetRotation(glm::vec3(yaw, pitch, GetCurrentScene()->GetActiveCamera()->GetTransform().Rotation.z));
		}

		// Save initial mouse position for rolling ball
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

		// Show/Hide UI
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_H))
		{
			if (document->IsVisible()) document->Hide();
			else document->Show();
		}

		// Set light position and pass info to shaders
		lightPosition = glm::vec3(based::math::Sin(based::core::Time::GetTime()) * 4, 
			lightPosition.y, lightPosition.z);

		lightPlaceholder->SetPosition(lightPosition);

		sunLight->SetRotation(sunDirection);

		crateEntity->SetRotation(cubeRot);

		boxMesh->material->SetUniformValue("cratePos", crateEntity->GetTransform().Position);
		boxMesh->material->SetUniformValue("useLight", static_cast<int>(useLight));

		crateMesh->material->SetUniformValue("ambientStrength", ambientStrength);
		crateMesh->material->SetUniformValue("useLight", static_cast<int>(useLight));

		planeMesh->material->SetUniformValue("ambientStrength", ambientStrength);
		planeMesh->material->SetUniformValue("heightCoef", heightCoef);
		planeMesh->material->SetUniformValue("useLight", static_cast<int>(useLight));

		// Disable lights when not using lighting
		const entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		const auto lights = registry.view<scene::PointLight, scene::EntityReference>();

		for (const auto light : lights)
		{
			scene::Entity* ent = registry.get<scene::EntityReference>(light).entity;
			ent->SetActive(useLight);
		}

		UpdateShaders(grassInstance->GetComponent<scene::MeshRenderer>().mesh, useLight, ambientStrength, heightCoef);
	}

	void Render() override
	{
	}

	void ImguiRender() override
	{
		if (Engine::Instance().GetWindow().GetShouldRenderToScreen()) return;

		// Draw rendered frame to an ImGui image to simulate a game view window
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
			// Camera Settings
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

			ImGui::Spacing();

			// Misc. parameters
			ImGui::Checkbox("Use Light", &useLight);

			ImGui::DragFloat("Rolling Ball Scale", &R, 0.01f);
			ImGui::DragFloat("Height Coefficient", &heightCoef, 0.01f);

			entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();

			// Lighting controls
			if (ImGui::CollapsingHeader("Lights"))
			{
				const auto lights = registry.view<scene::PointLight, scene::Transform, scene::EntityReference>();

				int i = 0;
				for (const auto light : lights)
				{
					scene::PointLight lightComponent = registry.get<scene::PointLight>(light);
					scene::Transform trans = registry.get<scene::Transform>(light);

					glm::vec3 col = lightComponent.color;
					glm::vec3 position = trans.Position;
					ImGui::PushID(i);
					ImGui::Text("Light %d", i);
					ImGui::DragFloat3("Light Color", glm::value_ptr(col), 0.01f);
					ImGui::DragFloat3("Light Position", glm::value_ptr(position), 0.01f);
					ImGui::PopID();
					registry.patch<scene::Transform>(light, [position](auto& t) {t.Position = position; });
					registry.patch<scene::PointLight>(light, [col](auto& l) {l.color = col; });
					i++;
				}

				ImGui::Text("Sun Controls");
				ImGui::DragFloat3("Sun Direction", glm::value_ptr(sunDirection), 0.01f);

				ImGui::Text("General");
				ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f);
			}

			if (ImGui::CollapsingHeader("Objects"))
			{
				const auto objects = registry.view<
					scene::Transform, scene::EntityReference>(entt::exclude<scene::PointLight, scene::DirectionalLight>);

				int i = 0;
				for (const auto obj : objects)
				{
					scene::Entity* ent = registry.get<scene::EntityReference>(obj).entity;
					scene::Transform trans = registry.get<scene::Transform>(obj);

					glm::vec3 position = trans.Position;
					glm::vec3 rotation = trans.Rotation;
					glm::vec3 scale = trans.Scale;
					ImGui::PushID(i);
					ImGui::Text(ent->GetEntityName().c_str());
					ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
					ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.01f);
					ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f);
					ImGui::PopID();
					registry.patch<scene::Transform>(obj, [position, rotation, scale](auto& t) 
						{ t.Position = position; t.Rotation = rotation; t.Scale = scale; });
					i++;
				}
			}
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

	void UpdateShaders(const graphics::Mesh* mesh, const bool useLight, float ambientStrength, float height)
	{
		mesh->material->SetUniformValue("ambientStrength", ambientStrength);

		mesh->material->SetUniformValue("heightCoef", height);

		mesh->material->SetUniformValue("useLight", static_cast<int>(useLight));
	}
};

based::App* CreateApp()
{
	return new Sandbox();
}