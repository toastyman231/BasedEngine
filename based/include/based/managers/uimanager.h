#pragma once

// Done because including SDL prevents compiling for some reason
#include "external/imgui/imgui_impl_sdl.h"

#include <RmlUi/Core.h>

#include "based/ui/elementbinding.h"
#include "external/glm/detail/_features.hpp"

namespace based::managers
{
	struct DocumentInfo
	{
		Rml::ElementDocument* document;
		std::string path;
		Rml::Context* context;

		bool operator== (const DocumentInfo& other) const
		{
			return path == other.path;
		}
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

		DocumentInfo* LoadWindow(const std::string& path, Rml::Context* context, std::string id = "", bool show = true);
		bool CloseWindow(const DocumentInfo& document);
		Rml::Context* CreateContext(const std::string& name, glm::ivec2 size);
		Rml::Context* GetContext(const std::string& name);
		void RemoveContext(const Rml::Context* context);
		void SetPathPrefix(const std::string& path);

		void SetTranslationTable(const std::string& path);
		void SetCurrentLanguageIndex(int index);

		void AddBinding(ui::ElementBinding& binding);

		std::deque<DocumentInfo> GetDocuments();

		Rml::SystemInterface& GetSystemInterface() const { return *mSystemInterface; }
		Rml::RenderInterface& GetRenderInterface() const { return *mRenderInterface; }
	private:
		void HotReloadDocuments();

	private:
		Rml::SystemInterface* mSystemInterface;
		Rml::RenderInterface* mRenderInterface;

		std::vector<Rml::Context*> mContexts;
		std::deque<DocumentInfo> mDocuments;
		std::vector<ui::ElementBinding> mBindings;

		Rml::String mPathPrefix;

		bool mUsePathPrefix;

		class CustomEventInstancer : public Rml::EventInstancer
		{
		protected:
			void Release() override;

		public:
			Rml::EventPtr InstanceEvent(Rml::Element* target, Rml::EventId id, const Rml::String& type,
				const Rml::Dictionary& parameters, bool interruptible) override;
			void ReleaseEvent(Rml::Event* event) override;
		};

		CustomEventInstancer mEventInstancer;
	};
}

