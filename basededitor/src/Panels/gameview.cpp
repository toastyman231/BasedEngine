#include "based/pch.h"
#include "gameview.h"

#include "../editorstatics.h"
#include "../external/imguizmo/ImGuizmo.h"
#include "based/app.h"
#include "based/engine.h"
#include "based/input/joystick.h"
#include "based/input/mouse.h"

namespace editor::panels
{
	void GameView::Render()
	{
		if (ImGui::Begin(mPanelTitle.c_str()))
		{
			if (ImGui::IsWindowHovered())
			{
				ImGui::CaptureMouseFromApp(false);
			}

			auto& window = based::Engine::Instance().GetWindow();

			ImVec2 winsize = ImGui::GetWindowSize();
			glm::ivec2 arsize = window.GetSizeInAspectRatio(static_cast<int>(winsize.x) - 15,
				static_cast<int>(winsize.y) - 35);
			ImVec2 size = { static_cast<float>(arsize.x), static_cast<float>(arsize.y) };
			ImVec2 pos = {
				(winsize.x - size.x) * 0.5f,
				((winsize.y - size.y) * 0.5f) + 10
			};
			ImVec2 uv0 = { 0, 1 };
			ImVec2 uv1 = { 1, 0 };
			ImGui::SetCursorPos(pos);
			ImGui::Image((void*)static_cast<intptr_t>(mBackingBuffer->GetTextureId()),
				size, uv0, uv1);
			based::input::Mouse::xBounds = {
				ImGui::GetItemRectMin().x,
				ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x };
			based::input::Mouse::yBounds = {
				ImGui::GetItemRectMin().y,
				ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y
			};
			ImGuizmo::SetRect(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y, 
				ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y);

			if (!Statics::GetSelectedEntities().empty())
			{
				if (auto entity = Statics::GetSelectedEntities()[0].lock())
				{
					auto viewMat = mViewCamera->GetViewMatrix();
					auto projMat = mViewCamera->GetProjectionMatrix();
					auto modelMat = entity->GetTransform().GetMatrix();
					ImGuizmo::Manipulate(
						glm::value_ptr(viewMat),
						glm::value_ptr(projMat),
						ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD,
						glm::value_ptr(modelMat));
					if (ImGuizmo::IsUsing())
					{
						float trans[3], rot[3], scale[3];
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMat), trans, rot, scale);
						entity->SetTransform({ trans[0], trans[1], trans[2] },
							{ rot[0], rot[1], rot[2] },
							{ scale[0], scale[1], scale[2] });
					}
				}
			}
		}
		mIsFocused = ImGui::IsWindowFocused();
		ImGui::End();
	}
}
