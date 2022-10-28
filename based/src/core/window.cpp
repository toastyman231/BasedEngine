#include "core/window.h"
#include "SDL2/SDL.h"
#include "engine.h"
#include "log.h"
#include "glad/glad.h"
#include "input/mouse.h"
#include "input/keyboard.h"

namespace based::core
{
	Window::Window() : mWindow(nullptr) {}
	Window::~Window()
	{
		if (mWindow)
		{
			Shutdown();
		}
	}

	bool Window::Create()
	{
		mWindow = SDL_CreateWindow("BasedGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

		SDL_SetWindowMinimumSize(mWindow, 200, 200);

		mGLContext = SDL_GL_CreateContext(mWindow);
		if (mGLContext == nullptr)
		{
			BASED_ERROR("Error creating OpenGL context: {}", SDL_GetError());
			return false;
		}

		gladLoadGLLoader(SDL_GL_GetProcAddress);

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
				default:
					break;
			}
		}

		// Update input
		input::Mouse::Update();
		input::Keyboard::Update();
	}

	void Window::GetSize(int& width, int& height)
	{
		SDL_GetWindowSize(mWindow, &width, &height);
	}

	void Window::BeginRender()
	{
		Engine::Instance().GetRenderManager().Clear();
	}

	void Window::EndRender()
	{
		SDL_GL_SwapWindow(mWindow);
	}
}