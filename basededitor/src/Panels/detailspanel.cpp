#include "based/pch.h"
#include "detailspanel.h"

#include "../EditorComponents.h"
#include "../editorstatics.h"
#include "../Widgets/ImGuiCustomWidgets.h"
#include "based/graphics/mesh.h"
#include "based/input/joystick.h"
#include "based/input/keyboard.h"

namespace editor::panels
{
	void DetailsPanel::Render()
	{
		using namespace based;

		if (ImGui::Begin(mPanelTitle.c_str()))
		{
			auto selections = editor::Statics::GetSelectedEntities();

			if (selections.empty()) { ImGui::End(); return; }

			auto entity = selections.front().lock();
			if (!entity) { ImGui::End(); return; }

			ImGui::PushID((uint32_t)entity->GetEntityHandle());

			bool isActive = entity->IsActive();
			bool previouslyActive = isActive;
			ImGui::Checkbox("##nolabel", &isActive);
			if (isActive != previouslyActive) Statics::EngineOperations.EditorSetEntityActive(entity, isActive);

			std::string name = entity->GetEntityName();
			char* buffer = (char*)name.c_str();

			ImGui::SameLine();
			if (ImGui::InputText("", buffer, IM_ARRAYSIZE(buffer)))
			{
				auto name = std::string(buffer);
				if (!name.empty()) Statics::EngineOperations.EditorSetEntityName(entity, name);
			}

			ImGui::Spacing();

			bool isChild = !entity->Parent.expired();
			auto transform = entity->GetTransform();
			static auto savedTransform = transform;
			auto pos = isChild ? transform.LocalPosition : transform.Position;
			auto rot = isChild ? transform.LocalRotation : transform.Rotation;
			auto scale = isChild ? transform.LocalScale : transform.Scale;

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				static auto madeAnyChange = false;
				static auto locationSaved = false;
				auto anyItemEdited = false;
				ImGui::Indent(); ImGui::Spacing();
				ImGui::Text("Position"); ImGui::SameLine();
				if (ImGui::DragFloat3("##Position", glm::value_ptr(pos), 0.01f)) madeAnyChange = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyItemEdited = true;
				ImGui::Text("Rotation"); ImGui::SameLine();
				if (ImGui::DragFloat3("##Rotation", glm::value_ptr(rot), 0.01f)) madeAnyChange = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyItemEdited = true;
				ImGui::Text("Scale"); ImGui::SameLine(0, 29);
				if (ImGui::DragFloat3("##Scale", glm::value_ptr(scale), 0.01f)) madeAnyChange = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyItemEdited = true;

				if (!locationSaved || entity->HasComponent<MovedDueToUndo>())
				{
					locationSaved = true;
					savedTransform = transform;
					entity->RemoveComponent<MovedDueToUndo>();
				}

				if (isChild)
					entity->SetLocalTransform(pos, rot, scale);
				else entity->SetTransform(pos, rot, scale);

				if (madeAnyChange && (ImGui::IsMouseReleased(0) || anyItemEdited))
				{
					Statics::EngineOperations.EditorSetEntityTransform(entity, 
						{ pos, rot , scale },
						savedTransform,
						isChild);
					locationSaved = false;
					madeAnyChange = false;
				}

				ImGui::Unindent();
			}

			DrawMeshRendererComponent(entity);

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
				static std::vector<std::string> componentTypes = { "MeshRenderer", "PointLight" };

				if (ImGui::BeginListBox("##nolabel", ImVec2(200,0)))
				{
					for (auto& type : componentTypes)
					{
						if (ImGui::Selectable(type.c_str()))
						{
							if (type == "MeshRenderer")
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
							else if (type == "PointLight")
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
			auto material = mesh ? mesh->material : nullptr;

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
					Statics::EngineOperations.EditorSetMeshMaterial(mesh, pickedMat);
				}
				ImGui::Spacing();

				if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent();
					bool castShadows = mesh->material->GetUniformValue("castShadows", 1);
					if (ImGui::Checkbox("Cast Shadows", &castShadows))
					{
						mesh->material->SetUniformValue("castShadows", (int)castShadows);
					}

					bool receiveShadows = mesh->material->GetUniformValue("receiveShadows", 1);
					if (ImGui::Checkbox("Receive Shadows", &receiveShadows))
					{
						mesh->material->SetUniformValue("receiveShadows", (int)receiveShadows);
					}
					ImGui::Unindent();
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

			ImGui::Text("Color"); ImGui::SameLine();
			ImGui::ColorEdit3("##color", glm::value_ptr(light.color));

			ImGui::Text("Constant"); ImGui::SameLine();
			ImGui::DragFloat("##constant", &light.constant, 0.01f);
			ImGui::Text("Linear"); ImGui::SameLine();
			ImGui::DragFloat("##linear", &light.linear, 0.01f);
			ImGui::Text("Quadratic"); ImGui::SameLine();
			ImGui::DragFloat("##quadratic", &light.quadratic, 0.01f);
			ImGui::Text("Intensity"); ImGui::SameLine();
			ImGui::DragFloat("##intensity", &light.intensity, 0.01f);

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

			ImGui::Unindent();
		}
	}
}
