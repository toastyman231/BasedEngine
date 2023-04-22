#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

//#include "external/stb/stb_image.h"
//#include "glad/glad.h"

#include <based/graphics/shader.h>
#include <based/graphics/texture.h>
#include <based/graphics/mesh.h>
#include <based/graphics/defaultassetlibraries.h>

#include <based/scene/entity.h>

namespace based::graphics
{
    class Model : public scene::Entity
    {
    public:
        Model(char* path)
        {
            LoadModel(path);
        }
        void Draw(std::shared_ptr<Material> material);
    private:
        // model data
        std::vector<graphics::Mesh> meshes;
        std::string directory;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        graphics::Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<graphics::Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };
}
