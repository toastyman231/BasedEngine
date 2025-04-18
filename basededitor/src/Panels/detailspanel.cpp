#include "based/pch.h"
#include "detailspanel.h"

#include <cinttypes>

#include "../EditorComponents.h"
#include "../editorstatics.h"
#include "../external/imguizmo/ImGuizmo.h"
#include "../Widgets/ImGuiCustomWidgets.h"
#include "based/graphics/mesh.h"
#include "based/input/keyboard.h"

namespace editor::panels
{
	void DetailsPanel::Render()
	{
		using namespace based;

		if (ImGui::Begin(mPanelTitle.c_str()))
		{
			auto selections = Statics::GetSelectedEntities();

			if (selections.empty()) { ImGui::End(); return; }

			auto entity = selections.front().lock();
			if (!entity) { ImGui::End(); return; }

			ImGui::PushID((uint32_t)entity->GetEntityHandle());

			bool isActive = entity->IsActive();
			bool previouslyActive = isActive;
			ImGui::Checkbox("##nolabel", &isActive);
			if (isActive != previouslyActive) Statics::EngineOperations.EditorSetEntityActive(entity, isActive);

			std::string name = entity->GetEntityName();

			ImGui::SameLine();
			if (ImGui::InputText("", &name))
			{
				if (!name.empty()) Statics::EngineOperations.EditorSetEntityName(entity, name);
			}

			ImGui::Text("UUID: %" PRIu64, entity->GetUUID());

			ImGui::Spacing();

			bool isChild = !entity->Parent.expired();
			auto& transform = entity->GetTransform();

			auto pos = isChild ? transform.LocalPosition() : transform.Position();
			auto rot = isChild ? transform.LocalEulerAngles() : transform.EulerAngles();
			auto scale = isChild ? transform.LocalScale() : transform.Scale();

			scene::Transform savedTransform;
			if (Statics::GetSavedTransforms().find(entity->GetUUID()) == Statics::GetSavedTransforms().end())
			{
				savedTransform = Statics::GetSavedTransforms()[entity->GetUUID()] = scene::Transform(pos, rot, scale);
			}
			else
				savedTransform = Statics::GetSavedTransforms()[entity->GetUUID()];

			ImVec2 guiPos = ImGui::GetCursorScreenPos();
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SetItemAllowOverlap();
				ImGui::SetCursorScreenPos(ImVec2(guiPos.x + (ImGui::GetItemRectSize().x - 50), guiPos.y));
				if (ImGui::Button("Reset", ImVec2(50, ImGui::GetItemRectSize().y)))
				{
					Statics::EngineOperations.EditorSetEntityTransform(entity,
						{
							{ 0, 0, 0 },
							{ 0, 0, 0 },
							{ 1, 1, 1 } },
						savedTransform,
						isChild);
					savedTransform = Statics::GetSavedTransforms()[entity->GetUUID()] =
						scene::Transform(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));
					ImGui::PopID();
					ImGui::End();
					return;
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL) &&
					(input::Keyboard::KeyDown(BASED_INPUT_KEY_Z)
						|| input::Keyboard::KeyDown(BASED_INPUT_KEY_Y)))
				{
					savedTransform = Statics::GetSavedTransforms()[entity->GetUUID()] = scene::Transform(pos, rot, scale);
				}

				auto savedPos = savedTransform.Position();
				auto savedRot = savedTransform.EulerAngles();
				auto savedScale = savedTransform.Scale();

				ImGui::Indent(); ImGui::Spacing();
				if (ImGui::TransformEditor( 
					glm::value_ptr(pos), 
					glm::value_ptr(savedPos),
					glm::value_ptr(rot),
					glm::value_ptr(savedRot),
					glm::value_ptr(scale),
					glm::value_ptr(savedScale),
					0.01f))
				{
					Statics::EngineOperations.EditorSetEntityTransform(entity,
						entity->GetTransform(), 
						savedTransform, isChild);
					Statics::GetSavedTransforms()[entity->GetUUID()] = scene::Transform(pos, rot, scale);
				}

				if (isChild)
				{
					if (pos != transform.LocalPosition() || rot != transform.LocalEulerAngles() || scale != transform.LocalScale())
						entity->SetLocalTransform(pos, rot, scale);
				}
				else
				{
					if (pos != transform.Position() || rot != transform.EulerAngles() || scale != transform.Scale())
						entity->SetTransform(pos, rot, scale);
				}

