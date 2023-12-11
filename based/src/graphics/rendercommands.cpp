#include "graphics/rendercommands.h"
#include "engine.h"
#include "log.h"

#include "graphics/camera.h"
#include "graphics/vertex.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "graphics/helpers.h"
#include "graphics/framebuffer.h"
#include "graphics/material.h"

#include "glad/glad.h"
#include "based/core/profiler.h"

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

				// TODO: Convert camera matrices to leverage UBOs
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

				// TODO: Convert camera matrices to leverage UBOs
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

				Shader* shader = mat->GetShader();
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
						glActiveTexture(0x84C0 + index); // See GL_TEXTURE macro
						BASED_CHECK_GL_ERROR;
						texture->Bind();
					}

					if (!mInstanced) shader->SetUniformMat4("model", mModelMatrix);

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

					for (const auto& texture : mat->GetTextures())
					{
						texture->Unbind();
					}
					shader->Unbind();
				}
				va->Unbind();
			}
		}
		else
		{
			BASED_WARN("Attempting to execute RenderVertexArrayMaterial with invalid data");
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
			Engine::Instance().GetRenderManager().PushFramebuffer(fb);
		}
		else
		{
			BASED_WARN("Attempting to execute a RenderMesh with invalid data");
		}
	}

	void PopFramebuffer::Execute()
	{
		PROFILE_FUNCTION();
		Engine::Instance().GetRenderManager().PopFramebuffer();
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
			BASED_WARN("Attempting to execute a RenderMesh with invalid data");
		}
	}

	void PopCamera::Execute()
	{
		PROFILE_FUNCTION();
		Engine::Instance().GetRenderManager().PopCamera();
	}
}