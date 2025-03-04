#pragma once
#include "engineoperations.h"
#include "external/history/History.h"

namespace based::scene
{
	class Entity;
}

namespace based::graphics
{
	class Camera;
}

namespace editor
{
	class Statics
	{
	public:
		static void InitializeEditorStatics();

		static bool NewScene();
		static bool OpenScene(bool keepLoadedAssets = true);
		static bool OpenScene(const std::string& path, bool keepLoadedAssets = true);
		static bool LoadScene(const std::string& path);
		static bool SaveScene(const std::string& path = "", bool forceSave = false);
		static bool LoadSceneSafe(const std::string& path);

		static std::shared_ptr<based::graphics::Camera> GetEditorCamera() { return mEditorCamera; }
		static std::vector<std::weak_ptr<based::scene::Entity>> GetSelectedEntities() { return mSelectedEntities; }
		static std::vector<std::string> GetSelectedFiles() { return mSelectedFiles; }
		static std::vector<std::string> GetSelectedDirectories() { return mSelectedDirectories; }
		static bool IsSceneDirty() { return mEditorSceneDirty; }

		static void SetSelectedEntities(const std::vector<std::weak_ptr<based::scene::Entity>>& entities) { mSelectedEntities = entities; }
		static bool RemoveEntityFromSelected(const std::shared_ptr<based::scene::Entity>& entity);
		static void SetSceneDirty(bool dirty);

		static bool SelectedEntitiesContains(std::shared_ptr<based::scene::Entity> entity);
		static bool SelectedEntitiesContains(std::vector<std::weak_ptr<based::scene::Entity>> entities);

		static bool SelectedFilesContains(const std::string& file);
		static bool SelectedDirectoriesContains(const std::string& dir);

		static void SetSelectedFiles(const std::vector<std::string>& files) { mSelectedFiles = files; }
		static void AddSelectedFile(const std::string& file) { mSelectedFiles.emplace_back(file); }
		static void SetSelectedDirectories(const std::vector<std::string>& dirs) { mSelectedDirectories = dirs; }
		static void AddSelectedDirectory(const std::string& dir) { mSelectedDirectories.emplace_back(dir); }

		static void RemoveSelectedFile(const std::string& file);
		static void RemoveSelectedDirectory(const std::string& dir);

		static HistoryContext& GetHistory() { return mHistoryContext; }

		static std::string GetCurrentSceneSaveLocation() { return mSaveLocation; }

		static std::string GetProjectDirectory() { return mProjectDirectory; }

		inline static EngineOperations EngineOperations;
	private:
		inline static std::shared_ptr<based::graphics::Camera> mEditorCamera;

		inline static std::shared_ptr<based::scene::Entity> mEditorPlayer;

		inline static bool mEditorSceneDirty = false;

		inline static std::string mProjectDirectory = "../Sandbox/";
		inline static std::string mSaveLocation;

		inline static std::vector<std::weak_ptr<based::scene::Entity>> mSelectedEntities;
		inline static std::vector<std::string> mSelectedDirectories;
		inline static std::vector<std::string> mSelectedFiles;

		inline static HistoryContext mHistoryContext;
	};
}
