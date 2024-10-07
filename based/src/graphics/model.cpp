#include "graphics/model.h"

namespace based::graphics
{
	void Model::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) const
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->Draw(position, rotation, scale, mMaterials[i]);
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

	std::shared_ptr<Mesh> Model::LoadSingleMesh(const std::string& path)
	{
		const Model* model = new Model(path.c_str());
		std::shared_ptr<Mesh> meshToCopy = model->meshes[0];
		delete model;

		return meshToCopy;
	}

	std::shared_ptr<Model> Model::CreateModel(const std::string& path, core::AssetLibrary<Model>& assetLibrary,
		const std::string& name)
	{
		auto model = std::make_shared<Model>(path.c_str(), name);
		assetLibrary.Load(name, model);
		return model;
	}

	void Model::LoadModel(std::string path)
	{
		PROFILE_FUNCTION();
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			BASED_ERROR("Assimp Error: {}", import.GetErrorString());
			return;
		}
		mDirectory = path.substr(0, path.find_last_of('/'));

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

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::shared_ptr<Material> meshMaterial = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

			mMaterials.insert(mMaterials.end(), meshMaterial);
		}

		ExtractBoneWeightForVertices(vertices, mesh, scene);

		meshes.emplace_back(std::make_shared<Mesh>(vertices, indices, textures));
	}

	std::shared_ptr<Material> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		PROFILE_FUNCTION();
		std::shared_ptr<Material> material = std::make_shared<Material>(DefaultLibraries::GetShaderLibrary().Get("Model"));

		if (mat->GetTextureCount(type) <= 0)
		{
			SetMaterialAttribute(mat, material, "$clr.diffuse", "diffuseMat", -1, aiTextureType_UNKNOWN);
			SetMaterialAttribute(mat, material, "$clr.ambient", "ambientMat", -1, aiTextureType_UNKNOWN);
			SetMaterialAttribute(mat, material, "$clr.specular", "specularMat", -1, aiTextureType_UNKNOWN);
			SetMaterialAttribute(mat, material, "$mat.shininess", "shininessMat", -1, aiTextureType_UNKNOWN);
			SetMaterialAttribute(mat, material, "$clr.emissive", "emissiveMat", -1, aiTextureType_UNKNOWN);
		} else
		{
			SetMaterialAttribute(mat, material, "$clr.diffuse", "diffuseMat", 0, aiTextureType_DIFFUSE);
			SetMaterialAttribute(mat, material, "$clr.ambient", "ambientMat", 1, aiTextureType_AMBIENT);
			SetMaterialAttribute(mat, material, "$clr.specular", "specularMat", 2, aiTextureType_SPECULAR);
			SetMaterialAttribute(mat, material, "none", "normalMat", 3, aiTextureType_NORMALS);
			SetMaterialAttribute(mat, material, "$mat.shininess", "shininessMat", 4, aiTextureType_SHININESS);
			SetMaterialAttribute(mat, material, "$clr.emissive", "emissiveMat", 5, aiTextureType_EMISSIVE);
		}

		return material;
	}

	void Model::SetMaterialAttribute(aiMaterial* mat, std::shared_ptr<Material> material, const char* key,
		const std::string& attributeName, int sampler, aiTextureType type)
	{
		if (key != "none")
		{
			aiColor4D color(0.f, 0.f, 0.f, 1.f);
			if (attributeName == "shininessMat")
			{
				ai_real shininess;
				mat->Get(AI_MATKEY_SHININESS, shininess);
				material->SetUniformValue("material." + attributeName + ".color", glm::vec4(shininess, 0.f, 0.f, 0.f));
			}
			else
			{
				aiGetMaterialColor(mat, key, 0, 0, &color);
				material->SetUniformValue("material." + attributeName + ".color", 
					glm::vec4(color.r, color.g, color.b, color.a));
			}
		}

		material->SetUniformValue("material." + attributeName + ".useSampler", (sampler != -1) ? 1 : 0);
		if (sampler != -1) 
		{
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				struct stat sb;
				bool result = stat(str.C_Str(), &sb) == 0;
				if (!result)
				{
					str = mDirectory + std::string("/") + str.C_Str();
				}
				BASED_TRACE("Texture location: {}", str.C_Str());

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
					mesh->mBones[boneIndex]->mOffsetMatrix);
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
