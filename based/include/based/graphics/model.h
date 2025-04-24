#pragma once

#include <based/graphics/shader.h>
#include <based/graphics/texture.h>
#include <based/graphics/mesh.h>
#include <based/graphics/defaultassetlibraries.h>

#include <based/scene/entity.h>

#include <utility>

#define DEFAULT_MODEL_LIB based::graphics::DefaultLibraries::GetModelLibrary()

struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;

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
        Model(const char* path, std::string name = "New Model")
    		: mModelName(std::move(name)), mUUID(core::UUID())
        {
            LoadModel(path);
        }
        Model(const char* path, core::UUID uuid, std::string name = "New Model")
	        : mModelName(std::move(name)), mUUID(uuid)
        {
            LoadModel(path);
        }
        ~Model() { BASED_TRACE("Destroying model {}!", mModelName); }
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) const;
        void Draw(scene::Transform& transform) const;
        void SetMaterial(const std::shared_ptr<Material>& mat, int index = 0);
        void SetMaterials(const std::vector<std::shared_ptr<Material>>& materials);

        static std::shared_ptr<Model> CreateModel(const std::string& path, 
            core::AssetLibrary<Model>& assetLibrary, const std::string& name);
        static std::shared_ptr<Model> CreateModelWithUUID(const std::string& path,
            core::AssetLibrary<Model>& assetLibrary, const std::string& name, core::UUID uuid);

        auto& GetBoneInfoMap() { return m_BoneInfoMap; }
        int& GetBoneCount() { return m_BoneCounter; }

        inline std::shared_ptr<Material> GetMaterial(int index = 0) const { return mMaterials[index]; }
        inline std::vector<std::shared_ptr<Material>> GetMaterials() const { return mMaterials; }

        inline int GetNumMeshes() const { return (int)meshes.size(); }

        inline bool IsFileModel() const { return !mModelSource.empty(); }
        inline std::string GetModelSource() const { return mModelSource; }
        inline std::string GetModelName() const { return mModelName; }

        inline core::UUID GetUUID() const { return mUUID; }
    private:
        // model data
        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<Material>> mMaterials;
        std::string mModelSource;
        std::string mDirectory;
        std::string mModelName;
        core::UUID mUUID;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;

        inline static std::vector<glm::mat4> mDefaultBones;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        void ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::shared_ptr<Material> LoadMaterial(aiMaterial* mat, const aiScene* scene);
        bool LoadMaterialTexture(aiMaterial* mat, const aiScene* scene,
                                 std::shared_ptr<Material> material, const std::string& attributeName, int sampler,
                                 int type);
        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
        void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
        void SetVertexBoneDataToDefault(Vertex& vertex);
        void ReadNodeHierarchy(const aiNode* node, glm::mat4& parentTransform);
    };
}
