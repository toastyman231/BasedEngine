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
		// Clear VAs at the start of the next frame (we can't clear in EndFrame because that actually runs before
		// the scene is even rendered, so we still need the VAs after that point)
		mVAs.clear();
		mProjection = glm::ortho(0.f,
			static_cast<float>(Engine::Instance().GetWindow().GetSize().x),
			static_cast<float>(Engine::Instance().GetWindow().GetSize().y), 0.f,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetNear(),
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetFar());
		SetTransform(nullptr);
	}

	void RenderInterface_GL4::EndFrame()
	{
	}

	void RenderInterface_GL4::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
	                                         Rml::TextureHandle texture, const Rml::Vector2f& translation)
	{
		// Build and upload vertex array
		auto va = std::make_shared<graphics::VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
		BASED_CREATE_VERTEX_BUFFER(col_vb, unsigned char);
		BASED_CREATE_VERTEX_BUFFER(uv_vb, float);

		for (const auto *vertex = vertices; vertex != vertices + num_vertices; ++vertex)
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

		va->SetElements(std::vector<uint32_t>(indices, indices + num_indices));
		va->Upload();

		mVAs.emplace_back(va);

		// Set uniforms and render
		if (texture)
		{
			// Use texture
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayUserInterface,
				mVAs.back(), mFragTexture, static_cast<uint32_t>(texture), mTransform, glm::vec2{translation.x, translation.y}));
		} else
		{
			// Render solid color
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayUserInterface,
				mVAs.back(), mFragColor, 0, mTransform, glm::vec2{translation.x, translation.y}));
		}
	}

	Rml::CompiledGeometryHandle RenderInterface_GL4::CompileGeometry(Rml::Vertex* vertices, int num_vertices,
		int* indices, int num_indices, Rml::TextureHandle texture)
	{
		// Build and upload vertex array
		auto va = std::make_shared<graphics::VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
		BASED_CREATE_VERTEX_BUFFER(col_vb, unsigned char);
		BASED_CREATE_VERTEX_BUFFER(uv_vb, float);

		for (const auto* vertex = vertices; vertex != vertices + num_vertices; ++vertex)
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

		va->SetElements(std::vector<uint32_t>(indices, indices + num_indices));
		va->Upload();

		mCompiledVAs.emplace_back(va);
		mCompiledTextures.emplace_back(texture);

		return mCompiledVAs.size() - 1;
	}

	void RenderInterface_GL4::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry,
		const Rml::Vector2f& translation)
	{
		auto va = mCompiledVAs[geometry];
		auto texture = mCompiledTextures[geometry];
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
		}
	}

	void RenderInterface_GL4::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
	{
		if (geometry < 0 || geometry > mCompiledVAs.size()) return;

		mCompiledVAs.erase(mCompiledVAs.begin() + geometry);
		mCompiledTextures.erase(mCompiledTextures.begin() + geometry);
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
				glDisable(GL_SCISSOR_TEST);
			else if (scissoring_state == ScissoringState::Stencil)
				glStencilFunc(GL_ALWAYS, 1, GLuint(-1));

			// Enable new
			if (new_state == ScissoringState::Scissor)
				glEnable(GL_SCISSOR_TEST);
			else if (new_state == ScissoringState::Stencil)
				glStencilFunc(GL_EQUAL, 1, GLuint(-1));

			scissoring_state = new_state;
		}
	}

	void RenderInterface_GL4::SetScissorRegion(int x, int y, int width, int height)
	{
		if (transform_active)
		{
			const float left = float(x);
			const float right = float(x + width);
			const float top = float(y);
			const float bottom = float(y + height);

			Rml::Vertex vertices[4];
			vertices[0].position = { left, top };
			vertices[1].position = { right, top };
			vertices[2].position = { right, bottom };
			vertices[3].position = { left, bottom };

			int indices[6] = { 0, 2, 1, 0, 3, 2 };

			glClear(GL_STENCIL_BUFFER_BIT);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glStencilFunc(GL_ALWAYS, 1, GLuint(-1));
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			RenderGeometry(vertices, 4, indices, 6, 0, Rml::Vector2f(0, 0));

			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilFunc(GL_EQUAL, 1, GLuint(-1));
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		else
		{
			glScissor(x, Engine::Instance().GetWindow().GetSize().y - (y + height), width, height);
		}
	}

	// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
#pragma pack(1)
	struct TGAHeader {
		char idLength;
		char colourMapType;
		char dataType;
		short int colourMapOrigin;
		short int colourMapLength;
		char colourMapDepth;
		short int xOrigin;
		short int yOrigin;
		short int width;
		short int height;
		char bitsPerPixel;
		char imageDescriptor;
	};
	// Restore packing
#pragma pack()

	bool RenderInterface_GL4::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions,
		const Rml::String& source)
	{
		Rml::FileInterface* file_interface = Rml::GetFileInterface();
		Rml::FileHandle file_handle = file_interface->Open(source);
		if (!file_handle)
		{
			return false;
		}

		file_interface->Seek(file_handle, 0, SEEK_END);
		size_t buffer_size = file_interface->Tell(file_handle);
		file_interface->Seek(file_handle, 0, SEEK_SET);

		if (buffer_size <= sizeof(TGAHeader))
		{
			//Rml::Log::Message(Rml::Log::LT_ERROR, "Texture file size is smaller than TGAHeader, file is not a valid TGA image.");
			BASED_ERROR("Texture file size is smaller than TGAHeader, file is not a valid TGA image.");
			file_interface->Close(file_handle);
			return false;
		}

		using Rml::byte;
		byte* buffer = new byte[buffer_size];
		file_interface->Read(buffer, buffer_size, file_handle);
		file_interface->Close(file_handle);

		TGAHeader header;
		memcpy(&header, buffer, sizeof(TGAHeader));

		int color_mode = header.bitsPerPixel / 8;
		int image_size = header.width * header.height * 4; // We always make 32bit textures

		if (header.dataType != 2)
		{
			delete[] buffer;
			// Try to load the texture using my texture API
			auto texture = std::make_shared<graphics::Texture>(source);
			texture_handle = (Rml::TextureHandle)texture->GetId();
			if (texture_handle == 0)
			{
				//Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to generate texture.");
				BASED_ERROR("Failed to generate texture.");
				return false;
			}

			return true;
		}

		// Ensure we have at least 3 colors
		if (color_mode < 3)
		{
			//Rml::Log::Message(Rml::Log::LT_ERROR, "Only 24 and 32bit textures are supported.");
			BASED_ERROR("Only 24 and 32bit textures are supported.");
			delete[] buffer;
			return false;
		}

		const byte* image_src = buffer + sizeof(TGAHeader);
		byte* image_dest = new byte[image_size];

		// Targa is BGR, swap to RGB and flip Y axis
		for (long y = 0; y < header.height; y++)
		{
			long read_index = y * header.width * color_mode;
			long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * 4;
			for (long x = 0; x < header.width; x++)
			{
				image_dest[write_index] = image_src[read_index + 2];
				image_dest[write_index + 1] = image_src[read_index + 1];
				image_dest[write_index + 2] = image_src[read_index];
				if (color_mode == 4)
				{
					const int alpha = image_src[read_index + 3];
#ifdef RMLUI_SRGB_PREMULTIPLIED_ALPHA
					image_dest[write_index + 0] = (image_dest[write_index + 0] * alpha) / 255;
					image_dest[write_index + 1] = (image_dest[write_index + 1] * alpha) / 255;
					image_dest[write_index + 2] = (image_dest[write_index + 2] * alpha) / 255;
#endif
					image_dest[write_index + 3] = (byte)alpha;
				}
				else
				{
					image_dest[write_index + 3] = 255;
				}

				write_index += 4;
				read_index += color_mode;
			}
		}

		texture_dimensions.x = header.width;
		texture_dimensions.y = header.height;

		bool success = GenerateTexture(texture_handle, image_dest, texture_dimensions);

		delete[] image_dest;
		delete[] buffer;

		return success;
	}

	bool RenderInterface_GL4::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source,
		const Rml::Vector2i& source_dimensions)
	{
		GLuint texture_id = 0;
		glGenTextures(1, &texture_id);
		if (texture_id == 0)
		{
			Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to generate texture.");
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);

		GLint internal_format = GL_RGBA8;
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		 
		texture_handle = (Rml::TextureHandle)texture_id;

		return true;
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
		mTransform = mProjection * (transform ? trans : glm::mat4(1.f));
	}
}
