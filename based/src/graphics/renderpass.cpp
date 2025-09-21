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

	CustomRenderPass::CustomRenderPass(const std::string& name, std::shared_ptr<Framebuffer> buffer,
		std::shared_ptr<Material> overrideMaterial, std::shared_ptr<Camera> passCam)
			: CustomRenderPass(name, buffer, overrideMaterial)
	{
		mPassCamera = passCam;
	}

	std::shared_ptr<Material> CustomRenderPass::GetOverrideMaterial()
	{
		return mOverrideMaterial;
	}

	const std::string& CustomRenderPass::GetPassName()
	{
		return mPassName;
	}

	void CustomRenderPass::AddOutputName(const std::string& name)
	{
		mOutputNames.emplace_back(name);
	}

	void CustomRenderPass::BeginRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();

		rm.Submit(BASED_SUBMIT_RC(PushFramebuffer, mPassBuffer, mPassName, mShouldClear));

		if (!mPassCamera)
		{
			mPassCamera = Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera();
			mDidBypassCamera = true;
		}
		rm.Submit(BASED_SUBMIT_RC(PushCamera, mPassCamera));
	}

	void CustomRenderPass::Render()
	{
		if (!mPassCamera)
		{
			BASED_ERROR("Attempting to render with invalid camera!");
			return;
		}

		Engine::Instance().GetApp().GetCurrentScene()->RenderScene(mRenderFlags);
	}

	void CustomRenderPass::EndRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Submit(BASED_SUBMIT_RC(PopCamera));
		rm.Submit(BASED_SUBMIT_RC(PopFramebuffer));
		rm.Flush();

		int index = 0;
		for (auto& name : mOutputNames)
		{
			if (name != CRP_NO_OUTPUT)
			{
				DefaultLibraries::GetRenderPassOutputs().Load(name, mPassBuffer->GetTexture(index), true);
			}
			index++;
		}
		

		if (mDidBypassCamera)
		{
			mPassCamera = nullptr;
			mDidBypassCamera = false;
		}

		mLastFrameBuffer = mPassBuffer;
	}

	void OpaqueMaskedColorPass::BeginRender()
	{
		CustomRenderPass::BeginRender();

		managers::RenderManager::SetBlendingEnabled(false);
	}

	void TranslucentColorPass::BeginRender()
	{
		CustomRenderPass::BeginRender();

		managers::RenderManager::SetBlendingEnabled(true);
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

		int index = 0;
		for (auto& name : mOutputNames)
		{
			if (name != CRP_NO_OUTPUT)
			{
				DefaultLibraries::GetRenderPassOutputs().Load(name, mPassBuffer->GetTexture(index), true);
			}
			index++;
		}

		mLastFrameBuffer = mPassBuffer;
	}

	PostProcessPass::PostProcessPass(const std::string& name, const std::string& output, std::shared_ptr<Material> material)
		: CustomRenderPass(name, Engine::Instance().GetWindow().GetFramebuffer(), std::move(material))
	{
		AddOutputName(output);

		mVA = std::make_shared<VertexArray>();

		mShouldClear = false;

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
		AddOutputName(output);

		mVA = std::make_shared<VertexArray>();

		mShouldClear = false;

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

	ComputePass::ComputePass(const std::string& name) : CustomRenderPass(std::move(name)) {}

	void ComputePass::BeginRender()
	{
		Engine::Instance().GetRenderManager().PushDebugGroup(mPassName);
	}

	void ComputePass::Render()
	{
		Engine::Instance().GetRenderManager().DispatchComputeShaders();
	}

	void ComputePass::EndRender()
	{
		Engine::Instance().GetRenderManager().PopDebugGroup();
	}
}
