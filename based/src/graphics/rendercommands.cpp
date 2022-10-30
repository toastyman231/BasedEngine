#include "graphics/rendercommands.h"
#include "engine.h"
#include "log.h"

#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/helpers.h"
#include "graphics/framebuffer.h"

#include "glad/glad.h"

namespace based::graphics::rendercommands
{
	void RenderMesh::Execute()
	{
		std::shared_ptr<Mesh> mesh = mMesh.lock();
		std::shared_ptr<Shader> shader = mShader.lock();
		if (mesh && shader)
		{
			mesh->Bind();
			shader->Bind();

			if (mesh->GetElementCount() > 0)
			{
				glDrawElements(GL_TRIANGLES, mesh->GetElementCount(), GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->GetVertexCount()); BASED_CHECK_GL_ERROR;
			}

			shader->Unbind();
			mesh->Unbind();
		}
		else
		{
			BASED_WARN("Attempting to execute RenderMesh with invalid data");
		}
	}

	void PushFramebuffer::Execute()
	{
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
		Engine::Instance().GetRenderManager().PopFramebuffer();
	}
}