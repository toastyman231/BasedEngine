#include "based/pch.h"
#include "gameview.h"

#include "../editorstatics.h"
#include "based/app.h"
#include "based/engine.h"

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
		}
		mIsFocused = ImGui::IsWindowFocused();
		ImGui::End();
	}
}
