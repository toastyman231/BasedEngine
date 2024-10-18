#pragma once

#include "graphics/rendercommands.h"

#define BASED_SUBMIT_RC(type, ...) std::move(std::make_unique<based::graphics::rendercommands::type>(__VA_ARGS__))

namespace based::managers
{
	class RenderManager
	{
		friend class graphics::rendercommands::PushFramebuffer;
		friend class graphics::rendercommands::PopFramebuffer;
		friend class graphics::rendercommands::PushCamera;
		friend class graphics::rendercommands::PopCamera;

	public:
		RenderManager() {}
		~RenderManager() {}

		const graphics::Camera* GetActiveCamera() const;

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
		void PushCamera(std::shared_ptr<graphics::Camera> camera);
		void PopCamera();

		glm::mat4 lightSpaceMatrix;
	private:
		void ConfigureShaderAndMatrices();

		std::queue<std::unique_ptr<graphics::rendercommands::RenderCommand>> mRenderCommands;
		std::stack<std::shared_ptr<graphics::Framebuffer>> mFramebuffers;
		std::stack<std::shared_ptr<graphics::Camera>> mCameras;
	};
}