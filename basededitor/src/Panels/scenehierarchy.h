#pragma once
#include "based/scene/scene.h"
#include "panelbase.h"

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

	private:
		void DrawEntity(const std::shared_ptr<based::scene::Entity>& entity);

		int mEmptyPayload = 0;
		bool mReparentCooldown = false;
	};
}
