#include "based/pch.h"
#include "editorstatics.h"

#include "based/app.h"
#include "based/graphics/camera.h"
#include "based/scene/entity.h"
#include "external/tfd/tinyfiledialogs.h"
#include "Player/editorplayer.h"

namespace editor
{
	void Statics::InitializeEditorStatics()
	{
		mProjectDirectory = based::Engine::Instance().GetArg(1);

		mEditorCamera = std::make_shared<based::graphics::Camera>();
		mEditorCamera->SetProjection(based::graphics::Projection::PERSPECTIVE);
		based::Engine::Instance().GetApp().GetCurrentScene()->SetActiveCamera(mEditorCamera);

		mEditorPlayer = based::scene::Entity::CreateEntity("EditorPlayer");
		mEditorPlayer->AddComponent<based::scene::CameraComponent>(mEditorCamera);
		mEditorPlayer->AddComponent<editor::EditorPlayer>();
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
		return true;
	}

	bool Statics::SaveScene()
	{
		if (!mEditorSceneDirty) return false;

		static char const* patterns = { "*.bscn" };
		const char* saveLocation = tinyfd_saveFileDialog(
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
		return true;
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
			window.GetWindowTitle().pop_back();
		}
	}

	bool Statics::SelectedEntitiesContains(std::shared_ptr<based::scene::Entity> entity)
	{
		for (auto& e : mSelectedEntities)
		{
			if (auto ent = e.lock())
			{
				if (ent->GetEntityHandle() == entity->GetEntityHandle()) return true;
			}
		}

		return false;
	}
}
