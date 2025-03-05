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
		}
		mIsFocused = ImGui::IsWindowFocused();
		ImGui::End();
	}

	void EditorView::Render()
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

			ImGui::SetCursorPos(pos);
			ImGui::Spacing();
			ImGui::Indent(5);
			if (ImGui::Button("##trans", ImVec2(50, 50)))
			{
				mOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			if (ImGui::Button("##rot", ImVec2(50, 50)))
			{
				mOperation = ImGuizmo::OPERATION::ROTATE;
			}
			if (ImGui::Button("##scale", ImVec2(50, 50)))
			{
				mOperation = ImGuizmo::OPERATION::SCALE;
			}
			ImGui::Unindent();

			if (!Statics::GetSelectedEntities().empty())
			{
				if (auto entity = Statics::GetSelectedEntities()[0].lock())
				{
					auto viewMat = mViewCamera->GetViewMatrix();
					auto projMat = mViewCamera->GetProjectionMatrix();
					auto modelMat = entity->GetTransform().GetMatrix();
					auto deltaMat = glm::mat4(1.f);
					ImGuizmo::Manipulate(
						glm::value_ptr(viewMat),
						glm::value_ptr(projMat),
						mOperation, mMode,
						glm::value_ptr(modelMat), glm::value_ptr(deltaMat));
					//ImGuizmo::SetGizmoSizeClipSpace(1.5);
					if (ImGuizmo::IsUsing())
					{
						float trans[3], rot[3], scale[3];
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMat), 
							trans, rot, scale);
						entity->SetPosition({ trans[0], trans[1], trans[2] });
						entity->SetScale({ scale[0], scale[1], scale[2] });
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(deltaMat),
							trans, rot, scale);
						auto temp = entity->GetTransform().Rotation;
						entity->SetRotation({ temp.x - rot[0], temp.y - rot[1], temp.z - rot[2] });
					}
				}
			}
		}
		mIsFocused = ImGui::IsWindowFocused();
		ImGui::End();

	}

	void EditorView::ProcessEvent(BasedEvent event)
	{
		GameView::ProcessEvent(event);

		if (event.EventType == BasedEventType::BASED_EVENT_TRANSLATE)
		{
			mOperation = ImGuizmo::TRANSLATE;
		}
		if (event.EventType == BasedEventType::BASED_EVENT_ROTATE)
		{
			mOperation = ImGuizmo::ROTATE;
		}
		if (event.EventType == BasedEventType::BASED_EVENT_SCALE)
		{
			mOperation = ImGuizmo::SCALE;
		}
	}
}
