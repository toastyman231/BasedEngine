#pragma once

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
		static bool OpenScene();
		static bool LoadScene(const std::string& path);
		static bool SaveScene();

		static std::shared_ptr<based::graphics::Camera> GetEditorCamera() { return mEditorCamera; }
		static std::vector<std::weak_ptr<based::scene::Entity>> GetSelectedEntities() { return mSelectedEntities; }

		static void SetSelectedEntities(const std::vector<std::weak_ptr<based::scene::Entity>>& entities) { mSelectedEntities = entities; }
		static void SetSceneDirty(bool dirty);

		static bool SelectedEntitiesContains(std::shared_ptr<based::scene::Entity> entity);
	private:
		inline static std::shared_ptr<based::graphics::Camera> mEditorCamera;

		inline static std::shared_ptr<based::scene::Entity> mEditorPlayer;

		inline static bool mEditorSceneDirty = false;

		inline static std::string mProjectDirectory;

		inline static std::vector<std::weak_ptr<based::scene::Entity>> mSelectedEntities;
	};
}
