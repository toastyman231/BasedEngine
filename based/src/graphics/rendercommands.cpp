#include "pch.h"

#include "graphics/rendercommands.h"

#include "app.h"
#include "basedtime.h"
#include "engine.h"

#include "graphics/camera.h"
#include "graphics/vertex.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "graphics/framebuffer.h"
#include "graphics/material.h"

#include "glad/glad.h"
#include "graphics/defaultassetlibraries.h"

namespace based::graphics::rendercommands
{
	void RenderVertexArray::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<VertexArray> va = mVertexArray.lock();
		std::shared_ptr<Shader> shader = mShader.lock();
		if (va && shader)
		{
			BASED_ASSERT(va->IsValid(), "Attempting to execute invalid RenderVertexArray - did you forget to call VertexArray::Upload()?");
			if (va->IsValid())
			{
				va->Bind();
				shader->Bind();

				const auto& rm = Engine::Instance().GetRenderManager();
				const auto& cam = rm.GetActiveCamera();
				if (cam)
				{
					shader->SetUniformMat4("proj", cam->GetProjectionMatrix());
					shader->SetUniformMat4("view", cam->GetViewMatrix());
				}

				shader->SetUniformMat4("model", mModelMatrix);

				if (va->GetElementCount() > 0)
				{
					glDrawElements(GL_TRIANGLES, va->GetElementCount(), GL_UNSIGNED_INT, 0);
				}
				else
				{
					glDrawArrays(GL_TRIANGLE_STRIP, 0, va->GetVertexCount()); BASED_CHECK_GL_ERROR;
				}

				shader->Unbind();
				va->Unbind();
			}
		}
		else
		{
			BASED_WARN("Attempting to execute RenderVertexArray with invalid data");
		}
	}

	void RenderVertexArrayTextured::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<VertexArray> va = mVertexArray.lock();
		std::shared_ptr<Texture> texture = mTexture.lock();
		std::shared_ptr<Shader> shader = mShader.lock();
		if (va && texture && shader)
		{
			BASED_ASSERT(va->IsValid(), "Attempting to execute invalid RenderVertexArrayTextured - did you forget to call VertexArray::Upload()?");
			if (va->IsValid())
			{
				va->Bind();
				texture->Bind();
				shader->Bind();

				const auto& rm = Engine::Instance().GetRenderManager();
				const auto& cam = rm.GetActiveCamera();
				if (cam)
				{
					shader->SetUniformMat4("proj", cam->GetProjectionMatrix());
					shader->SetUniformMat4("view", cam->GetViewMatrix());
				}

				shader->SetUniformMat4("model", mModelMatrix);

				if (va->GetElementCount() > 0)
				{
					glDrawElements(GL_TRIANGLES, va->GetElementCount(), GL_UNSIGNED_INT, 0);
				}
				else
				{
					glDrawArrays(GL_TRIANGLE_STRIP, 0, va->GetVertexCount()); BASED_CHECK_GL_ERROR;
				}

				shader->Unbind();
				texture->Unbind();
				va->Unbind();
			}
		}
		else
		{
			BASED_WARN("Attempting to execute RenderVertexArrayTextured with invalid data");
		}
	}

	void RenderVertexArrayMaterial::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<VertexArray> va = mVertexArray.lock();
		std::shared_ptr<Material> mat = mMaterial.lock();

		if (va && mat)
		{
			BASED_ASSERT(va->IsValid(), "Attempting to execute invalid RenderVertexArrayMaterial - did you forget to call VertexArray::Upload()?");
			if (va->IsValid())
			{
				va->Bind();

				std::shared_ptr<Shader> shader = mat->GetShader().lock();
				BASED_ASSERT(shader, "Attempting to execute invalid RenderVertexArrayMaterial - shader is nullptr");
				if (shader)
				{
					managers::RenderManager::SetDepthFunction(mDepthFunc);
					mat->UpdateShaderUniforms();
					shader->Bind();
					int index = -1;
					for (const auto& texture : mat->GetTextures())
					{
						index++;
						if (!texture) continue;
						glActiveTexture(GL_TEXTURE0 + index); // See GL_TEXTURE macro
						BASED_CHECK_GL_ERROR;
						texture->Bind();
					}

					if (Engine::Instance().GetRenderManager().GetCurrentPassName() != "ShadowDepthPass")
					{
						for (const auto& output :
							DefaultLibraries::GetRenderPassOutputs().GetAll())
						{
							index++;
							shader->SetUniformInt(output.first, index);
							glActiveTexture(GL_TEXTURE0 + index); BASED_CHECK_GL_ERROR;
							glBindTexture(GL_TEXTURE_2D, output.second); BASED_CHECK_GL_ERROR;
						}
					}

					if (!mInstanced)
					{
						shader->SetUniformMat4("model", mModelMatrix);
						shader->SetUniformMat4("normalMat", glm::transpose(glm::inverse(mModelMatrix)));
					}
					shader->SetUniformMat4("lightSpaceMatrix", Engine::Instance().GetRenderManager().lightSpaceMatrix);

					if (va->GetElementCount() > 0)
					{
						if (!mInstanced) 
						{
							glDrawElements(GL_TRIANGLES, va->GetElementCount(), GL_UNSIGNED_INT, 0);
							BASED_CHECK_GL_ERROR;
						}
						else 
						{
							glDrawElementsInstanced(
								GL_TRIANGLES, va->GetElementCount(), 
								GL_UNSIGNED_INT, 0, mInstanceCount);
							BASED_CHECK_GL_ERROR;
						}
					}
					else
					{
						if (!mInstanced) 
						{
							glDrawArrays(GL_TRIANGLE_STRIP, 0, va->GetVertexCount());
							BASED_CHECK_GL_ERROR;
						}
						else 
						{
							glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, va->GetVertexCount(), mInstanceCount);
							BASED_CHECK_GL_ERROR;
						}
					}

					while (index >= 0)
					{
						glActiveTexture(GL_TEXTURE0 + index);
						glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
						index--;
					}
					glActiveTexture(GL_TEXTURE0); BASED_CHECK_GL_ERROR;
					shader->Unbind();

					managers::RenderManager::SetDepthFunction(GL_LEQUAL);
				}
				va->Unbind();
			}
		}
		else
		{
			BASED_WARN("Attempting to execute RenderVertexArrayMaterial with invalid data");
		}
	}

	void RenderLineMaterial::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<VertexArray> va = mVertexArray.lock();
		std::shared_ptr<Material> mat = mMaterial.lock();

		if (va && mat)
		{
			BASED_ASSERT(va->IsValid(), "Attempting to execute invalid RenderVertexArrayMaterial - did you forget to call VertexArray::Upload()?");
			if (va->IsValid())
			{
				va->Bind();

				std::shared_ptr<Shader> shader = mat->GetShader().lock();
				BASED_ASSERT(shader, "Attempting to execute invalid RenderVertexArrayMaterial - shader is nullptr");
				if (shader)
				{
					managers::RenderManager::SetDepthFunction(mDepthFunc);
					mat->UpdateShaderUniforms();
					shader->Bind();

					glLineWidth(mat->GetUniformValue<float>("width"));

					if (!mInstanced)
					{
						shader->SetUniformMat4("model", mModelMatrix);
					}

					if (va->GetElementCount() > 0)
					{
						if (!mInstanced)
						{
							glDrawElements(GL_LINES, va->GetElementCount(), GL_UNSIGNED_INT, 0);
							BASED_CHECK_GL_ERROR;
						}
						else
						{
							glDrawElementsInstanced(
								GL_LINES, va->GetElementCount(),
								GL_UNSIGNED_INT, 0, mInstanceCount);
							BASED_CHECK_GL_ERROR;
						}
					}
					else
					{
						if (!mInstanced)
						{
							glDrawArrays(GL_LINES, 0, va->GetVertexCount());
							BASED_CHECK_GL_ERROR;
						}
						else
						{
							glDrawArraysInstanced(GL_LINES, 0, va->GetVertexCount(), mInstanceCount);
							BASED_CHECK_GL_ERROR;
						}
					}

					glLineWidth(1);
					managers::RenderManager::SetDepthFunction(GL_LEQUAL);
				}
				va->Unbind();
			}
		}
		else
		{
			BASED_WARN("Attempting to execute RenderVertexArrayMaterial with invalid data");
		}
	}

	void RenderVertexArrayPostProcess::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<VertexArray> va = mVertexArray.lock();
		std::shared_ptr<Material> mat = mMaterial.lock();

		if (va && mat)
		{
			BASED_ASSERT(va->IsValid(), "Attempting to execute invalid RenderVertexArrayMaterial - did you forget to call VertexArray::Upload()?");
			if (va->IsValid())
			{
				va->Bind();

				std::shared_ptr<Shader> shader = mat->GetShader().lock();
				BASED_ASSERT(shader, "Attempting to execute invalid RenderVertexArrayMaterial - shader is nullptr");
				if (shader)
				{
					mat->UpdateShaderUniforms();
					shader->Bind();
					int index = -1;
					for (const auto& texture : mat->GetTextures())
					{
						index++;
						if (!texture) continue;
						glActiveTexture(GL_TEXTURE0 + index); // See GL_TEXTURE macro
						BASED_CHECK_GL_ERROR;
						texture->Bind();
					}

					for (const auto& output : 
						DefaultLibraries::GetRenderPassOutputs().GetAll())
					{
						index++;
						shader->SetUniformInt(output.first, index);
						glActiveTexture(GL_TEXTURE0 + index); BASED_CHECK_GL_ERROR;
						glBindTexture(GL_TEXTURE_2D, output.second); BASED_CHECK_GL_ERROR;
					}

					auto& window = Engine::Instance().GetWindow();
					glm::vec2 scale = window.GetFramebufferSize() / (glm::vec2)window.GetSize();
					mModelMatrix = glm::scale(mModelMatrix, { scale.x, scale.y, 1.f });
					shader->SetUniformMat4("model", mModelMatrix);

					if (va->GetElementCount() > 0)
					{
						glDrawElements(GL_TRIANGLES, va->GetElementCount(), GL_UNSIGNED_INT, 0);
						BASED_CHECK_GL_ERROR;
					}
					else
					{
						glDrawArrays(GL_TRIANGLE_STRIP, 0, va->GetVertexCount());
						BASED_CHECK_GL_ERROR;
					}

					index = 0;
					for (const auto& texture : mat->GetTextures())
					{
						glActiveTexture(GL_TEXTURE0 + index); BASED_CHECK_GL_ERROR;
						index++;
						texture->Unbind();
					}
					glActiveTexture(GL_TEXTURE0); BASED_CHECK_GL_ERROR;
					shader->Unbind();
				}
				va->Unbind();
			}
		}
		else
		{
			BASED_WARN("Attempting to execute RenderVertexArrayPostProcess with invalid data");
		}
	}

	void RenderVertexArrayUserInterface::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<VertexArray> va = mVertexArray.lock();
		std::shared_ptr<Shader> shader = mShader.lock();
		if (va && shader)
		{
			BASED_ASSERT(va->IsValid(), "Attempting to execute invalid RenderVertexArrayUserInterface - did you forget to call VertexArray::Upload()?");
			if (va->IsValid())
			{
				va->Bind();
				if (mTexture != 0)
				{
					glBindTexture(GL_TEXTURE_2D, mTexture); BASED_CHECK_GL_ERROR;
				}
				shader->Bind();

				shader->SetUniformMat4("_transform", mTransform);
				shader->SetUniformFloat2("_translate", glm::vec2(mTranslation.x, mTranslation.y));

				if (va->GetElementCount() > 0)
				{
					glDrawElements(GL_TRIANGLES, va->GetElementCount(), GL_UNSIGNED_INT, 0);
				}
				else
				{
					glDrawArrays(GL_TRIANGLE_STRIP, 0, va->GetVertexCount()); BASED_CHECK_GL_ERROR;
				}

				shader->Unbind();
				if (mTexture != 0)
				{
					glBindTexture(GL_TEXTURE_2D, 0); BASED_CHECK_GL_ERROR;
				}
				va->Unbind();
			}
		} else
		{
			BASED_WARN("Attempting to execute RenderVertexArrayUserInterface with invalid data");
		}
	}


	void PushFramebuffer::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<Framebuffer> fb = mFramebuffer.lock();
		if (fb)
		{
			auto& rm = Engine::Instance().GetRenderManager();
			rm.PushFramebuffer(fb, ShouldClearBuffer());
			rm.PushDebugGroup(mName);
		}
		else
		{
			BASED_WARN("Attempting to execute a RenderMesh with invalid data");
		}
	}

	void PopFramebuffer::Execute()
	{
		PROFILE_FUNCTION();
		auto& rm = Engine::Instance().GetRenderManager();
		rm.PopFramebuffer();
		rm.PopDebugGroup();
	}

	void UpdateGlobals::Execute()
	{
		PROFILE_FUNCTION();
		ShaderGlobals globals;

		if (auto cam = Engine::Instance().GetRenderManager().GetActiveCamera())
		{
			globals.proj = cam->GetProjectionMatrix();
			globals.view = cam->GetViewMatrix();
			globals.eyePos = glm::vec4(cam->GetTransform().Position(), 1.f);
			globals.eyeForward = glm::vec4(cam->GetForward(), 1.f);
		}

		globals.time = based::core::Time::GetTime();
		globals.renderMode = static_cast<int32_t>(managers::RenderManager::GetRenderMode());

		glBindBuffer(GL_UNIFORM_BUFFER, graphics::Shader::GetGlobalBufferID()); BASED_CHECK_GL_ERROR;
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderGlobals), &globals); BASED_CHECK_GL_ERROR;
		glBindBuffer(GL_UNIFORM_BUFFER, 0); BASED_CHECK_GL_ERROR;
	}

	void SetWireframe::Execute()
	{
		glPolygonMode(GL_FRONT_AND_BACK, mWireframe ? GL_LINE : GL_FILL); BASED_CHECK_GL_ERROR;
	}

	void PushCamera::Execute()
	{
		PROFILE_FUNCTION();
		std::shared_ptr<Camera> cam = mCamera.lock();
		if (cam)
		{
			Engine::Instance().GetRenderManager().PushCamera(cam);
		}
		else
		{
			BASED_WARN("Attempting to push a camera with invalid data!");
		}
	}

	void PopCamera::Execute()
	{
		PROFILE_FUNCTION();
		Engine::Instance().GetRenderManager().PopCamera();
	}
}
