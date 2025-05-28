#pragma once

#ifndef RMLUI_BACKENDS_RENDERER_GL3_H
#define RMLUI_BACKENDS_RENDERER_GL3_H

#include "uuid.h"
#include "based/graphics/vertex.h"
#include "based/graphics/shader.h"
#include "glad/glad.h"

namespace based::ui
{
	class RenderInterface_GL4 : public Rml::RenderInterface {
	public:
		RenderInterface_GL4();
		~RenderInterface_GL4() override;

		// Sets up OpenGL states for taking rendering commands from RmlUi.
		void BeginFrame();
		void EndFrame();

		// -- Inherited from Rml::RenderInterface --

		void EnableScissorRegion(bool enable) override;

		void ReleaseTexture(Rml::TextureHandle texture_handle) override;

		void SetTransform(const Rml::Matrix4f* transform) override;

		Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
		void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
			Rml::TextureHandle texture) override;
		void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
		Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
		Rml::TextureHandle GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions) override;
		void SetScissorRegion(Rml::Rectanglei region) override;
		void EnableClipMask(bool enable) override;
		void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry,
			Rml::Vector2f translation) override;

		// Can be passed to RenderGeometry() to enable texture rendering without changing the bound texture.
		static const Rml::TextureHandle TextureEnableWithoutBinding = Rml::TextureHandle(-1);
	private:
		std::unordered_map<core::UUID, std::shared_ptr<graphics::VertexArray>> mCompiledVAs;
		std::shared_ptr<graphics::Shader> mFragColor;
		std::shared_ptr<graphics::Shader> mFragTexture;

		enum class ScissoringState : uint8_t { Disable, Scissor, Stencil };
		ScissoringState scissoring_state = ScissoringState::Disable;

		bool transform_active = false;
		glm::mat4 mTransform, mProjection;
	};
}


#endif