#include <utility>

#include "pch.h"
#include "graphics/renderpass.h"

#include "app.h"
#include "engine.h"
#include "graphics/defaultassetlibraries.h"

namespace based::graphics
{
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
			graphics::DefaultLibraries::GetRenderPassOutputs().Load(mOutputName, mPassBuffer->GetTextureId());
		}
	}

	UIRenderPass::UIRenderPass(std::string name) : CustomRenderPass(std::move(name)) {}

	UIRenderPass::UIRenderPass(std::string name, std::shared_ptr<Framebuffer> buffer)
		: CustomRenderPass(name, std::move(buffer))
	{
	}

	void UIRenderPass::BeginRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();

		rm.Submit(BASED_SUBMIT_RC(PushFramebuffer, mPassBuffer, mPassName, false));

		return;
	}

	void UIRenderPass::Render()
	{
		Engine::Instance().GetUiManager().Render();
	}

	void UIRenderPass::EndRender()
	{
		CustomRenderPass::EndRender();
	}
}
