#include "based/pch.h"

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
#include "external/imgui/imgui.h"

#include "based/animation/animation.h"
#include "based/animation/animator.h"
#include "based/core/basedtime.h"
#include "based/graphics/sprite.h"
#include "based/math/random.h"
#include "based/scene/audio.h"
#include "based/ui/textentity.h"
#include "Models-Surfaces/Generators.h"
#include "Water/water.h"

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

class MyAnimationTransition : public animation::AnimationTransition
{
public:
	MyAnimationTransition(
		const std::shared_ptr<animation::AnimationState>& source,
		const std::shared_ptr<animation::AnimationState>& destination,
		const std::shared_ptr<animation::Animator>& animator,
		const std::shared_ptr<animation::AnimationStateMachine>& stateMachine)
		: AnimationTransition(source, destination, animator)
		, mStateMachine(stateMachine) {}
	~MyAnimationTransition() override = default;

	bool ShouldStateTransition() override
	{
		if (auto stateMachine = mStateMachine.lock())
		{
			return stateMachine->GetBool("punch", false);
		}
		else
		{
			BASED_WARN("State machine invalid!");
			return false;
		}
	}

private:
	std::weak_ptr<animation::AnimationStateMachine> mStateMachine;
};

class Sandbox : public based::App
{
private:
	std::shared_ptr<scene::Scene> secondScene;

	std::shared_ptr<scene::Entity> modelEntity;
	std::shared_ptr<scene::Entity> boxEntity;
	std::shared_ptr<scene::Entity> skyEntity;
	std::shared_ptr<scene::Entity> planeEntity;
	std::shared_ptr<scene::Entity> crateEntity;
	std::shared_ptr<scene::Entity> lightPlaceholder;
	std::shared_ptr<scene::Entity> otherLight;
	std::shared_ptr<scene::Entity> grassInstance;
	std::shared_ptr<scene::Entity> sunLight;
	std::shared_ptr<scene::Entity> arms;
	std::shared_ptr<scene::Entity> sphere;
	std::shared_ptr<scene::Entity> wallEntity;
	std::shared_ptr<scene::Entity> cameraEntity;
	std::shared_ptr<scene::Entity> iconEntity;
	std::shared_ptr<ui::TextEntity> text;

	std::shared_ptr<graphics::Mesh> temp;

	bool mouseControl = false;
	float speed = 2.5f;
	float yaw = 0.f;
	float pitch = 0.0f;
	float sensitivity = 0.8f;
	float ambientStrength = 0.1f;
	float R = 100.f;
	float heightCoef = 1.f;

	int32_t curRenderMode = 0;

	bool useNormalMaps = true;
	glm::vec3 camPos = glm::vec3(0.f, 0.f, 1.5f);
	glm::vec3 camRot = glm::vec3(0.f);

	glm::vec3 cubeRot;
	glm::vec3 lightPosition;
	glm::ivec2 initialPos;
	glm::vec3 sunDirection;
	glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 albedo = glm::vec3(1.0f, 0.84f, 0.0f);

	std::shared_ptr<animation::Animation> handsAnim;
	std::shared_ptr<animation::Animation> handsAnim2;
	std::shared_ptr<animation::Animator> animator;
	std::shared_ptr<animation::AnimationStateMachine> armsStateMachine;
	std::shared_ptr<animation::AnimationState> idleState;
	std::shared_ptr<animation::AnimationState> punchState;
	std::shared_ptr<MyAnimationTransition> idleToPunchTransition;
	std::shared_ptr<animation::AnimationTransition> punchToIdleTransition;

	std::shared_ptr<graphics::ComputeShader> compShader;

