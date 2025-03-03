#pragma once
#include "panelbase.h"
#include "based/scene/scene.h"

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

	private:
		void DrawMeshRendererComponent(std::shared_ptr<based::scene::Entity> entity);
		void DrawPointLightComponent(std::shared_ptr<based::scene::Entity> entity);
		void DrawDirectionalLightComponent(std::shared_ptr<based::scene::Entity> entity);
		void DrawCameraComponent(std::shared_ptr<based::scene::Entity> entity);
	};
}
