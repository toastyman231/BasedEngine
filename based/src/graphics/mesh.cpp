#include "pch.h"
#include "graphics/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "app.h"
#include "graphics/shader.h"
#include "graphics/defaultassetlibraries.h"
#include "graphics/model.h"

namespace based::graphics
{
	std::shared_ptr<Mesh> Mesh::CreateMesh(
		const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, 
		core::AssetLibrary<Mesh>& assetLibrary, 
		const std::string& name)
	{
		auto asset = std::make_shared<Mesh>(vertices, indices);
		assetLibrary.Load(name, asset);
		return asset;
	}

	std::shared_ptr<Mesh> Mesh::CreateMesh(
		const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, 
		core::AssetLibrary<Mesh>& assetLibrary, 
		core::UUID uuid, const std::string& name)
	{
		auto asset = std::make_shared<Mesh>(vertices, indices);
		asset->mUUID = uuid;
		assetLibrary.Load(name, asset);
		return asset;
	}

	std::shared_ptr<InstancedMesh> Mesh::CreateInstancedMesh(
		const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, 
		core::AssetLibrary<Mesh>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<InstancedMesh>(vertices, indices);
		assetLibrary.Load(name, asset);
		asset->mMeshName = name;
		return asset;
	}

	std::shared_ptr<InstancedMesh> Mesh::CreateInstancedMesh(const std::shared_ptr<Mesh>& mesh,
		core::AssetLibrary<Mesh>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<InstancedMesh>(mesh->mVA, 1);
		asset->mMeshSource = mesh->mMeshSource;
		assetLibrary.Load(name, asset);
		asset->mMeshName = name;
		return asset;
	}

	std::shared_ptr<Mesh> Mesh::CreateMesh( const std::shared_ptr<VertexArray>& va,
											core::AssetLibrary<Mesh>& assetLibrary, 
											const std::string& name)
	{
		auto asset = std::make_shared<Mesh>(va);
		assetLibrary.Load(name, asset);
		asset->mMeshName = name;
		return asset;
	}

	std::shared_ptr<Mesh> Mesh::LoadMeshFromFile(const std::string& filepath, core::AssetLibrary<Mesh>& assetLibrary)
	{
		return LoadMeshWithUUID(filepath, assetLibrary, core::UUID());
	}

	std::shared_ptr<Mesh> Mesh::LoadMeshWithUUID(const std::string& filepath, core::AssetLibrary<Mesh>& assetLibrary,
		core::UUID uuid)
	{
		PROFILE_FUNCTION();
		Assimp::Importer import;
		const aiScene * scene = import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			BASED_ERROR("Assimp Error: {}", import.GetErrorString());
			return nullptr;
		}

		if (aiMesh* mesh = scene->mMeshes[0])
		{
			std::vector<unsigned int> indices;

			auto va = std::make_shared<VertexArray>();

			BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
			BASED_CREATE_VERTEX_BUFFER(norm_vb, float);
			BASED_CREATE_VERTEX_BUFFER(uv_vb, float);
			BASED_CREATE_VERTEX_BUFFER(tan_vb, float);
			BASED_CREATE_VERTEX_BUFFER(bitan_vb, float);

			const bool hasNormals   = mesh->HasNormals();
			const bool hasUVs       = mesh->mTextureCoords[0] != nullptr;
			const bool hasTangents  = mesh->mTangents != nullptr;
			const bool hasBitangents= mesh->mBitangents != nullptr;

			pos_vb->Resize(mesh->mNumVertices * 3);
			if (hasNormals) norm_vb->Resize(mesh->mNumVertices * 3);
			if (hasUVs) uv_vb->Resize(mesh->mNumVertices * 2);
			if (hasTangents) tan_vb->Resize(mesh->mNumVertices * 3);
			if (hasBitangents) bitan_vb->Resize(mesh->mNumVertices * 3);

			PROFILE_ZONE(fillVerticesZone, "Fill vertices", true);
			for (size_t i = 0; i < mesh->mNumVertices; ++i) {
				auto pos = mesh->mVertices[i];
				pos_vb->GetData()[i*3 + 0] = pos.x;
				pos_vb->GetData()[i*3 + 1] = pos.y;
				pos_vb->GetData()[i*3 + 2] = pos.z;

				if (hasNormals)
				{
					auto normal = mesh->mNormals[i];
					norm_vb->GetData()[i*3 + 0] = normal.x;
					norm_vb->GetData()[i*3 + 1] = normal.y;
					norm_vb->GetData()[i*3 + 2] = normal.z;
				}

				if (hasUVs)
				{
					auto texcoord = mesh->mTextureCoords[0][i];
					uv_vb->GetData()[i*2 + 0] = texcoord.x;
					uv_vb->GetData()[i*2 + 1] = texcoord.y;
				}
			
				if (hasTangents)
				{
					auto tangent = mesh->mTangents[i];
					tan_vb->GetData()[i*3 + 0] = tangent.x;
					tan_vb->GetData()[i*3 + 1] = tangent.y;
					tan_vb->GetData()[i*3 + 2] = tangent.z;
				}

				if (hasBitangents)
				{
					auto bitangent = mesh->mBitangents[i];
					bitan_vb->GetData()[i*3 + 0] = bitangent.x;
					bitan_vb->GetData()[i*3 + 1] = bitangent.y;
					bitan_vb->GetData()[i*3 + 2] = bitangent.z;
				}
			}
			PROFILE_ZONE_END(fillVerticesZone);

			pos_vb->SetLayout({ 3 });
			norm_vb->SetLayout({ 3 });
			uv_vb->SetLayout({ 2 });
			tan_vb->SetLayout({ 3 });
			bitan_vb->SetLayout({ 3 });

			// process indices
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.emplace_back(face.mIndices[j]);
			}

