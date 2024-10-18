#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <based/graphics/shader.h>
#include <based/graphics/texture.h>
#include <based/graphics/mesh.h>
#include <based/graphics/defaultassetlibraries.h>

#include <based/scene/entity.h>

#define DEFAULT_MODEL_LIB based::graphics::DefaultLibraries::GetModelLibrary()

namespace based::graphics
{
    struct BoneInfo
    {
        /*id is index in finalBoneMatrices*/
        int id;

        /*offset matrix transforms vertex from model space to bone space*/
        glm::mat4 offset;

    };

    class Model
    {
    public:
        Model() = default;
        Model(const char* path, std::string name = "New Model") : mModelName(std::move(name))
        {
            LoadModel(path);
        }
        ~Model() { BASED_TRACE("Destroying model {}!", mModelName); }
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) const;
        void Draw(scene::Transform transform) const;
        void SetMaterial(const std::shared_ptr<Material>& mat, int index = 0);

        static std::shared_ptr<Mesh> LoadSingleMesh(const std::string& path);
        static std::shared_ptr<Model> CreateModel(const std::string& path, 
            core::AssetLibrary<Model>& assetLibrary, const std::string& name);

        auto& GetBoneInfoMap() { return m_BoneInfoMap; }
        int& GetBoneCount() { return m_BoneCounter; }

        inline std::shared_ptr<Material> GetMaterial(int index = 0) const { return mMaterials[index]; }
    private:
        // model data
        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<Material>> mMaterials;
        std::string mDirectory;
        std::string mModelName;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        void ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::shared_ptr<Material> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
        void SetMaterialAttribute(aiMaterial* mat, std::shared_ptr<Material> material, const char* key, 
            const std::string& attributeName, int sampler, aiTextureType type);
        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
        void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
        void SetVertexBoneDataToDefault(Vertex& vertex);
    };
}
