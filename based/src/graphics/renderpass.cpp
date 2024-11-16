#include "pch.h"
#include "graphics/renderpass.h"

#include "app.h"
#include "engine.h"
#include "graphics/defaultassetlibraries.h"

namespace based::graphics
{
	std::shared_ptr<Framebuffer> CustomRenderPass::mLastFrameBuffer = nullptr;

	CustomRenderPass::CustomRenderPass(std::string name)
		: mPassName(std::move(name))
	{
		auto size = Engine::Instance().GetWindow().GetSize();
		mPassBuffer = std::make_shared<Framebuffer>(size.x, size.y);
	}

	CustomRenderPass::CustomRenderPass(const std::string& name, std::shared_ptr<Framebuffer> buffer)
		: mPassName(name)
		, mPassBuffer(std::move(buffer))
	{}

	CustomRenderPass::CustomRenderPass(const std::string& name, std::shared_ptr<Material> overrideMaterial)
		: mPassName(name)
		, mOverrideMaterial(std::move(overrideMaterial))
	{
		auto size = Engine::Instance().GetWindow().GetSize();
		mPassBuffer = std::make_shared<Framebuffer>(size.x, size.y);
	}

	CustomRenderPass::CustomRenderPass(const std::string& name, std::shared_ptr<Framebuffer> buffer,
		std::shared_ptr<Material> overrideMaterial)
		: mPassName(name)
		, mPassBuffer(std::move(buffer))
		, mOverrideMaterial(std::move(overrideMaterial))
	{}

	std::shared_ptr<Material> CustomRenderPass::GetOverrideMaterial()
	{
		return mOverrideMaterial;
	}

	const std::string& CustomRenderPass::GetPassName()
	{
		return mPassName;
	}

	void CustomRenderPass::SetOutputName(const std::string& name)
	{
		mOutputName = name;
	}

	void CustomRenderPass::BeginRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();

		rm.Submit(BASED_SUBMIT_RC(PushFramebuffer, mPassBuffer, mPassName));
	}

	void CustomRenderPass::Render()
	{
		Engine::Instance().GetApp().GetCurrentScene()->RenderScene();
	}

	void CustomRenderPass::EndRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Submit(BASED_SUBMIT_RC(PopFramebuffer));
		rm.Flush();

		if (mOutputName != CRP_NO_OUTPUT)
		{
			graphics::DefaultLibraries::GetRenderPassOutputs().Load(mOutputName, mPassBuffer->GetTextureId(), true);
		}

		mLastFrameBuffer = mPassBuffer;
	}

	UIRenderPass::UIRenderPass(std::string name) : CustomRenderPass(std::move(name)) {}

	UIRenderPass::UIRenderPass(std::string name, std::shared_ptr<Framebuffer> buffer)
		: CustomRenderPass(name, std::move(buffer))
	{
	}

	void UIRenderPass::BeginRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();

		rm.Submit(BASED_SUBMIT_RC(PushFramebuffer, mLastFrameBuffer, mPassName, false));

		return;
	}

	void UIRenderPass::Render()
	{
		Engine::Instance().GetUiManager().Render();
	}

	void UIRenderPass::EndRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Submit(BASED_SUBMIT_RC(PopFramebuffer));
		rm.Flush();

		if (mOutputName != CRP_NO_OUTPUT)
		{
			graphics::DefaultLibraries::GetRenderPassOutputs().Load(mOutputName, mLastFrameBuffer->GetTextureId(), true);
		}

		mLastFrameBuffer = mPassBuffer;
	}

	PostProcessPass::PostProcessPass(const std::string& name, const std::string& output, std::shared_ptr<Material> material)
		: CustomRenderPass(name, std::move(material))
	{
		mOutputName = output;

		mVA = std::make_shared<VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(vb, short);
		vb->PushVertex({ 1, 1, 1, 1 });
		vb->PushVertex({ 1, -1, 1, 0 });
		vb->PushVertex({ -1, -1, 0, 0 });
		vb->PushVertex({ -1, 1, 0, 1 });
		vb->SetLayout({ 2, 2 });
		mVA->PushBuffer(std::move(vb));
		mVA->SetElements({ 0, 3, 1, 1, 3, 2 });
		mVA->Upload();
	}

	PostProcessPass::PostProcessPass(const std::string& name, const std::string& output, std::shared_ptr<Material> material, 
	                                 std::shared_ptr<Framebuffer> buffer)
		: CustomRenderPass(name, std::move(buffer), std::move(material))
	{
		mOutputName = output;

		mVA = std::make_shared<VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(vb, short);
		vb->PushVertex({ 1, 1, 1, 1 });
		vb->PushVertex({ 1, -1, 1, 0 });
		vb->PushVertex({ -1, -1, 0, 0 });
		vb->PushVertex({ -1, 1, 0, 1 });
		vb->SetLayout({ 2, 2 });
		mVA->PushBuffer(std::move(vb));
		mVA->SetElements({ 0, 3, 1, 1, 3, 2 });
		mVA->Upload();
	}

	void PostProcessPass::BeginRender()
	{
		CustomRenderPass::BeginRender();
	}

	void PostProcessPass::Render()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Submit(BASED_SUBMIT_RC(RenderVertexArrayPostProcess, mVA, mOverrideMaterial));
	}

	void PostProcessPass::EndRender()
	{
		CustomRenderPass::EndRender();
	}
}
