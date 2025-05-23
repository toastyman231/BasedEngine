#include "pch.h"
#include "core/imguiwindow.h"

#include "engine.h"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_sdl.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "SDL2/SDL.h"

namespace based::core
{
	void ImguiWindow::Create(const ImguiWindowProperties& props)
	{
		IMGUI_CHECKVERSION();

		ImGui::SetCurrentContext(ImGui::CreateContext());

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigWindowsMoveFromTitleBarOnly = props.MoveFromTitleBarOnly;
		if (props.IsDockingEnabled)
		{
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		}
		if (props.IsViewportEnabled)
		{
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}

		auto& window = Engine::Instance().GetWindow();
		ImGui_ImplSDL2_InitForOpenGL(window.GetSDLWindow(), window.GetGLContext());
		ImGui_ImplOpenGL3_Init("#version 430");
	}

	void ImguiWindow::Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext(ImGui::GetCurrentContext());
	}

	void ImguiWindow::HandleSDLEvent(SDL_Event& e)
	{
		ImGui_ImplSDL2_ProcessEvent(&e);
	}

	bool ImguiWindow::WantCaptureMouse()
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool ImguiWindow::WantCaptureKeyboard()
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	void ImguiWindow::BeginRender()
	{
		auto& rm = Engine::Instance().GetRenderManager();
		rm.PushDebugGroup("ImGuiPass");
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(Engine::Instance().GetWindow().GetSDLWindow());
		ImGui::NewFrame();
	}

	void ImguiWindow::EndRender()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			auto& window = Engine::Instance().GetWindow();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(window.GetSDLWindow(), window.GetGLContext());
		}
		auto& rm = Engine::Instance().GetRenderManager();
		rm.PopDebugGroup();
	}
}