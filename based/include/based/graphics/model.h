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
        Model() = default;
        Model(const char* path)
        {
            LoadModel(path);
        }
        ~Model() { BASED_TRACE("Destroying model!"); }
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

        static scene::Entity* CreateModelEntity(const std::string& path);
    private:
        // model data
        std::vector<graphics::Mesh*> meshes;
        std::vector<std::shared_ptr<Material>> mMaterials;
        std::string directory;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        void ProcessNodeEntity(scene::Entity* parent, aiNode* node, const aiScene* scene);
        graphics::Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::shared_ptr<Material> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
        void SetMaterialAttribute(aiMaterial* mat, std::shared_ptr<Material> material, const char* key, 
            const std::string& attributeName, int sampler, aiTextureType type);
    };
}
