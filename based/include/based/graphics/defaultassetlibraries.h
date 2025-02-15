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
		inline static core::SimpleAssetLibrary<uint32_t> mRenderPassOutputs;
		inline static core::AssetLibrary<graphics::Texture> mTextureLibrary;
		inline static core::AssetLibrary<graphics::Material> mMaterialLibrary;
		inline static core::AssetLibrary<graphics::Mesh> mMeshLibrary;
		inline static core::AssetLibrary<graphics::Model> mModelLibrary;
	public:
		inline static core::AssetLibrary<VertexArray>& GetVALibrary() { return mVALibrary; }
		inline static core::AssetLibrary<Shader>& GetShaderLibrary() { return mShaderLibrary; }
		inline static core::SimpleAssetLibrary<uint32_t>& GetRenderPassOutputs() { return mRenderPassOutputs; }
		inline static core::AssetLibrary<Texture>& GetTextureLibrary() { return mTextureLibrary; }
		inline static core::AssetLibrary<Material>& GetMaterialLibrary() { return mMaterialLibrary; }
		inline static core::AssetLibrary<Mesh>& GetMeshLibrary() { return mMeshLibrary; }
		inline static core::AssetLibrary<Model>& GetModelLibrary() { return mModelLibrary; }

		inline static void InitializeLibraries()
		{
#pragma region VertexArrays
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
			{
				auto va = std::make_shared<graphics::VertexArray>();

				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					// Positions

					// Front
					vb->PushVertex({ 0.5f, 0.5f, 0.5f });
					vb->PushVertex({ -0.5f, 0.5f, 0.5f });
					vb->PushVertex({ -0.5f, -0.5f, 0.5f });
					vb->PushVertex({ 0.5f, -0.5f, 0.5f });

					// Right
					vb->PushVertex({ 0.5f, 0.5f, 0.5f });
					vb->PushVertex({ 0.5f, -0.5f, 0.5f });
					vb->PushVertex({ 0.5f, -0.5f, -0.5f });
					vb->PushVertex({ 0.5f, 0.5f, -0.5f });

					// Top
					vb->PushVertex({ .5f, .5f, .5f });
					vb->PushVertex({ .5f, .5f,-.5f });
					vb->PushVertex({ -.5f, .5f,-.5f });
					vb->PushVertex({ -.5f, .5f, .5f });

					// Left
					vb->PushVertex({ -.5f, .5f, .5f });
					vb->PushVertex({ -.5f, .5f,-.5f });
					vb->PushVertex({ -.5f,-.5f,-.5f });
					vb->PushVertex({ -.5f,-.5f, .5f });

					// Bottom
					vb->PushVertex({ -.5f,-.5f,-.5f });
					vb->PushVertex({ .5f,-.5f,-.5f });
					vb->PushVertex({ .5f,-.5f, .5f });
					vb->PushVertex({ -.5f,-.5f, .5f });

					// Back
					vb->PushVertex({ .5f,-.5f,-.5f });
					vb->PushVertex({ -.5f,-.5f,-.5f });
					vb->PushVertex({ -.5f, .5f,-.5f });
					vb->PushVertex({ .5f, .5f,-.5f });

					vb->SetLayout({ 3 });
					va->PushBuffer(std::move(vb));
				}
				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					// UVs

					// Front
					vb->PushVertex({ 1, 0 });
					vb->PushVertex({ 0, 0 });
					vb->PushVertex({ 0, 1 });
					vb->PushVertex({ 1, 1 });

					// Right
					vb->PushVertex({ 0, 0 });
					vb->PushVertex({ 0, 1 });
					vb->PushVertex({ 1, 1 });
					vb->PushVertex({ 1, 0 });

					// Top
					vb->PushVertex({ 1, 1 });
					vb->PushVertex({ 1, 0 });
					vb->PushVertex({ 0, 0 });
					vb->PushVertex({ 0, 1 });

					// Left
					vb->PushVertex({ 1, 0 });
					vb->PushVertex({ 0, 0 });
					vb->PushVertex({ 0, 1 });
					vb->PushVertex({ 1, 1 });

					// Bottom
					vb->PushVertex({ 0, 1 });
					vb->PushVertex({ 1, 1 });
					vb->PushVertex({ 1, 0 });
					vb->PushVertex({ 0, 0 });

					// Back
					vb->PushVertex({ 0, 1 });
					vb->PushVertex({ 1, 1 });
					vb->PushVertex({ 1, 0 });
					vb->PushVertex({ 0, 0 });

					vb->SetLayout({ 2 });
					va->PushBuffer(std::move(vb));
				}
				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					// Normals

					// Front
					vb->PushVertex({ 0.0f, 0.0f, 1.0f });
					vb->PushVertex({ 0.0f, 0.0f, 1.0f });
					vb->PushVertex({ 0.0f, 0.0f, 1.0f });
					vb->PushVertex({ 0.0f, 0.0f, 1.0f });

					// Right
					vb->PushVertex({ 1.0f, 0.0f, 0.0f });
					vb->PushVertex({ 1.0f, 0.0f, 0.0f });
					vb->PushVertex({ 1.0f, 0.0f, 0.0f });
					vb->PushVertex({ 1.0f, 0.0f, 0.0f });

					// Top
					vb->PushVertex({ 0.0f, 1.0f, 0.0f });
					vb->PushVertex({ 0.0f, 1.0f, 0.0f });
					vb->PushVertex({ 0.0f, 1.0f, 0.0f });
					vb->PushVertex({ 0.0f, 1.0f, 0.0f });

					// Left
					vb->PushVertex({ -1.0f, 0.0f, 0.0f });
					vb->PushVertex({ -1.0f, 0.0f, 0.0f });
					vb->PushVertex({ -1.0f, 0.0f, 0.0f });
					vb->PushVertex({ -1.0f, 0.0f, 0.0f });

					// Bottom
					vb->PushVertex({ 0.0f, -1.0f, 0.0f });
					vb->PushVertex({ 0.0f, -1.0f, 0.0f });
					vb->PushVertex({ 0.0f, -1.0f, 0.0f });
					vb->PushVertex({ 0.0f, -1.0f, 0.0f });

					// Back
					vb->PushVertex({ 0.0f, 0.0f, -1.0f });
					vb->PushVertex({ 0.0f, 0.0f, -1.0f });
					vb->PushVertex({ 0.0f, 0.0f, -1.0f });
					vb->PushVertex({ 0.0f, 0.0f, -1.0f });

					vb->SetLayout({ 3 });
					va->PushBuffer(std::move(vb));
				}

				va->SetElements({
					 0, 1, 2,   2, 3, 0,    // v0-v1-v2, v2-v3-v0 (front)
					 4, 5, 6,   6, 7, 4,    // v0-v3-v4, v4-v5-v0 (right)
					 8, 9,10,  10,11, 8,    // v0-v5-v6, v6-v1-v0 (top)
					12,13,14,  14,15,12,    // v1-v6-v7, v7-v2-v1 (left)
					16,17,18,  18,19,16,    // v7-v4-v3, v3-v2-v7 (bottom)
					20,21,22,  22,23,20     // v4-v7-v6, v6-v5-v4 (back)
				});
				va->Upload();

				mVALibrary.Load("TexturedCube", va);
			}
			{
				auto va = std::make_shared<graphics::VertexArray>();

				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					// Positions

					// Front
					vb->PushVertex({ 0.5f, 0.5f, 0.5f });
					vb->PushVertex({ -0.5f, 0.5f, 0.5f });
					vb->PushVertex({ -0.5f, -0.5f, 0.5f });
					vb->PushVertex({ 0.5f, -0.5f, 0.5f });

					// Right
					vb->PushVertex({ 0.5f, 0.5f, 0.5f });
					vb->PushVertex({ 0.5f, -0.5f, 0.5f });
					vb->PushVertex({ 0.5f, -0.5f, -0.5f });
					vb->PushVertex({ 0.5f, 0.5f, -0.5f });

					// Top
					vb->PushVertex({ .5f, .5f, .5f });
					vb->PushVertex({ .5f, .5f,-.5f });
					vb->PushVertex({ -.5f, .5f,-.5f });
					vb->PushVertex({ -.5f, .5f, .5f });

					// Left
					vb->PushVertex({ -.5f, .5f, .5f });
					vb->PushVertex({ -.5f, .5f,-.5f });
					vb->PushVertex({ -.5f,-.5f,-.5f });
					vb->PushVertex({ -.5f,-.5f, .5f });

					// Bottom
					vb->PushVertex({ -.5f,-.5f,-.5f });
					vb->PushVertex({ .5f,-.5f,-.5f });
					vb->PushVertex({ .5f,-.5f, .5f });
					vb->PushVertex({ -.5f,-.5f, .5f });

					// Back
					vb->PushVertex({ .5f,-.5f,-.5f });
					vb->PushVertex({ -.5f,-.5f,-.5f });
					vb->PushVertex({ -.5f, .5f,-.5f });
					vb->PushVertex({ .5f, .5f,-.5f });

					vb->SetLayout({ 3 });
					va->PushBuffer(std::move(vb));
				}
				{
					BASED_CREATE_VERTEX_BUFFER(vb, float);
					// UVs

					// Front
					vb->PushVertex({ GetAdjustedUV(1, 1, 4), GetAdjustedUV(1, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 0, 4), GetAdjustedUV(1, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(1,0, 4), GetAdjustedUV(1, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 1, 4), GetAdjustedUV(1, 1, 3) });

					// Right
					vb->PushVertex({ GetAdjustedUV(2, 0, 4), GetAdjustedUV(1, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(2, 0, 4), GetAdjustedUV(1, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(2, 1, 4), GetAdjustedUV(1, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(2, 1, 4), GetAdjustedUV(1, 0, 3) });

					// Top
					vb->PushVertex({ GetAdjustedUV(1, 1, 4), GetAdjustedUV(0, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 1, 4), GetAdjustedUV(0, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 0, 4), GetAdjustedUV(0, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 0, 4), GetAdjustedUV(0, 1, 3) });

					// Left
					vb->PushVertex({ GetAdjustedUV(0, 1, 4), GetAdjustedUV(1, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(0, 0, 4), GetAdjustedUV(1, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(0, 0, 4), GetAdjustedUV(1, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(0, 1, 4), GetAdjustedUV(1, 1, 3) });

					// Bottom
					vb->PushVertex({ GetAdjustedUV(1, 0, 4), GetAdjustedUV(2, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 1, 4), GetAdjustedUV(2, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 1, 4), GetAdjustedUV(2, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(1, 0, 4), GetAdjustedUV(2, 0, 3) });

					// Back
					vb->PushVertex({ GetAdjustedUV(3, 0, 4), GetAdjustedUV(1, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(3, 1, 4), GetAdjustedUV(1, 1, 3) });
					vb->PushVertex({ GetAdjustedUV(3, 1, 4), GetAdjustedUV(1, 0, 3) });
					vb->PushVertex({ GetAdjustedUV(3, 0, 4), GetAdjustedUV(1, 0, 3) });

					vb->SetLayout({ 2 });
					va->PushBuffer(std::move(vb));
				}

				va->SetElements({
					 0, 1, 2,   2, 3, 0,    // v0-v1-v2, v2-v3-v0 (front)
					 4, 5, 6,   6, 7, 4,    // v0-v3-v4, v4-v5-v0 (right)
					 8, 9,10,  10,11, 8,    // v0-v5-v6, v6-v1-v0 (top)
					12,13,14,  14,15,12,    // v1-v6-v7, v7-v2-v1 (left)
					16,17,18,  18,19,16,    // v7-v4-v3, v3-v2-v7 (bottom)
					20,21,22,  22,23,20     // v4-v7-v6, v6-v5-v4 (back)
					});
				va->Upload();

				mVALibrary.Load("AtlasTextureCube", va);
			}
#pragma endregion VertexArrays

#pragma region Shaders
			{
				auto vertexShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;

					layout (std140) uniform Globals
					{                   // base alignment  // aligned offset
					    mat4 proj;      // 16              // 0
					                    // 16              // 16
					                    // 16              // 32
					                    // 16              // 48
					    mat4 view;      // 16              // 64
					                    // 16              // 80
					                    // 16              // 96
					                    // 16              // 112
					    vec4 eyePos;    // 16              // 128
					    vec4 eyeForward;// 16              // 144

					    float time;     // 4               // 148
					};

                    //uniform mat4 proj = mat4(1.0);
                    //uniform mat4 view = mat4(1.0);
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
			{
				mShaderLibrary.Load("Model", LOAD_SHADER(ASSET_PATH("Shaders/basic_lit.vert"), ASSET_PATH("Shaders/basic_lit.frag")));
			}
			{
				auto vertexShader = R"(
                    #version 410 core

					layout (location = 0) in vec3 position;
					layout (location = 1) in vec2 texcoords;
					out vec2 uvs;

					uniform mat4 projection = mat4(1.0);
					uniform mat4 model = mat4(1.0);
					void main()
					{
					    uvs = texcoords;
					    gl_Position = projection * model * vec4(position.xy, 0.0, 1.0);
					    gl_Position = vec4(gl_Position.xy, 0.0, 1.0);
					}
                )";
				auto fragmentShader = R"(
                    #version 410 core

					out vec4 outColor;
					in vec2 uvs;
					
					uniform int textureSample = 0;
					uniform sampler2D tex;
					uniform vec4 bgColor = vec4(1.0, 1.0, 1.0, 1.0);
					void main()
					{
						if (textureSample == 1) {
							outColor = texture(tex, uvs) * bgColor;
						} else {
							outColor = bgColor;
						}
					}
                )";
				mShaderLibrary.Load("UI", std::make_shared<graphics::Shader>(vertexShader, fragmentShader));
			}
			{
				auto vertexShader = R"(
                    #version 410 core

					layout (location = 0) in vec3 position;

					uniform mat4 lightSpaceMatrix;
					uniform mat4 model;
					void main()
					{
					    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0);
					}
                )";
				auto fragmentShader = R"(
                    #version 410 core

					void main()
					{
					}
                )";
				mShaderLibrary.Load("ShadowDepthShader", std::make_shared<graphics::Shader>(vertexShader, fragmentShader));
			}
#pragma endregion Shaders

#pragma region Materials
			{
				auto mat = std::make_shared<graphics::Material>(mShaderLibrary.Get("Rect"), "RectRed");
				mat->SetUniformValue("col", glm::vec4(1, 0, 0, 1));
				mMaterialLibrary.Load("RectRed", mat);
			}

			{
				auto mat = std::make_shared<graphics::Material>(mShaderLibrary.Get("Rect"), "RectGreen");
				mat->SetUniformValue("col", glm::vec4(0, 1, 0, 1));
				mMaterialLibrary.Load("RectGreen", mat);
			}
			{
				auto mat = std::make_shared<graphics::Material>(mShaderLibrary.Get("UI"), "UI");
				mMaterialLibrary.Load("UI", mat);
			}
			{
				auto mat = std::make_shared<graphics::Material>(mShaderLibrary.Get("ShadowDepthShader"), "ShadowDepth");
				mMaterialLibrary.Load("ShadowDepthMaterial", mat);
			}
		}
#pragma endregion Materials

		inline static void UninitializeLibraries()
		{
			mMeshLibrary.Clear();
			mVALibrary.Clear();
			mMaterialLibrary.Clear();
			mShaderLibrary.Clear();
			mTextureLibrary.Clear();
		}

		static float GetAdjustedUV(int i, int x, int size)
		{
			return static_cast<float>((i % size) + x) / size;
		}
	};

}
