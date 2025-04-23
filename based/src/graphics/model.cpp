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

	void Model::LoadModel(std::string path)
	{
		PROFILE_FUNCTION();
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

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

		meshes.emplace_back(std::make_shared<Mesh>(vertices, indices));
	}

	std::shared_ptr<Material> Model::LoadMaterial(aiMaterial* mat, const aiScene* scene)
	{
		PROFILE_FUNCTION();

		aiString name;
		mat->Get(AI_MATKEY_NAME, name);
		auto _name = std::string(name.C_Str());

		std::shared_ptr<Material> material = graphics::Material::LoadMaterialFromFile(ASSET_PATH("Materials/Lit.bmat"),
			Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage(), "", _name);

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

		ai_real x;
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
		//material->SetUniformValue("ClearCoat", x);

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

		LoadMaterialTexture(mat, scene, material, "albedo", 0, albedoType);
		LoadMaterialTexture(mat, scene, material, "normal", 1, normalType);
		LoadMaterialTexture(mat, scene, material, "metallic", 2, aiTextureType_METALNESS);
		LoadMaterialTexture(mat, scene, material, "roughness", 3, roughnessType);
		LoadMaterialTexture(mat, scene, material, "ambientOcclusion", 4, aiTextureType_AMBIENT_OCCLUSION);

		return material;
	}

	void Model::LoadMaterialTexture(aiMaterial* mat, const aiScene* scene,
	                                std::shared_ptr<Material> material, const std::string& attributeName, int sampler, int type)
	{
		aiTextureType textureType = static_cast<aiTextureType>(type);

		if (mat->GetTextureCount(textureType) <= 0) return;

		material->SetUniformValue("material." + attributeName + ".useSampler", (sampler != -1) ? 1 : 0);
		if (sampler != -1) 
		{
			for (unsigned int i = 0; i < mat->GetTextureCount(textureType); i++)
			{
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
						} else
						{
							int w;
							int h;
							int numChannels;
							unsigned char* data = stbi_load_from_memory((unsigned char*)texInfo->pcData, 
								texInfo->mWidth,
								&w, &h, &numChannels, 3);
							material->AddTexture(std::make_shared<Texture>(w, h, numChannels, data));
						}
						material->SetUniformValue("material." + attributeName + ".tex", sampler);
						return;
					} else
					{
						str = mDirectory + std::string("/") + str.C_Str();
					}
				}
				BASED_TRACE("{} texture location: {}", attributeName, str.C_Str());

				material->AddTexture(std::make_shared<Texture>(str.C_Str(), true));
			}
			material->SetUniformValue("material." + attributeName + ".tex", sampler);
		}
	}

	void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
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
}
