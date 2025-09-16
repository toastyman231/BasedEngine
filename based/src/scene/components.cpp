#include "pch.h"
#include "scene/components.h"

#include "app.h"
#include "graphics/mesh.h"

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

	MeshShapeComponent::MeshShapeComponent(std::shared_ptr<graphics::Mesh> mesh)
	{
		JPH::MeshShapeSettings settings;
		JPH::VertexList v;
		JPH::IndexedTriangleList t;
		v.reserve(mesh->GetVertexArray()->GetVertexBuffer(0)->GetVertexCount());
		t.reserve(mesh->indices.size());

		graphics::VertexBuffer<float>* vb = (graphics::VertexBuffer<float>*)mesh->GetVertexArray()->GetVertexBuffer(0);
		auto vertices = vb->GetData();

		for (size_t i = 0; i < vb->GetVertexCount(); ++i)
		{
			v.push_back(JPH::Float3(vertices[i*3 + 0], vertices[i*3 + 1], vertices[i*3 + 2]));
		}

		for (auto i = 0; i < mesh->indices.size() - 3; i += 3)
		{
			t.push_back(JPH::IndexedTriangle(mesh->indices[i], mesh->indices[i + 1], mesh->indices[i + 2]));
		}

		settings.mTriangleVertices = v;
		settings.mIndexedTriangles = t;
		settings.Sanitize();

		JPH::ShapeSettings::ShapeResult result = settings.Create();

		if (result.HasError())
		{
			BASED_ERROR("Error loading mesh shape: {}!", result.GetError().c_str());
		}
		else shape = result.Get();
	}
}
