#include "pch.h"
#include "ui/uirenderinterface.h"

#include "app.h"
#include "graphics/texture.h"
#include "graphics/defaultassetlibraries.h"
#include "engine.h"

static const char* shader_main_vertex = R"(
	#version 410 core
    layout (location = 0) in vec2 inPosition;;
	layout (location = 1) in vec4 inColor0;
    layout (location = 2) in vec2 inTexCoord0;

    out vec2 fragTexCoord;
	out vec4 fragColor;

    uniform vec2 _translate;
	uniform mat4 _transform;

    void main()
    {
        fragTexCoord = inTexCoord0;
		fragColor = inColor0;

		vec2 translatedPos = inPosition + _translate.xy;
		vec4 outPos = _transform * vec4(translatedPos, 0, 1);

	    gl_Position = vec4(outPos.x, outPos.y, 0, 1);
    }
)";

static const char* shader_main_fragment_texture = R"(
	#version 410 core
	in vec2 fragTexCoord;
	in vec4 fragColor;
	
	out vec4 finalColor;

	uniform sampler2D _tex;

	void main() {
		vec4 texColor = texture(_tex, fragTexCoord);
		vec4 col = fragColor * texColor;
		finalColor = vec4(col.x / 255, col.y / 255, col.z / 255, col.w / 255);
	}
)";

static const char* shader_main_fragment_color = R"(
	#version 410 core
	in vec2 fragTexCoord;
	in vec4 fragColor;

	out vec4 finalColor;

	void main() {
		finalColor = fragColor / 255;
	}
)";

namespace based::ui
{
	RenderInterface_GL4::RenderInterface_GL4()
	{
		// Create shaders
		mFragColor = std::make_shared<graphics::Shader>(shader_main_vertex, shader_main_fragment_color);
		mFragTexture = std::make_shared<graphics::Shader>(shader_main_vertex, shader_main_fragment_texture);
	}

	RenderInterface_GL4::~RenderInterface_GL4()
	{
		// Release shaders
	}

	void RenderInterface_GL4::BeginFrame()
	{
		auto cam = Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera();
		mProjection = glm::ortho(0.f,
			static_cast<float>(Engine::Instance().GetWindow().GetSize().x),
			static_cast<float>(Engine::Instance().GetWindow().GetSize().y), 0.f,
			cam ? cam->GetNear() : 0.001f,
			cam ? cam->GetFar() : 1000.f);
		SetTransform(nullptr);
	}

	void RenderInterface_GL4::EndFrame()
	{
	}

	void RenderInterface_GL4::EnableScissorRegion(bool enable)
	{
		ScissoringState new_state = ScissoringState::Disable;

		if (enable)
			new_state = (transform_active ? ScissoringState::Stencil : ScissoringState::Scissor);

		if (new_state != scissoring_state)
		{
			// Disable old
			if (scissoring_state == ScissoringState::Scissor)
			{
				glDisable(GL_SCISSOR_TEST); BASED_CHECK_GL_ERROR;
			}
			else if (scissoring_state == ScissoringState::Stencil)
			{
				glStencilFunc(GL_ALWAYS, 1, 0xFF); BASED_CHECK_GL_ERROR;
			}

			// Enable new
			if (new_state == ScissoringState::Scissor)
			{
				glEnable(GL_SCISSOR_TEST); BASED_CHECK_GL_ERROR;
			}
			else if (new_state == ScissoringState::Stencil)
			{
				glStencilFunc(GL_EQUAL, 1, 0xFF); BASED_CHECK_GL_ERROR;
			}

			scissoring_state = new_state;
		}
	}

	void RenderInterface_GL4::ReleaseTexture(Rml::TextureHandle texture_handle)
	{
		glDeleteTextures(1, (GLuint*)&texture_handle);
	}

	void RenderInterface_GL4::SetTransform(const Rml::Matrix4f* transform)
	{
		glm::mat4 trans = glm::mat4(1.0f);
		if (transform)
		{
			for (int i = 0; i < 4; i++)
			{
				auto column = transform->GetColumn(i);
				trans[i] = glm::vec4(column.x, column.y, column.z, column.w);
			}
		}
		transform_active = (transform != nullptr);
		mTransform = mProjection * trans;
	}

