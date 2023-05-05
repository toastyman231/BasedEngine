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

// Taken from learnopengl.com : https://learnopengl.com/Model-Loading/Mesh

namespace based::graphics
{
	class Shader;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
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
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::shared_ptr<Material> material);
        void Draw(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
    private:
        //  render data
        //unsigned int VAO, VBO, EBO;
        std::shared_ptr<VertexArray> mVA;

        void SetupMesh();
	};
}
