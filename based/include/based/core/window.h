#pragma once

#include "imguiwindow.h"
#include "graphics/framebuffer.h"

#include <memory>
#include <external/glm/vec2.hpp>
#include <external/glm/vec3.hpp>

struct SDL_Cursor;
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

		void SetCursor(const std::string& cursorName);

		glm::ivec2 GetSize();
		glm::vec2 GetFramebufferSize() const { return mFramebufferSize; }
		std::string GetWindowTitle() const { return mWindowProperties.title; }

		inline SDL_Window* GetSDLWindow() const { return mWindow; }
		inline SDL_GLContext GetGLContext() const { return mGLContext; }
		inline graphics::Framebuffer* GetFramebufferRaw() { return mFramebuffer.get(); }
		inline std::shared_ptr<graphics::Framebuffer> GetFramebuffer() { return mFramebuffer; }
		inline std::shared_ptr<graphics::Framebuffer> GetShadowBuffer() { return mShadowbuffer; }
		void SetMaintainAspectRatio(bool newMaintain);
		void SetWindowTitle(const std::string& title);
		void SetFullscreen(bool isFullscreen);

		void BeginRender();
		void EndRender();

		glm::ivec2 GetSizeInAspectRatio(int width, int height);
	private:
		void InitializeScreenRender();
		void RenderToScreen();
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

		SDL_Cursor* cursor_default = nullptr;
		SDL_Cursor* cursor_move = nullptr;
		SDL_Cursor* cursor_pointer = nullptr;
		SDL_Cursor* cursor_resize = nullptr;
		SDL_Cursor* cursor_cross = nullptr;
		SDL_Cursor* cursor_text = nullptr;
		SDL_Cursor* cursor_unavailable = nullptr;
	};
}