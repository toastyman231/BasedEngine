#include "based/pch.h"
#include "scenehierarchy.h"

#include "based/app.h"
#include "based/input/joystick.h"
#include "based/input/keyboard.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "external/imgui/imgui.h"
#include "src/editorstatics.h"

namespace editor::panels
{
	void SceneHierarchy::Render()
	{
		if (ImGui::Begin("Hierarchy"))
		{
			auto& registry = based::Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
			auto view = registry.view<based::scene::EntityReference>();

			for (auto e : view)
			{
				auto ent = view.get<based::scene::EntityReference>(e).entity;
				if (auto entity = ent.lock())
				{
					if (entity->GetEntityName() == "EditorPlayer") continue;

					if (entity->Parent.lock() != nullptr) continue;

					DrawEntity(entity);
				}
			}
		}
		ImGui::End();
	}

	void SceneHierarchy::DrawEntity(const std::shared_ptr<based::scene::Entity>& entity)
	{
		ImGuiTreeNodeFlags flags = 0;
		if (entity->Children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
		if (Statics::SelectedEntitiesContains(entity)) flags |= ImGuiTreeNodeFlags_Selected;

		auto open = ImGui::TreeNodeEx(entity->GetEntityName().c_str(), flags);
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0) &&
			!(based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT)
				|| based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)))
		{
			Statics::SetSelectedEntities({ entity });
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)
			&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
		{
			auto entities = Statics::GetSelectedEntities();
			entities.emplace_back(entity);
			Statics::SetSelectedEntities(entities);
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::TextUnformatted(entity->GetEntityName().c_str());
			ImGui::SetDragDropPayload("null", &mEmptyPayload, sizeof(int));
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (auto _ = ImGui::AcceptDragDropPayload("null"))
			{
				for (auto& e : Statics::GetSelectedEntities())
				{
					if (auto ent = e.lock())
					{
						ent->SetParent(entity);
					}
				}
				Statics::SetSceneDirty(true);
			}
			ImGui::EndDragDropTarget();
		}

		for (auto c : entity->Children)
		{
			if (auto child = c.lock())
			{
				DrawEntity(child);
			}
		}

		if (open) ImGui::TreePop();
	}
}
