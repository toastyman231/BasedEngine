#pragma once

#include "graphics/rendercommands.h"

#include <queue>

namespace based::managers
{
	class RenderManager
	{
	public:
		RenderManager() {}
		~RenderManager() {}

		void Initialize();;
		void Shutdown();

		void Clear();
		void SetClearColor(float r, float g, float b, float a);
		void SetWireframeMode(bool enabled);

		void Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc);
		
		// Execute submitted RenderCommands in order received
		// We can extend the API if we need to mitigate performance impact
		void Flush();

	private:
		std::queue<std::unique_ptr<graphics::rendercommands::RenderCommand>> mRenderCommands;
	};
}