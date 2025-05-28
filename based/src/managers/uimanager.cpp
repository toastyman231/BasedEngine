#include "pch.h"
#include "managers/uimanager.h"

#include "engine.h"
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

		Rml::LoadFontFace(ASSET_PATH("Fonts/Arimo-Regular.ttf"), true);
		Rml::LoadFontFace(ASSET_PATH("Fonts/Arimo-Italic.ttf"));
		Rml::LoadFontFace(ASSET_PATH("Fonts/Arimo-BoldItalic.ttf"));
		Rml::LoadFontFace(ASSET_PATH("Fonts/Arimo-Bold.ttf"));

		Engine::Instance().GetUiManager().CreateContext("main", Engine::Instance().GetWindow().GetSize());
	}

	void UiManager::ProcessEvents(SDL_Event e) const
	{
		PROFILE_FUNCTION();
		for (const auto context : mContexts)
		{
			RmlSDL::InputEventHandler(context, e);
		}
	}

	void UiManager::Update()
	{
		PROFILE_FUNCTION();
#ifdef BASED_CONFIG_DEBUG
		if (input::Keyboard::Key(BASED_INPUT_KEY_LALT) && input::Keyboard::KeyDown(BASED_INPUT_KEY_R))
			HotReloadDocuments();
#endif

		for (const auto& binding : mBindings)
		{
			binding.ResolveBinding();
		}

		for (const auto context : mContexts)
		{
			if (context) context->Update();
		}
	}

	void UiManager::Render() const
	{
		PROFILE_FUNCTION();
		dynamic_cast<ui::RenderInterface_GL4*>(mRenderInterface)->BeginFrame();
		for (const auto context : mContexts)
		{
			if (context) context->Render();
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

	DocumentInfo* UiManager::LoadWindow(const std::string& path, Rml::Context* context, std::string id)
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

		if (!id.empty()) document->SetId(id);
		document->Show();

		mDocuments.emplace_back(DocumentInfo{ document, documentPath, context });

		return &mDocuments.back();
	}

	bool UiManager::CloseWindow(const DocumentInfo& document)
	{
		document.document->Close();

		auto it = std::find(mDocuments.begin(), mDocuments.end(), document);

		if (it != mDocuments.end())
		{
			mDocuments.erase(it);
			return true;
		}

		return false;
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

	Rml::Context* UiManager::GetContext(const std::string& name)
	{
		for (auto context : mContexts)
		{
			if (context->GetName() == name) return context;
		}

		BASED_WARN("Context {} does not exist! Returning null.", name);
		return nullptr;
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

	void UiManager::SetTranslationTable(const std::string& path)
	{
		ui::SystemInterface_SDL* sys = (ui::SystemInterface_SDL*)&Engine::Instance().GetUiManager().GetSystemInterface();
		sys->SetTranslationTable(path);
	}

	void UiManager::SetCurrentLanguageIndex(int index)
	{
		ui::SystemInterface_SDL* sys = (ui::SystemInterface_SDL*)&Engine::Instance().GetUiManager().GetSystemInterface();
		sys->SetCurrentLanguageIndex(index);
	}

	void UiManager::AddBinding(ui::ElementBinding& binding)
	{
		mBindings.emplace_back(std::move(binding));
	}

	std::deque<DocumentInfo> UiManager::GetDocuments()
	{
		return mDocuments;
	}

	void UiManager::HotReloadDocuments()
	{
		std::vector<DocumentInfo> docsToDelete;

		Rml::Factory::ClearStyleSheetCache();
		Rml::Factory::ClearTemplateCache();

		for (auto& docInfo : mDocuments)
		{
			Rml::ElementDocument* rawPointer = docInfo.document;
			Rml::Context* context = docInfo.context;
			const auto& path = docInfo.path;

			if (!rawPointer || !context)
			{
				BASED_TRACE("Bad pointer to document or context, skipping {}!", path);
				docsToDelete.emplace_back(docInfo);
				continue;
			}

			BASED_TRACE("Reloading {}", rawPointer->GetSourceURL());

			bool isShown = rawPointer->IsVisible();

			rawPointer = context->LoadDocument(path);
			rawPointer->ReloadStyleSheet();
			if (!rawPointer)
			{
				BASED_ERROR("Failed to hot reload document at {}", path);
				continue;
			}

			if (Rml::Element* title = rawPointer->GetElementById("title"))
				title->SetInnerRML(rawPointer->GetTitle());

			rawPointer->SetId(docInfo.document->GetId());

			Rml::ElementList elements;
			rawPointer->GetElementsByClassName(elements, "BASED_REFRESH");
			for (auto element : elements)
			{
				element->SetInnerRML(docInfo.document->GetElementById(element->GetId())->GetInnerRML());
			}

			if (isShown)
				rawPointer->Show();

			docInfo.document->Close();
			docInfo.document = rawPointer;
		}

		for (auto& docInfo : docsToDelete)
		{
			auto it = std::find(mDocuments.begin(), mDocuments.end(), docInfo);

			if (it != mDocuments.end())
			{
				mDocuments.erase(it);
			}
		}
	}
}
