#pragma once

#ifndef RMLUI_BACKENDS_RENDERER_GL3_H
#define RMLUI_BACKENDS_RENDERER_GL3_H

#include "based/graphics/vertex.h"
#include "based/graphics/shader.h"
#include "glad/glad.h"

namespace based::ui
{
	class RenderInterface_GL4 : public Rml::RenderInterface {
	public:
		RenderInterface_GL4();
		~RenderInterface_GL4();

		// Sets up OpenGL states for taking rendering commands from RmlUi.
		void BeginFrame();
		void EndFrame();

		// -- Inherited from Rml::RenderInterface --

		void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture,
			const Rml::Vector2f& translation) override;

		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices,
			Rml::TextureHandle texture) override {
			return Rml::CompiledGeometryHandle();
		}
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) override {}
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override {}

		void EnableScissorRegion(bool enable) override;
		void SetScissorRegion(int x, int y, int width, int height) override;

		bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
		void ReleaseTexture(Rml::TextureHandle texture_handle) override;

		void SetTransform(const Rml::Matrix4f* transform) override;

		// Can be passed to RenderGeometry() to enable texture rendering without changing the bound texture.
		static const Rml::TextureHandle TextureEnableWithoutBinding = Rml::TextureHandle(-1);
	private:
		std::shared_ptr<graphics::VertexArray> mVA;
		std::vector<std::shared_ptr<graphics::VertexArray>> mVAs;
		std::shared_ptr<graphics::Shader> mFragColor;
		std::shared_ptr<graphics::Shader> mFragTexture;

		enum class ScissoringState { Disable, Scissor, Stencil };
		ScissoringState scissoring_state = ScissoringState::Disable;

		bool transform_active = false;
		glm::mat4 mTransform, mProjection;
	};
}


#endif