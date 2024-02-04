#include "graphics/mesh.h"

#include "app.h"
#include "based/core/profiler.h"
#include "graphics/shader.h"
#include "graphics/defaultassetlibraries.h"

namespace based::graphics
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		SetupMesh();
	}

	Mesh::Mesh(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat)
	{
		mVA = va;
		material = mat;
	}

	void Mesh::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material)
	{
		PROFILE_FUNCTION();
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, 
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		auto model = glm::mat4(1.f);
		model = glm::translate(model, position);
		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, rotation.x * 0.0174533f, glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, rotation.y * 0.0174533f, glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, rotation.z * 0.0174533f, glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, scale);
		Shader::UpdateShaderPointLighting(material->GetShader(), position);
		Shader::UpdateShaderDirectionalLighting(material->GetShader());
		if (Engine::Instance().GetWindow().isInDepthPass)
		{
			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					graphics::DefaultLibraries::GetMaterialLibrary().Get("ShadowDepthMaterial"), model));
		} else
		{
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, model));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void Mesh::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		if (material)
		{
			Draw(position, rotation, scale, material);
		} else
		{
			BASED_ERROR("ERROR: Trying to render mesh with no material set!");
			// TODO: Replace this with default material
		}
	}

	void Mesh::SetupMesh(bool upload)
	{
		PROFILE_FUNCTION();
		auto va = std::make_shared<graphics::VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
		BASED_CREATE_VERTEX_BUFFER(norm_vb, float);
		BASED_CREATE_VERTEX_BUFFER(uv_vb, float);
		BASED_CREATE_VERTEX_BUFFER(tan_vb, float);
		BASED_CREATE_VERTEX_BUFFER(bitan_vb, float);
		BASED_CREATE_VERTEX_BUFFER(boneid_vb, int);
		BASED_CREATE_VERTEX_BUFFER(weight_vb, float);

		for (const auto vertex : vertices)
		{
			pos_vb->PushVertex({ vertex.Position.x, vertex.Position.y, vertex.Position.z });
			norm_vb->PushVertex({ vertex.Normal.x, vertex.Normal.y, vertex.Normal.z });
			uv_vb->PushVertex({ vertex.TexCoords.x, vertex.TexCoords.y });
			tan_vb->PushVertex({ vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z });
			bitan_vb->PushVertex({ vertex.Bitangent.x, vertex.Bitangent.y, vertex.Bitangent.z });
			boneid_vb->PushVertex({ vertex.m_BoneIDs[0], vertex.m_BoneIDs[1], vertex.m_BoneIDs[2], vertex.m_BoneIDs[3] });
			weight_vb->PushVertex({ vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2], vertex.m_Weights[3] });
		}

		pos_vb->SetLayout({ 3 });
		norm_vb->SetLayout({ 3 });
		uv_vb->SetLayout({ 2 });
		tan_vb->SetLayout({ 3 });
		bitan_vb->SetLayout({ 3 });
		boneid_vb->SetLayout({ 4 });
		weight_vb->SetLayout({ 4 });
		va->PushBuffer(std::move(pos_vb));
		va->PushBuffer(std::move(uv_vb));
		va->PushBuffer(std::move(norm_vb));
		va->PushBuffer(std::move(tan_vb));
		va->PushBuffer(std::move(bitan_vb));
		va->PushBuffer(std::move(boneid_vb));
		va->PushBuffer(std::move(weight_vb));

		va->SetElements(indices);
		if (upload) va->Upload();

		mVA = va;
	}

	void InstancedMesh::SetInstanceTransform(int index, const scene::Transform& transform)
	{
		if (index < 0 || index > mInstanceCount) return;
		mInstanceTransforms[index] = transform;
	}

	int InstancedMesh::AddInstance(scene::Transform transform)
	{
		mInstanceTransforms.emplace_back(transform);
		mInstanceCount++;
		mIsDirty = true;
		return static_cast<int>(mInstanceTransforms.size());
	}

	void InstancedMesh::AddInstances(const std::vector<scene::Transform>& transforms)
	{
		for (const auto transform : transforms)
		{
			AddInstance(transform);
		}
	}

	bool InstancedMesh::RemoveInstance(int index)
	{
		const auto originalSize = mInstanceTransforms.size();
		mInstanceTransforms.erase(mInstanceTransforms.begin() + index);
		mIsDirty = true;
		return mInstanceTransforms.size() < originalSize;
	}

	bool InstancedMesh::RemoveInstances(std::vector<int> indices)
	{
		const auto originalSize = mInstanceTransforms.size();
		for (const auto index : indices)
		{
			RemoveInstance(index);
		}
		return mInstanceTransforms.size() < originalSize;
	}

	void InstancedMesh::ClearInstances()
	{
		mInstanceTransforms.clear();
		mInstanceCount = 0;
	}

	void InstancedMesh::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
	                         std::shared_ptr<Material> material)
	{
		PROFILE_FUNCTION();
		if (mIsDirty) RegenVertexArray();

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		auto model = glm::mat4(1.f);
		model = glm::translate(model, position);
		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, rotation.x * 0.0174533f, glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, rotation.y * 0.0174533f, glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, rotation.z * 0.0174533f, glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, scale);
		Shader::UpdateShaderPointLighting(material->GetShader(), position);
		Shader::UpdateShaderDirectionalLighting(material->GetShader());
		if (Engine::Instance().GetWindow().isInDepthPass)
		{
			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					graphics::DefaultLibraries::GetMaterialLibrary().Get("ShadowDepthMaterial"), model, true, mInstanceCount));
		}
		else
		{
			Engine::Instance().GetRenderManager().Submit(
				BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, model, true, mInstanceCount));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void InstancedMesh::RegenVertexArray()
	{
		SetupMesh(false);

		BASED_CREATE_INSTANCED_VERTEX_BUFFER(model_vb, float);

		for (int i = 0; i < mInstanceCount; i++)
		{
			auto transform = mInstanceTransforms[i];

			auto model = glm::mat4(1.f);
			model = glm::translate(model, transform.Position);
			// Rotations are passed as degrees and converted to radians here automatically
			model = glm::rotate(model, transform.Rotation.x * 0.0174533f, glm::vec3(1.f, 0.f, 0.f));
			model = glm::rotate(model, transform.Rotation.y * 0.0174533f, glm::vec3(0.f, 1.f, 0.f));
			model = glm::rotate(model, transform.Rotation.z * 0.0174533f, glm::vec3(0.f, 0.f, 1.f));
			model = glm::scale(model, transform.Scale);

			for (int j = 0; j < 4; j++)
			{
				model_vb->PushVertex({model[j][0], model[j][1], model[j][2], model[j][3]});
			}
		}

		model_vb->SetLayout({ 4, 4, 4, 4 });
		mVA->PushBuffer(std::move(model_vb));
		mVA->Upload();

		mIsDirty = false;
	}
}
