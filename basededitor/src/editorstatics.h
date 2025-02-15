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
	private:
		inline static std::shared_ptr<based::graphics::Camera> mEditorCamera;

		inline static std::shared_ptr<based::scene::Entity> mEditorPlayer;

		inline static bool mEditorSceneDirty = false;

		inline static std::string mProjectDirectory;
	};
}
