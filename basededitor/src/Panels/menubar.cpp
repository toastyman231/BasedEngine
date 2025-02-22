#include "based/pch.h"
#include "menubar.h"

#include "../editorstatics.h"
#include "external/imgui/imgui.h"

namespace editor::panels
{
	void MenuBar::Render()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", nullptr)) Statics::NewScene();
				if (ImGui::MenuItem("Open Scene", nullptr)) Statics::OpenScene();
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) Statics::SaveScene("FORCE");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo Last Action", "Ctrl+Z")) Statics::GetHistory().Undo();
				if (ImGui::MenuItem("Redo Last Action", "Ctrl+Y")) Statics::GetHistory().Redo();
				ImGui::MenuItem("Engine Settings", nullptr);
				ImGui::MenuItem("Project Settings", nullptr);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}
}
