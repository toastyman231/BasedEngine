#include "managers/rendermanager.h"

#include "graphics/helpers.h"
#include "graphics/framebuffer.h"

#include "engine.h"
#include "log.h"
#include "glad/glad.h"

namespace based::managers
{
	void RenderManager::Initialize()
	{
		BASED_INFO("OpenGL Info:\n  Vendor:\t{}\n  Renderer:\t{}\n  Version:\t{}",
			(const char*) glGetString(GL_VENDOR), 
			(const char*) glGetString(GL_RENDERER),
			(const char*) glGetString(GL_VERSION)); 

		// Initialize OpenGL
		glEnable(GL_DEPTH_TEST); BASED_CHECK_GL_ERROR;
		glDepthFunc(GL_LEQUAL); BASED_CHECK_GL_ERROR;

		glEnable(GL_BLEND); BASED_CHECK_GL_ERROR;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); BASED_CHECK_GL_ERROR;

		SetClearColor({
			static_cast<float>(0x64) / static_cast<float>(0xFF),
			static_cast<float>(0x95) / static_cast<float>(0xFF),
			static_cast<float>(0xED) / static_cast<float>(0xFF),
			1 }
		);
	}

	void RenderManager::Shutdown()
	{
		while (mRenderCommands.size() > 0)
		{
			mRenderCommands.pop();
		}
	}

	void RenderManager::Clear()
	{
		BASED_ASSERT(mRenderCommands.size() == 0, "Unflushed render commands in queue!");
		while (mRenderCommands.size() > 0)
		{
			mRenderCommands.pop();
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); BASED_CHECK_GL_ERROR;
	}

	void RenderManager::SetViewport(const glm::ivec4 dimensions)
	{
		glViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w); BASED_CHECK_GL_ERROR;
	}

	void RenderManager::SetClearColor(const glm::vec4 clearColor)
	{
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a); BASED_CHECK_GL_ERROR;
	}

	void RenderManager::SetWireframeMode(bool enabled)
	{
		if (enabled)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); BASED_CHECK_GL_ERROR;
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); BASED_CHECK_GL_ERROR;
		}
	}

	void RenderManager::Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc)
	{
		mRenderCommands.push(std::move(rc));
	}

	void RenderManager::Flush()
	{
		while (mRenderCommands.size() > 0)
		{
			auto rc = std::move(mRenderCommands.front());
			mRenderCommands.pop();

			rc->Execute();
		}
	}

	void RenderManager::PushFramebuffer(std::shared_ptr<graphics::Framebuffer> framebuffer)
	{
		mFramebuffers.push(framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->GetFbo()); BASED_CHECK_GL_ERROR;
		SetViewport({ 0, 0, framebuffer->GetSize().x, framebuffer->GetSize().y });

		auto cc = framebuffer->GetClearColor(); 
		glClearColor(cc.r, cc.g, cc.b, cc.a); BASED_CHECK_GL_ERROR;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); BASED_CHECK_GL_ERROR;
	}

	void RenderManager::PopFramebuffer()
	{
		BASED_ASSERT(mFramebuffers.size() > 0, "RenderManager::PopFramebuffer - empty stack");
		if (mFramebuffers.size() > 0)
		{
			mFramebuffers.pop();
			if (mFramebuffers.size() > 0)
			{
				auto nextfb = mFramebuffers.top();
				glBindFramebuffer(GL_FRAMEBUFFER, nextfb->GetFbo()); BASED_CHECK_GL_ERROR;
				SetViewport({ 0, 0, nextfb->GetSize().x, nextfb->GetSize().y });
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0); BASED_CHECK_GL_ERROR;
				auto& window = Engine::Instance().GetWindow();
				SetViewport({ 0, 0, window.GetSize().x, window.GetSize().y });
			}
		}
	}
}