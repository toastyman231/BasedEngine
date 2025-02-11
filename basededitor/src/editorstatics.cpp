#include "based/pch.h"
#include "editorstatics.h"

#include "based/app.h"
#include "based/graphics/camera.h"

namespace editor
{
	void Statics::InitializeEditorStatics()
	{
		mEditorCamera = std::make_shared<based::graphics::Camera>();
		mEditorCamera->SetProjection(based::graphics::Projection::PERSPECTIVE);
		based::Engine::Instance().GetApp().GetCurrentScene()->SetActiveCamera(mEditorCamera);
	}

	bool Statics::LoadScene(const std::string& path)
	{
		auto serializer = based::scene::SceneSerializer(based::Engine::Instance().GetApp().GetCurrentScene());
		return serializer.Deserialize(path);
	}
}
