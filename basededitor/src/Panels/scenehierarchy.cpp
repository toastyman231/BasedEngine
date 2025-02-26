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
		if (ImGui::Begin(mPanelTitle.c_str()))
		{
			auto& registry = based::Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
			auto view = registry.view<based::scene::EntityReference>();

			if (ImGui::IsMouseClicked(0))
				mRenameIndex = -1;

			mCurrentIndex = 0;
			mCurrentCount = 0;
			for (auto e : view)
			{
				auto ent = view.get<based::scene::EntityReference>(e).entity;
				if (auto entity = ent.lock())
				{
					if (entity->GetEntityName() == "EditorPlayer") continue;

					if (entity->Parent.lock() != nullptr) continue;

					ImGui::PushID((uint32_t)entity->GetEntityHandle());
					++mCurrentIndex;
					DrawEntity(entity);
					ImGui::PopID();
				}
			}

			if (ImGui::BeginDragDropTargetCustom(
				ImRect(ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax()),
				ImGui::GetWindowDockID()))
			{
				if (auto _ = ImGui::AcceptDragDropPayload("entity"))
				{
					for (auto& e : Statics::GetSelectedEntities())
					{
						if (auto ent = e.lock())
						{
							Statics::EngineOperations.EditorSetEntityParent(nullptr, ent);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
			{
				ImGui::OpenPopup("HierarchyRightClick");
			}
			DrawRightClickMenu();
		}
		ImGui::End();
	}

	void SceneHierarchy::DrawEntity(const std::shared_ptr<based::scene::Entity>& entity)
	{
		if (!Statics::GetSelectedEntities().empty())
		{
			if (auto back = Statics::GetSelectedEntities().back().lock())
			{
				if (back == entity) mMultiSelectBegin = mCurrentIndex;
			}
		}

		int32_t min = std::min(mMultiSelectBegin, mMultiSelectEnd);
		int32_t max = std::max(mMultiSelectBegin, mMultiSelectEnd);
		mCountMax = max - min;
		if (mMultiSelectEnd != -1 && mCurrentIndex >= min 
			&& mCurrentIndex <= max)
		{
			auto entities = Statics::GetSelectedEntities();
			entities.emplace_back(entity);
			if (!Statics::SelectedEntitiesContains(entities))
				Statics::SetSelectedEntities(entities);
			mCurrentCount++;

			if (mCurrentCount >= mCountMax + 1)
			{
				mMultiSelectBegin = -1;
				mMultiSelectEnd = -1;
			}
		}

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (entity->Children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
		if (Statics::SelectedEntitiesContains(entity)) flags |= ImGuiTreeNodeFlags_Selected;

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			mRenameIndex = mCurrentIndex - 1;

		if (mRenameIndex == mCurrentIndex)
		{
			std::string temp = entity->GetEntityName();
			char* buffer = (char*)temp.c_str();
			ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
			if (ImGui::InputText("", buffer, IM_ARRAYSIZE(buffer), textFlags))
			{
				auto name = std::string(buffer);
				if (!name.empty())
				{
					Statics::EngineOperations.EditorSetEntityName(entity, name);
					mRenameIndex = -1;
				}
			}
			ImGui::SetKeyboardFocusHere(-1);
			return;
		}

		if (!entity->IsActive()) ImGui::PushStyleColor(ImGuiCol_Text, { 0.5f, 0.5f, 0.5f, 1.f });
		auto isOpen = ImGui::TreeNodeEx(entity->GetEntityName().c_str(), flags);
		if (!entity->IsActive()) ImGui::PopStyleColor();

		if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) &&
			!(based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT)
				|| based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL)))
		{
			if (!Statics::SelectedEntitiesContains(entity))
				Statics::SetSelectedEntities({ entity });
			mRenameIndex = -1;
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)
			&& based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL))
		{
			auto entities = Statics::GetSelectedEntities();
			entities.emplace_back(entity);
			if (!Statics::SelectedEntitiesContains(entity))
			{
				Statics::SetSelectedEntities(entities);
			}
			else
			{
				Statics::RemoveEntityFromSelected(entity);
			}
		} else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)
			&& based::input::Keyboard::Key(BASED_INPUT_KEY_LSHIFT))
		{
			mMultiSelectEnd = mCurrentIndex;
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
				if (isOpen)
				{
					++mCurrentIndex;
					ImGui::PushID((uint32_t)child->GetEntityHandle());
					DrawEntity(child);
					ImGui::PopID();
				}
			}
		}

		if (isOpen) 
			ImGui::TreePop();
	}

	void SceneHierarchy::DrawRightClickMenu()
	{
		if (ImGui::BeginPopup("HierarchyRightClick"))
		{
			if (ImGui::Button("Add Entity", ImVec2(200.f, 0.f)))
			{
				auto selections = Statics::GetSelectedEntities();
				if (!selections.empty())
				{
					for (auto& e : selections)
					{
						if (auto entity = e.lock())
						{
							std::string name;
							Statics::EngineOperations.EditorCreateEntity(&name);
							auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
							auto newEnt = scene->GetEntityStorage().Get(name);
							newEnt->SetParent(entity);
						}
					}
				} else
				{
					std::string name;
					Statics::EngineOperations.EditorCreateEntity(&name);
					mRenameIndex = 1;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}
