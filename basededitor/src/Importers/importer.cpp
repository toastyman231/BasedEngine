#include "based/pch.h"
#include "importer.h"

#include "../editorstatics.h"
#include "../../../external/assimp/include/assimp/Importer.hpp"
#include "../../../external/assimp/include/assimp/postprocess.h"
#include "../../../external/assimp/include/assimp/scene.h"
#include "based/app.h"
#include "based/animation/animation.h"
#include "based/graphics/mesh.h"
#include "based/graphics/model.h"

namespace editor
{
	bool MeshImporter::CanHandleFile(const std::string& path)
	{
		if (!(IsFileOfType(path, ".fbx") || IsFileOfType(path, ".obj")
			|| IsFileOfType(path, ".dae") || IsFileOfType(path, ".glTF")
			|| IsFileOfType(path, ".glb") || IsFileOfType(path, ".3ds")))
		{
			return false;
		} else
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

			if (scene->mNumAnimations > 0) return false;
			if (scene->mNumMeshes > 1) return false;
			for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
			{
				if (scene->mMeshes[i]->mNumBones > 0)
				{
					return false;
				}
			}

			return true;
		}
	}

	void MeshImporter::HandleImport(const std::string& path)
	{
		based::graphics::Mesh::LoadMeshFromFile(path,
			based::Engine::Instance().GetApp().GetCurrentScene()->GetMeshStorage());
	}

	bool ModelImporter::CanHandleFile(const std::string& path)
	{
		if (!(IsFileOfType(path, ".fbx") || IsFileOfType(path, ".obj")
			|| IsFileOfType(path, ".dae") || IsFileOfType(path, ".glTF")
			|| IsFileOfType(path, ".glb") || IsFileOfType(path, ".3ds")))
		{
			return false;
		}
		else return true;
	}

	void ModelImporter::HandleImport(const std::string& path)
	{
		based::graphics::Model::CreateModel(path,
			based::Engine::Instance().GetApp().GetCurrentScene()->GetModelStorage(),
			"TEMP");
	}

	bool TextureImporter::CanHandleFile(const std::string& path)
	{
		if (path.find(".png") != std::string::npos
			|| path.find(".PNG") != std::string::npos
			|| path.find(".jpg") != std::string::npos
			|| path.find(".JPG") != std::string::npos
			|| path.find(".jpeg") != std::string::npos
			|| path.find(".JPEG") != std::string::npos
			|| path.find(".tga") != std::string::npos
			|| path.find(".TGA") != std::string::npos
			|| path.find(".ico") != std::string::npos
			|| path.find(".ICO") != std::string::npos)
			return true;
		else return false;
	}

	void TextureImporter::HandleImport(const std::string& path)
	{
		auto tex = std::make_shared<based::graphics::Texture>(path);
		auto storage = based::Engine::Instance().GetApp().GetCurrentScene()->GetTextureStorage();
		storage.Load(std::filesystem::path(path).filename().replace_extension("").string(), tex);
	}

	bool MaterialImporter::CanHandleFile(const std::string& path)
	{
		if (IsFileOfType(path, ".bmat")) return true;
		else return false;
	}

	void MaterialImporter::HandleImport(const std::string& path)
	{
		based::graphics::Material::LoadMaterialFromFileAbsolute(path,
			Statics::GetProjectDirectory() + "/");
	}

	bool AnimationImporter::CanHandleFile(const std::string& path)
	{
		if (IsFileOfType(path, ".banim")) return true;
		else return false;
	}

	void AnimationImporter::HandleImport(const std::string& path)
	{
		based::animation::Animation::LoadAnimationFromFileAbsolute(path,
			based::Engine::Instance().GetApp().GetCurrentScene()->GetAnimationStorage(),
			Statics::GetProjectDirectory() + "/");
	}
}
