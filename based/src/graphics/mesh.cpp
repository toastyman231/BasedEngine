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
	std::shared_ptr<Mesh> Mesh::CreateMesh(const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, const std::vector<Texture>& textures,
		core::AssetLibrary<Mesh>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<Mesh>(vertices, indices, textures);
		assetLibrary.Load(name, asset);
		return asset;
	}

	std::shared_ptr<Mesh> Mesh::CreateMesh(const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, const std::vector<Texture>& textures,
		core::AssetLibrary<Mesh>& assetLibrary, core::UUID uuid, const std::string& name)
	{
		auto asset = std::make_shared<Mesh>(vertices, indices, textures);
		asset->mUUID = uuid;
		assetLibrary.Load(name, asset);
		return asset;
	}

	std::shared_ptr<InstancedMesh> Mesh::CreateInstancedMesh(const std::vector<Vertex>& vertices,
	                                                         const std::vector<unsigned int>& indices, const std::vector<Texture>& textures,
	                                                         core::AssetLibrary<Mesh>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<InstancedMesh>(vertices, indices, textures);
		assetLibrary.Load(name, asset);
		return asset;
	}

	std::shared_ptr<InstancedMesh> Mesh::CreateInstancedMesh(const std::shared_ptr<Mesh>& mesh,
		core::AssetLibrary<Mesh>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<InstancedMesh>(mesh->vertices, mesh->indices, mesh->textures);
		asset->mMeshSource = mesh->mMeshSource;
		assetLibrary.Load(name, asset);
		return asset;
	}

	std::shared_ptr<Mesh> Mesh::CreateMesh(const std::shared_ptr<VertexArray>& va, const std::shared_ptr<Material>& mat,
	                                       core::AssetLibrary<Mesh>& assetLibrary, const std::string& name)
	{
		auto asset = std::make_shared<Mesh>(va, mat);
		assetLibrary.Load(name, asset);
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
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<Texture> textures;

			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;

				//SetVertexBoneDataToDefault(vertex);

				// process vertex positions, normals and texture coordinates
				glm::vec3 vector;
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;

				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;

				if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}
				else
					vertex.TexCoords = glm::vec2(0.0f, 0.0f);

				// process vertex tangent space
				if (mesh->mTangents)
				{
					vector.x = mesh->mTangents[i].x;
					vector.y = mesh->mTangents[i].y;
					vector.z = mesh->mTangents[i].z;
					vertex.Tangent = vector;
				}

				vertices.emplace_back(vertex);
			}

			// process indices
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.emplace_back(face.mIndices[j]);
			}

			auto loadedMesh = std::make_shared<Mesh>(vertices, indices, textures, uuid);
			loadedMesh->mMeshSource = filepath;
			loadedMesh->material = Material::LoadMaterialFromFile(
				ASSET_PATH("Materials/Unlit.bmat"),
				Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage());
			assetLibrary.Load(scene->mName.C_Str(), loadedMesh);
			return loadedMesh;
		}

		BASED_ERROR("No meshes found in loaded file");
		return nullptr;
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, const std::vector<Texture>& textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		mUUID = core::UUID();

		SetupMesh();
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
		const std::vector<Texture>& textures, core::UUID uuid)
			: Mesh(vertices, indices, textures)
	{
		mUUID = uuid;
	}

	Mesh::Mesh(const std::shared_ptr<VertexArray>& va, const std::shared_ptr<Material>& mat)
	{
		mVA = va;
		material = mat;
		mUUID = core::UUID();
	}

	Mesh::Mesh(const std::shared_ptr<VertexArray>& va, const std::shared_ptr<Material>& mat, core::UUID uuid)
		: Mesh(va, mat)
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

	void Mesh::Draw(scene::Transform transform, std::shared_ptr<Material> material)
	{
		const glm::vec3 pos = transform.Position;
		const glm::vec3 rot = transform.Rotation;
		const glm::vec3 scale = transform.Scale;
		const glm::vec3 localPos = transform.LocalPosition;
		const glm::vec3 localRot = transform.LocalRotation;

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		// Translations are applied in the opposite order they are defined here
		// So - scale, rotate by local rotation, translate by local position, rotate by global rotation,
		// translate by global translation

		auto model = glm::mat4(1.f);
		model = glm::translate(model, pos - localPos);

		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(-(rot.y - localRot.y)), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(-(rot.x - localRot.x)), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(-(rot.z - localRot.z)), glm::vec3(0.f, 0.f, 1.f));

		model = glm::translate(model, localPos);

		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(localRot.y), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(localRot.x), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(localRot.z), glm::vec3(0.f, 0.f, 1.f));

		model = glm::scale(model, scale);
		Shader::UpdateShaderPointLighting(material->GetShader(), pos);
		Shader::UpdateShaderDirectionalLighting(material->GetShader());
		if (auto overrideMat = Engine::Instance().GetRenderManager().GetCurrentPassOverrideMaterial())
		{
			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					overrideMat, model));
		}
		else
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

	void Mesh::Draw(scene::Transform transform)
	{
		if (material)
		{
			Draw(transform, material);
		}
		else
		{
			BASED_ERROR("ERROR: Trying to render mesh with no material set!");
			// TODO: Replace this with default material
		}
	}

	void Mesh::ProcessNode(aiNode* node, const aiScene* scene)
	{
		PROFILE_FUNCTION();

		if (node->mNumMeshes > 0)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
			ProcessMesh(mesh, scene);
			return;
		}

		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		PROFILE_FUNCTION();
		

		// process material
		/*if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::shared_ptr<Material> meshMaterial = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

			mMaterials.insert(mMaterials.end(), meshMaterial);
		}

		ExtractBoneWeightForVertices(vertices, mesh, scene);*/

		//meshes.emplace_back(std::make_shared<Mesh>(vertices, indices, textures));
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

		for (const auto& vertex : vertices)
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

		//mVA.reset();
		mVA = va;
	}

	void InstancedMesh::SetInstanceTransform(int index, const scene::Transform& transform)
	{
		if (index < 0 || index > mInstanceCount) return;
		mInstanceTransforms[index] = transform;
	}

	int InstancedMesh::AddInstance(scene::Transform transform, bool markDirty)
	{
		mInstanceTransforms.emplace_back(transform);
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

	void InstancedMesh::Draw(scene::Transform transform, std::shared_ptr<Material> material)
	{
		PROFILE_FUNCTION();
		if (mIsDirty) RegenVertexArray();

		const glm::vec3 pos = transform.Position;
		const glm::vec3 rot = transform.Rotation;
		const glm::vec3 scale = transform.Scale;
		const glm::vec3 localPos = transform.LocalPosition;
		const glm::vec3 localRot = transform.LocalRotation;
		const glm::vec3 localScale = transform.LocalScale;

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera,
			Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()));
		// Translations are applied in the opposite order they are defined here
		// So - scale, rotate by local rotation, translate by local position, rotate by global rotation,
		// translate by global translation

		auto model = glm::mat4(1.f);
		model = glm::translate(model, pos - localPos);

		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(-(rot.y - localRot.y)), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(-(rot.x - localRot.x)), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(-(rot.z - localRot.z)), glm::vec3(0.f, 0.f, 1.f));

		model = glm::translate(model, localPos);

		// Rotations are passed as degrees and converted to radians here automatically
		model = glm::rotate(model, glm::radians(localRot.y), glm::vec3(0.f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(localRot.x), glm::vec3(1.f, 0.f, 0.f));
		model = glm::rotate(model, glm::radians(localRot.z), glm::vec3(0.f, 0.f, 1.f));

		model = glm::scale(model, scale * localScale);
		Shader::UpdateShaderPointLighting(material->GetShader(), pos);
		Shader::UpdateShaderDirectionalLighting(material->GetShader());
		if (auto overrideMat = Engine::Instance().GetRenderManager().GetCurrentPassOverrideMaterial())
		{

			if (material->GetUniformValue<int>("castShadows", 1) != 0)
				Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA,
					overrideMat, model, GL_LEQUAL, true, mInstanceCount));
		}
		else
		{
			Engine::Instance().GetRenderManager().Submit(
				BASED_SUBMIT_RC(RenderVertexArrayMaterial, mVA, material, model, GL_LEQUAL, true, mInstanceCount));
		}
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void InstancedMesh::RegenVertexArray()
	{
		SetupMesh(false);
		

		BASED_CREATE_INSTANCED_VERTEX_BUFFER_FULL(model_vb, float, mInstanceCount * 4, true);

		for (int i = 0; i < mInstanceCount; i++)
		{
			auto transform = mInstanceTransforms[i];

			auto model = glm::mat4(1.f);
			model = glm::translate(model, transform.Position);
			// Rotations are passed as degrees and converted to radians here automatically
			model = glm::rotate(model, glm::radians(transform.Rotation.z), glm::vec3(0.f, 0.f, 1.f));
			model = glm::rotate(model, glm::radians(transform.Rotation.x), glm::vec3(1.f, 0.f, 0.f));
			model = glm::rotate(model, glm::radians(transform.Rotation.y), glm::vec3(0.f, 1.f, 0.f));
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
