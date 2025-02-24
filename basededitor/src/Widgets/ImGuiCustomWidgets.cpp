#include "based/pch.h"
#include "ImGuiCustomWidgets.h"

#include "../editorstatics.h"
#include "based/app.h"
#include "based/engine.h"
#include "based/graphics/material.h"
#include "based/graphics/mesh.h"
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
}
