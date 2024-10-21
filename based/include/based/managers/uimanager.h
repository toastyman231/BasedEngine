#pragma once

// Done because including SDL prevents compiling for some reason
#include "external/imgui/imgui_impl_sdl.h"

#include <RmlUi/Core.h>

namespace based::managers
{
	class UiManager
	{
	public:
		UiManager() = default;
		~UiManager() = default;

		void Initialize();
		void ProcessEvents(SDL_Event e) const;
		void Update() const;
		void Render() const;
		void Shutdown();

		Rml::ElementDocument* LoadWindow(const std::string& path, Rml::Context* context) const;
		Rml::Context* CreateContext(const std::string& name, glm::ivec2 size);
		void RemoveContext(const Rml::Context* context);
		void SetPathPrefix(const std::string& path);

		Rml::SystemInterface& GetSystemInterface() const { return *mSystemInterface; }
		Rml::RenderInterface& GetRenderInterface() const { return *mRenderInterface; }
	private:
		Rml::SystemInterface* mSystemInterface;
		Rml::RenderInterface* mRenderInterface;

		std::vector<Rml::Context*> mContexts;

		Rml::String mPathPrefix;

		bool mUsePathPrefix;
	};
}

