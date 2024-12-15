#pragma once
#include "framebuffer.h"
#include "material.h"
#include "rendercommands.h"
#include "vertex.h"

#define CRP_NO_OUTPUT "No Output"

namespace based::graphics
{
	class CustomRenderPass
	{
	public:
		CustomRenderPass(std::string name);
		CustomRenderPass(const std::string& name, std::shared_ptr<Framebuffer> buffer);
		CustomRenderPass(const std::string& name, std::shared_ptr<Material> overrideMaterial);
		CustomRenderPass(const std::string& name, std::shared_ptr<Framebuffer> buffer, std::shared_ptr<Material> overrideMaterial);
		virtual ~CustomRenderPass() = default;

		std::shared_ptr<Material> GetOverrideMaterial();
		const std::string& GetPassName();
		void SetOutputName(const std::string& name);

		virtual void BeginRender();
		virtual void Render();
		virtual void EndRender();

	protected:
		std::string mPassName;
		std::string mOutputName = CRP_NO_OUTPUT;
		std::shared_ptr<Framebuffer> mPassBuffer;
		std::shared_ptr<Material> mOverrideMaterial;

		static std::shared_ptr<Framebuffer> mLastFrameBuffer;
	};

	class UIRenderPass : public CustomRenderPass
	{
	public:
		UIRenderPass(std::string name);
		UIRenderPass(std::string name, std::shared_ptr<Framebuffer> buffer);
		~UIRenderPass() override = default;

		void BeginRender() override;
		void Render() override;
		void EndRender() override;
	};

	class PostProcessPass : public CustomRenderPass
	{
	public:
		PostProcessPass(const std::string& name, const std::string& output, std::shared_ptr<Material> material);
		PostProcessPass(const std::string& name, const std::string& output, std::shared_ptr<Material> material, 
		                std::shared_ptr<Framebuffer> buffer);
		~PostProcessPass() override = default;

		void BeginRender() override;
		void Render() override;
		void EndRender() override;

	private:
		std::shared_ptr<VertexArray> mVA;
	};

	class ComputePass : public CustomRenderPass
	{
	public:
		ComputePass(const std::string& name);
		~ComputePass() override = default;

		void BeginRender() override;
		void Render() override;
		void EndRender() override;
	};
}
