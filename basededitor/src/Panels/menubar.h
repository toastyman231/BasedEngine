#pragma once
#include "panelbase.h"

namespace editor::panels
{
	class MenuBar : public Panel
	{
	public:
		explicit MenuBar(const std::string& title)
			: Panel(title)
		{}

		void Initialize() override {}
		void Update(float deltaTime) override {}
		void Render() override;
		void Shutdown() override {}
	};
}