	managers::DocumentInfo* document;

public:
	core::WindowProperties GetWindowProperties() override
	{
		core::WindowProperties props;
		props.title = "Sandbox";
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
		document->document->Hide();

		// Old stuff, plus setting camera to perspective mode
		cubeRot = glm::vec3(0.f);
		sunDirection = glm::vec3(60.f, -60.f, 0.f);
		auto camera = GetCurrentScene()->GetActiveCamera();
		camera->SetProjection(based::graphics::PERSPECTIVE);

		const auto& camTransform = camera->GetTransform();
		cameraEntity = scene::Entity::CreateEntity<scene::Entity>("Camera", camTransform.Position,
			camTransform.Rotation, camTransform.Scale);
		cameraEntity->AddComponent<scene::CameraComponent>(camera);

		// TODO: Confirm local transforms work in 2D
		
		// Set up crate object and material
		const auto crateTex = std::make_shared<graphics::Texture>("Assets/crate.png");
		const auto crateMat = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_lit.frag")),
			DEFAULT_MAT_LIB, "Crate");
		crateMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		crateMat->SetUniformValue("material.shininessMat.color", glm::vec4(128.f));
		crateMat->SetUniformValue("material.diffuseMat.useSampler", 1);
		crateMat->AddTexture(crateTex, "material.diffuseMat.tex");
		const auto crateMesh = graphics::Mesh::CreateMesh(
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"),
			crateMat, DEFAULT_MESH_LIB, "CrateMesh");
		crateEntity = scene::Entity::CreateEntity<scene::Entity>("Crate");
		crateEntity->AddComponent<scene::MeshRenderer>(crateMesh);
		crateEntity->SetPosition(glm::vec3(2.7f, 1.f, 1.7f));
		crateEntity->SetEntityName("Crate");
		crateEntity->SetActive(false);

