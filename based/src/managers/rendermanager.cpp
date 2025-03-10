#include "pch.h"
#include "managers/rendermanager.h"

#include "app.h"
#include "graphics/framebuffer.h"

#include "engine.h"
#include "glad/glad.h"
#include "graphics/defaultassetlibraries.h"
#include "graphics/shader.h"

#include "math/basedmath.h"

namespace based::managers
{
	void RenderManager::Initialize()
	{
		PROFILE_FUNCTION();
		BASED_INFO("OpenGL Info:\n  Vendor:\t{}\n  Renderer:\t{}\n  Version:\t{}",
			(const char*) glGetString(GL_VENDOR), 
			(const char*) glGetString(GL_RENDERER),
			(const char*) glGetString(GL_VERSION)); 

		// Initialize OpenGL
		glEnable(GL_DEPTH_TEST); BASED_CHECK_GL_ERROR;
		glDepthFunc(GL_LEQUAL); BASED_CHECK_GL_ERROR;

		glEnable(GL_BLEND); BASED_CHECK_GL_ERROR;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); BASED_CHECK_GL_ERROR;

#ifdef BASED_CONFIG_DEBUG
		glEnable(GL_DEBUG_OUTPUT); BASED_CHECK_GL_ERROR;
#endif

		SetClearColor({
			static_cast<float>(0x64) / static_cast<float>(0xFF),
			static_cast<float>(0x95) / static_cast<float>(0xFF),
			static_cast<float>(0xED) / static_cast<float>(0xFF),
			1 }
		);
	}

	void RenderManager::Shutdown()
	{
		PROFILE_FUNCTION();
		while (mRenderCommands.size() > 0)
		{
			mRenderCommands.pop();
		}
	}

	void RenderManager::Clear()
	{
		PROFILE_FUNCTION();
		/*if (!mRenderCommands.empty()) 
			BASED_WARN("Unflushed render commands in queue! Emptying queue without executing!");*/
		while (!mRenderCommands.empty())
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

	void RenderManager::SetDepthFunction(uint32_t func)
	{
		glDepthFunc(func); BASED_CHECK_GL_ERROR;
	}

	void RenderManager::SetEnablePassInjection(bool enable)
	{
		mAllowPassInjection = enable;
	}

	bool RenderManager::InjectPass(graphics::CustomRenderPass* pass, int index)
	{
		if (!mAllowPassInjection)
		{
			BASED_WARN("Attempting to inject a pass while rendering - skipping!");
			return false;
		}

		if (index < 0)
		{
			mRenderPasses.insert(mRenderPasses.end(), std::unique_ptr<graphics::CustomRenderPass>(pass));
			return true;
		}

		mRenderPasses.insert(mRenderPasses.begin() + index, std::unique_ptr<graphics::CustomRenderPass>(pass));

		return true;
	}

	bool RenderManager::RemovePass(int index)
	{
		if (index < 0 || index > mRenderPasses.size()) return false;

		mRenderPasses.erase(mRenderPasses.begin() + index);
		return false;
	}

	void RenderManager::IncrementPassCount()
	{
		mCurrentPass++;
	}

	void RenderManager::ResetPassCount()
	{
		mCurrentPass = 0;
	}

	const std::vector<std::unique_ptr<graphics::CustomRenderPass>>& RenderManager::GetRenderPasses()
	{
		return mRenderPasses;
	}

	const std::string& RenderManager::GetCurrentPassName() const
	{
		if (mCurrentPass >= mRenderPasses.size()) return mInvalidPassName;

		return mRenderPasses[mCurrentPass]->GetPassName();
	}

	std::shared_ptr<graphics::Material> RenderManager::GetCurrentPassOverrideMaterial() const
	{
		if (mCurrentPass >= mRenderPasses.size()) return nullptr;

		return mRenderPasses[mCurrentPass]->GetOverrideMaterial();
	}

	void RenderManager::PushDebugGroup(const std::string& group) const
	{
#ifdef BASED_CONFIG_DEBUG
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, mCurrentPass, static_cast<GLsizei>(group.length()), 
			group.c_str()); BASED_CHECK_GL_ERROR;
#endif
	}

	void RenderManager::PopDebugGroup()
	{
#ifdef BASED_CONFIG_DEBUG
		glPopDebugGroup(); BASED_CHECK_GL_ERROR;
#endif
	}

	void RenderManager::Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc)
	{
		PROFILE_FUNCTION();
		mRenderCommands.push(std::move(rc));
	}

	void RenderManager::Flush()
	{
		PROFILE_FUNCTION();
		while (!mRenderCommands.empty())
		{
			auto rc = std::move(mRenderCommands.front());
			mRenderCommands.pop();

			rc->Execute();
		}
	}

	void RenderManager::PushFramebuffer(std::shared_ptr<graphics::Framebuffer> framebuffer, bool clear)
	{
		PROFILE_FUNCTION();
		BASED_ASSERT(framebuffer, "Framebuffer is null");
		mFramebuffers.push(framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->GetFbo()); BASED_CHECK_GL_ERROR;
		SetViewport({ 0, 0, framebuffer->GetSize().x, framebuffer->GetSize().y });

		if (clear)
		{
			auto cc = framebuffer->GetClearColor();
			glClearColor(cc.r, cc.g, cc.b, cc.a); BASED_CHECK_GL_ERROR;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); BASED_CHECK_GL_ERROR;
		}

		ConfigureShaderAndMatrices();
	}

	void RenderManager::PopFramebuffer()
	{
		PROFILE_FUNCTION();
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

	void RenderManager::PushCamera(std::shared_ptr<graphics::Camera> camera)
	{
		PROFILE_FUNCTION();
		BASED_ASSERT(camera, "Camera is null");
		mCameras.push(camera);
	}

	void RenderManager::PopCamera()
	{
		PROFILE_FUNCTION();
		//BASED_ASSERT(mCameras.size() > 0, "RenderManager::PopCamera - empty stack");
		if (mCameras.size() > 0)
		{
			mCameras.pop();
		}
	}

	void RenderManager::SetRenderMode(RenderMode newMode)
	{
		mRenderMode = newMode;
	}

	RenderMode RenderManager::GetRenderMode()
	{
		return mRenderMode;
	}

	void RenderManager::AddComputeShaderDispatch(std::shared_ptr<graphics::ComputeShader> shader, glm::vec3 workGroupSize)
	{
		shader->SetWorkGroupSize(workGroupSize);
		mDispatchedComputeShaders.push(shader);
	}

	void RenderManager::ConfigureShaderAndMatrices()
	{
		constexpr float nearPlane = 1.f;
		constexpr float farPlane = 100.f;
		glm::vec3 lightPosition = glm::vec3(-2.f, 4.f, -1.f);
		entt::registry& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		auto directionalLightView = registry.view<scene::DirectionalLight>();
		if (directionalLightView.size() > 0)
		{
			auto& directionalLight = registry.get<scene::Transform>(directionalLightView.front());
			lightPosition = -directionalLight.EulerAngles();
		}
		const glm::mat4 lightProjection = glm::ortho(-10.f, 10.f, -10.f, 10.f, nearPlane, farPlane);
		const glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		lightSpaceMatrix = lightProjection * lightView;
	}

	void RenderManager::DispatchComputeShaders()
	{
		while (!mDispatchedComputeShaders.empty())
		{
			auto shader = mDispatchedComputeShaders.front();
			mDispatchedComputeShaders.pop();
			shader->Bind();
			auto workGroups = shader->GetWorkGroupSize();

			int index = -1;
			for (const auto& texture : shader->GetTextures())
			{
				index++;
				if (!texture) continue;
				glActiveTexture(GL_TEXTURE0 + index); // See GL_TEXTURE macro
				BASED_CHECK_GL_ERROR;
				texture->Bind();
			}

			glDispatchCompute(workGroups.x, workGroups.y, workGroups.z); BASED_CHECK_GL_ERROR;

			index = 0;
			for (const auto& texture : shader->GetTextures())
			{
				glActiveTexture(GL_TEXTURE0 + index); BASED_CHECK_GL_ERROR;
				index++;
				texture->Unbind();
			}
			shader->Unbind();
		}

		glMemoryBarrier(GL_ALL_BARRIER_BITS); BASED_CHECK_GL_ERROR;
	}

	std::shared_ptr<graphics::Camera> RenderManager::GetActiveCamera() const
	{
		if (!mCameras.empty())
		{
			return mCameras.top();
		}

		return Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera();
	}
}
