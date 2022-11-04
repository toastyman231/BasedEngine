#include "core/window.h"
#include "SDL2/SDL.h"
#include "engine.h"
#include "app.h"
#include "log.h"
#include "glad/glad.h"
#include "graphics/framebuffer.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/joystick.h"

namespace based::core
{
	WindowProperties::WindowProperties()
	{
		title = "BasedApp";
		x = SDL_WINDOWPOS_CENTERED;
		y = SDL_WINDOWPOS_CENTERED;
		w = 1920;
		h = 1080;
		wMin = 320;
		hMin = 180;
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
		clearColor = glm::vec3(
			static_cast<float>(0x64) / static_cast<float>(0xFF),
			static_cast<float>(0x95) / static_cast<float>(0xFF),
			static_cast<float>(0xED) / static_cast<float>(0xFF));
	}

	Window::Window() : mWindow(nullptr) {}
	Window::~Window()
	{
		if (mWindow)
		{
			Shutdown();
		}
	}

	bool Window::Create(const WindowProperties& props)
	{
		mWindow = SDL_CreateWindow(props.title.c_str(), props.x, props.y, props.w, props.h, props.flags);
		if (!mWindow)
		{
			BASED_ERROR("Error creating window: {}", SDL_GetError());
			return false;
		}

#ifdef BASED_PLATFORM_MAC
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_SetWindowMinimumSize(mWindow, props.wMin, props.hMin);

		mGLContext = SDL_GL_CreateContext(mWindow);
		if (mGLContext == nullptr)
		{
			BASED_ERROR("Error creating OpenGL context: {}", SDL_GetError());
			return false;
		}

		gladLoadGLLoader(SDL_GL_GetProcAddress);

		mFramebuffer = std::make_shared<graphics::Framebuffer>(props.w, props.h);
		mFramebuffer->SetClearColor({ props.clearColor.r, props.clearColor.g, props.clearColor.b, 1.f });

		mImguiWindow.Create(props.imguiProps);
		return true;
	}

	void Window::Shutdown()
	{
		SDL_DestroyWindow(mWindow);
		SDL_GL_DeleteContext(mGLContext);
		mWindow = nullptr;
	}

	void Window::PumpEvents()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
					Engine::Instance().Quit();
					break;
				case SDL_CONTROLLERDEVICEADDED:
					input::Joystick::OnJoystickConnected(e.cdevice);
					break;
				case SDL_CONTROLLERDEVICEREMOVED:
					input::Joystick::OnJoystickDisconnected(e.cdevice);
					break;
				default:
					break;
			}

			mImguiWindow.HandleSDLEvent(e);
		}

		// Update input
		if (!mImguiWindow.WantCaptureMouse()) input::Mouse::Update();
		if (!mImguiWindow.WantCaptureKeyboard()) input::Keyboard::Update();
		
		input::Joystick::Update();
	}

	glm::ivec2 Window::GetSize()
	{
		int w, h;
		SDL_GetWindowSize(mWindow, &w, &h);
		return glm::ivec2(w, h);
	}

	void Window::BeginRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Clear();
		rm.Submit(BASED_SUBMIT_RC(PushFramebuffer, mFramebuffer));
	}

	void Window::EndRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Submit(BASED_SUBMIT_RC(PopFramebuffer));
		rm.Flush();

		mImguiWindow.BeginRender();
		Engine::Instance().GetApp().ImguiRender();
		mImguiWindow.EndRender();

		SDL_GL_SwapWindow(mWindow);
	}
}