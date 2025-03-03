#pragma once
#include "based/scene/scene.h"
#include "panelbase.h"
#include "external/imgui/imgui.h"

namespace editor::panels
{
	class SceneHierarchy : public Panel
	{
	public:
		explicit SceneHierarchy(const std::string& title)
			: Panel(title)
		{
		}

		void Initialize() override {}
		void Update(float deltaTime) override {}
		void Render() override;
		void Shutdown() override {}

		void ProcessEvent(BasedEvent event) override;

	private:
		void DrawEntity(const std::shared_ptr<based::scene::Entity>& entity);
		void DrawRightClickMenu();

		int mEmptyPayload = 0;
		bool mReparentCooldown = false;

		int mCurrentIndex = 0;
		int mCurrentCount = 0;
		int mCountMax = 0;

		int32_t mRenameIndex = -1;

		int32_t mMultiSelectBegin = -1;
		int32_t mMultiSelectEnd = -1;
	};
}
