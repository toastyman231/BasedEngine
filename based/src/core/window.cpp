#include "core/window.h"
#include "SDL2/SDL.h"
#include "engine.h"
#include "app.h"
#include "log.h"
#include "glad/glad.h"
#include "external/glm/gtc/matrix_transform.hpp"
#include "graphics/framebuffer.h"
#include "graphics/vertex.h"
#include "graphics/shader.h"
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
		aspectRatio = 16.f / 9.f;
		clearColor = glm::vec3(
			static_cast<float>(0x64) / static_cast<float>(0xFF),
			static_cast<float>(0x95) / static_cast<float>(0xFF),
			static_cast<float>(0xED) / static_cast<float>(0xFF));
	}

	Window::Window() 
		: mWindow(nullptr) 
		, mShouldRenderToScreen(true)
	{}
	Window::~Window()
	{
		if (mWindow)
		{
			Shutdown();
		}
	}

	bool Window::Create(const WindowProperties& props)
	{
		mWindowProperties = props;
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

		InitializeScreenRender();
		HandleResize(props.w, props.h);

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
				case SDL_WINDOWEVENT:
					if (e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						HandleResize(e.window.data1, e.window.data2);
					}
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

		if (mShouldRenderToScreen)
		{
			RenderToScreen();
		}

		mImguiWindow.BeginRender();
		Engine::Instance().GetApp().ImguiRender();
		mImguiWindow.EndRender();

		SDL_GL_SwapWindow(mWindow);
	}

	void Window::InitializeScreenRender()
	{
		// VertexArray
		mScreenVA = std::make_shared<graphics::VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(vb, short);
		vb->PushVertex({ 1, 1, 1, 1 });
		vb->PushVertex({ 1, -1, 1, 0 });
		vb->PushVertex({ -1, -1, 0, 0 });
		vb->PushVertex({ -1, 1, 0, 1 });
		vb->SetLayout({ 2, 2 });
		mScreenVA->PushBuffer(std::move(vb));
		mScreenVA->SetElements({ 0, 3, 1, 1, 3, 2 });
		mScreenVA->Upload();

		// Shaders
		const char* vertexShader = R"(
                    #version 410 core
                    layout (location = 0) in vec2 position;
                    layout (location = 1) in vec2 texcoords;
                    out vec2 uvs;

                    uniform mat4 model = mat4(1.0);
                    void main()
                    {
                        uvs = texcoords;
                        gl_Position = model * vec4(position, 0.0, 1.0);
                    }
                )";
		const char* fragmentShader = R"(
                    #version 410 core
                    out vec4 outColor;
                    in vec2 uvs;

                    uniform sampler2D tex;
                    void main()
                    {
                        outColor = texture(tex, uvs);
                    }
                )";
		mScreenShader = std::make_shared<graphics::Shader>(vertexShader, fragmentShader);
	}

	void Window::RenderToScreen()
	{
		BASED_ASSERT(mScreenVA->IsValid(), "Attempting to render with invalid VertexArray - did you forget to call VertexArray::Upload()?");
		if (mScreenVA->IsValid())
		{
			// Black bars
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			mScreenVA->Bind();
			glBindTexture(GL_TEXTURE_2D, mFramebuffer->GetTextureId());
			mScreenShader->Bind();

			glm::vec2 scale = mFramebufferSize / (glm::vec2) GetSize();
			glm::mat4 model(1.0);
			model = glm::scale(model, { scale.x, scale.y, 1.f });
			mScreenShader->SetUniformMat4("model", model);
			glDrawElements(GL_TRIANGLES, mScreenVA->GetElementCount(), GL_UNSIGNED_INT, 0);

			mScreenVA->Unbind();
			glBindTexture(GL_TEXTURE_2D, 0);
			mScreenShader->Unbind();
		}
	}

	void Window::HandleResize(int width, int height)
	{
		

		mFramebufferSize = GetSizeInAspectRatio(width, height);
	}

	glm::ivec2 Window::GetSizeInAspectRatio(int width, int height)
	{
		int framebufferWidth = (int)(height * mWindowProperties.aspectRatio);
		int framebufferHeight = (int)(width * (1.f / mWindowProperties.aspectRatio));

		if (height >= framebufferHeight)
		{
			framebufferWidth = width;
		}
		else
		{
			framebufferHeight = height;
		}

		return { framebufferWidth, framebufferHeight };
	}
}