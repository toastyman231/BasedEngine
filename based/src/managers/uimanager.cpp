#include "pch.h"
#include "managers/uimanager.h"

#include "basedtime.h"
#include "engine.h"
#include "based/ui/uisysteminterface.h"
#include "based/ui/uirenderinterface.h"
#include "input/joystick.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "math/basedmath.h"

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
		Rml::Factory::RegisterEventInstancer(&mEventInstancer);
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

		auto rawYValue = input::Joystick::GetAxisFromAnyController(input::Joystick::Axis::LeftStickVertical);
		auto rawXValue = input::Joystick::GetAxisFromAnyController(input::Joystick::Axis::LeftStickHorizontal);
		float valueY = math::MapRange(rawYValue, -32768.f, 32767.f, -1.f, 1.f);
		float valueX = math::MapRange(rawXValue, -32768.f, 32767.f, -1.f, 1.f);

		static float lastAxisTime = 0.f;
		constexpr float minAxisTime = 0.25f;
		if (core::Time::GetUnscaledTime() - lastAxisTime >= minAxisTime)
		{
			if (math::Abs(valueY) > 0.5f || math::Abs(valueX) > 0.5f)
			{
				input::Mouse::SetCursorVisible(false);
				lastAxisTime = core::Time::GetUnscaledTime();
			}

			if (valueY < -0.5f)
			{
				for (auto context : mContexts)
				{
					context->ProcessKeyDown(RmlSDL::ConvertKey(SDLK_UP), 0);
				}
			}
			else if (valueY > 0.5f)
			{
				for (auto context : mContexts)
				{
					context->ProcessKeyDown(RmlSDL::ConvertKey(SDLK_DOWN), 0);
				}
			}

			if (valueX < -0.5f)
			{
				for (auto context : mContexts)
				{
					context->ProcessKeyDown(RmlSDL::ConvertKey(SDLK_LEFT), 0);
				}
			}
			else if (valueX > 0.5f)
			{
				for (auto context : mContexts)
				{
					context->ProcessKeyDown(RmlSDL::ConvertKey(SDLK_RIGHT), 0);
				}
			}
		}

		if (math::Abs(valueY) >= 0.2f)
		{
			for (auto context : mContexts)
			{
				context->ProcessMouseWheel(Rml::Vector2f(0.f, valueY), 0);
			}
		}
		if (math::Abs(valueX) >= 0.2f)
		{
			for (auto context : mContexts)
			{
				context->ProcessMouseWheel(Rml::Vector2f(valueX, 0.f), 0);
			}
		}

		std::vector<std::vector<ui::ElementBinding>::const_iterator> invalidBindings;
		for (auto it = mBindings.cbegin(); it != mBindings.cend(); ++it)
		{
			const auto& binding = *it;

			if (binding.IsValid())
				binding.ResolveBinding();
			else invalidBindings.emplace_back(it);
		}

		for (const auto& it : invalidBindings)
		{
			mBindings.erase(it);
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

	DocumentInfo* UiManager::LoadWindow(const std::string& path, Rml::Context* context, std::string id, bool show)
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
		if (show) document->Show();

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

	Rml::EventPtr UiManager::CustomEventInstancer::InstanceEvent(
		Rml::Element* target, 
		Rml::EventId id, 
		const Rml::String& type, 
		const Rml::Dictionary& parameters, 
		bool interruptible)
	{
		switch (id)
		{
		case Rml::EventId::Invalid:
			break;
		case Rml::EventId::Mousedown:
			break;
		case Rml::EventId::Mousescroll:
			break;
		case Rml::EventId::Mouseover:
			break;
		case Rml::EventId::Mouseout:
			break;
		case Rml::EventId::Focus:
			BASED_TRACE("FOCUSING {}", target->GetId());
			if (target)
			{
				Rml::Rectanglef rect;
				Rml::ElementUtilities::GetBoundingBox(rect, target, Rml::BoxArea::Border);
				input::Mouse::SetMousePosition(glm::ivec2(rect.Center().x, rect.Center().y), true);
				auto context = target->GetContext();
				if (!context) break;
				context->ProcessMouseMove(static_cast<int>(rect.Center().x), static_cast<int>(rect.Center().y), 0);
			}
			break;
		case Rml::EventId::Blur:
			BASED_TRACE("BLUR {}", target->GetId());
			break;
		case Rml::EventId::Keydown:
			break;
		case Rml::EventId::Keyup:
			break;
		case Rml::EventId::Textinput:
			break;
		case Rml::EventId::Mouseup:
			break;
		case Rml::EventId::Click:
			break;
		case Rml::EventId::Dblclick:
			break;
		case Rml::EventId::Load:
			break;
		case Rml::EventId::Unload:
			break;
		case Rml::EventId::Show:
			break;
		case Rml::EventId::Hide:
			break;
		case Rml::EventId::Mousemove:
			break;
		case Rml::EventId::Dragmove:
			break;
		case Rml::EventId::Drag:
			break;
		case Rml::EventId::Dragstart:
			break;
		case Rml::EventId::Dragover:
			break;
		case Rml::EventId::Dragdrop:
			break;
		case Rml::EventId::Dragout:
			break;
		case Rml::EventId::Dragend:
			break;
		case Rml::EventId::Handledrag:
			break;
		case Rml::EventId::Resize:
			break;
		case Rml::EventId::Scroll:
			break;
		case Rml::EventId::Animationend:
			break;
		case Rml::EventId::Transitionend:
			break;
		case Rml::EventId::Change:
			break;
		case Rml::EventId::Submit:
			break;
		case Rml::EventId::Tabchange:
			break;
		case Rml::EventId::NumDefinedIds:
			break;
		case Rml::EventId::MaxNumIds:
			break;
		}

		return Rml::EventPtr(new Rml::Event(target, id, type, parameters, interruptible));
	}

	void UiManager::CustomEventInstancer::ReleaseEvent(Rml::Event* event)
	{
		delete event;
	}

	void UiManager::CustomEventInstancer::Release()
	{
		
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
