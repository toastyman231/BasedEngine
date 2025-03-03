#include "pch.h"
#include "scene/components.h"

#include "app.h"

namespace based::scene
{
	MeshRenderer::MeshRenderer()
	{
		auto storage = Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage();
		if (auto defaultMat = storage.Get("Lit"))
			material = defaultMat;
		else
		{
			material = graphics::Material::LoadMaterialFromFile(
			ASSET_PATH("Materials/Lit.bmat"),
			Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage());
		}
	}

	CameraComponent::CameraComponent()
	{
		auto cam = std::make_shared<graphics::Camera>();
		Engine::Instance().GetApp().GetCurrentScene()->GetCameraStorage().Load("Camera", cam);
		camera = cam;
	}
}
