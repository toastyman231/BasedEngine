#include "pch.h"
#include "managers/uimanager.h"

#include "based/ui/uisysteminterface.h"
#include "based/ui/uirenderinterface.h"
#include "input/keyboard.h"

namespace based::managers
{
	void UiManager::Initialize()
	{
		PROFILE_FUNCTION();
		mSystemInterface = new ui::SystemInterface_SDL();
		mRenderInterface = new ui::RenderInterface_GL4();

		Rml::SetSystemInterface(mSystemInterface);
		Rml::SetRenderInterface(mRenderInterface);

		Rml::Initialise();

		Rml::LoadFontFace("Assets/fonts/Arimo-Regular.ttf");
	}

	void UiManager::ProcessEvents(SDL_Event e) const
	{
		PROFILE_FUNCTION();
		for (const auto context : mContexts)
		{
			RmlSDL::InputEventHandler(context, e);
		}
	}

	void UiManager::Update() const
	{
		PROFILE_FUNCTION();
#ifdef BASED_CONFIG_DEBUG
		if (input::Keyboard::Key(BASED_INPUT_KEY_LALT) && input::Keyboard::KeyDown(BASED_INPUT_KEY_R))
			HotReloadDocuments();
#endif

		for (const auto context : mContexts)
		{
			context->Update();
		}
	}

	void UiManager::Render() const
	{
		PROFILE_FUNCTION();
		dynamic_cast<ui::RenderInterface_GL4*>(mRenderInterface)->BeginFrame();
		for (const auto context : mContexts)
		{
			context->Render();
		}
		dynamic_cast<ui::RenderInterface_GL4*>(mRenderInterface)->EndFrame();
	}

	void UiManager::Shutdown()
	{
		PROFILE_FUNCTION();
		Rml::Shutdown();
		delete mRenderInterface;
		delete mSystemInterface;
	}

	DocumentInfo* UiManager::LoadWindow(const std::string& path, Rml::Context* context)
	{
		PROFILE_FUNCTION();
		Rml::String documentPath = path + Rml::String(".rml");
		if (mUsePathPrefix) documentPath = mPathPrefix + documentPath;

		Rml::ElementDocument* document = context->LoadDocument(documentPath);
		if (!document)
		{
			BASED_ERROR("Failed to load document at {}", documentPath);
			return new DocumentInfo();
		}

		if (Rml::Element* title = document->GetElementById("title"))
			title->SetInnerRML(document->GetTitle());

		document->SetId(std::to_string(context->GetNumDocuments() - 1));
		document->Show();

		mDocuments.emplace_back(new DocumentInfo{ document, documentPath, context });

		return mDocuments.back();
	}

	Rml::Context* UiManager::CreateContext(const std::string& name, glm::ivec2 size)
	{
		PROFILE_FUNCTION();
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
		PROFILE_FUNCTION();
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
		if (path.empty())
		{
			mUsePathPrefix = false;
			mPathPrefix = "";
			return;
		}

		mPathPrefix = path;
		mUsePathPrefix = true;
	}

	void UiManager::HotReloadDocuments() const
	{
		for (auto docInfo : mDocuments)
		{
			Rml::ElementDocument* rawPointer = docInfo->document;
			Rml::Context* context = docInfo->context;
			const auto& path = docInfo->path;

			bool isShown = rawPointer->IsVisible();

			Rml::ElementDocument* document = context->LoadDocument(path);
			document->ReloadStyleSheet();
			if (!document)
			{
				BASED_ERROR("Failed to hot reload document at {}", path);
				continue;
			}

			if (Rml::Element* title = document->GetElementById("title"))
				title->SetInnerRML(document->GetTitle());

			if (isShown)
				document->Show();

			rawPointer->Close();
			docInfo->document = document;
		}
	}
}
