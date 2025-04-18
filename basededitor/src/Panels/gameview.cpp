#include "based/pch.h"
#include "gameview.h"

#include "../EditorComponents.h"
#include "../editorstatics.h"
#include "../external/imguizmo/ImGuizmo.h"
#include "../Widgets/ImGuiCustomWidgets.h"
#include "based/app.h"
#include "based/engine.h"
#include "based/input/joystick.h"
#include "based/input/keyboard.h"
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
		using namespace based;

		if (ImGui::Begin(mPanelTitle.c_str()))
		{
			if (ImGui::IsWindowHovered())
			{
				ImGui::CaptureMouseFromApp(false);
				if (ImGui::IsMouseDown(1)) // Allow right mouse button to grab focus
				{
					ImGui::SetWindowFocus();
				}
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

			ImVec2 buttonSize(25, 25);

			ImGui::SetCursorPos(pos);
			ImGui::Spacing();
			ImGui::Indent(5);
			if (ImGui::ToggleButton(mTranslateIcon->GetId(), buttonSize,
				mOperation == ImGuizmo::TRANSLATE))
			{
				mOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton(
				mMode == ImGuizmo::WORLD ?
				(void*)static_cast<intptr_t>(mGlobalIcon->GetId())
				: (void*)static_cast<intptr_t>(mLocalIcon->GetId()),
				buttonSize))
			{
				if (mMode == ImGuizmo::WORLD)
				{
					mMode = ImGuizmo::LOCAL;
				} else
				{
					mMode = ImGuizmo::WORLD;
				}
			}
			if (ImGui::ToggleButton(mRotateIcon->GetId(), buttonSize,
				mOperation == ImGuizmo::ROTATE))
			{
				mOperation = ImGuizmo::OPERATION::ROTATE;
			}
			if (ImGui::ToggleButton(mScaleIcon->GetId(), buttonSize,
				mOperation == ImGuizmo::SCALE))
			{
				mOperation = ImGuizmo::OPERATION::SCALE;
			}
			ImGui::Unindent();

			ImGui::SetCursorPos(ImVec2(pos.x + (size.x - buttonSize.x) - 15.f, pos.y + 5.f));
			static bool t = false;
			ImGui::ToggleButton(mScaleIcon->GetId(), buttonSize, t);

			if (!Statics::GetSelectedEntities().empty())
			{
				if (auto entity = Statics::GetSelectedEntities()[0].lock())
				{
					const bool isChild = !entity->Parent.expired();
					static bool wasUsingLastFrame = false;

					scene::Transform savedTransform;
					if (Statics::GetSavedTransforms().find(entity->GetUUID()) == Statics::GetSavedTransforms().end())
					{
						auto& trans = entity->GetTransform();
						savedTransform = Statics::GetSavedTransforms()[entity->GetUUID()] = 
							scene::Transform(trans.Position(), trans.EulerAngles(), trans.Scale());
					}
					else
						savedTransform = Statics::GetSavedTransforms()[entity->GetUUID()];

					auto viewMat = mViewCamera->GetViewMatrix();
					auto projMat = mViewCamera->GetProjectionMatrix();
					auto modelMat = entity->GetTransform().GetGlobalMatrix();
					auto deltaMat = glm::mat4(0.f);
					auto snap = GetSnap();
					ImGuizmo::SetDrawlist();
					ImGuizmo::Manipulate(
						glm::value_ptr(viewMat),
						glm::value_ptr(projMat),
						mOperation, mOperation == ImGuizmo::SCALE ? ImGuizmo::LOCAL : mMode,
						glm::value_ptr(modelMat), glm::value_ptr(deltaMat),
						input::Keyboard::Key(BASED_INPUT_KEY_LCTRL) ? glm::value_ptr(snap) : nullptr);
					ImGuizmo::SetGizmoSizeClipSpace(0.25f);

					if (based::input::Keyboard::Key(BASED_INPUT_KEY_LCTRL) &&
						(input::Keyboard::KeyDown(BASED_INPUT_KEY_Z)
							|| input::Keyboard::KeyDown(BASED_INPUT_KEY_Y)))
					{
						auto& trans = entity->GetTransform();
						Statics::GetSavedTransforms()[entity->GetUUID()] = 
							scene::Transform(trans.Position(), trans.EulerAngles(), trans.Scale());
					}

					if (ImGuizmo::IsUsing())
					{
						float trans[3], rot[3], scale[3];
						float dTrans[3], dRot[3], dScale[3];
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMat), 
							trans, rot, scale);
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(deltaMat),
							dTrans, dRot, dScale);
						auto& transform = entity->GetTransform();
						
						transform.SetGlobalTransformFromMatrix(modelMat);
						wasUsingLastFrame = true;
					} else if (wasUsingLastFrame)
					{
						// TODO: This overwrites the matrix
						/*Statics::EngineOperations.EditorSetEntityTransform(
							entity,
							entity->GetTransform(),
							savedTransform,
							isChild
						);
						auto& trans = entity->GetTransform();
						Statics::GetSavedTransforms()[entity->GetUUID()] =
							scene::Transform(trans.Position(), trans.EulerAngles(), trans.Scale());
						wasUsingLastFrame = false;*/
					}
				}
			}
		}
		mIsFocused = ImGui::IsWindowFocused() && !ImGuizmo::IsUsing();
		ImGui::End();

	}

	void EditorView::ProcessEvent(BasedEvent event)
	{
		GameView::ProcessEvent(event);

		if (!(based::input::Mouse::Button(BASED_INPUT_MOUSE_RIGHT) ||
			based::input::Mouse::Button(BASED_INPUT_MOUSE_LEFT)))
		{
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

	glm::vec3 EditorView::GetSnap()
	{
		switch (mOperation)
		{
		case ImGuizmo::TRANSLATE:
			return Statics::GetEditorSettings().mPositionSnap;
		case ImGuizmo::ROTATE:
			return Statics::GetEditorSettings().mRotationSnap;
		case ImGuizmo::SCALE:
			return Statics::GetEditorSettings().mScaleSnap;
		default:
			return glm::vec3(1.f);
		}
	}
}
