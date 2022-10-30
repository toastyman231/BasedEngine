#pragma once

#include "graphics/rendercommands.h"
#include "external/glm/glm.hpp"
#include <queue>
#include <stack>

#define BASED_SUBMIT_RC(type, ...) std::move(std::make_unique<based::graphics::rendercommands::type>(__VA_ARGS__))

namespace based::managers
{
	class RenderManager
	{
		friend class graphics::rendercommands::PushFramebuffer;
		friend class graphics::rendercommands::PopFramebuffer;

	public:
		RenderManager() {}
		~RenderManager() {}

		void Initialize();;
		void Shutdown();

		void Clear();
		void SetViewport(const glm::ivec4 dimensions);
		void SetClearColor(const glm::vec4 clearColor);
		void SetWireframeMode(bool enabled);

		void Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc);
		
		// Execute submitted RenderCommands in order received
		// We can extend the API if we need to mitigate performance impact
		void Flush();

		void PushFramebuffer(std::shared_ptr<graphics::Framebuffer> framebuffer);
		void PopFramebuffer();

	private:
		std::queue<std::unique_ptr<graphics::rendercommands::RenderCommand>> mRenderCommands;
		std::stack<std::shared_ptr<graphics::Framebuffer>> mFramebuffers;
	};
}