			va->PushBuffer(std::move(pos_vb));
			if (uv_vb->GetVertexCount() > 0) va->PushBuffer(std::move(uv_vb));
			if (norm_vb->GetVertexCount() > 0) va->PushBuffer(std::move(norm_vb));
			if (tan_vb->GetVertexCount() > 0) va->PushBuffer(std::move(tan_vb));
			if (bitan_vb->GetVertexCount() > 0) va->PushBuffer(std::move(bitan_vb));

			va->SetElements(indices);
			va->Upload();
			
			auto loadedMesh = std::make_shared<Mesh>(va, uuid);
			loadedMesh->mMeshSource = filepath;
			assetLibrary.Load(scene->mMeshes[0]->mName.C_Str(), loadedMesh);
			loadedMesh->mMeshName = scene->mMeshes[0]->mName.C_Str();
			return loadedMesh;
		}

		BASED_ERROR("No meshes found in loaded file");
		return nullptr;
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	{
		BASED_ASSERT(vertices.size() > 0, "Trying to create mesh with no vertices!");
		
		auto va = std::make_shared<VertexArray>();

		BASED_CREATE_VERTEX_BUFFER(pos_vb, float);
		BASED_CREATE_VERTEX_BUFFER(norm_vb, float);
		BASED_CREATE_VERTEX_BUFFER(uv_vb, float);
		BASED_CREATE_VERTEX_BUFFER(tan_vb, float);
		BASED_CREATE_VERTEX_BUFFER(bitan_vb, float);

		pos_vb->Resize(vertices.size() * 3);
		norm_vb->Resize(vertices.size() * 3);
		uv_vb->Resize(vertices.size() * 2);
		tan_vb->Resize(vertices.size() * 3);
		bitan_vb->Resize(vertices.size() * 3);

		PROFILE_ZONE(fillVerticesZone, "Fill vertices", true);
		for (size_t i = 0; i < vertices.size(); ++i) {
			auto& vertex = vertices[i];
			auto pos = vertex.Position;
			pos_vb->GetData()[i*3 + 0] = pos.x;
			pos_vb->GetData()[i*3 + 1] = pos.y;
			pos_vb->GetData()[i*3 + 2] = pos.z;

			auto normal = vertex.Normal;
			norm_vb->GetData()[i*3 + 0] = normal.x;
			norm_vb->GetData()[i*3 + 1] = normal.y;
			norm_vb->GetData()[i*3 + 2] = normal.z;

			auto texcoord = vertex.TexCoords;
			uv_vb->GetData()[i*2 + 0] = texcoord.x;
			uv_vb->GetData()[i*2 + 1] = texcoord.y;
			
			auto tangent = vertex.Tangent;
			tan_vb->GetData()[i*3 + 0] = tangent.x;
			tan_vb->GetData()[i*3 + 1] = tangent.y;
			tan_vb->GetData()[i*3 + 2] = tangent.z;

			auto bitangent = vertex.Bitangent;
			bitan_vb->GetData()[i*3 + 0] = bitangent.x;
			bitan_vb->GetData()[i*3 + 1] = bitangent.y;
			bitan_vb->GetData()[i*3 + 2] = bitangent.z;
		}
		PROFILE_ZONE_END(fillVerticesZone);

		pos_vb->SetLayout({ 3 });
		norm_vb->SetLayout({ 3 });
		uv_vb->SetLayout({ 2 });
		tan_vb->SetLayout({ 3 });
		bitan_vb->SetLayout({ 3 });

		va->PushBuffer(std::move(pos_vb));
		if (uv_vb->GetVertexCount() > 0) va->PushBuffer(std::move(uv_vb));
		if (norm_vb->GetVertexCount() > 0) va->PushBuffer(std::move(norm_vb));
		if (tan_vb->GetVertexCount() > 0) va->PushBuffer(std::move(tan_vb));
		if (bitan_vb->GetVertexCount() > 0) va->PushBuffer(std::move(bitan_vb));

		va->SetElements(indices);
		va->Upload();
		
		this->indices = indices;
		mUUID = core::UUID();
		mVA = va;
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, core::UUID uuid)
			: Mesh(vertices, indices)
	{
		mUUID = uuid;
	}

	Mesh::Mesh(const std::shared_ptr<VertexArray>& va)
	{
		mVA = va;
		mUUID = core::UUID();
	}

	Mesh::Mesh(const std::shared_ptr<VertexArray>& va, core::UUID uuid)
		: Mesh(va)
	{
		mUUID = uuid;
	}

	void Mesh::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material)
	{
		PROFILE_FUNCTION();
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, 
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		auto model = glm::mat4(1.f);
		model = glm::translate(model, position);
		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, scale);
		Shader::UpdateShaderPointLighting(material->GetShader(), position);
		Shader::UpdateShaderDirectionalLighting(material->GetShader());
		if (auto overrideMat = Engine::Instance().GetRenderManager().GetCurrentPassOverrideMaterial())
		{
			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					overrideMat, model));
		} else
		{
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, model));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void Mesh::Draw(scene::Transform& transform, std::shared_ptr<Material> material)
	{
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));

		Shader::UpdateShaderPointLighting(material->GetShader(), transform.Position());
		Shader::UpdateShaderDirectionalLighting(material->GetShader());

		if (auto overrideMat = Engine::Instance().GetRenderManager().GetCurrentPassOverrideMaterial())
		{
			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					overrideMat, transform.GetGlobalMatrix()));
		}
		else
		{
			Engine::Instance().GetRenderManager().Submit(
				BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, transform.GetGlobalMatrix()));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void InstancedMesh::SetInstanceTransform(int index, const scene::Transform& transform)
	{
		if (index < 0 || index > mInstanceCount) return;
		mInstanceTransforms[index] = transform;
	}

	int InstancedMesh::AddInstance(const scene::Transform& transform, bool markDirty)
	{
		scene::Transform copyTransform;
		copyTransform = transform;
		mInstanceTransforms.push_back(copyTransform);
		mInstanceCount++;
		if (markDirty) mIsDirty = true;
		return static_cast<int>(mInstanceTransforms.size());
	}

	void InstancedMesh::AddInstances(const std::vector<scene::Transform>& transforms)
	{
		for (const auto transform : transforms)
		{
			AddInstance(transform);
		}
		mIsDirty = true;
	}

	bool InstancedMesh::RemoveInstance(int index, bool markDirty)
	{
		const auto originalSize = mInstanceTransforms.size();
		mInstanceTransforms.erase(mInstanceTransforms.begin() + index);
		if (markDirty) mIsDirty = true;
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
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
		model = glm::scale(model, scale);
		Shader::UpdateShaderPointLighting(material->GetShader(), position);
		Shader::UpdateShaderDirectionalLighting(material->GetShader());
		if (auto overrideMat = Engine::Instance().GetRenderManager().GetCurrentPassOverrideMaterial())
		{
			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					overrideMat, model, true, mInstanceCount));
		}
		else
		{
			Engine::Instance().GetRenderManager().Submit(
				BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, model, true, mInstanceCount));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void InstancedMesh::Draw(scene::Transform& transform, std::shared_ptr<Material> material)
	{
		PROFILE_FUNCTION();
		if (mIsDirty) RegenVertexArray();

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));

		Shader::UpdateShaderPointLighting(material->GetShader(), transform.Position());
		Shader::UpdateShaderDirectionalLighting(material->GetShader());

		if (auto overrideMat = Engine::Instance().GetRenderManager().GetCurrentPassOverrideMaterial())
		{

			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					overrideMat, transform.GetGlobalMatrix(), GL_LEQUAL, true, mInstanceCount));
		}
		else
		{
			Engine::Instance().GetRenderManager().Submit(
				BASED_SUBMIT_RC(
					RenderVertexArrayMaterial, mVA, material, transform.GetGlobalMatrix(), GL_LEQUAL, true, mInstanceCount));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void InstancedMesh::RegenVertexArray()
	{
		static bool uploaded_models = false;
		InstancedVertexBuffer<float>* model_vb;

		BASED_CREATE_INSTANCED_VERTEX_BUFFER_FULL(vb, float, mInstanceCount * 4, true);
		
		if (uploaded_models)
		{
			model_vb = (InstancedVertexBuffer<float>*)mVA->GetVertexBuffer(mVA->GetVBOCount() - 1u);
		} else
		{
			model_vb = vb.get();
		}

		for (int i = 0; i < mInstanceCount; i++)
		{
			auto& transform = mInstanceTransforms[i];

			auto model = transform.GetGlobalMatrix();

			for (int j = 0; j < 4; j++)
			{
				model_vb->PushVertex({model[j][0], model[j][1], model[j][2], model[j][3]});
			}
		}

		model_vb->SetLayout({ 4, 4, 4, 4 });
		if (!uploaded_models) mVA->PushBuffer(std::move(vb));
		mVA->Upload();
		uploaded_models = true;

		mIsDirty = false;
	}
}