	Rml::CompiledGeometryHandle RenderInterface_GL4::CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
		Rml::Span<const int> indices)
	{
		// Build and upload vertex array
		auto va = std::make_shared<graphics::VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
		BASED_CREATE_VERTEX_BUFFER(col_vb, unsigned char);
		BASED_CREATE_VERTEX_BUFFER(uv_vb, float);

		for (const auto* vertex = vertices.begin(); vertex != vertices.end(); ++vertex)
		{
			pos_vb->PushVertex({ vertex->position.x, vertex->position.y });
			col_vb->PushVertex({ vertex->colour.red, vertex->colour.green, vertex->colour.blue, vertex->colour.alpha });
			uv_vb->PushVertex({ vertex->tex_coord.x, vertex->tex_coord.y });
		}

		pos_vb->SetLayout({ 2 });
		col_vb->SetLayout({ 4 });
		uv_vb->SetLayout({ 2 });
		va->PushBuffer(std::move(pos_vb));
		va->PushBuffer(std::move(col_vb));
		va->PushBuffer(std::move(uv_vb));

		va->SetElements(std::vector<uint32_t>(indices.begin(), indices.end()));
		va->Upload();

		auto id = core::UUID();

		mCompiledVAs[id] = va;

		return id;
	}

	void RenderInterface_GL4::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
		Rml::TextureHandle texture)
	{
		auto va = mCompiledVAs[geometry];

		if (va)
		{
			if (texture)
			{
				// Use texture
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayUserInterface,
					va, mFragTexture, static_cast<uint32_t>(texture), mTransform, glm::vec2{ translation.x, translation.y }));
			}
			else
			{
				// Render solid color
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayUserInterface,
					va, mFragColor, 0, mTransform, glm::vec2{ translation.x, translation.y }));
			}

			Engine::Instance().GetRenderManager().Flush();
		}
	}

	void RenderInterface_GL4::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
	{
		auto it = std::find_if(mCompiledVAs.begin(), mCompiledVAs.end(),
			[geometry](auto pair)
			{
				return pair.first == (core::UUID)geometry;
			});
		if (it != mCompiledVAs.end())
			mCompiledVAs.erase(it);

	}

	Rml::TextureHandle RenderInterface_GL4::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		Rml::FileInterface* file_interface = Rml::GetFileInterface();
		Rml::FileHandle file_handle = file_interface->Open(source);
		if (!file_handle)
		{
			BASED_ERROR("Could not load RML texture from {}", source);
			return false;
		}

		auto texture = std::make_shared<graphics::Texture>(source, true);
		if (texture->GetId() != 0)
		{
			texture_dimensions = Rml::Vector2i(texture->GetWidth(), texture->GetHeight());
			return texture->GetId();
		}

		BASED_ERROR("RML texture at {} was valid, but there was an error creating the texture!", source);
		return false;
	}

	Rml::TextureHandle RenderInterface_GL4::GenerateTexture(Rml::Span<const unsigned char> source,
		Rml::Vector2i source_dimensions)
	{
		GLuint texture_id = 0;
		glGenTextures(1, &texture_id);
		if (texture_id == 0)
		{
			BASED_ERROR("Failed to generate RML texture!");
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);

		GLint internal_format = GL_RGBA8;
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, source_dimensions.x, 
			source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return texture_id;
	}

	void RenderInterface_GL4::SetScissorRegion(Rml::Rectanglei region)
	{
		if (transform_active)
		{
			const float left = static_cast<float>(region.Left());
			const float right = static_cast<float>(region.Right());
			const float top = static_cast<float>(region.Top());
			const float bottom = static_cast<float>(region.Bottom());

			Rml::Vertex p0;
			Rml::Vertex p1;
			Rml::Vertex p2;
			Rml::Vertex p3;
			p0.position = { left, top };
			p1.position = { right, top };
			p2.position = { right, bottom };
			p3.position = { left, bottom };

			Rml::Span<const Rml::Vertex> vertices = {
				{
					p0, p1, p2, p3
				}
			};

			Rml::Span<const int> indices = { { 0,2,1,0,3,2 } };

			glClear(GL_STENCIL_BUFFER_BIT); BASED_CHECK_GL_ERROR;
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); BASED_CHECK_GL_ERROR;
			glStencilFunc(GL_ALWAYS, 1, 0xFF); BASED_CHECK_GL_ERROR;
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); BASED_CHECK_GL_ERROR;

			auto handle = CompileGeometry(vertices, indices);
			RenderGeometry(handle, Rml::Vector2f(0, 0), false);

			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); BASED_CHECK_GL_ERROR;
			glStencilFunc(GL_EQUAL, 1, 0xFF); BASED_CHECK_GL_ERROR;
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); BASED_CHECK_GL_ERROR;
		}
		else
		{
			glScissor(region.Left(), 
				Engine::Instance().GetWindow().GetSize().y - region.Bottom(), 
				region.Width(), region.Height()); BASED_CHECK_GL_ERROR;
		}
	}

	void RenderInterface_GL4::EnableClipMask(bool enable)
	{
		if (enable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}

	void RenderInterface_GL4::RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry,
		Rml::Vector2f translation)
	{
		BASED_ASSERT(glIsEnabled(GL_STENCIL_TEST), "Stencil testing needs to be enabled to render to clip mask!");
		using Rml::ClipMaskOperation;

		const bool clear_stencil = (operation == ClipMaskOperation::Set || operation == ClipMaskOperation::SetInverse);
		if (clear_stencil)
		{
			// @performance Increment the reference value instead of clearing each time.
			glClear(GL_STENCIL_BUFFER_BIT);
		}

		GLint stencil_test_value = 0;
		glGetIntegerv(GL_STENCIL_REF, &stencil_test_value);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_ALWAYS, GLint(1), GLuint(-1));

		switch (operation)
		{
		case ClipMaskOperation::Set:
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			stencil_test_value = 1;
		}
		break;
		case ClipMaskOperation::SetInverse:
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			stencil_test_value = 0;
		}
		break;
		case ClipMaskOperation::Intersect:
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			stencil_test_value += 1;
		}
		break;
		}

		RenderGeometry(geometry, translation, {});

		// Restore state
		// @performance Cache state so we don't toggle it unnecessarily.
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_EQUAL, stencil_test_value, GLuint(-1));
	}
}
