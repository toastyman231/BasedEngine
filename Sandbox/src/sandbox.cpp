#include <typeindex>

#include "based/pch.h"
#include "based/core/assetlibrary.h"
#include "based/engine.h"
#include "based/graphics/camera.h"
#include "based/graphics/defaultassetlibraries.h"
#include "based/graphics/model.h"
#include "based/input/keyboard.h"
#include "based/input/mouse.h"
#include "based/log.h"
#include "based/main.h"
#include "based/math/basedmath.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "external/imgui/imgui.h"
#include "based/core/basedtime.h"
#include "external/entt/core/hashed_string.hpp"
#include "external/imgui/imgui_internal.h"

using namespace based;

class Sandbox : public based::App
{
	std::shared_ptr<scene::Entity> cameraEntity;
	std::shared_ptr<scene::Entity> sponzaEntity;
	std::shared_ptr<scene::Entity> curtainsEntity;
	std::shared_ptr<scene::Entity> sphereEntity;

	float speed = 6.f;
	float pitch = 0.f;
	float yaw = 0.f;
	float sensitivity = 6.f;
	float ambientStrength = 0.1f;

	int curRenderMode = 0;

	bool useNormalMaps = true;

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

	struct TestStruct
	{
		int testData = 0;
	};

	void Initialize() override
	{
		App::Initialize();
		Engine::Instance().GetWindow().SetShouldRenderToScreen(false);
		input::Mouse::SetCursorVisible(!Engine::Instance().GetWindow().GetShouldRenderToScreen());
		input::Mouse::SetCursorMode(Engine::Instance().GetWindow().GetShouldRenderToScreen() ?
			input::CursorMode::Confined : input::CursorMode::Free);

		scene::Scene::LoadScene(ASSET_PATH("Scenes/Default3DMinimal.bscn"));

		cameraEntity = GetCurrentScene()->GetEntityStorage().Get("Main Camera");
		GetCurrentScene()->GetEntityStorage().Get("Directional Light")->SetPosition(glm::vec3(7.f, 0.f, -3.f));

		sponzaEntity = scene::Entity::CreateEntity("Sponza Geometry");
		curtainsEntity = scene::Entity::CreateEntity("Curtains Geometry");
		sphereEntity = scene::Entity::CreateEntity("Sphere");

		auto sponzaMesh = graphics::Model::CreateModel("Assets/Models/sponza.gltf",
		                                               DEFAULT_MODEL_LIB, "Sponza");
		auto curtainsMesh = graphics::Model::CreateModel("Assets/Models/sponza_curtains.gltf",
		                                                 DEFAULT_MODEL_LIB, "Sponza Curtains");

		sponzaEntity->AddComponent<scene::ModelRenderer>(sponzaMesh);
		curtainsEntity->AddComponent<scene::ModelRenderer>(curtainsMesh);

		curtainsEntity->SetRotation(glm::vec3(90.f, 0.f, 0.f));
		curtainsEntity->SetPosition(glm::vec3(0.05f, 0.f, -6.36f));

		sphereEntity->SetPosition(glm::vec3(3.f, 0.f, -3.f));
		
		auto sphereMat = graphics::Material::LoadMaterialFromFile(ASSET_PATH("Materials/Lit.bmat"),
			DEFAULT_MAT_LIB);
		auto sphereMesh = graphics::Mesh::LoadMeshFromFile(ASSET_PATH("Meshes/sphere.obj"),
			DEFAULT_MESH_LIB);
		sphereMat->SetUniformValue("material.albedo.color", glm::vec4(0.94f, 0.75f, 0.016f, 1.f));
		sphereEntity->AddComponent<scene::MeshRenderer>(sphereMesh, sphereMat);

		cameraEntity->SetPosition(glm::vec3(7.f, 0.f, -3.f));
		cameraEntity->SetRotation(glm::vec3(6.f, 270.f, 0.f));

		entt::meta_factory<TestStruct>{}
			.type(entt::hashed_string("TestStruct"), "TestStruct")
			.data<&TestStruct::testData>(entt::hashed_string("TestData"), "testData");

		auto structType = entt::resolve(entt::hashed_string("TestStruct"));
		auto myStruct = structType.construct();
		myStruct.set(entt::hashed_string("TestData"), 42);

		BASED_TRACE("The value is: {}", myStruct.cast<TestStruct>().testData);
		for (auto& [id, data] : structType.data())
		{
			BASED_TRACE("Data: {}, Value: {}", data.name(), myStruct.get(id).cast<int>());
		}

		/*auto gammaMaterial = graphics::Material::LoadMaterialFromFile(ASSET_PATH("Materials/PP_Gamma.bmat"),
			DEFAULT_MAT_LIB);
		auto post_process = new graphics::PostProcessPass("PP_Gamma", "SceneColor", gammaMaterial);
		Engine::Instance().GetRenderManager().InjectPass(post_process,
			(int)graphics::PassInjectionPoint::BeforeUserInterface);*/

		core::Time::SetTimeScale(0);
		BASED_TRACE("Done initializing");

		// TODO: Decide what to do about Sprites
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
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position() + speed * core::Time::UnscaledDeltaTime() * camera->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_S))
		{
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position() - speed * core::Time::UnscaledDeltaTime() * camera->GetForward());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_A))
		{
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position() - speed * core::Time::UnscaledDeltaTime() * camera->GetRight());
		}
		if (input::Keyboard::Key(BASED_INPUT_KEY_D))
		{
			const auto& transform = cameraEntity->GetTransform();
			const auto& camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetPosition(transform.Position() + speed * core::Time::UnscaledDeltaTime() * camera->GetRight());
		}
		
		if (input::Mouse::Button(BASED_INPUT_MOUSE_RIGHT))
		{
			pitch += static_cast<float>(input::Mouse::DX()) * sensitivity;
			yaw += static_cast<float>(input::Mouse::DY()) * sensitivity;

			yaw = based::math::Clamp(yaw, -89.f, 89.f);

			const auto camera = cameraEntity->GetComponent<scene::CameraComponent>().camera.lock();
			cameraEntity->SetRotation(glm::vec3(yaw, pitch, camera->GetTransform().EulerAngles().z));
		}
	}

	void Render() override
	{
	}

	void ImguiRender() override
	{
		PROFILE_FUNCTION();
		
		auto& registry = GetCurrentScene()->GetRegistry();

		if (Engine::Instance().GetWindow().GetShouldRenderToScreen()) return;

		ImGuiWindowFlags flags = 0;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("MainWindow", nullptr, flags);

		if (ImGui::DockBuilderGetNode(ImGui::GetID("MainDockspace")) == NULL)
		{
			ImGuiID dockspaceId = ImGui::GetID("MainDockspace");
			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoTabBar);
			ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

			ImGuiID mainDockspaceId = dockspaceId;
			ImGuiID leftBarId = ImGui::DockBuilderSplitNode(mainDockspaceId,
				ImGuiDir_Left, 0.25f, nullptr, &mainDockspaceId);

			ImGui::DockBuilderDockWindow("Settings", leftBarId);
			ImGui::DockBuilderDockWindow("GameView", mainDockspaceId);
			ImGui::DockBuilderFinish(dockspaceId);
		}

		ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0.0f, 0.0f), 0);
		ImGui::End();

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
			int currentFps = static_cast<int>(std::floor(1.f / core::Time::UnscaledDeltaTime()));
			static uint64_t allTimeFps = 0;
			static uint64_t fpsCount = 0;
			allTimeFps += currentFps;
			fpsCount++;
			ImGui::Text("FPS: %i", currentFps);
			ImGui::Text("Avg FPS: %zu", allTimeFps / fpsCount);
			
			if (persistentScene->GetActiveCamera())
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
			}

			// Other settings

			float timescale = core::Time::TimeScale();
			ImGui::SliderFloat("Time Scale", &timescale, 0.f, 2.f);
			core::Time::SetTimeScale(timescale);

			//ImGui::DragFloat("Blend Speed", &animator->blendSpeed, 1.f, 0.f, 100.f);

			ImGui::Spacing();

			// Misc. parameters
			const char* renderModes[] = { "Lit", "Unlit", "Normal", "Metallic", "Roughness", "AO", "Emission" };
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

			// Lighting controls
			if (ImGui::CollapsingHeader("Lights"))
			{
				const auto lights = registry.view<scene::PointLight, scene::Transform, scene::EntityReference>();

				ImGui::Text("Point Lights");
				int i = 0;
				for (const auto light : lights)
				{
					scene::PointLight& lightComponent = registry.get<scene::PointLight>(light);
					scene::Transform& trans = registry.get<scene::Transform>(light);

					glm::vec3 col = lightComponent.color;
					glm::vec3 position = trans.Position();
					ImGui::PushID(i);
					ImGui::Text("Light %d", i);
					ImGui::DragFloat3("Light Color", glm::value_ptr(col), 0.01f);
					ImGui::DragFloat3("Light Position", glm::value_ptr(position), 0.01f);
					ImGui::PopID();
					trans.SetPosition(position);
					lightComponent.color = col;
					i++;
				}

				ImGui::Text("Directional Lights");
				const auto dirLights = registry.view<scene::DirectionalLight, scene::Transform, scene::EntityReference>();

				for (const auto light : dirLights)
				{
					scene::DirectionalLight& lightComponent = registry.get<scene::DirectionalLight>(light);
					scene::Transform& trans = registry.get<scene::Transform>(light);

					glm::vec3 col = lightComponent.color;
					glm::vec3 direction = trans.EulerAngles();
					ImGui::PushID(i);
					ImGui::Text("Light %d", i);
					ImGui::DragFloat3("Light Color", glm::value_ptr(col), 0.01f);
					ImGui::DragFloat3("Light Direction", glm::value_ptr(direction), 10.f);
					ImGui::PopID();
					lightComponent.color = col;
					trans.SetEulerAngles(direction);
					auto model = glm::mat4(1.f);
					model = glm::rotate(model, glm::radians(direction.y), glm::vec3(0.f, 1.f, 0.f));
					model = glm::rotate(model, glm::radians(direction.x), glm::vec3(1.f, 0.f, 0.f));
					model = glm::rotate(model, glm::radians(direction.z), glm::vec3(0.f, 0.f, 1.f));

					glm::vec3 toPos = trans.Position() + glm::mat3(model) * glm::vec3(0.f, 0.f, 1.f);
					Engine::Instance().GetPhysicsManager().GetDebugRenderer()->DrawArrow(
							JPH::Vec3(trans.Position().x, trans.Position().y, trans.Position().z),
							JPH::Vec3(toPos.x, toPos.y, toPos.z),
							JPH::Color::sRed,
							0.25f
						);
					i++;
				}

				ImGui::Text("General");
				ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f);
				ImGui::Checkbox("Use Normal Maps", &useNormalMaps);

				auto mats = Engine::Instance().GetResourceManager().GetMaterialStorage();
				for (auto& mat : mats.GetAll())
				{
					mat.second->SetUniformValue("ambientStrength", ambientStrength);
					mat.second->SetUniformValue("material.normal.useSampler", (int)useNormalMaps);
				}
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
					scene::Transform& trans = registry.get<scene::Transform>(obj);

					if (auto e = ent.lock())
					{
						glm::vec3 position = trans.Position();
						glm::vec3 rotation = trans.EulerAngles();
						glm::vec3 scale = trans.Scale();
						glm::vec3 localPos = trans.LocalPosition();
						glm::vec3 localRot = trans.LocalEulerAngles();
						glm::vec3 localScale = trans.LocalScale();
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
				for (const auto& mat : GetCurrentScene()->GetMaterialStorage().GetAll())
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

								// Setup for texture combo box, each texture must have its own current index
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