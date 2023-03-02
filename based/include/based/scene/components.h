#pragma once
#include <memory>

#include "external/glm/glm.hpp"
#include "based/graphics/material.h"
#include "based/graphics/vertex.h"

namespace based::ui
{
	class TextEntity;
}

namespace based::scene
{
	class Entity;

	struct EntityReference
	{
		Entity* entity;

		EntityReference() = default;
		EntityReference(const EntityReference&) = default;
		EntityReference(Entity* entityPtr) : entity(entityPtr) {}
	};

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
		std::shared_ptr<graphics::Material> material;

		SpriteRenderer() = default;
		SpriteRenderer(const std::weak_ptr<graphics::VertexArray> va,
			const std::weak_ptr<graphics::Material> mat) : vertexArray(va), material(mat) {}
		SpriteRenderer(const SpriteRenderer&) = default;
	};

	struct TextRenderer
	{
		ui::TextEntity* text;
		//std::shared_ptr<ui::TextEntity> text;

		TextRenderer() = default;
		TextRenderer(ui::TextEntity* textPtr) : text(textPtr) {}
		//TextRenderer(const std::weak_ptr<ui::TextEntity> textEntity) : text(textEntity) {}
	};

	struct Enabled
	{
		Enabled() = default;
	};
}
