#include "pch.h"
#include "graphics/model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "external/stb/stb_image.h"

namespace based::graphics
{
	void Model::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) const
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->Draw(position, rotation, scale, mMaterials[i]);
		}
	}

	void Model::Draw(scene::Transform& transform) const
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->Draw(transform, mMaterials[i]);
		}
	}

	void Model::SetMaterial(const std::shared_ptr<Material>& mat, int index)
	{
		if (index < 0 || index >= static_cast<int>(mMaterials.size()))
		{
			BASED_WARN("Trying to set index {}, but there are only {} materials!", index, mMaterials.size());
			return;
		}

		mMaterials[index] = mat;
	}

	void Model::SetMaterials(const std::vector<std::shared_ptr<Material>>& materials)
	{
		mMaterials = materials;
	}

	std::shared_ptr<Model> Model::CreateModel(const std::string& path, core::AssetLibrary<Model>& assetLibrary,
	                                          const std::string& name)
	{
		return CreateModelWithUUID(path, assetLibrary, name, core::UUID());
	}

	std::shared_ptr<Model> Model::CreateModelWithUUID(const std::string& path, core::AssetLibrary<Model>& assetLibrary,
		const std::string& name, core::UUID uuid)
	{
		auto model = std::make_shared<Model>(path.c_str(), name);
		model->mUUID = uuid;
		assetLibrary.Load(model->mModelName, model);
		return model;
	}

	std::vector<std::shared_ptr<scene::Entity>> Model::GenerateModelCollisions(const scene::Transform& origin)
	{
		std::vector<std::shared_ptr<scene::Entity>> returnedEntities;

		for (const auto& mesh : meshes)
		{
			auto collider = scene::Entity::CreateEntity(mesh->GetMeshName() + " Gen Collision");
			collider->AddComponent<scene::MeshShapeComponent>(mesh);
			auto meshShape = collider->GetComponent<scene::MeshShapeComponent>();
			collider->AddComponent<scene::RigidbodyComponent>(meshShape, JPH::EMotionType::Static,
				physics::Layers::STATIC, origin.Position(), origin.EulerAngles());

			Engine::Instance().GetApp().GetCurrentScene()->GetEntityStorage().Load(collider->GetEntityName(), collider);
			returnedEntities.push_back(collider);
		}

		return returnedEntities;
	}

	void Model::LoadModel(std::string path)
	{
		PROFILE_FUNCTION();
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, 
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_PopulateArmatureData
			| aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_GenBoundingBoxes);

		if (mDefaultBones.empty())
		{
			mDefaultBones.reserve(100);

			for (int i = 0; i < 100; i++)
			{
				mDefaultBones.emplace_back(glm::mat4(1.f));
			}
		}

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			BASED_ERROR("Assimp Error: {}", import.GetErrorString());
			return;
		}
		mDirectory = path.substr(0, path.find_last_of('/'));
		mModelSource = path;
		mModelName = scene->mMeshes[0]->mName.C_Str();

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		PROFILE_FUNCTION();
		BASED_TRACE("Node: {}", node->mName.C_Str());
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene);
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	glm::mat4 AssimpMatrixToGLM(const aiMatrix4x4& mat) {
		return glm::mat4(
			mat.a1, mat.b1, mat.c1, mat.d1,
			mat.a2, mat.b2, mat.c2, mat.d2,
			mat.a3, mat.b3, mat.c3, mat.d3,
			mat.a4, mat.b4, mat.c4, mat.d4
		);
	}

	void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		PROFILE_FUNCTION();
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			SetVertexBoneDataToDefault(vertex);

			// process vertex positions, normals and texture coordinates
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			if (mesh->mNormals)
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

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

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::shared_ptr<Material> meshMaterial = LoadMaterial(material, scene);

			mMaterials.insert(mMaterials.end(), meshMaterial);
		}

		ExtractBoneWeightForVertices(vertices, mesh, scene);

		auto m = std::make_shared<Mesh>(vertices, indices);
		meshes.emplace_back(m);

		if (mesh->HasBones())
		{
			ReadNodeHierarchy(scene->mRootNode, glm::mat4(1.f));

			for (int i = 0; i < mDefaultBones.size(); i++)
			{
				mMaterials.back()->SetUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", 
					mDefaultBones[i]);
				mDefaultBones[i] = glm::mat4(1.f);
			}
		}
	}

	std::shared_ptr<Material> Model::LoadMaterial(aiMaterial* mat, const aiScene* scene)
	{
		PROFILE_FUNCTION();

		aiString name;
		mat->Get(AI_MATKEY_NAME, name);
		auto _name = std::string(name.C_Str());

		auto& matStorage = Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage();

		if (matStorage.Exists(_name))
		{
			BASED_WARN("Loading existing material {}!", _name);
			return matStorage.Get(_name);
		}

		std::shared_ptr<Material> material = graphics::Material::LoadMaterialFromFile(ASSET_PATH("Materials/Lit.bmat"),
			matStorage, "", _name);
		BASED_WARN("Creating new material {}!", _name);

		material->mMaterialName = _name;

		aiColor4D color(0.f, 0.f, 0.f, 1.f);
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->SetUniformValue("material.albedo.color",
			glm::vec4(color.r, color.g, color.b, color.a));

		mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, color);
		material->SetUniformValue("material.roughness.color",
			glm::vec4(color.r, color.g, color.b, color.a));
		BASED_TRACE("Roughness: {} {} {} {}", color.r, color.g, color.b, color.a);

		mat->Get(AI_MATKEY_METALLIC_FACTOR, color);
		material->SetUniformValue("material.metallic.color",
			glm::vec4(color.r, color.g, color.b, color.a));

		mat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->SetUniformValue("material.emission.color",
			glm::vec4(color.r, color.g, color.b, color.a));

		/*ai_real x;
		mat->Get(AI_MATKEY_SPECULAR_FACTOR, x);
		BASED_TRACE("Specular: {}", x);
		//material->SetUniformValue("Specular", x);

		mat->Get(AI_MATKEY_ANISOTROPY_FACTOR, x);
		BASED_TRACE("Anisotropy: {}", x);
		//material->SetUniformValue("Anisotropic", x);

		mat->Get(AI_MATKEY_SHEEN_COLOR_FACTOR, x);
		BASED_TRACE("Sheen: {}", x);
		//material->SetUniformValue("Sheen", x);

		mat->Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, x);
		BASED_TRACE("Sheen Tint (Roughness): {}", x);
		//material->SetUniformValue("SheenTint", x);

		mat->Get(AI_MATKEY_CLEARCOAT_FACTOR, x);
		BASED_TRACE("ClearCoat: {}", x);
		//material->SetUniformValue("ClearCoat", x);*/

		auto albedoType = aiTextureType_BASE_COLOR;
		if (mat->GetTextureCount(aiTextureType_BASE_COLOR) <= 0)
		{
			albedoType = aiTextureType_DIFFUSE;
		}
		auto normalType = aiTextureType_NORMAL_CAMERA;
		if (mat->GetTextureCount(aiTextureType_NORMAL_CAMERA) <= 0)
		{
			normalType = aiTextureType_NORMALS;
		}
		auto roughnessType = aiTextureType_DIFFUSE_ROUGHNESS;
		if (mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) <= 0)
		{
			roughnessType = aiTextureType_SHININESS;
		}
		auto emissionType = aiTextureType_EMISSION_COLOR;
		if (mat->GetTextureCount(aiTextureType_EMISSION_COLOR) <= 0)
		{
			emissionType = aiTextureType_EMISSIVE;
		}

		int samplerIndex = 0;
		if (LoadMaterialTexture(mat, scene, material, "albedo", samplerIndex, albedoType)) samplerIndex++;
		if (LoadMaterialTexture(mat, scene, material, "normal", samplerIndex, normalType)) samplerIndex++;
		if (LoadMaterialTexture(mat, scene, material, "metallic", samplerIndex, aiTextureType_METALNESS)) samplerIndex++;
		if (LoadMaterialTexture(mat, scene, material, "roughness", samplerIndex, roughnessType)) samplerIndex++;
		if (LoadMaterialTexture(mat, scene, material, "ambientOcclusion", samplerIndex, 
			aiTextureType_AMBIENT_OCCLUSION)) samplerIndex++;
		if (LoadMaterialTexture(mat, scene, material, "emission", samplerIndex, emissionType)) samplerIndex++;

		return material;
	}

	bool Model::LoadMaterialTexture(aiMaterial* mat, const aiScene* scene,
	                                std::shared_ptr<Material> material, const std::string& attributeName, int sampler,
	                                int type)
	{
		PROFILE_FUNCTION();

		aiTextureType textureType = static_cast<aiTextureType>(type);

		if (mat->GetTextureCount(textureType) <= 0 || sampler < 0) return false;

		material->SetUniformValue("material." + attributeName + ".useSampler", (sampler != -1) ? 1 : 0);
		
		for (unsigned int i = 0; i < mat->GetTextureCount(textureType); i++)
		{
			// TODO: Don't generate a new texture if it's already been generated
			aiString str;
			mat->GetTexture(textureType, i, &str);
			struct stat sb;
			bool result = stat(str.C_Str(), &sb) == 0;
			if (!result)
			{
				if (str.data[0] == '*')
				{
					auto texInfo = scene->GetEmbeddedTexture(str.C_Str());
					if (texInfo->achFormatHint == "rgba8888" || texInfo->achFormatHint == "argb8888")
					{
						material->AddTexture(std::make_shared<Texture>(texInfo->mWidth, texInfo->mHeight, 3,
							(unsigned char*)texInfo->pcData));
					}
					else
					{
						// TODO: Make this actually work
						int w;
						int h;
						int numChannels;
						unsigned char* data = stbi_load_from_memory((unsigned char*)texInfo->pcData,
							texInfo->mWidth,
							&w, &h, &numChannels, 3);
						material->AddTexture(std::make_shared<Texture>(w, h, numChannels, data));
					}
					material->SetUniformValue("material." + attributeName + ".tex", sampler);
					return true;
				}
				else
				{
					str = mDirectory + std::string("/") + str.C_Str();
				}
			}
			BASED_TRACE("{} texture location: {}", attributeName, str.C_Str());

			material->AddTexture(
				Engine::Instance().GetResourceManager().LoadTextureAsync(std::string(str.C_Str()), true)
			/*std::make_shared<Texture>(str.C_Str(), true)*/);
		}
		material->SetUniformValue("material." + attributeName + ".tex", sampler);
		return true;
	}

	void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		aiMatrix4x4 parentTransform;

		for (int boneIndex = 0; boneIndex < (int) (mesh->mNumBones); ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
					&mesh->mBones[boneIndex]->mOffsetMatrix);
				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++;
			}
			else
			{
				boneID = m_BoneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}

			/*auto bone = mesh->mBones[boneIndex];
			auto nodeTransform = bone->mOffsetMatrix;
			mDefaultBones[boneID] = AssimpMatrixToGLM(parentTransform * nodeTransform * bone->mNode->mTransformation);*/

			//parentTransform = nodeTransform;
		}
	}

	void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0)
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
			}
		}
	}

	void Model::SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}

	void Model::ReadNodeHierarchy(const aiNode* node, glm::mat4& parentTransform)
	{
		aiString nodeName = node->mName;

		aiMatrix4x4 nodeTransform = node->mTransformation;

		glm::mat4 globalTransform = parentTransform * AssimpMatrixToGLM(nodeTransform);

		if (m_BoneInfoMap.find(std::string(nodeName.C_Str())) != m_BoneInfoMap.end())
		{
			int boneID = m_BoneInfoMap[std::string(nodeName.C_Str())].id;
			mDefaultBones[boneID] = globalTransform *
				m_BoneInfoMap[std::string(nodeName.C_Str())].offset;
		}

		for (int i = 0; i < node->mNumChildren; i++)
		{
			ReadNodeHierarchy(node->mChildren[i], globalTransform);
		}
	}
}
