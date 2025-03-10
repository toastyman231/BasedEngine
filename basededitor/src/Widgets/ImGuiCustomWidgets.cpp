#include "based/pch.h"
#include "ImGuiCustomWidgets.h"

#include "../editorstatics.h"
#include "based/app.h"
#include "based/engine.h"
#include "based/graphics/material.h"
#include "based/graphics/mesh.h"
#include "based/input/mouse.h"
#include "external/imgui/imgui.h"

namespace ImGui
{
    using namespace based;

    std::string ShortenMeshName(const std::string& path)
    {
        auto slashIndex = path.find_last_of("/");
        if (slashIndex == std::string::npos) slashIndex = path.find_last_of("\\");
        if (slashIndex == std::string::npos) return path;

        return path.substr(slashIndex + 1);
    }

    std::shared_ptr<graphics::Material> ObjectPicker<graphics::Material>(
        const std::string& label, std::shared_ptr<graphics::Material> current)
    {
        static bool selected = true;
        Text(label.c_str()); SameLine();
        Selectable("##nolabel", &selected, ImGuiSelectableFlags_AllowDoubleClick);
        selected = true;

        auto scene = Engine::Instance().GetApp().GetCurrentScene();
        std::shared_ptr<graphics::Material> returnPointer = nullptr;

        if (IsItemHovered() && IsMouseDoubleClicked(0))
        {
            OpenPopup("MaterialPicker");
        }

        if (current)
        {
            SameLine();
            Text(current->mMaterialName.c_str());
        }
        else
        {
            SameLine(); Text("None");
        }

        if (BeginPopup("MaterialPicker"))
        {
            Text("Select Material"); Spacing();
            if (BeginListBox("##nolabel", ImVec2(200, 0)))
            {
                static std::string selectedId = current ? current->mMaterialName : "None";
                for (auto& [id, material] 
                    : scene->GetMaterialStorage().GetAll())
                {
                    const bool isSelected = selectedId == id;
                    if (Selectable(material->mMaterialName.c_str(), 
                        isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(200, 0)))
                    {
                        BASED_TRACE("Selected {}", id);
                        selectedId = id;
                        returnPointer = material;
                        if (IsItemHovered() && IsMouseDoubleClicked(0)) CloseCurrentPopup();
                    }

                    if (isSelected) SetItemDefaultFocus();
                }
                EndListBox();
            }
            EndPopup();
        }

        return returnPointer;
    }

    std::shared_ptr<graphics::Mesh> ObjectPicker<graphics::Mesh>(
        const std::string& label, std::shared_ptr<graphics::Mesh> current)
    {
        static bool selected = true;
        Text(label.c_str()); SameLine();
        Selectable("##nolabel", &selected, ImGuiSelectableFlags_AllowDoubleClick);
        selected = true;

        auto scene = Engine::Instance().GetApp().GetCurrentScene();
        std::shared_ptr<graphics::Mesh> returnPointer = nullptr;

        if (IsItemHovered() && IsMouseDoubleClicked(0))
        {
            OpenPopup("MeshPicker");
        }

        if (current)
        {
            SameLine();
            Text(ShortenMeshName(current->GetMeshSource()).c_str());
        }
        else
        {
            SameLine(); Text("None");
        }

        if (BeginPopup("MeshPicker"))
        {
            Text("Select Mesh"); Spacing();
            if (BeginListBox("##nolabel", ImVec2(200, 0)))
            {
                static std::string selectedId = current ? current->GetMeshName() : "None";
                for (auto& [id, mesh] 
                    : scene->GetMeshStorage().GetAll())
                {
                    const bool isSelected = selectedId == id;
                    if (Selectable(ShortenMeshName(mesh->GetMeshSource()).c_str(),
                        isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(200, 0)))
                    {
                        BASED_TRACE("Selected {}", id);
                        selectedId = id;
                        returnPointer = mesh;
                        if (IsItemHovered() && IsMouseDoubleClicked(0)) CloseCurrentPopup();
                    }

                    if (isSelected) SetItemDefaultFocus();
                }
                EndListBox();
            }
            EndPopup();
        }

        return returnPointer;
    }

    bool ToggleButton(uint32_t textureId, ImVec2 size, bool isSelected)
    {
        bool wasPressed = false;
        if (isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3 / 7.0f, 1, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3 / 7.0f, 1, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3 / 7.0f, 0.5f, 0.5f));
        }
        if (ImGui::ImageButton((void*)static_cast<intptr_t>(textureId),
            size))
        {
            isSelected = !isSelected;
            wasPressed = true;
        }
        if (isSelected && !wasPressed)
        {
            ImGui::PopStyleColor(3);
        }
        return wasPressed;
    }

    bool TransformEditor(
        float pos[3],
        float oldPos[3],
        float rot[3],
        float oldRot[3],
        float scale[3],
        float oldScale[3],
        float speed)
    {
        static bool isDragging = false;
        static float savedPos[3] = {pos[0], pos[1], pos[2]};
        static float savedRot[3] = { rot[0], rot[1], rot[2] };
        static float savedScale[3] = { scale[0], scale[1], scale[2] };

        ImGui::Text("Position"); ImGui::SameLine();
        if (ImGui::DragFloat3("##pos", pos, speed))
        {
	        if (!isDragging)
	        {
                isDragging = true;
                savedPos[0] = pos[0];
                savedPos[1] = pos[1];
                savedPos[2] = pos[2];
	        }

        }
        ImGui::Text("Rotation"); ImGui::SameLine();
        if (ImGui::DragFloat3("##rot", rot, speed))
        {
            if (!isDragging)
            {
                isDragging = true;
                savedRot[0] = rot[0];
                savedRot[1] = rot[1];
                savedRot[2] = rot[2];
            }

            //based::input::Mouse::SetCursorMode(input::CursorMode::Confined);
        }
        ImGui::Text("Scale"); ImGui::SameLine();
        if (ImGui::DragFloat3("##scale", scale, speed))
        {
            if (!isDragging)
            {
                isDragging = true;
                savedScale[0] = scale[0];
                savedScale[1] = scale[1];
                savedScale[2] = scale[2];
            }
        }

        oldPos = savedPos;
        oldRot = savedRot;
        oldScale = savedScale;

        if (isDragging && ImGui::IsMouseReleased(0))
        {
            isDragging = false;
            //based::input::Mouse::SetCursorMode(input::CursorMode::Free);
            return true;
        }

        return false;
    }
}
