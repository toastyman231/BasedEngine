#pragma once

// Done because including SDL prevents compiling for some reason
#include "external/imgui/imgui_impl_sdl.h"

#include <RmlUi/Core.h>

namespace based::managers
{
	struct DocumentInfo
	{
		Rml::ElementDocument* document;
		std::string path;
		Rml::Context* context;
	};

	class UiManager
	{
	public:
		UiManager() = default;
		~UiManager() = default;

		void Initialize();
		void ProcessEvents(SDL_Event e) const;
		void Update();
		void Render() const;
		void Shutdown();

		DocumentInfo* LoadWindow(const std::string& path, Rml::Context* context);
		Rml::Context* CreateContext(const std::string& name, glm::ivec2 size);
		Rml::Context* GetContext(const std::string& name);
		void RemoveContext(const Rml::Context* context);
		void SetPathPrefix(const std::string& path);

		std::vector<DocumentInfo*> GetDocuments();

		Rml::SystemInterface& GetSystemInterface() const { return *mSystemInterface; }
		Rml::RenderInterface& GetRenderInterface() const { return *mRenderInterface; }
	private:
		void HotReloadDocuments();

		Rml::SystemInterface* mSystemInterface;
		Rml::RenderInterface* mRenderInterface;

		std::vector<Rml::Context*> mContexts;
		std::vector<DocumentInfo*> mDocuments;

		Rml::String mPathPrefix;

		bool mUsePathPrefix;
	};
}

