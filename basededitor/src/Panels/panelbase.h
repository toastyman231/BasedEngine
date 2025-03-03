#pragma once
#include "../basedevent.h"
#include "external/imgui/imgui.h"

namespace editor::panels
{
	class Panel
	{
	public:
		Panel(const std::string& title) : mPanelTitle(title) {}
		virtual ~Panel() = default;

		std::string GetPanelTitle() const { return mPanelTitle; }
		bool IsFocused() const { return mIsFocused; }

		virtual void Initialize() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		virtual void Shutdown() = 0;

		virtual void ProcessEvent(BasedEvent event)
		{
			//if (!ImGui::IsWindowFocused()) return;
		}

	protected:
		std::string mPanelTitle = "Untitled Panel";
		bool mIsFocused = false;
	};
}
