#include "based/pch.h"
#include "detailspanel.h"

#include "../editorstatics.h"
#include "../Widgets/ImGuiCustomWidgets.h"
#include "based/graphics/mesh.h"

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
			auto& transform = entity->GetComponent<based::scene::Transform>();
			auto pos = isChild ? transform.LocalPosition : transform.Position;
			auto rot = isChild ? transform.LocalRotation : transform.Rotation;
			auto scale = isChild ? transform.LocalScale : transform.Scale;

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(); ImGui::Spacing();
				ImGui::Text("Position"); ImGui::SameLine();
				ImGui::DragFloat3("##Position", glm::value_ptr(pos), 0.01f);
				ImGui::Text("Rotation"); ImGui::SameLine();
				ImGui::DragFloat3("##Rotation", glm::value_ptr(rot), 0.01f);
				ImGui::Text("Scale"); ImGui::SameLine(0, 29);
				ImGui::DragFloat3("##Scale", glm::value_ptr(scale), 0.01f);
				if (isChild)
					entity->SetLocalTransform(pos, rot, scale);
				else entity->SetTransform(pos, rot, scale);
				ImGui::Unindent();
			}

			auto hasMeshRenderer = entity->HasComponent<based::scene::MeshRenderer>();
			if (ImGui::CollapsingHeader("Mesh Renderer", &hasMeshRenderer, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(); ImGui::Spacing();
				auto& renderer = entity->GetComponent<based::scene::MeshRenderer>();
				if (!hasMeshRenderer) entity->RemoveComponent<based::scene::MeshRenderer>();

				if (auto mesh = renderer.mesh.lock())
				{
					if (auto pickedMat = 
						ImGui::ObjectPicker<graphics::Material>("Material", mesh->material))
					{
						Statics::EngineOperations.EditorSetMeshMaterial(mesh, pickedMat);
					}
					ImGui::Spacing();

					if (auto pickedMesh = 
						ImGui::ObjectPicker<graphics::Mesh>("Mesh", mesh))
					{
						Statics::EngineOperations.EditorSetMeshRendererMesh(entity, pickedMesh);
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

			ImGui::PopID();
		}
		ImGui::End();
	}
}
