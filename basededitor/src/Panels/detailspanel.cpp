#include "based/pch.h"
#include "detailspanel.h"

#include "../editorstatics.h"

namespace editor::panels
{
	void DetailsPanel::Render()
	{
		if (ImGui::Begin(mPanelTitle.c_str()))
		{
			auto selections = editor::Statics::GetSelectedEntities();

			if (selections.empty()) { ImGui::End(); return; }

			auto entity = selections.front().lock();
			if (!entity) { ImGui::End(); return; }

			ImGui::PushID((uint32_t)entity->GetEntityHandle());

			bool isActive = entity->IsActive();
			ImGui::Checkbox("", &isActive);
			Statics::EngineOperations.EditorSetEntityActive(entity, isActive);

			std::string name = entity->GetEntityName();
			char* buffer = (char*)name.c_str();

			ImGui::SameLine();
			if (ImGui::InputText("", buffer, IM_ARRAYSIZE(buffer)))
			{
				Statics::EngineOperations.EditorSetEntityName(entity, std::string(buffer));
			}

			//auto& transform = entity->GetComponent<based::scene::Transform>();
			ImGui::PopID();
		}
		ImGui::End();
	}
}
