#include "managers/uimanager.h"

#include "based/ui/uisysteminterface.h"
#include "based/ui/uirenderinterface.h"

namespace based::managers
{
	void UiManager::Initialize()
	{
		mSystemInterface = new ui::SystemInterface_SDL();
		mRenderInterface = new ui::RenderInterface_GL4();

		Rml::SetSystemInterface(mSystemInterface);
		Rml::SetRenderInterface(mRenderInterface);

		Rml::Initialise();

		Rml::LoadFontFace("Assets/fonts/Arimo-Regular.ttf");
	}

	void UiManager::ProcessEvents(SDL_Event e) const
	{
		for (const auto context : mContexts)
		{
			RmlSDL::InputEventHandler(context, e);
		}
	}

	void UiManager::Update() const
	{
		for (const auto context : mContexts)
		{
			context->Update();
		}
	}

	void UiManager::Render() const
	{
		dynamic_cast<ui::RenderInterface_GL4*>(mRenderInterface)->BeginFrame();
		for (const auto context : mContexts)
		{
			context->Render();
		}
		dynamic_cast<ui::RenderInterface_GL4*>(mRenderInterface)->EndFrame();
	}

	void UiManager::Shutdown()
	{
		Rml::Shutdown();
	}

	Rml::ElementDocument* UiManager::LoadWindow(const std::string& path, Rml::Context* context) const
	{
		Rml::String documentPath = path + Rml::String(".rml");
		if (mUsePathPrefix) documentPath = mPathPrefix + documentPath;

		Rml::ElementDocument* document = context->LoadDocument(documentPath);
		if (!document)
		{
			BASED_ERROR("Failed to load document at {}", documentPath);
			return nullptr;
		}

		if (Rml::Element* title = document->GetElementById("title"))
			title->SetInnerRML(document->GetTitle());

		document->Show();

		return document;
	}

	Rml::Context* UiManager::CreateContext(const std::string& name, glm::ivec2 size)
	{
		Rml::Context* context = Rml::CreateContext(name, Rml::Vector2i(size.x, size.y));
		if (!context)
		{
			BASED_ERROR("Error initializing context!");
			return nullptr;
		}

		mContexts.emplace_back(context);
		return context;
	}

	void UiManager::RemoveContext(const Rml::Context* context)
	{
		if (context)
		{
			mContexts.erase(
				std::remove_if(
					mContexts.begin(),
					mContexts.end(),
					[&](Rml::Context* cont)
					{
						return cont->GetName() == context->GetName();
					}
				), mContexts.end());
			Rml::RemoveContext(context->GetName());
		}
	}

	void UiManager::SetPathPrefix(const std::string& path)
	{
		if (path == "")
		{
			mUsePathPrefix = false;
			mPathPrefix = "";
			return;
		}

		mPathPrefix = path;
		mUsePathPrefix = true;
	}
}
