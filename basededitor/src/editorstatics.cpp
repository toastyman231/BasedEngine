#include "based/pch.h"
#include "editorstatics.h"

#include "based/app.h"
#include "based/graphics/camera.h"
#include "based/scene/entity.h"
#include "external/tfd/tinyfiledialogs.h"
#include "external/history/History.h"
#include "Player/editorplayer.h"
#include "external/entt/core/hashed_string.hpp"

namespace editor
{
	void Statics::InitializeEditorStatics()
	{
		if (auto dir = based::Engine::Instance().GetArg(1); dir != "")
			mProjectDirectory = dir;
		else
			mProjectDirectory = std::filesystem::canonical("../Sandbox").string();

		mEditorCamera = std::make_shared<based::graphics::Camera>();
		mEditorCamera->SetProjection(based::graphics::Projection::PERSPECTIVE);
		based::Engine::Instance().GetApp().GetCurrentScene()->SetActiveCamera(mEditorCamera);

		mEditorPlayer = based::scene::Entity::CreateEntity("EditorPlayer");
		mEditorPlayer->AddComponent<based::scene::CameraComponent>(mEditorCamera);
		mEditorPlayer->AddComponent<EditorPlayer>();
		mEditorPlayer->AddComponent<based::scene::DontDestroyOnLoad>();

		History::SetContext(&mHistoryContext);
	}

	void Statics::InitializeImGui()
	{
		ImGuiStyle &style = ImGui::GetStyle();
		ImVec4 *colors = style.Colors;

		// Primary background
		colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318

		colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);

		// Headers
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
		//colors[ImGuiCol_TabSelected] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
		//colors[ImGuiCol_TabSelectedOverline] = COLOR_HIGHLIGHT;
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

		// Borders
		colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

		// Highlights
		colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