				ImGui::Unindent();
			}

			DrawCameraComponent(entity);

			ImGui::Spacing();

			DrawMeshRendererComponent(entity);

			ImGui::Spacing();

			DrawDirectionalLightComponent(entity);

			ImGui::Spacing();

			DrawPointLightComponent(entity);

			auto isPopupOpen = ImGui::IsPopupOpen("ComponentPicker");
			if (isPopupOpen) ImGui::PushDisabled();
			if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
			{
				ImGui::OpenPopup("ComponentPicker");
			}
			if (isPopupOpen) ImGui::PopDisabled();
			if (ImGui::BeginPopup("ComponentPicker"))
			{
				ImGui::Text("Add Component");
				static std::vector<std::string> componentTypes = 
					{ "Mesh Renderer", "Point Light", "Directional Light", "Camera Component" };

				if (ImGui::BeginListBox("##nolabel", ImVec2(200,0)))
				{
					for (auto& type : componentTypes)
					{
						if (ImGui::Selectable(type.c_str()))
						{
							if (type == "Mesh Renderer")
							{
								auto meta_type = entt::resolve(entt::type_hash<scene::MeshRenderer>());
								if (!meta_type)
								{
									BASED_ERROR("Mesh Renderer has not been reflected!");
								}
								else
								{
									Statics::EngineOperations.EditorAddComponent(meta_type, entity);
								}
							}
							else if (type == "Point Light")
							{
								auto meta_type = entt::resolve(entt::type_hash<scene::PointLight>());
								if (!meta_type)
								{
									BASED_ERROR("Mesh Renderer has not been reflected!");
								}
								else
								{
									Statics::EngineOperations.EditorAddComponent(meta_type, entity);
								}
							}
							else if (type == "Directional Light")
							{
								auto meta_type = entt::resolve(entt::type_hash<scene::DirectionalLight>());
								if (!meta_type)
								{
									BASED_ERROR("Directional Light has not been reflected!");
								}
								else
								{
									Statics::EngineOperations.EditorAddComponent(meta_type, entity);
								}
							} else if (type == "Camera Component")
							{
								auto meta_type = entt::resolve(entt::type_hash<scene::CameraComponent>());
								if (!meta_type)
								{
									BASED_ERROR("Camera Component has not been reflected!");
								}
								else
								{
									Statics::EngineOperations.EditorAddComponent(meta_type, entity);
								}
							}
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndListBox();
				}
				ImGui::EndPopup();
			}

			ImGui::PopID();
		}
		ImGui::End();
	}

	void DetailsPanel::DrawMeshRendererComponent(std::shared_ptr<based::scene::Entity> entity)
	{
		auto hasMeshRenderer = entity->HasComponent<based::scene::MeshRenderer>();
		if (ImGui::CollapsingHeader("Mesh Renderer", &hasMeshRenderer, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(); ImGui::Spacing();
			auto& renderer = entity->GetComponent<based::scene::MeshRenderer>();
			if (!hasMeshRenderer)
			{
				auto meta_type = entt::resolve(entt::type_hash<based::scene::MeshRenderer>());
				if (!meta_type)
				{
					BASED_ERROR("Mesh Renderer has not been reflected!");
				}
				else
				{
					Statics::EngineOperations.EditorRemoveComponent(meta_type, entity);
				}
			}

			auto mesh = renderer.mesh.lock();
			auto material = renderer.material.lock();

			if (auto pickedMesh =
				ImGui::ObjectPicker<based::graphics::Mesh>("Mesh", mesh))
			{
				Statics::EngineOperations.EditorSetMeshRendererMesh(entity, pickedMesh);
			}
			ImGui::Spacing();

			if (mesh)
			{
				if (auto pickedMat =
					ImGui::ObjectPicker<based::graphics::Material>("Material", material))
				{
					Statics::EngineOperations.EditorSetMeshMaterial(entity, pickedMat);
				}
				ImGui::Spacing();

				if (material)
				{
					if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Indent();
						bool castShadows = material->GetUniformValue("castShadows", 1);
						if (ImGui::Checkbox("Cast Shadows", &castShadows))
						{
							material->SetUniformValue("castShadows", (int)castShadows);
						}

						bool receiveShadows = material->GetUniformValue("receiveShadows", 1);
						if (ImGui::Checkbox("Receive Shadows", &receiveShadows))
						{
							material->SetUniformValue("receiveShadows", (int)receiveShadows);
						}
						ImGui::Unindent();
					}
				}
			}

			ImGui::Unindent();
		}
	}

	void DetailsPanel::DrawPointLightComponent(std::shared_ptr<based::scene::Entity> entity)
	{
		using namespace based;

		auto hasPointLight = entity->HasComponent<scene::PointLight>();
		if (ImGui::CollapsingHeader("Point Light", &hasPointLight, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(); ImGui::Spacing();
			auto light = entity->GetComponent<scene::PointLight>();
			static auto savedLightData = light;
			if (!hasPointLight)
			{
				auto meta_type = entt::resolve(entt::type_hash<scene::PointLight>());
				if (!meta_type)
				{
					BASED_ERROR("Point Light has not been reflected!");
				}
				else
				{
					Statics::EngineOperations.EditorRemoveComponent(meta_type, entity);
					return;
				}
			}

			static auto madeAnyChange = false;
			static auto lightSaved = false;

			if (!lightSaved || entity->HasComponent<LightChangedDueToUndo>())
			{
				lightSaved = true;
				savedLightData = light;
				entity->RemoveComponent<LightChangedDueToUndo>();
			}

			ImGui::Text("Color"); ImGui::SameLine();
			if (ImGui::ColorEdit3("##color", glm::value_ptr(light.color)))
				madeAnyChange = true;

			ImGui::Text("Constant"); ImGui::SameLine();
			if (ImGui::DragFloat("##constant", &light.constant, 0.01f)) madeAnyChange = true;
			ImGui::Text("Linear"); ImGui::SameLine();
			if (ImGui::DragFloat("##linear", &light.linear, 0.01f)) madeAnyChange = true;
			ImGui::Text("Quadratic"); ImGui::SameLine();
			if (ImGui::DragFloat("##quadratic", &light.quadratic, 0.01f)) madeAnyChange = true;
			ImGui::Text("Intensity"); ImGui::SameLine();
			if (ImGui::DragFloat("##intensity", &light.intensity, 0.01f)) madeAnyChange = true;

			auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
			registry.patch<scene::PointLight>(entity->GetEntityHandle(),
				[light](scene::PointLight& pl)
				{
					pl.intensity = light.intensity;
					pl.linear = light.linear;
					pl.quadratic = light.quadratic;
					pl.constant = light.constant;
					pl.color = light.color;
				});

			if (madeAnyChange && ImGui::IsMouseReleased(0))
			{
				Statics::EngineOperations.EditorSetPointLightData(entity, savedLightData, light);
				madeAnyChange = false;
				lightSaved = false;
			}

			ImGui::Unindent();
		}
	}

	void DetailsPanel::DrawDirectionalLightComponent(std::shared_ptr<based::scene::Entity> entity)
	{
		using namespace based;

		auto hasDirLight = entity->HasComponent<scene::DirectionalLight>();
		if (ImGui::CollapsingHeader("Directional Light", &hasDirLight, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(); ImGui::Spacing();
			auto light = entity->GetComponent<scene::DirectionalLight>();
			static auto savedLightData = light;
			if (!hasDirLight)
			{
				auto meta_type = entt::resolve(entt::type_hash<scene::DirectionalLight>());
				if (!meta_type)
				{
					BASED_ERROR("Directional Light has not been reflected!");
				}
				else
				{
					Statics::EngineOperations.EditorRemoveComponent(meta_type, entity);
					return;
				}
			}

			static auto madeAnyChange = false;
			static auto lightSaved = false;

			if (!lightSaved || entity->HasComponent<DLightChangedDueToUndo>())
			{
				lightSaved = true;
				savedLightData = light;
				entity->RemoveComponent<DLightChangedDueToUndo>();
			}

			ImGui::Text("Color"); ImGui::SameLine();
			if (ImGui::ColorEdit3("##color", glm::value_ptr(light.color)))
				madeAnyChange = true;

			ImGui::Text("Intensity"); ImGui::SameLine();
			if (ImGui::DragFloat("##intensity", &light.intensity, 0.01f)) madeAnyChange = true;

			auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
			registry.patch<scene::DirectionalLight>(entity->GetEntityHandle(),
				[light](scene::DirectionalLight& pl)
				{
					pl.intensity = light.intensity;
					pl.color = light.color;
				});

			if (madeAnyChange && ImGui::IsMouseReleased(0))
			{
				Statics::EngineOperations.EditorSetDirectionalLightData(entity, savedLightData, light);
				madeAnyChange = false;
				lightSaved = false;
			}

			ImGui::Unindent();
		}
	}

	void DetailsPanel::DrawCameraComponent(std::shared_ptr<based::scene::Entity> entity)
	{
		using namespace based;

		auto hasCamera = entity->HasComponent<scene::CameraComponent>();
		if (ImGui::CollapsingHeader("Camera", &hasCamera, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(); ImGui::Spacing();
			auto cameraComp = entity->GetComponent<scene::CameraComponent>();
			if (!hasCamera)
			{
				auto meta_type = entt::resolve(entt::type_hash<scene::CameraComponent>());
				if (!meta_type)
				{
					BASED_ERROR("Camera Component has not been reflected!");
				}
				else
				{
					Statics::EngineOperations.EditorRemoveComponent(meta_type, entity);
					return;
				}
			}

			static auto madeAnyChange = false;
			static auto cameraSaved = false;

			if (auto camera = cameraComp.camera.lock())
			{
				static std::vector<std::string> projectionOptions = { "Orthographic", "Perspective" };
				auto currentProjectionString = camera->GetProjectionString(camera->GetProjectionType());
				auto currentProjection = camera->GetProjectionType();

				static auto oldCameraData = graphics::CameraData{
					camera->GetProjectionType(), camera->GetFOV(),
					camera->GetNear(), camera->GetFar(), camera->GetHeight(),
					camera->main
				};

				if (!cameraSaved || entity->HasComponent<CamChangedDueToUndo>())
				{
					cameraSaved = true;
					oldCameraData = graphics::CameraData{
						camera->GetProjectionType(), camera->GetFOV(),
						camera->GetNear(), camera->GetFar(), camera->GetHeight(),
						camera->main
					};
					entity->RemoveComponent<CamChangedDueToUndo>();
				}

				bool isMain = camera->main;
				ImGui::Text("Main Camera"); ImGui::SameLine();
				if (ImGui::Checkbox("##maincam", &isMain))
				{
					madeAnyChange = true;
				}

				ImGui::Text("Projection"); ImGui::SameLine();
				if (ImGui::BeginCombo("##proj", currentProjectionString.c_str()))
				{
					for (size_t n = 0; n < projectionOptions.size(); n++)
					{
						auto item = projectionOptions[n];
						const bool selected = currentProjectionString.c_str() == item;
						if (ImGui::Selectable(item.c_str(), selected))
						{
							if (item == camera->GetProjectionString(graphics::ORTHOGRAPHIC).c_str())
							{
								currentProjection = graphics::ORTHOGRAPHIC;
							}
							else currentProjection = graphics::PERSPECTIVE;
							madeAnyChange = true;
						}
					}

					ImGui::EndCombo();
				}

				auto currentFOV = camera->GetFOV();

				ImGui::Text("FOV"); ImGui::SameLine();
				if (ImGui::DragFloat("##fov", &currentFOV, 0.01f))
				{
					madeAnyChange = true;
				}

				auto currentNearPlane = camera->GetNear();

				ImGui::Text("Near"); ImGui::SameLine();
				if (ImGui::DragFloat("##near", &currentNearPlane, 0.01f))
				{
					madeAnyChange = true;
				}

				auto currentFarPlane = camera->GetFar();

				ImGui::Text("Far"); ImGui::SameLine();
				if (ImGui::DragFloat("##far", &currentFarPlane, 0.01f))
				{
					madeAnyChange = true;
				}

				auto currentHeight = camera->GetHeight();
				if (currentProjection == graphics::ORTHOGRAPHIC)
				{
					ImGui::Text("Height"); ImGui::SameLine();
					if (ImGui::DragFloat("##height", &currentHeight, 0.01f))
					{
						madeAnyChange = true;
					}
				}

				camera->SetProjection(currentProjection);
				camera->SetFOV(currentFOV);
				camera->SetNear(currentNearPlane);
				camera->SetFar(currentFarPlane);
				camera->SetHeight(currentHeight);

				if (madeAnyChange && ImGui::IsMouseReleased(0))
				{
					auto newCameraData = graphics::CameraData{
						camera->GetProjectionType(), camera->GetFOV(),
						camera->GetNear(), camera->GetFar(), camera->GetHeight(),
						isMain
					};

					Statics::EngineOperations.EditorSetCameraData(entity, oldCameraData, newCameraData);
					madeAnyChange = false;
					cameraSaved = false;
				}
			}

			ImGui::Unindent();
		}
	}
}
