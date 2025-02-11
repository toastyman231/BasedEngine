#include "based/pch.h"
#include "editorstatics.h"

#include "based/app.h"
#include "based/graphics/camera.h"
#include "based/scene/entity.h"
#include "Player/editorplayer.h"

namespace editor
{
	void Statics::InitializeEditorStatics()
	{
		mEditorCamera = std::make_shared<based::graphics::Camera>();
		mEditorCamera->SetProjection(based::graphics::Projection::PERSPECTIVE);
		based::Engine::Instance().GetApp().GetCurrentScene()->SetActiveCamera(mEditorCamera);

		mEditorPlayer = based::scene::Entity::CreateEntity("EditorPlayer");
		mEditorPlayer->AddComponent<based::scene::CameraComponent>(mEditorCamera);
		mEditorPlayer->AddComponent<editor::EditorPlayer>();
	}

	bool Statics::LoadScene(const std::string& path)
	{
		auto serializer = based::scene::SceneSerializer(based::Engine::Instance().GetApp().GetCurrentScene());
		return serializer.Deserialize(path);
	}
}
