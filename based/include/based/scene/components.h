#pragma once
#include <memory>

#include "external/glm/glm.hpp"
#include "based/graphics/material.h"
#include "based/graphics/vertex.h"
#include "based/ui/textentity.h"

namespace based::scene
{
	struct Transform
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		Transform() = default;
		Transform(const Transform&) = default;
		Transform(const glm::vec3& pos) : Position(pos) {}
		Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
		: Position(pos), Rotation(rot), Scale(scale) {}
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

	struct TextRenderer
	{
		//std::shared_ptr<ui::TextEntity> text;

		TextRenderer() = default;
		//TextRenderer(const std::weak_ptr<ui::TextEntity> textEntity) : text(textEntity) {}
	};
}
