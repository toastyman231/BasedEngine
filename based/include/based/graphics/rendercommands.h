#pragma once

#include <external/glm/mat4x4.hpp>

#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207

namespace based::graphics
{
	class VertexArray;
	class Shader;
	class Texture;
	class Framebuffer;
	class Camera;
	class Material;

	enum class PassInjectionPoint : uint8_t
	{
		BeforeShadowDepth = 0,
		BeforeMainColor = 1,
		BeforeUserInterface = 2
	};

	namespace rendercommands
	{
		class RenderCommand
		{
		public:
			virtual void Execute() = 0;
			virtual ~RenderCommand() {}

			bool ShouldClearBuffer() const { return mClearBuffer; }
		protected:
			bool mClearBuffer = true;
		};

		class RenderVertexArray : public RenderCommand
		{
		public:
			RenderVertexArray(std::weak_ptr<VertexArray> vertexArray, std::weak_ptr<Shader> shader, 
				const glm::mat4 modelMatrix = glm::mat4(1.f))
				: mVertexArray(std::move(vertexArray))
				  , mShader(std::move(shader))
				  , mModelMatrix(modelMatrix)
			{}

			virtual void Execute() override;
		private:
			std::weak_ptr<VertexArray> mVertexArray;
			std::weak_ptr<Shader> mShader;
			glm::mat4 mModelMatrix;
		};

		class RenderVertexArrayTextured : public RenderCommand
		{
		public:
			RenderVertexArrayTextured(std::weak_ptr<VertexArray> vertexArray, 
				std::weak_ptr<Texture> texture, std::weak_ptr<Shader> shader, 
				const glm::mat4& modelMatrix = glm::mat4(1.f))
				: mVertexArray(std::move(vertexArray))
				  , mTexture(std::move(texture))
				  , mShader(std::move(shader))
				  , mModelMatrix(modelMatrix)
			{}

			virtual void Execute() override;
		private:
			std::weak_ptr<VertexArray> mVertexArray;
			std::weak_ptr<Texture> mTexture;
			std::weak_ptr<Shader> mShader;
			glm::mat4 mModelMatrix;
		};

		class RenderVertexArrayMaterial : public RenderCommand
		{
		public:
			RenderVertexArrayMaterial(std::weak_ptr<VertexArray> vertexArray, std::weak_ptr<Material> material,
				const glm::mat4 modelMatrix = glm::mat4(1.f), const uint32_t depthFunc = GL_LEQUAL, 
				const bool instanced = false, const int count = 0,
				const int index = 0)
				: mVertexArray(std::move(vertexArray))
				  , mMaterial(std::move(material))
				  , mModelMatrix(modelMatrix)
				, mInstanced(instanced)
				, mDepthFunc(depthFunc)
				, mInstanceCount(count)
				, mIndex(index)
			{
			}

			virtual void Execute() override;
		private:
			std::weak_ptr<VertexArray> mVertexArray;
			std::weak_ptr<Material> mMaterial;
			glm::mat4 mModelMatrix;
			bool mInstanced;
			uint32_t mDepthFunc;
			int mInstanceCount;
			int mIndex;
		};

		class RenderLineMaterial : public RenderCommand
		{
		public:
			RenderLineMaterial(std::weak_ptr<VertexArray> vertexArray, std::weak_ptr<Material> material,
				const glm::mat4 modelMatrix = glm::mat4(1.f), const uint32_t depthFunc = GL_LEQUAL,
				const bool instanced = false, const int count = 0,
				const int index = 0)
				: mVertexArray(std::move(vertexArray))
				, mMaterial(std::move(material))
				, mModelMatrix(modelMatrix)
				, mInstanced(instanced)
				, mDepthFunc(depthFunc)
				, mInstanceCount(count)
				, mIndex(index)
			{
			}

			virtual void Execute() override;
		private:
			std::weak_ptr<VertexArray> mVertexArray;
			std::weak_ptr<Material> mMaterial;
			glm::mat4 mModelMatrix;
			bool mInstanced;
			uint32_t mDepthFunc;
			int mInstanceCount;
			int mIndex;
		};

		class RenderVertexArrayPostProcess : public RenderCommand
		{
		public:
			RenderVertexArrayPostProcess(std::weak_ptr<VertexArray> vertexArray, std::weak_ptr<Material> material,
				const glm::mat4 modelMatrix = glm::mat4(1.f))
				: mVertexArray(std::move(vertexArray))
				, mMaterial(std::move(material))
				, mModelMatrix(modelMatrix)
			{}

			virtual void Execute() override;
		private:
			std::weak_ptr<VertexArray> mVertexArray;
			std::weak_ptr<Material> mMaterial;
			glm::mat4 mModelMatrix;
		};

		class RenderVertexArrayUserInterface : public RenderCommand
		{
		public:
			RenderVertexArrayUserInterface(std::weak_ptr<VertexArray> vertexArray, std::weak_ptr<Shader> shader,
				uint32_t texture, const glm::mat4& transform, const glm::vec2 translation)
				: mVertexArray(std::move(vertexArray))
				  , mShader(std::move(shader))
				  , mTexture(texture)
				, mTransform(transform)
				, mTranslation(translation)
			{
			}

			virtual void Execute() override;
		private:
			std::weak_ptr<VertexArray> mVertexArray;
			std::weak_ptr<Shader> mShader;
			uint32_t mTexture;
			glm::mat4 mTransform;
			glm::vec2 mTranslation;
		};

		class PushFramebuffer : public RenderCommand
		{
		public:
			PushFramebuffer(std::weak_ptr<Framebuffer> framebuffer, 
				const std::string& name = "Unnamed Pass",
				bool clearBuffer = true)
			: mFramebuffer(std::move(framebuffer)), mName(name)
			{
				mClearBuffer = clearBuffer;
			}
			virtual void Execute() override;
		private:
			std::weak_ptr<Framebuffer> mFramebuffer;
			std::string mName;
		};

		class PopFramebuffer : public RenderCommand
		{
		public:
			PopFramebuffer() = default;
			virtual void Execute() override;
		};

		class UpdateGlobals : public RenderCommand
		{
		public:
			UpdateGlobals(){}
			virtual void Execute() override;
		};

		class SetWireframe : public RenderCommand
		{
		public:
			SetWireframe(bool wireframe) : mWireframe(wireframe) {}
			virtual void Execute() override;
		private:
			bool mWireframe;
		};

		class PushCamera : public RenderCommand
		{
		public:
			PushCamera(std::weak_ptr<Camera> camera) : mCamera(std::move(camera)) {}
			virtual void Execute() override;
		private:
			std::weak_ptr<Camera> mCamera;
		};

		class PopCamera : public RenderCommand
		{
		public:
			PopCamera() = default;
			virtual void Execute() override;
		};
	}
}