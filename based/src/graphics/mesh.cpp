#include "graphics/mesh.h"

#include "app.h"

namespace based::graphics
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		SetupMesh();
	}

	Mesh::Mesh(std::shared_ptr<VertexArray> va)
	{
		mVA = va;
	}

	void Mesh::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material)
	{
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, 
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		auto model = glm::mat4(1.f);
		model = glm::translate(model, position);
		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, rotation.x * 0.0174533f, glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, rotation.y * 0.0174533f, glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, rotation.z * 0.0174533f, glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, scale);
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, model));
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void Mesh::SetupMesh()
	{
		auto va = std::make_shared<graphics::VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
		BASED_CREATE_VERTEX_BUFFER(norm_vb, float);
		BASED_CREATE_VERTEX_BUFFER(uv_vb, float);

		for (auto vertex : vertices)
		{
			pos_vb->PushVertex({ vertex.Position.x, vertex.Position.y, vertex.Position.z });
			norm_vb->PushVertex({ vertex.Normal.x, vertex.Normal.y, vertex.Normal.z });
			uv_vb->PushVertex({ vertex.TexCoords.x, vertex.TexCoords.y });
		}

		pos_vb->SetLayout({ 3 });
		norm_vb->SetLayout({ 3 });
		uv_vb->SetLayout({ 2 });
		va->PushBuffer(std::move(pos_vb));
		va->PushBuffer(std::move(norm_vb));
		va->PushBuffer(std::move(uv_vb));

		va->SetElements(indices);
		va->Upload();

		mVA = va;
	}
}
