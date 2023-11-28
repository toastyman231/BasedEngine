#include "graphics/model.h"

namespace based::graphics
{
	void Model::Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->Draw(position, rotation, scale, mMaterials[i]);
		}
	}

	scene::Entity* Model::CreateModelEntity(const std::string& path)
	{
		PROFILE_FUNCTION();
		Model* model = new Model();
		auto* rootEntity = scene::Entity::CreateEntity<scene::Entity>();

		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		rootEntity->SetEntityName(std::string(scene->mName.C_Str()));

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			BASED_ERROR("Assimp Error: {}", import.GetErrorString());
			return rootEntity;
		}
		model->directory = path.substr(0, path.find_last_of('/'));

		model->ProcessNodeEntity(rootEntity, scene->mRootNode, scene);

		return rootEntity;
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
		directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		PROFILE_FUNCTION();
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	void Model::ProcessNodeEntity(scene::Entity* parent, aiNode* node, const aiScene* scene)
	{
		PROFILE_FUNCTION();
		auto* childEntity = scene::Entity::CreateEntity<scene::Entity>();
		if (node->mNumMeshes > 0) childEntity->SetParent(parent);

		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			childEntity->SetEntityName(std::string(mesh->mName.C_Str()));
			Mesh* processedMesh = ProcessMesh(mesh, scene);
			meshes.push_back(processedMesh);
			meshes.back()->material = std::move(mMaterials.back());
			childEntity->AddComponent<scene::MeshRenderer>(processedMesh, this);
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			if (node->mNumMeshes == 0) ProcessNodeEntity(parent, node->mChildren[i], scene);
			else ProcessNodeEntity(childEntity, node->mChildren[i], scene);
		}
	}

	graphics::Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		PROFILE_FUNCTION();
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
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

			vertices.push_back(vertex);
		}

		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::shared_ptr<Material> meshMaterial = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

			mMaterials.insert(mMaterials.end(), meshMaterial);
			/*std::vector<Texture> diffuseMaps = LoadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Texture> specularMaps = LoadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());*/
		}

		return new Mesh(vertices, indices, textures);
	}

	std::shared_ptr<Material> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		PROFILE_FUNCTION();
		std::shared_ptr<Material> material = std::make_shared<Material>(DefaultLibraries::GetShaderLibrary().Get("Model"));

		aiColor4D color(0.f, 0.f, 0.f, 1.f);
		aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color);
		material->SetUniformValue("color_diffuse", glm::vec4(color.r, color.g, color.b, color.a));

		// TODO: Add more complicated material support

		if (mat->GetTextureCount(type) <= 0)
		{
			material->SetUniformValue("textureSample", 0);
		} else
		{
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				BASED_TRACE("Texture location: {}", str.C_Str());
				Texture texture(str.C_Str());

				material->SetTexture(std::make_shared<Texture>(texture));
			}
		}

		return material;
	}
}