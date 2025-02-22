#include "based/pch.h"
#include "scenehierarchy.h"

#include "based/app.h"
#include "based/input/joystick.h"
#include "based/input/keyboard.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "external/imgui/imgui.h"
#include "../editorstatics.h"
#include "external/imgui/imgui_internal.h"

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

					ImGui::PushID((uint32_t)entity->GetEntityHandle());
					DrawEntity(entity);
					ImGui::PopID();
				}
			}

			if (ImGui::BeginDragDropTargetCustom(
				ImRect(ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax()),
				ImGui::GetWindowDockID()))
			{
				//BASED_TRACE("CHECKING DROP");
				if (auto _ = ImGui::AcceptDragDropPayload("entity"))
				{
					BASED_TRACE("DROP");
					for (auto& e : Statics::GetSelectedEntities())
					{
						if (auto ent = e.lock())
						{
							BASED_TRACE("Setting parent to null");
							Statics::EngineOperations.EditorSetEntityParent(nullptr, ent);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();
	}

	void SceneHierarchy::DrawEntity(const std::shared_ptr<based::scene::Entity>& entity)
	{
		ImGuiTreeNodeFlags flags = 0;
		if (entity->Children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
		if (Statics::SelectedEntitiesContains(entity)) flags |= ImGuiTreeNodeFlags_Selected;

		auto isOpen = ImGui::TreeNodeEx(entity->GetEntityName().c_str(), flags);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0) &&
			!(based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT)
				|| based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)))
		{
			if (!Statics::SelectedEntitiesContains(entity))
				Statics::SetSelectedEntities({ entity });
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0)
			&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
		{
			auto entities = Statics::GetSelectedEntities();
			entities.emplace_back(entity);
			if (!Statics::SelectedEntitiesContains(entities))
				Statics::SetSelectedEntities(entities);
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::TextUnformatted(entity->GetEntityName().c_str());
			ImGui::SetDragDropPayload("entity", &mEmptyPayload, sizeof(int));
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (auto _ = ImGui::AcceptDragDropPayload("entity"))
			{
				for (auto& e : Statics::GetSelectedEntities())
				{
					if (auto ent = e.lock())
					{
						if (ent == entity) continue;
						if (mReparentCooldown)
						{
							// Required because ImGui registers the drop operation two frames
							// in a row for some reason.
							mReparentCooldown = false;
							continue;
						}

						if (ent->Parent.lock() == entity)
						{
							BASED_TRACE("Setting parent of {} to null", ent->GetEntityName());
							Statics::EngineOperations.EditorSetEntityParent(nullptr, ent);
							mReparentCooldown = true;
							continue;
						}

						BASED_TRACE("Setting parent of {}", ent->GetEntityName());
						Statics::EngineOperations.EditorSetEntityParent(entity, ent);
						mReparentCooldown = true;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		for (auto c : entity->Children)
		{
			if (auto child = c.lock())
			{
				if (isOpen) DrawEntity(child);
			}
		}

		if (isOpen) 
			ImGui::TreePop();
	}
}