		// Set up second cube object
		const auto distanceMat = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), "Assets/shaders/custom/cube_distance.frag"),
			DEFAULT_MAT_LIB, "DistCube");
		distanceMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		distanceMat->SetUniformValue("material.shininessMat.color", glm::vec4(32.f));
		const auto boxMesh = graphics::Mesh::CreateMesh(
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"), distanceMat,
			DEFAULT_MESH_LIB, "BoxMesh");
		boxEntity = scene::Entity::CreateEntity<scene::Entity>("Box");
		boxEntity->AddComponent<scene::MeshRenderer>(boxMesh);
		boxEntity->SetPosition(glm::vec3(0.f, 2.f, 0.f));
		boxEntity->SetEntityName("Box");
		boxEntity->SetActive(false);

		lightPosition = glm::vec3(1, 1.2f, 0.3f);

		// Skybox material setup

		const auto skyboxTex = std::make_shared<graphics::Texture>("Assets/skybox_tex.png", true);
		const auto skybox = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_unlit.frag")),
			DEFAULT_MAT_LIB, "Skybox");
		skybox->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		skybox->SetUniformValue("material.diffuseMat.useSampler", 1);
		skybox->AddTexture(skyboxTex);

		// Generate plane mesh and skybox cube
		const auto planeMesh = GeneratePlane(100, 100);
		const auto skyboxMesh = graphics::Mesh::CreateMesh(
			graphics::DefaultLibraries::GetVALibrary().Get("AtlasTextureCube"),
			skybox, DEFAULT_MESH_LIB, "SkyboxMesh");

		// Skybox setup
		skyEntity = scene::Entity::CreateEntity<scene::Entity>("Sky");
		skyEntity->AddComponent<scene::MeshRenderer>(skyboxMesh);
		skyEntity->SetScale(glm::vec3(500.f));
		skyEntity->SetEntityName("Skybox");

		// Set up plane material
		planeEntity = scene::Entity::CreateEntity<scene::Entity>("Plane");
		planeEntity->SetPosition({-50.f, 0.f, -50.f});
		planeMesh->material = graphics::Material::CreateMaterial(
			LOAD_SHADER("Assets/shaders/custom/water.vert", "Assets/shaders/custom/water.frag"),
			DEFAULT_MAT_LIB, "Plane");
		planeMesh->material->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		planeMesh->material->SetUniformValue("material.shininessMat.color", glm::vec4(32.f));
		planeMesh->material->SetUniformValue("material.diffuseMat.useSampler", 0);
		const auto heightMap = std::make_shared<graphics::Texture>("Assets/heightmap.png");
		planeMesh->material->AddTexture(heightMap);
		planeEntity->AddComponent<scene::MeshRenderer>(planeMesh);
		planeEntity->SetEntityName("Ground");

		waterMaterial = planeMesh->material;
		UpdateWaterShader();

		// Load grass mesh and set up material
		const auto grassMesh = graphics::Model::LoadSingleMesh("Assets/Models/grass_highPoly.obj");
		graphics::DefaultLibraries::GetMeshLibrary().Load("GrassMesh", grassMesh);
		const auto grassMatBase = graphics::Material::CreateMaterial(
			LOAD_SHADER("Assets/shaders/custom/grass.vert", "Assets/shaders/custom/grass.frag"),
			DEFAULT_MAT_LIB, "Grass");
		grassMesh->material = grassMatBase;
		grassMesh->material->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		grassMesh->material->SetUniformValue("material.shininessMat.color", glm::vec4(12.f));
		grassMesh->material->SetUniformValue("material.diffuseMat.useSampler", 0);
		grassMesh->material->SetUniformValue("castShadows", 0);
		grassMatBase->AddTexture(heightMap);

		// Set up grass instancing
		const auto grassInstanceMesh = graphics::Mesh::CreateInstancedMesh(
			grassMesh->vertices, grassMesh->indices, grassMesh->textures,
			DEFAULT_MESH_LIB, "GrassInstanceMesh");
		grassInstanceMesh->material = grassMatBase;
		grassInstance = scene::Entity::CreateEntity<scene::Entity>("Grass");
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
		grassInstance->SetActive(false);

		// Set up light placeholder
		const auto cubeMat = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_unlit.frag")),
			DEFAULT_MAT_LIB, "Cube");
		cubeMat->SetUniformValue("material.diffuseMat.color", glm::vec4(1.f));
		cubeMat->SetUniformValue("material.diffuseMat.useSampler", 0);
		cubeMat->SetUniformValue("castShadows", 0);
		const auto cubeMesh = graphics::Mesh::CreateMesh(
			graphics::DefaultLibraries::GetVALibrary().Get("TexturedCube"),
			cubeMat, DEFAULT_MESH_LIB, "LightCubeMesh");
		lightPlaceholder = scene::Entity::CreateEntity<scene::Entity>("Light1");
		lightPlaceholder->AddComponent<scene::MeshRenderer>(cubeMesh);
		lightPlaceholder->AddComponent<scene::PointLight>(1.0f, 0.0014f, 0.0007f, glm::vec3(1.f));
		lightPlaceholder->SetPosition(lightPosition);
		lightPlaceholder->SetScale(glm::vec3(0.1f));
		lightPlaceholder->SetEntityName("LIGHT 1");

		// Set up second light
		otherLight = scene::Entity::CreateEntity<scene::Entity>("Light2");
		otherLight->AddComponent<scene::MeshRenderer>(cubeMesh);
		otherLight->AddComponent<scene::PointLight>(1.0f, 0.09f, 0.032f, glm::vec3(1.f));
		otherLight->SetPosition(glm::vec3(1.8f, 2.4f, 2.2f));
		otherLight->SetScale(glm::vec3(0.1f));
		otherLight->SetEntityName("LIGHT 2");

		// Add sun light
		sunLight = scene::Entity::CreateEntity<scene::Entity>("Sun");
		sunLight->AddComponent<scene::DirectionalLight>(glm::vec3(1.f));
		sunLight->SetEntityName("Sun");

		// Set up brick wall material
		const auto wallDiffuseTex = std::make_shared<graphics::Texture>("Assets/brick-wall/brickwall-diff.jpg", true);
		const auto wallNormalMapTex = std::make_shared<graphics::Texture>("Assets/brick-wall/brickwall-norm.jpg", true);
		const auto wallMat = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_lit.frag")),
			DEFAULT_MAT_LIB, "Wall");
		wallMat->AddTexture(wallDiffuseTex, "material.diffuseMat.tex");
		wallMat->SetUniformValue("material.diffuseMat.useSampler", 1);
		wallMat->AddTexture(wallNormalMapTex, "material.normalMat.tex");
		wallMat->SetUniformValue("material.normalMat.useSampler", 1);
		wallMat->SetUniformValue("material.shininessMat.color", glm::vec4(32.f));
		// Add brick wall entity
		wallEntity = scene::Entity::CreateEntity<scene::Entity>("Wall", glm::vec3(5, 3.5f, 0)
			, glm::vec3(90, 0, 0));
		const auto wallMesh = GeneratePlane(2, 2);
		wallMesh->material = wallMat;
		wallEntity->AddComponent<scene::MeshRenderer>(wallMesh);
		wallEntity->SetEntityName("Wall");
		wallEntity->SetActive(false);

		// Create arms material
		const auto armsMat = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/basic_lit_bones.vert"), ASSET_PATH("Shaders/basic_lit.frag")),
			DEFAULT_MAT_LIB, "Arms");
		const auto armsTex = std::make_shared<graphics::Texture>("Assets/Models/Base Color Palette Diffuse.png", true);
		armsMat->AddTexture(armsTex, "material.diffuseMat.tex");
		armsMat->SetUniformValue("material.diffuseMat.tint", glm::vec4(0.77f, 0.4f, 0.35f, 1.f));
		armsMat->SetUniformValue("material.diffuseMat.useSampler", 1);
		armsMat->SetUniformValue("receiveShadows", 0);
		// Create arms
		const auto armModel = graphics::Model::CreateModel(
			"Assets/Models/Arms.fbx", DEFAULT_MODEL_LIB, "ArmsModel");
		auto lib = graphics::DefaultLibraries::GetModelLibrary();
		armModel->SetMaterial(armsMat);
		arms = scene::Entity::CreateEntity<scene::Entity>("Arms");
		arms->AddComponent<scene::ModelRenderer>(armModel);
		arms->SetPosition({ 0, 5, 0 });
		arms->SetScale({ 0.01f, 0.01f, 0.01f });
		// Create arms animations and animator
		handsAnim = std::make_shared<animation::Animation>("Assets/Models/Arms.fbx", armModel, 0);
		handsAnim2 = std::make_shared<animation::Animation>("Assets/Models/Arms.fbx", armModel, "HumanFPS|Punch");
		handsAnim->SetPlaybackSpeed(0.5f);
		handsAnim->SetLooping(true);
		animator = std::make_shared<animation::Animator>(handsAnim);
		// Create state machine, states, and transitions
		armsStateMachine = std::make_shared<animation::AnimationStateMachine>(animator);
		idleState = std::make_shared<animation::AnimationState>(handsAnim, "IdleState");
		punchState = std::make_shared<animation::AnimationState>(handsAnim2, "PunchState");

		idleToPunchTransition = std::make_shared<MyAnimationTransition>(idleState, punchState, animator, armsStateMachine);
		punchToIdleTransition = std::make_shared<animation::AnimationTransition>(punchState, idleState, animator);
		idleState->AddTransition(idleToPunchTransition);
		punchState->AddTransition(punchToIdleTransition);
		armsStateMachine->AddState(idleState, true);
		armsStateMachine->AddState(punchState);
		animator->SetStateMachine(armsStateMachine);
		animator->SetTimeMode(animation::TimeMode::Unscaled);
		arms->AddComponent<scene::AnimatorComponent>(animator);
		arms->SetEntityName("Arms");

		const auto sphereMat = graphics::Material::CreateMaterial(
			LOAD_SHADER(ASSET_PATH("Shaders/pbr_lit.vert"), ASSET_PATH("Shaders/pbr_lit.frag")),
			DEFAULT_MAT_LIB, "Sphere");
		const auto sandAlbedo = std::make_shared<graphics::Texture>("Assets/sand_albedo.png");
		const auto sandRoughness = std::make_shared<graphics::Texture>("Assets/sand_roughness.png");
		const auto sandNormal = std::make_shared<graphics::Texture>("Assets/sand_normal.png");
		const auto sandAo = std::make_shared<graphics::Texture>("Assets/sand_ao.png");
		graphics::DefaultLibraries::GetTextureLibrary().Load("SandAlbedo", sandAlbedo);
		graphics::DefaultLibraries::GetTextureLibrary().Load("SandRough", sandRoughness);
		graphics::DefaultLibraries::GetTextureLibrary().Load("SandNormal", sandNormal);
		graphics::DefaultLibraries::GetTextureLibrary().Load("SandAo", sandAo);
		sphereMat->AddTexture(sandAlbedo, "material.albedo.tex");
		sphereMat->AddTexture(sandRoughness, "material.roughness.tex");
		sphereMat->AddTexture(sandNormal, "material.normal.tex");
		sphereMat->AddTexture(sandAlbedo, "material.ambientOcclusion.tex");
		sphereMat->SetUniformValue("material.albedo.useSampler", 1);
		sphereMat->SetUniformValue("material.roughness.useSampler", 1);
		sphereMat->SetUniformValue("material.normal.useSampler", 1);
		sphereMat->SetUniformValue("material.ambientOcclusion.useSampler", 1);
		//sphereMat->SetUniformValue("material.albedo.color", glm::vec4(1.0f, 0.84f, 0.0f, 1.0f));
		const auto sphereMesh = graphics::Model::LoadSingleMesh("Assets/Models/sphere.obj");
		graphics::DefaultLibraries::GetMeshLibrary().Load("Sphere", sphereMesh);
		sphereMesh->material = sphereMat;
		sphere = scene::Entity::CreateEntity<scene::Entity>("Sphere");
		sphere->AddComponent<scene::MeshRenderer>(sphereMesh);
		sphere->SetEntityName("Sphere");
		sphere->SetPosition({ 0.f, 2.f, 0.f });
		sphere->SetActive(false);

		// TODO: Add a way to tie object lifetimes to scene lifetime (scene serialization)

		cameraEntity->SetPosition(glm::vec3(-1, 2, 4));
		cameraEntity->SetRotation(glm::vec3(6, 53, 0));

		arms->SetParent(cameraEntity);
		arms->SetLocalPosition(glm::vec3(0.f, 0.f, -0.2f));
		arms->SetLocalRotation(glm::vec3(0.f, 180.f, 0.f));

		graphics::Texture::CreateImageTexture("CompTex", 512, 512,
			graphics::TextureAccessLevel::ReadWrite, DEFAULT_TEX_LIB);
		compShader = LOAD_COMPUTE_SHADER("Assets/shaders/test_compute.comp");
		auto compTex = graphics::DefaultLibraries::GetTextureLibrary().Get("CompTex");
		compShader->AddTexture(compTex, "tex");
		Engine::Instance().GetRenderManager().AddComputeShaderDispatch(compShader, 
			glm::vec<3, glm::uint>(compTex->GetWidth(), compTex->GetHeight(), 1));

		BASED_TRACE("Done initializing");

		// TODO: Fix text rendering behind sprites even when handled last
		// TODO: Add Scriptable Components 
		// TODO: Decide what to do about Sprites
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
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position - speed * core::Time::UnscaledDeltaTime() * camera->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_S))
		{
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position + speed * core::Time::UnscaledDeltaTime() * camera->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_A))
		{
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position - speed * core::Time::UnscaledDeltaTime() * camera->GetRight());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_D))
		{
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position + speed * core::Time::UnscaledDeltaTime() * camera->GetRight());
		}

		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_SPACE))
		{
			//if (core::Time::TimeScale() == 0.1f) core::Time::SetTimeScale(1.0f);
			//else core::Time::SetTimeScale(0.1f);
			core::Time::SetTimeScale(1.0f - core::Time::TimeScale());
		}

		// Enable/disable mouse control
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_R))
		{
			mouseControl = !mouseControl;
		}

		// Mouse input
		if (mouseControl)
		{
			pitch += static_cast<float>(input::Mouse::DX()) * sensitivity;
			yaw += static_cast<float>(input::Mouse::DY()) * sensitivity;

			yaw = based::math::Clamp(yaw, -89.f, 89.f);

			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetRotation(glm::vec3(yaw, pitch, camera->GetTransform().Rotation.z));
		}

		// Save initial mouse position for rolling ball
		if (input::Mouse::ButtonDown(BASED_INPUT_MOUSE_LEFT))
		{
			initialPos = input::Mouse::GetMousePosition();
			animator->GetStateMachine()->SetBool("punch", true);
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

		if (input::Mouse::ButtonUp(BASED_INPUT_MOUSE_LEFT))
		{
			animator->GetStateMachine()->SetBool("punch", false);
		}

		// Show/Hide UI
		if (input::Keyboard::KeyDown(BASED_INPUT_KEY_H))
		{
			if (document->document->IsVisible()) document->document->Hide();
			else document->document->Show();
		}

		// Set light position and pass info to shaders
		lightPosition = glm::vec3(based::math::Sin(based::core::Time::GetTime()) * 4, 
			lightPosition.y, lightPosition.z);

		lightPlaceholder->SetPosition(lightPosition);

		sunLight->SetRotation(sunDirection);

		crateEntity->SetRotation(cubeRot);

		auto matLib = graphics::DefaultLibraries::GetMaterialLibrary();
		const auto boxMat = matLib.Get("DistCube");
		const auto crateMat = matLib.Get("Crate");
		const auto planeMat = matLib.Get("Plane");
		const auto sphereMat = matLib.Get("Sphere");
		const auto wallMat = matLib.Get("Wall");
		const auto grassMat = matLib.Get("Grass");

		if (boxMat && crateMat && planeMat)
		{
			boxMat->SetUniformValue("cratePos", crateEntity->GetTransform().Position);

			crateMat->SetUniformValue("ambientStrength", ambientStrength);

			planeMat->SetUniformValue("ambientStrength", ambientStrength);
			planeMat->SetUniformValue("heightCoef", heightCoef);
		}

		// Disable lights when not using lighting
		entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		registry.patch<scene::DirectionalLight>(sunLight->GetEntityHandle(),
			[this](auto& l)
			{
				l.direction = sunDirection;
				l.color = sunColor;
			});

		const auto lights = registry.view<scene::PointLight, scene::EntityReference>();

		for (const auto light : lights)
		{
			auto ent = registry.get<scene::EntityReference>(light).entity;
			if (auto e = ent.lock()) e->SetActive(curRenderMode == 0);
		}

		UpdateShaders(grassMat, ambientStrength, heightCoef);

		// Disable normal maps when not using them
		if (!useNormalMaps)
		{
			wallMat->SetUniformValue("material.normalMat.useSampler", 0);
		} else
		{
			wallMat->SetUniformValue("material.normalMat.useSampler", 1);
		}
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
			ImGui::Image((void*)static_cast<intptr_t>(
				graphics::DefaultLibraries::GetRenderPassOutputs().Get("SceneColor")), 
				size, uv0, uv1);
		}
		ImGui::End();

		if (ImGui::Begin("Settings"))
		{
			// Camera Settings
			float fov = persistentScene->GetActiveCamera()->GetFOV();
			ImGui::DragFloat("FOV", &fov, 0.5f);
			persistentScene->GetActiveCamera()->SetFOV(fov);

			ImGui::DragFloat("Sensitivity", &sensitivity, 0.5f);

			float nearPlane = persistentScene->GetActiveCamera()->GetNear();
			ImGui::DragFloat("Near", &nearPlane, 0.5f);
			persistentScene->GetActiveCamera()->SetNear(nearPlane);

			float farPlane = persistentScene->GetActiveCamera()->GetFar();
			ImGui::DragFloat("Far", &farPlane, 0.5f);
			persistentScene->GetActiveCamera()->SetFar(farPlane);

			// Other settings

			float timescale = core::Time::TimeScale();
			ImGui::SliderFloat("Time Scale", &timescale, 0.f, 2.f);
			core::Time::SetTimeScale(timescale);

			ImGui::DragFloat("Blend Speed", &animator->blendSpeed, 1.f, 0.f, 100.f);

			ImGui::Spacing();

			// Misc. parameters
			const char* renderModes[] = {"Lit", "Unlit"};
			if (ImGui::BeginCombo("Render Mode", renderModes[curRenderMode]))
			{
				for (int i = 0; i < IM_ARRAYSIZE(renderModes); i++)
				{
					const bool isSelected = (curRenderMode == i);
					if (ImGui::Selectable(renderModes[i], isSelected))
						curRenderMode = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			managers::RenderManager::SetRenderMode(static_cast<managers::RenderMode>(curRenderMode));

			static bool wireFrame = false;
			ImGui::Checkbox("Wireframe", &wireFrame);
			Engine::Instance().GetRenderManager().SetWireframeMode(wireFrame);

			ImGui::DragFloat("Rolling Ball Scale", &R, 0.01f);
			ImGui::DragFloat("Height Coefficient", &heightCoef, 0.01f);

			entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();

			WaterSettings();
			UpdateShaderVisuals();
			UpdateWaterShader();

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
				ImGui::DragFloat3("Sun Color", glm::value_ptr(sunColor), 0.01f);

				ImGui::Text("General");
				ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f);
				ImGui::Checkbox("Use Normal Maps", &useNormalMaps);
			}

			// Object controls
			if (ImGui::CollapsingHeader("Objects"))
			{
				const auto objects = registry.view<
					scene::Transform, scene::EntityReference>(entt::exclude<scene::PointLight, scene::DirectionalLight>);

				int i = 0;
				for (const auto obj : objects)
				{
					auto ent = registry.get<scene::EntityReference>(obj).entity;
					scene::Transform trans = registry.get<scene::Transform>(obj);

					if (auto e = ent.lock())
					{
						glm::vec3 position = trans.Position;
						glm::vec3 rotation = trans.Rotation;
						glm::vec3 scale = trans.Scale;
						glm::vec3 localPos = trans.LocalPosition;
						glm::vec3 localRot = trans.LocalRotation;
						glm::vec3 localScale = trans.LocalScale;
						bool enabled = e->IsActive();
						ImGui::PushID(i);
						ImGui::Checkbox("", &enabled);
						ImGui::SameLine();
						ImGui::Text(e->GetEntityName().c_str());
						ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f);
						ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.01f);
						ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f);
						if (!e->Parent.expired())
						{
							ImGui::DragFloat3("Local Position", glm::value_ptr(localPos), 0.01f);
							ImGui::DragFloat3("Local Rotation", glm::value_ptr(localRot), 0.01f);
							ImGui::DragFloat3("Local Scale", glm::value_ptr(localScale), 0.01f);
						}
						ImGui::PopID();
						e->SetTransform(position, rotation, scale);
						if (!e->Parent.expired()) e->SetLocalTransform(localPos, localRot, localScale);
						e->SetActive(enabled);
						i++;
					}
				}
			}

			// Material editor
			if (ImGui::CollapsingHeader("Materials"))
			{
				int i = 0;
				ImGui::Indent(10.0f);
				// Loop over each saved material and create a dropdown for each one
				for (const auto mat : graphics::DefaultLibraries::GetMaterialLibrary().GetAll())
				{
					if (auto matPtr = mat.second)
					{
						ImGui::PushID(i);

						if (ImGui::CollapsingHeader(mat.first.c_str()))
						{
							ImGui::Indent(10.0f);
							if (ImGui::Button("Add Texture"))
							{
								ImGui::OpenPopup("Texture Setup");
							}
							if (ImGui::BeginPopup("Texture Setup"))
							{
								// Read in a new texture and activate it in the shader
								ImGui::Text("Texture Setup");
								static char texturePath[256] = "";
								static char samplerName[256] = "";
								static char enableName[256] = "";
								ImGui::InputText("Path", texturePath, IM_ARRAYSIZE(texturePath));
								ImGui::InputText("Sampler", samplerName, IM_ARRAYSIZE(samplerName));
								// Mainly just for my custom materials, since all of them have a bool to disable texture sampling
								ImGui::InputText("Enable Sampler", enableName, IM_ARRAYSIZE(enableName));
								if (ImGui::Button("Submit"))
								{
									std::shared_ptr<graphics::Texture> tex = std::make_shared<graphics::Texture>(texturePath);
									matPtr->AddTexture(tex, samplerName);
									if (enableName != "") matPtr->SetUniformValue(enableName, 1);
									ImGui::CloseCurrentPopup();
								}
								ImGui::EndPopup();
							}
							// Get all float uniforms and create an editable slider
							if (auto shader = matPtr->GetShader().lock())
							{
								for (const auto& f : shader->GetUniformFloats())
								{
									if (f.first.find("pointLight") != -1) continue;
									float temp = f.second;
									ImGui::DragFloat(f.first.c_str(), &temp, 0.01f);
									matPtr->SetUniformValue(f.first, temp);
								}

								// Setup for texture combo box, each texture must have it's own current index
								int j = 0;
								static std::vector<int> itemIndex;
								itemIndex.resize(static_cast<int>(shader->GetUniformSamplers().size()));
								// Get all texture samplers and create combo boxes to select what texture they sample
								for (const auto& f : shader->GetUniformSamplers())
								{
									// Get texture names from library key set, plus None
									std::vector<std::string> items;
									items.reserve(graphics::DefaultLibraries::GetTextureLibrary().Size());

									for (const auto& kv : 
										graphics::DefaultLibraries::GetTextureLibrary().GetAll())
									{
										items.emplace_back(kv.first);
									}

									items.insert(items.begin(), "None");
									auto preview = items[matPtr->GetTextureOrder()[f.first]];

									ImGui::PushID(j);
									if (ImGui::BeginCombo(f.first.c_str(), preview.c_str()))
									{
										// Create a selectable in the dropdown for each texture
										for (int n = 0; n < (int)(graphics::DefaultLibraries::GetTextureLibrary().GetAll().size()) + 1; n++)
										{
											auto item = items[n];
											const bool isSelected = itemIndex[j] == n;
											if (ImGui::Selectable(item.c_str(), isSelected))
											{
												// On selected, decide what happens
												itemIndex[j] = n;
												if (items[n] == "None")
												{
													matPtr->RemoveTexture(f.first);
													int index = static_cast<int>(f.first.find(".tex"));
													matPtr->SetUniformValue(f.first.substr(*f.first.begin() - f.first[0],
														index) + ".useSampler", 0);
												}
												else
												{
													matPtr->AddTexture(graphics::DefaultLibraries::GetTextureLibrary().Get(items[n]),
														f.first);
													int index = static_cast<int>(f.first.find(".tex"));
													matPtr->SetUniformValue(f.first.substr(*f.first.begin() - f.first[0],
														index) + ".useSampler", 1);
												}
											}
											if (isSelected) ImGui::SetItemDefaultFocus();
										}
										ImGui::EndCombo();
									}
									ImGui::PopID();
									j++;
								}
							}
						}
						ImGui::PopID();
						i++;
					}
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

	static void UpdateShaders(const std::shared_ptr<graphics::Material>& mat, float ambientStrength, float height)
	{
		mat->SetUniformValue("ambientStrength", ambientStrength);

		mat->SetUniformValue("heightCoef", height);
	}
};

based::App* CreateApp()
{
	return new Sandbox();
}