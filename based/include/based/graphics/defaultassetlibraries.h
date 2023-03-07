#pragma once

#include "based/core/assetlibrary.h"
#include "material.h"
#include "shader.h"
#include "texture.h"
#include "vertex.h"

namespace based::graphics
{
	class DefaultLibraries
	{
	private:
		inline static core::AssetLibrary<graphics::VertexArray> mVALibrary;
		inline static core::AssetLibrary<graphics::Shader> mShaderLibrary;
		inline static core::AssetLibrary<graphics::Texture> mTextureLibrary;
		inline static core::AssetLibrary<graphics::Material> mMaterialLibrary;
	public:
		inline static core::AssetLibrary<VertexArray>& GetVALibrary() { return mVALibrary; }
		inline static core::AssetLibrary<Shader>& GetShaderLibrary() { return mShaderLibrary; }
		inline static core::AssetLibrary<Texture>& GetTextureLibrary() { return mTextureLibrary; }
		inline static core::AssetLibrary<Material>& GetMaterialLibrary() { return mMaterialLibrary; }

		inline static void InitializeLibraries()
		{
			// VertexArray
			{
				auto va = std::make_shared<graphics::VertexArray>();

				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					vb->PushVertex({ 0.5f, 0.5, 0.f });
					vb->PushVertex({ 0.5f, -0.5, 0.f });
					vb->PushVertex({ -0.5f, -0.5, 0.f });
					vb->PushVertex({ -0.5f, 0.5, 0.f });
					vb->SetLayout({ 3 });
					va->PushBuffer(std::move(vb));
				}

				va->SetElements({ 0, 3, 1, 1, 3, 2 });
				va->Upload();

				mVALibrary.Load("Rect", va);
			}
			{
				auto va = std::make_shared<graphics::VertexArray>();

				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					vb->PushVertex({ 0.5f, 0.5, 0.f });
					vb->PushVertex({ 0.5f, -0.5, 0.f });
					vb->PushVertex({ -0.5f, -0.5, 0.f });
					vb->PushVertex({ -0.5f, 0.5, 0.f });
					vb->SetLayout({ 3 });
					va->PushBuffer(std::move(vb));
				}
				{
					BASED_CREATE_VERTEX_BUFFER(vb, short);
					vb->PushVertex({ 1, 1 });
					vb->PushVertex({ 1, 0 });
					vb->PushVertex({ 0, 0 });
					vb->PushVertex({ 0, 1 });
					vb->SetLayout({ 2 });
					va->PushBuffer(std::move(vb));
				}

				va->SetElements({ 0, 3, 1, 1, 3, 2 });
				va->Upload();

				mVALibrary.Load("TexturedRect", va);
			}

			// Shaders
			{
				auto vertexShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;

                    uniform mat4 proj = mat4(1.0);
                    uniform mat4 view = mat4(1.0);
                    uniform mat4 model = mat4(1.0);
                    void main()
                    {
                        gl_Position = proj * view * model * vec4(position, 1.0);
                    }
                )";
				auto fragmentShader = R"(
                    #version 410 core
                    out vec4 outColor;

                    uniform vec4 col = vec4(1.0);
                    void main()
                    {
                        outColor = col;
                    }
                )";
				mShaderLibrary.Load("Rect", std::make_shared<graphics::Shader>(vertexShader, fragmentShader));
			}
			{
				auto vertexShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;
                    layout (location = 1) in vec2 texcoords;
                    out vec2 uvs;

                    uniform mat4 proj = mat4(1.0);
                    uniform mat4 view = mat4(1.0);
                    uniform mat4 model = mat4(1.0);
                    void main()
                    {
                        uvs = texcoords;
                        gl_Position = proj * view * model * vec4(position, 1.0);
                    }
                )";
				auto fragmentShader = R"(
                    #version 410 core
                    out vec4 outColor;
                    in vec2 uvs;

                    uniform sampler2D tex;
					uniform vec4 col;
                    void main()
                    {
                        outColor = texture(tex, uvs) * col;
                    }
                )";
				mShaderLibrary.Load("TexturedRect", std::make_shared<graphics::Shader>(vertexShader, fragmentShader));
			}

			// Textures
			{
				auto tex = std::make_shared<graphics::Texture>("res/bro.png");
				tex->SetTextureFilter(graphics::TextureFilter::Nearest);
				mTextureLibrary.Load("Bro", tex);
			}
			{
				auto tex = std::make_shared<graphics::Texture>("res/bro2.png");
				tex->SetTextureFilter(graphics::TextureFilter::Nearest);
				mTextureLibrary.Load("Bro2", tex);
			}

			// Materials
			{
				auto mat = std::make_shared<graphics::Material>(mShaderLibrary.Get("Rect"),
					mTextureLibrary.Get("Bro"));
				mat->SetUniformValue("col", glm::vec4(1, 0, 0, 1));
				mMaterialLibrary.Load("RectRed", mat);
			}

			{
				auto mat = std::make_shared<graphics::Material>(mShaderLibrary.Get("Rect"));
				mat->SetUniformValue("col", glm::vec4(0, 1, 0, 1));
				mMaterialLibrary.Load("RectGreen", mat);
			}
		}
	};

}
