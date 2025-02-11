#include "based/pch.h"
#include "menubar.h"

#include "external/imgui/imgui.h"

namespace editor::panels
{
	void MenuBar::Render()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("Open Scene", nullptr);
				ImGui::MenuItem("Save Scene", nullptr);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Engine Settings", nullptr);
				ImGui::MenuItem("Project Settings", nullptr);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}
}
