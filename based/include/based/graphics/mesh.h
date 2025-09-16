#pragma once

#include "based/engine.h"
#include "material.h"
#include "texture.h"
#include "vertex.h"
#include "based/scene/scene.h"
#include "based/scene/components.h"
#include "based/log.h"

// Taken from learnopengl.com : https://learnopengl.com/Model-Loading/Mesh

#define DEFAULT_MESH_LIB based::Engine::Instance().GetResourceManager().GetMeshStorage()

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
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Normal = glm::vec3(0.f);
		glm::vec2 TexCoords = glm::vec3(0.f);
        glm::vec3 Tangent = glm::vec3(0.f);
        glm::vec3 Bitangent = glm::vec3(0.f);
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
	};

	class Mesh
	{
    public:
        // mesh data
	    std::vector<std::array<int, MAX_BONE_INFLUENCE>> boneIds;
	    std::vector<std::array<float, MAX_BONE_INFLUENCE>> boneWeights;
        std::vector<unsigned int> indices;

        static std::shared_ptr<Mesh> CreateMesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices,
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Mesh");

        static std::shared_ptr<Mesh> CreateMesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices,
            core::AssetLibrary<Mesh>& assetLibrary,
            core::UUID uuid,
            const std::string& name = "New Mesh");

        static std::shared_ptr<InstancedMesh> CreateInstancedMesh(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& indices,
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Instanced Mesh");

        static std::shared_ptr<InstancedMesh> CreateInstancedMesh(
            const std::shared_ptr<Mesh>& mesh,
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Instanced Mesh"
        );

        static std::shared_ptr<Mesh> CreateMesh(
            const std::shared_ptr<VertexArray>& va, 
            core::AssetLibrary<Mesh>& assetLibrary,
            const std::string& name = "New Mesh");

        static std::shared_ptr<Mesh> LoadMeshFromFile(const std::string& filepath,
            core::AssetLibrary<Mesh>& assetLibrary);
        static std::shared_ptr<Mesh> LoadMeshWithUUID(const std::string& filepath,
            core::AssetLibrary<Mesh>& assetLibrary, core::UUID uuid);

        Mesh() = default;
        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
        Mesh(const std::vector<Vertex>& vertices, 
            const std::vector<unsigned int>& indices, 
            core::UUID uuid);
        Mesh(const std::shared_ptr<VertexArray>& va);
        Mesh(const std::shared_ptr<VertexArray>& va, core::UUID uuid);

		Mesh(const Mesh& other) = delete;
        Mesh& operator = (const Mesh& other) = delete;

        Mesh(Mesh&& other) = default;
        Mesh& operator = (Mesh&& other) = default;

        virtual ~Mesh()
        {
            BASED_TRACE("Destroying mesh {}", mMeshSource);
        }
        virtual void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material);
        virtual void Draw(scene::Transform& transform, std::shared_ptr<Material> material);

        bool IsFileMesh() const { return !mMeshSource.empty(); }
        std::string GetMeshSource() const { return mMeshSource; }

        void SetMeshName(const std::string& name) { mMeshName = name; }
        std::string GetMeshName() const { return mMeshName; }

	    std::shared_ptr<VertexArray> GetVertexArray() { return mVA; }

        core::UUID GetUUID() const { return mUUID; }
    protected:
        //  render data
        core::UUID mUUID;
        std::shared_ptr<VertexArray> mVA; // Mesh data is kept on the CPU here
        std::string mMeshSource;
        std::string mMeshName;

        friend class SceneSerializer;
	};

    class InstancedMesh : public Mesh
    {
    public:
        InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
	        : Mesh(vertices, indices), mInstanceCount(0), mIsDirty(true) {}
        InstancedMesh(std::shared_ptr<VertexArray> va, int count)
    		: Mesh(va), mInstanceCount(count), mIsDirty(true) {}
        ~InstancedMesh() override = default;

        void SetInstanceTransform(int index, const scene::Transform& transform);
        int AddInstance(const scene::Transform& transform, bool markDirty = true);
        void AddInstances(const std::vector<scene::Transform>& transforms);
        bool RemoveInstance(int index, bool markDirty = true);
        bool RemoveInstances(std::vector<int> indices);
        void ClearInstances();
        std::vector<scene::Transform> GetInstanceTransforms() const { return mInstanceTransforms; }
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material) override;
        void Draw(scene::Transform&, std::shared_ptr<Material> material) override;

        int GetInstanceCount() const { return mInstanceCount; }
    private:
        int mInstanceCount;
        std::vector<scene::Transform> mInstanceTransforms;
        bool mIsDirty;

        void RegenVertexArray();
    };
}
