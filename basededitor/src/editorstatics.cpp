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

		mEditorCamera = std::make_shared<based::graphics::Camera>();
		mEditorCamera->SetProjection(based::graphics::Projection::PERSPECTIVE);
		based::Engine::Instance().GetApp().GetCurrentScene()->SetActiveCamera(mEditorCamera);

		mEditorPlayer = based::scene::Entity::CreateEntity("EditorPlayer");
		mEditorPlayer->AddComponent<based::scene::CameraComponent>(mEditorCamera);
		mEditorPlayer->AddComponent<EditorPlayer>();

		History::SetContext(&mHistoryContext);

		using namespace entt::literals;
		entt::meta<based::scene::MeshRenderer>()
			.type(entt::type_hash<based::scene::MeshRenderer>())
			.func<&EngineOperations::AddComponent<based::scene::MeshRenderer>>("AddComponent"_hs)
			.func<&EngineOperations::RemoveComponent<based::scene::MeshRenderer>>("RemoveComponent"_hs);
		entt::meta<based::scene::PointLight>()
			.type(entt::type_hash<based::scene::PointLight>())
			.func<&EngineOperations::AddComponent<based::scene::PointLight>>("AddComponent"_hs)
			.func<&EngineOperations::RemoveComponent<based::scene::PointLight>>("RemoveComponent"_hs);
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

		BASED_TRACE("Creating new scene!");
		auto newScene = std::make_shared<based::scene::Scene>();
		based::Engine::Instance().GetApp().LoadScene(newScene);
		
		auto res = LoadScene(ASSET_PATH("Scenes/Default3D.bscn"));

		if (!res)
		{
			BASED_WARN("Error deserializing default scene, aborting!");
			return false;
		}

		SetSceneDirty(true);
		return true;
	}

	bool Statics::OpenScene()
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

		BASED_TRACE("Creating new scene!");
		auto newScene = std::make_shared<based::scene::Scene>();
		based::Engine::Instance().GetApp().LoadScene(newScene);

		auto res = LoadScene(fileLocation);

		if (!res)
		{
			BASED_WARN("Error deserializing default scene, aborting!");
			return false;
		}

		SetSceneDirty(false);
		return true;
	}

	bool Statics::OpenScene(const std::string& path, bool keepLoadedAssets)
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

		BASED_TRACE("Creating new scene!");
		auto newScene = std::make_shared<based::scene::Scene>();
		auto currentScene = based::Engine::Instance().GetApp().GetCurrentScene();
		if (keepLoadedAssets)
		{
			newScene->GetMeshStorage() = currentScene->GetMeshStorage();
			newScene->GetModelStorage() = currentScene->GetModelStorage();
			newScene->GetMaterialStorage() = currentScene->GetMaterialStorage();
			newScene->GetTextureStorage() = currentScene->GetTextureStorage();
			newScene->GetAnimationStorage() = currentScene->GetAnimationStorage();
			newScene->GetAnimatorStorage() = currentScene->GetAnimatorStorage();
		}
		based::Engine::Instance().GetApp().LoadScene(newScene);

		auto res = LoadScene(path);

		if (!res)
		{
			BASED_WARN("Error deserializing default scene, aborting!");
			return false;
		}

		SetSceneDirty(false);
		return true;
	}

	bool Statics::LoadScene(const std::string& path)
	{
		auto scene = based::Engine::Instance().GetApp().GetCurrentScene();
		auto serializer = based::scene::SceneSerializer(scene);
		serializer.SetProjectDirectory(mProjectDirectory);
		auto res = serializer.Deserialize(path);

		if (!res)
		{
			BASED_ERROR("Error loading scene {}!", path);
			return false;
		}

		based::Engine::Instance().GetWindow().SetWindowTitle("Based Editor - " + scene->GetSceneName());

		SetSceneDirty(false);
		mSaveLocation = path;
		return true;
	}

	bool Statics::SaveScene(const std::string& path)
	{
		if (!mEditorSceneDirty) return false;

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

	bool Statics::LoadSceneSafe(const std::string& path)
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

		return LoadScene(path);
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

	bool Statics::SelectedFilesContains(const std::string& file)
	{
		return std::find(mSelectedFiles.begin(), mSelectedFiles.end(), file) != mSelectedFiles.end();
	}

	bool Statics::SelectedDirectoriesContains(const std::string& dir)
	{
		return std::find(mSelectedDirectories.begin(), mSelectedDirectories.end(), dir) != mSelectedDirectories.end();
	}

	void Statics::RemoveSelectedFile(const std::string& file)
	{
		mSelectedFiles.erase(
			std::find(mSelectedFiles.begin(), mSelectedFiles.end(), file)
		);
	}

	void Statics::RemoveSelectedDirectory(const std::string& dir)
	{
		mSelectedDirectories.erase(
			std::find(mSelectedDirectories.begin(), mSelectedDirectories.end(), dir)
		);
	}
}
