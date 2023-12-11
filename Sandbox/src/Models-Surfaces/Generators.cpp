#include "Generators.h"

based::graphics::Mesh* GeneratePlane(int xSize, int ySize)
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

	return new based::graphics::Mesh(vertices, indices, {});
}

based::graphics::Mesh* GenerateGrassBlade(glm::vec3 scale)
{
	std::vector<based::graphics::Vertex> vertices;
	std::vector<unsigned int> indices;

	vertices.insert(vertices.end(), {
			based::graphics::Vertex { glm::vec3(-0.5f, 0.f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(0.f, 0.f)},
			based::graphics::Vertex { glm::vec3(0.5f, 0.f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(1.f, 0.f)},
			based::graphics::Vertex { glm::vec3(0.5f, 0.5f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(1.f, 0.33f)},
			based::graphics::Vertex { glm::vec3(-0.5f, 0.5f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(0.f, 0.33f)},
			based::graphics::Vertex { glm::vec3(0.5f, 1.0f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(1.f, 0.66f)},
			based::graphics::Vertex { glm::vec3(-0.5f, 1.0f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(0.f, 0.66f)},
			based::graphics::Vertex { glm::vec3(0.0f, 1.5f, 0.f) * scale,
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec2(0.5f, 1.f)}
		});
	indices.insert(indices.end(), {
			0, 1, 2,    0, 2, 3,
			3, 2, 4,    3, 4, 5,
			5, 4, 6
		});

	return new based::graphics::Mesh(vertices, indices, {});
}
