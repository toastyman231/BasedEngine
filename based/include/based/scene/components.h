#pragma once
#include <memory>

#include "external/glm/glm.hpp"
#include "based/graphics/material.h"
#include "based/graphics/vertex.h"

namespace based::scene
{
	struct Position
	{
		float x;
		float y;
		float z;

		Position() = default;
		Position(const Position&) = default;
		explicit Position(const glm::vec3& pos) : x(pos.x), y(pos.y), z(pos.z) {}
		Position(float xPos, float yPos, float zPos) : x(xPos), y(yPos), z(zPos) {}
	};

	struct Scale
	{
		float x;
		float y;
		float z;

		Scale() = default;
		Scale(const Scale&) = default;
		explicit Scale(const glm::vec3& scale) : x(scale.x), y(scale.y), z(scale.z) {}
		Scale(float scaleX, float scaleY, float scaleZ) : x(scaleX), y(scaleY), z(scaleZ) {}
	};

	struct Velocity
	{
		float dx;
		float dy;

		Velocity() = default;
		Velocity(float xVel, float yVel) : dx(xVel), dy(yVel) {}
		Velocity(const Velocity&) = default;
		Velocity(const glm::vec2& vel) : dx(vel.x), dy(vel.y) {}
	};

	struct SpriteRenderer
	{
		std::shared_ptr <graphics::VertexArray> vertexArray;
		//std::shared_ptr<graphics::Texture> texture;
		std::shared_ptr<graphics::Material> material;

		SpriteRenderer() = default;
		SpriteRenderer(const std::weak_ptr<graphics::VertexArray> va,
			//const std::weak_ptr<graphics::Texture> tex,
			const std::weak_ptr<graphics::Material> mat) : vertexArray(va), /*texture(tex),*/ material(mat) {}
		SpriteRenderer(const SpriteRenderer&) = default;
	};
}
