#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <based/graphics/shader.h>
#include <based/graphics/texture.h>
#include <based/graphics/mesh.h>
#include <based/graphics/defaultassetlibraries.h>

#include <based/scene/entity.h>

namespace based::graphics
{
    class Model
    {
    public:
        Model(const char* path)
        {
            LoadModel(path);
        }
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
    private:
        // model data
        std::vector<graphics::Mesh> meshes;
        std::vector<std::shared_ptr<Material>> mMaterials;
        std::string directory;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        graphics::Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::shared_ptr<Material> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };
}
