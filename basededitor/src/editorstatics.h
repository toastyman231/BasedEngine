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

		static bool LoadScene(const std::string& path);

		static std::shared_ptr<based::graphics::Camera> GetEditorCamera() { return mEditorCamera; }
	private:
		inline static std::shared_ptr<based::graphics::Camera> mEditorCamera;

		inline static std::shared_ptr<based::scene::Entity> mEditorPlayer;
	};
}