		// Style tweaks
		constexpr float rounding = 3.0f;
		//style.TabBarOverlineSize = 2.0f;
		style.WindowRounding = rounding;
		style.FrameRounding = rounding;
		style.GrabRounding = rounding;
		style.TabRounding = 0;
		style.PopupRounding = rounding;
		style.ScrollbarRounding = rounding;
		style.WindowPadding = ImVec2(5, 5);
		style.FramePadding = ImVec2(6, 4);
		style.ItemSpacing = ImVec2(8, 6);
		style.PopupBorderSize = 0.f;
	}

	bool Statics::NewScene()
	{
		if (mEditorSceneDirty)
		{
			auto shouldSave = tinyfd_messageBox(
				"Save Current Scene?",
				"You have unsaved changes, would you like to save the current scene?",
				"yesno",
				"question",
				1
			);

			if (shouldSave == 1)
			{
				auto saveResult = SaveScene();
				if (!saveResult)
				{
					BASED_WARN("Scene did not save properly, aborting create new scene!");
					return false;
				}
			}
		}

		auto res = LoadScene(ASSET_PATH("Scenes/Default3D.bscn"), true, true);

		if (!res)
		{
			BASED_WARN("Error deserializing default scene, aborting!");
			return false;
		}

		SetSceneDirty(true);
		return true;
	}

	bool Statics::OpenScene(const std::string& path, bool keepLoadedAssets)
	{
		auto openPath = path;
		if (openPath.empty())
		{
			static char const* patterns = { "*.bscn" };
			const char* fileLocation = tinyfd_openFileDialog(
				"Select Scene",
				nullptr,
				1,
				&patterns,
				NULL,
				0
			);

			if (!fileLocation)
			{
				BASED_WARN("Invalid scene chosen, aborting!");
				return false;
			}

			openPath = fileLocation;
		}

		return LoadScene(openPath, keepLoadedAssets);
	}

	bool Statics::LoadScene(const std::string& path, bool keepLoadedAssets, bool absolutePath)
	{
		if (mEditorSceneDirty)
		{
			auto shouldSave = tinyfd_messageBox(
				"Save Current Scene?",
				"You have unsaved changes, would you like to save the current scene?",
				"yesno",
				"question",
				1
			);

			if (shouldSave == 1)
			{
				auto saveResult = SaveScene();
				if (!saveResult)
				{
					BASED_WARN("Scene did not save properly, aborting create new scene!");
					return false;
				}
			}
		}

		std::string prefix = "";
		std::string filepath = path;
		if (!absolutePath)
		{
			prefix = mProjectDirectory;
			if (!EndsWith(mProjectDirectory, "/") && !EndsWith(mProjectDirectory, "\\"))
				prefix.append("/");
			BASED_TRACE("Deserializing {}", prefix + filepath);
			filepath = path.substr(path.find("Assets"));
		}

		auto res = based::scene::Scene::LoadScene(filepath, prefix, keepLoadedAssets);

		if (!res)
		{
			BASED_ERROR("Error loading scene {}!", path);
			return false;
		}

		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		based::Engine::Instance().GetWindow().SetWindowTitle("Based Editor - " + scene->GetSceneName());

		SetSceneDirty(false);
		mSaveLocation = path;
		return true;
	}

	bool Statics::SaveScene(const std::string& path, bool forceSave)
	{
		static char const* patterns = { "*.bscn" };
		const char* saveLocation = (!path.empty() && path != "FORCE") ? path.c_str() :
			tinyfd_saveFileDialog(
			"Select Save Location",
			"NewScene",
			1,
			&patterns,
			"*.bscn"
		);

		if (!saveLocation)
		{
			BASED_WARN("Invalid save location chosen, aborting!");
			return false;
		}

		auto sceneName = std::string(saveLocation);
		auto startIndex = sceneName.find_last_of("\\") + 1;
		auto endIndex = sceneName.find(".bscn");
		sceneName = sceneName.substr(startIndex, endIndex - startIndex);

		based::Engine::Instance().GetApp().GetCurrentScene()->SetSceneName(sceneName);
		based::Engine::Instance().GetWindow().SetWindowTitle("Based Editor - " + sceneName);

		auto serializer = based::scene::SceneSerializer(based::Engine::Instance().GetApp().GetCurrentScene());
		serializer.Serialize(saveLocation);
		SetSceneDirty(false);
		mSaveLocation = std::string(saveLocation);
		return true;
	}

	bool Statics::RemoveEntityFromSelected(const std::shared_ptr<based::scene::Entity>& entity)
	{
		auto it = 
			std::find_if(mSelectedEntities.begin(), mSelectedEntities.end(), 
				[entity](const std::weak_ptr<based::scene::Entity>& e)
				{
					return e.lock() == entity;
				});

		if (it != mSelectedEntities.end())
		{
			mSelectedEntities.erase(it);
			return true;
		}

		return false;
	}

	void Statics::SetSceneDirty(bool dirty)
	{
		if (mEditorSceneDirty == dirty) return;

		mEditorSceneDirty = dirty;
		auto& window = based::Engine::Instance().GetWindow();
		if (dirty)
		{
			window.SetWindowTitle(window.GetWindowTitle() + "*");
		} else
		{
			auto starIndex = window.GetWindowTitle().find_last_of("*");
			window.SetWindowTitle(window.GetWindowTitle().substr(0, starIndex));
		}
	}

	bool Statics::SelectedEntitiesContains(std::shared_ptr<based::scene::Entity> entity)
	{
		return std::any_of(mSelectedEntities.begin(), mSelectedEntities.end(), 
			[entity](const std::weak_ptr<based::scene::Entity>& e)
			{
				return entity == e.lock();
			});
	}

	bool Statics::SelectedEntitiesContains(std::vector<std::weak_ptr<based::scene::Entity>> entities)
	{
		for (const auto& e : entities)
		{
			if (!SelectedEntitiesContains(e.lock())) return false;
		}

		return true;
	}

	bool Statics::SelectedFilesContains(const std::filesystem::path& file)
	{
		return std::find(mSelectedFiles.begin(), mSelectedFiles.end(), file) != mSelectedFiles.end();
	}

	bool Statics::SelectedDirectoriesContains(const std::filesystem::path& dir)
	{
		return std::find(mSelectedDirectories.begin(), mSelectedDirectories.end(), dir) != mSelectedDirectories.end();
	}

	void Statics::RemoveSelectedFile(const std::filesystem::path& file)
	{
		mSelectedFiles.erase(
			std::find(mSelectedFiles.begin(), mSelectedFiles.end(), file)
		);
	}

	void Statics::RemoveSelectedDirectory(const std::filesystem::path& dir)
	{
		mSelectedDirectories.erase(
			std::find(mSelectedDirectories.begin(), mSelectedDirectories.end(), dir)
		);
	}

	bool Statics::EndsWith(const std::string& value, const std::string& ending)
	{
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}
}
