#pragma once

#include "based/engine.h"
#include "material.h"
#include "texture.h"
#include "vertex.h"
#include "based/scene/scene.h"
#include "based/scene/components.h"
#include "based/log.h"

// Taken from learnopengl.com : https://learnopengl.com/Model-Loading/Mesh

#define DEFAULT_MESH_LIB based::graphics::DefaultLibraries::GetMeshLibrary()

struct aiNode;
struct aiMesh;
struct aiScene;

namespace based::graphics
{
	class InstancedMesh;
	class Shader;

#define MAX_BONE_INFLUENCE 4
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
	};

	class Mesh
	{
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;
        std::shared_ptr<Material> material;

        static std::shared_ptr<Mesh> CreateMesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices,
            const std::vector<Texture>& textures,
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Mesh");

        static std::shared_ptr<InstancedMesh> CreateInstancedMesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices,
            const std::vector<Texture>& textures,
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Instanced Mesh");

        static std::shared_ptr<Mesh> CreateMesh(
            const std::shared_ptr<VertexArray>& va, 
            const std::shared_ptr<Material>& mat,
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Mesh");

        static std::shared_ptr<Mesh> LoadMeshFromFile(const std::string& filepath,
            core::AssetLibrary<Mesh>& assetLibrary);
        static std::shared_ptr<Mesh> LoadMeshWithUUID(const std::string& filepath,
            core::AssetLibrary<Mesh>& assetLibrary, core::UUID uuid);

        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
        Mesh(const std::vector<Vertex>& vertices, 
            const std::vector<unsigned int>& indices, 
            const std::vector<Texture>& textures,
            core::UUID uuid);
        Mesh(const std::shared_ptr<VertexArray>& va, const std::shared_ptr<Material>& mat);
        Mesh(const std::shared_ptr<VertexArray>& va, const std::shared_ptr<Material>& mat, core::UUID uuid);
        Mesh(const Mesh& other) = default;
        virtual ~Mesh()
        {
            BASED_TRACE("Destroying mesh");
        }
        virtual void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material);
        virtual void Draw(scene::Transform transform, std::shared_ptr<Material> material);
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
        void Draw(scene::Transform transform);

        bool IsFileMesh() const { return !mMeshSource.empty(); }
        std::string GetMeshSource() const { return mMeshSource; }

        core::UUID GetUUID() const { return mUUID; }
    protected:
        //  render data
        //unsigned int VAO, VBO, EBO;
        core::UUID mUUID;
        std::shared_ptr<VertexArray> mVA;
        std::string mMeshSource;
        Mesh() = default;

        void ProcessNode(aiNode* node, const aiScene* scene);
        void ProcessMesh(aiMesh* mesh, const aiScene* scene);

        void SetupMesh(bool upload = true);
	};

    class InstancedMesh : public Mesh
    {
    public:
        InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	        : Mesh(vertices, indices, textures), mInstanceCount(0), mIsDirty(true) {}
        InstancedMesh(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat, int count)
    		: Mesh(va, mat), mInstanceCount(count), mIsDirty(true) {}
        ~InstancedMesh() override = default;

        void SetInstanceTransform(int index, const scene::Transform& transform);
        int AddInstance(scene::Transform transform, bool markDirty = true);
        void AddInstances(const std::vector<scene::Transform>& transforms);
        bool RemoveInstance(int index, bool markDirty = true);
        bool RemoveInstances(std::vector<int> indices);
        void ClearInstances();
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material) override;
        void Draw(scene::Transform, std::shared_ptr<Material> material) override;

        int GetInstanceCount() const { return mInstanceCount; }
    private:
        int mInstanceCount;
        std::vector<scene::Transform> mInstanceTransforms;
        bool mIsDirty;

        void RegenVertexArray();
    };
}
