#include "Generators.h"

#include "based/graphics/defaultassetlibraries.h"
#include "based/graphics/mesh.h"

std::shared_ptr<based::graphics::Mesh> GeneratePlane(int xSize, int ySize)
{
	std::vector<based::graphics::Vertex> vertices;
	std::vector<unsigned int> indices(xSize * ySize * 6);

	for (int i = 0, y = 0; y <= ySize; y++) {
		for (int x = 0; x <= xSize; x++, i++) {
			based::graphics::Vertex vertex;
			vertex.Position = glm::vec3(x, 0, y);
			vertex.Normal = glm::vec3(0, 1, 0);
			vertex.TexCoords = glm::vec2(static_cast<float>(x) / static_cast<float>(xSize), 
				static_cast<float>(y) / static_cast<float>(ySize));
			vertex.Tangent = glm::vec3(0, 0, 1);
			vertex.Bitangent = glm::vec3(1, 0, 0);
			vertices.emplace_back(vertex);
		}
	}

	for (int ti = 0, vi = 0, y = 0; y < ySize; y++, vi++) {
		for (int x = 0; x < xSize; x++, ti += 6, vi++) {
			indices[ti] = vi;
			indices[ti + 3] = indices[ti + 2] = vi + 1;
			indices[ti + 4] = indices[ti + 1] = vi + xSize + 1;
			indices[ti + 5] = vi + xSize + 2;
		}
	}

	return based::graphics::Mesh::CreateMesh(vertices, indices,
		DEFAULT_MESH_LIB, "Plane");
}
