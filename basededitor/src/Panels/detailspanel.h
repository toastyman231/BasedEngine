#pragma once
#include "panelbase.h"

namespace editor::panels
{
	class DetailsPanel : public Panel
	{
	public:
		explicit DetailsPanel(const std::string& title)
			: Panel(title)
		{
		}

		void Initialize() override {}
		void Update(float deltaTime) override {}
		void Render() override;
		void Shutdown() override {}
	};
}
