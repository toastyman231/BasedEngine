#pragma once

#include "based/pch.h"

#include "imguiwindow.h"
#include "graphics/framebuffer.h"

struct SDL_Window;
using SDL_GLContext = void*;
namespace based::graphics
{
	class Framebuffer;
	class VertexArray;
	class Shader;
	class Texture;
}

namespace based::core
{
	struct WindowProperties
	{
		std::string title;
		int x, y, w, h;
		int wMin, hMin;
		int flags;
		float aspectRatio;
		glm::vec3 clearColor;
		ImguiWindowProperties imguiProps;

		WindowProperties();
	};

	class Window
	{
	public:
		Window();
		~Window();

		inline void SetShouldRenderToScreen(bool render) { mShouldRenderToScreen = render; }
		bool GetShouldRenderToScreen() const { return mShouldRenderToScreen; }

		bool Create(const WindowProperties& props);
		void Shutdown();

		void PumpEvents();

		glm::ivec2 GetSize();

		inline SDL_Window* GetSDLWindow() { return mWindow; }
		inline SDL_GLContext GetGLContext() { return mGLContext; }
		inline graphics::Framebuffer* GetFramebuffer() { return mFramebuffer.get(); }
		void SetMaintainAspectRatio(bool newMaintain);

		void BeginRender();
		void EndRender();

		glm::ivec2 GetSizeInAspectRatio(int width, int height);

		uint32_t GetShadowMapTextureID() const { return mShadowbuffer->GetTextureId(); }

		bool isInDepthPass = false;
	private:
		void InitializeScreenRender();
		void RenderToScreen();
		void RenderShadowDepth();
		void HandleResize(int width, int height);

		WindowProperties mWindowProperties;
		SDL_Window* mWindow;
		SDL_GLContext mGLContext;
		ImguiWindow mImguiWindow;
		std::shared_ptr<graphics::Framebuffer> mFramebuffer;
		std::shared_ptr<graphics::Framebuffer> mShadowbuffer;
		bool mMaintainAspectRatio = true;

		// Screen render
		bool mShouldRenderToScreen;
		glm::vec2 mFramebufferSize;
		std::shared_ptr<graphics::VertexArray> mScreenVA;
		std::shared_ptr<graphics::Shader> mScreenShader;
	};
}