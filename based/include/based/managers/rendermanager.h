#pragma once

#include "graphics/rendercommands.h"

#include <memory>
#include <stack>
#include <queue>

#include "graphics/renderpass.h"

#define BASED_SUBMIT_RC(type, ...) std::move(std::make_unique<based::graphics::rendercommands::type>(__VA_ARGS__))

namespace based::graphics
{
	class ComputeShader;
}

namespace based::managers
{
	enum class RenderMode : int32_t
	{
		Lit = 0,
		Unlit
	};

	class RenderManager
	{
		friend class graphics::rendercommands::PushFramebuffer;
		friend class graphics::rendercommands::PopFramebuffer;
		friend class graphics::rendercommands::PushCamera;
		friend class graphics::rendercommands::PopCamera;
		friend class graphics::ComputePass;

	public:
		RenderManager() = default;
		~RenderManager() = default;

		std::shared_ptr<graphics::Camera> GetActiveCamera() const;

		void Initialize();;
		void Shutdown();

		void Clear();
		void SetViewport(const glm::ivec4 dimensions);
		void SetClearColor(const glm::vec4 clearColor);
		void SetWireframeMode(bool enabled);
		static void SetDepthFunction(uint32_t func);

		// Render pass handling
		void SetEnablePassInjection(bool enable);
		bool InjectPass(graphics::CustomRenderPass* pass, int index = -1);
		bool RemovePass(int index);
		void IncrementPassCount();
		void ResetPassCount();
		const std::vector<std::unique_ptr<graphics::CustomRenderPass>>& GetRenderPasses();
		const std::string& GetCurrentPassName() const;
		std::shared_ptr<graphics::Material> GetCurrentPassOverrideMaterial() const;
		void PushDebugGroup(const std::string& group) const;
		void PopDebugGroup();

		void Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc);
		
		// Execute submitted RenderCommands in order received
		// We can extend the API if we need to mitigate performance impact
		void Flush();

		void PushFramebuffer(std::shared_ptr<graphics::Framebuffer> framebuffer, bool clear = true);
		void PopFramebuffer();
		void PushCamera(std::shared_ptr<graphics::Camera> camera);
		void PopCamera();

		static void SetRenderMode(RenderMode newMode);
		static RenderMode GetRenderMode();

		void AddComputeShaderDispatch(std::shared_ptr<graphics::ComputeShader> shader, glm::vec3 workGroupSize);

		glm::mat4 lightSpaceMatrix;
	private:
		void ConfigureShaderAndMatrices();

		void DispatchComputeShaders();

		bool mAllowPassInjection;
		uint32_t mCurrentPass = 0;
		const std::string mInvalidPassName = "INVALID PASS";

		std::vector<std::unique_ptr<graphics::CustomRenderPass>> mRenderPasses;
		std::queue<std::unique_ptr<graphics::rendercommands::RenderCommand>> mRenderCommands;
		std::stack<std::shared_ptr<graphics::Framebuffer>> mFramebuffers;
		std::stack<std::shared_ptr<graphics::Camera>> mCameras;
		std::queue<std::shared_ptr<graphics::ComputeShader>> mDispatchedComputeShaders;

		inline static RenderMode mRenderMode;
	};
}
