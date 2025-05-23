#include "pch.h"
#include "core/window.h"
#include "SDL2/SDL.h"
#include "engine.h"
#include "app.h"
#include "log.h"
#include "glad/glad.h"
#include "graphics/framebuffer.h"
#include "graphics/vertex.h"
#include "graphics/shader.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/joystick.h"
#include "graphics/defaultassetlibraries.h"

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
	{
	}

	Window::~Window()
	{
		if (mWindow)
		{
			Shutdown();
			SDL_FreeCursor(cursor_default);
			SDL_FreeCursor(cursor_move);
			SDL_FreeCursor(cursor_pointer);
			SDL_FreeCursor(cursor_resize);
			SDL_FreeCursor(cursor_cross);
			SDL_FreeCursor(cursor_text);
			SDL_FreeCursor(cursor_unavailable);
		}
	}

	bool Window::Create(const WindowProperties& props)
	{
		PROFILE_FUNCTION();
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
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#ifdef BASED_CONFIG_DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

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

		mShadowbuffer = std::make_shared<graphics::Framebuffer>(2048, 2048, GL_DEPTH_COMPONENT, GL_FLOAT,
			graphics::TextureFilter::Nearest, GL_DEPTH_ATTACHMENT, false);
		mShadowbuffer->SetClearColor({ props.clearColor.r, props.clearColor.g, props.clearColor.b, 1.f });

		InitializeScreenRender();
		HandleResize(props.w, props.h);

		cursor_default = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		cursor_move = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
		cursor_pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		cursor_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
		cursor_cross = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
		cursor_text = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		cursor_unavailable = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

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
		PROFILE_FUNCTION();
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
					if (Engine::Instance().GetApp().ValidateShutdown())
						Engine::Instance().Quit();
					break;
				case SDL_CONTROLLERDEVICEADDED:
					input::Joystick::OnJoystickConnected(e.cdevice);
					break;
				case SDL_CONTROLLERDEVICEREMOVED:
					input::Joystick::OnJoystickDisconnected(e.cdevice);
					break;
				case SDL_DROPBEGIN:
					BASED_TRACE("Drop beginning!");
					break;
				case SDL_DROPCOMPLETE:
					BASED_TRACE("Drop complete!");
					break;
				case SDL_DROPFILE:
					Engine::Instance().GetApp().HandleFileDrop(e.drop.file);
					SDL_free(e.drop.file);
					break;
				case SDL_WINDOWEVENT:
					switch (e.window.event)
					{
						case SDL_WINDOWEVENT_RESIZED:
							HandleResize(e.window.data1, e.window.data2);
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}

			Engine::Instance().GetUiManager().ProcessEvents(e);
			mImguiWindow.HandleSDLEvent(e);
		}

		// Update input
		if (!mImguiWindow.WantCaptureMouse()) input::Mouse::Update();
		if (!mImguiWindow.WantCaptureKeyboard()) input::Keyboard::Update();
		
		input::Joystick::Update();
	}

	void Window::SetCursor(const std::string& cursorName)
	{
		SDL_Cursor* cursor = nullptr;

		if (cursorName.empty() || cursorName == "arrow")
			cursor = cursor_default;
		else if (cursorName == "move")
			cursor = cursor_move;
		else if (cursorName == "pointer")
			cursor = cursor_pointer;
		else if (cursorName == "resize")
			cursor = cursor_resize;
		else if (cursorName == "cross")
			cursor = cursor_cross;
		else if (cursorName == "text")
			cursor = cursor_text;
		else if (cursorName == "unavailable")
			cursor = cursor_unavailable;

		if (cursor)
			SDL_SetCursor(cursor);
	}

	glm::ivec2 Window::GetSize()
	{
		int w, h;
		SDL_GetWindowSize(mWindow, &w, &h);
		return glm::ivec2(w, h);
	}

	void Window::SetMaintainAspectRatio(bool newMaintain)
	{
		mMaintainAspectRatio = newMaintain; 
		HandleResize(GetSize().x, GetSize().y);
	}

	void Window::SetWindowTitle(const std::string& title)
	{
		SDL_SetWindowTitle(mWindow, title.c_str());
		mWindowProperties.title = title;
	}

	void Window::SetFullscreen(bool isFullscreen)
	{
		Uint32 flags = 0;
		if (isFullscreen) flags = SDL_WINDOW_FULLSCREEN_DESKTOP; // TODO: Add support for true fullscreen
		SDL_SetWindowFullscreen(mWindow, flags);
	}

	void Window::BeginRender()
	{
		PROFILE_FUNCTION();
		auto& rm = Engine::Instance().GetRenderManager();
		rm.Clear();
	}

	void Window::EndRender()
	{
		PROFILE_FUNCTION();

		if (mShouldRenderToScreen)
		{
			RenderToScreen();
		}

		if (!mShouldRenderToScreen)
		{
			mImguiWindow.BeginRender();
			Engine::Instance().GetApp().ImguiRender();
			mImguiWindow.EndRender();
		}

		SDL_GL_SwapWindow(mWindow);
	}

	void Window::InitializeScreenRender()
	{
		PROFILE_FUNCTION();
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
		PROFILE_FUNCTION();
		BASED_ASSERT(mScreenVA->IsValid(), "Attempting to render with invalid VertexArray - did you forget to call VertexArray::Upload()?");
		if (mScreenVA->IsValid())
		{
			// Black bars
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			mScreenVA->Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 
				graphics::DefaultLibraries::GetRenderPassOutputs().Get("SceneColor"));
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
		if (!mMaintainAspectRatio) 
		{
			mFramebufferSize = { width, height };
			return;
		}

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
