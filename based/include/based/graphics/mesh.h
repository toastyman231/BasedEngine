#pragma once
#include <memory>
#include <vector>

#include "based/engine.h"
#include "material.h"
#include "texture.h"
#include "vertex.h"
#include "external/glm/vec2.hpp"
#include "external/glm/vec3.hpp"
#include "external/glm/gtx/transform.hpp"
#include "based/scene/scene.h"
#include "based/scene/components.h"

// Taken from learnopengl.com : https://learnopengl.com/Model-Loading/Mesh

namespace based::graphics
{
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

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        Mesh(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat);
        ~Mesh() { BASED_TRACE("Destroying mesh!"); }
        virtual void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material);
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
    protected:
        //  render data
        //unsigned int VAO, VBO, EBO;
        std::shared_ptr<VertexArray> mVA;
        Mesh() = default;

        void SetupMesh(bool upload = true);
	};

    class InstancedMesh : public Mesh
    {
    public:
        InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	        : Mesh(vertices, indices, textures), mInstanceCount(0), mIsDirty(true) {}
        InstancedMesh(std::shared_ptr<VertexArray> va, std::shared_ptr<Material> mat, int count)
    		: Mesh(va, mat), mInstanceCount(count), mIsDirty(true) {}

        void SetInstanceTransform(int index, const scene::Transform& transform);
        int AddInstance(scene::Transform transform);
        void AddInstances(const std::vector<scene::Transform>& transforms);
        bool RemoveInstance(int index);
        bool RemoveInstances(std::vector<int> indices);
        void ClearInstances();
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material) override;

        int GetInstanceCount() const { return mInstanceCount; }
    private:
        int mInstanceCount;
        std::vector<scene::Transform> mInstanceTransforms;
        bool mIsDirty;

        void RegenVertexArray();
    };
}
