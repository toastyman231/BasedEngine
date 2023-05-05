#pragma once

#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

namespace based::ui
{
	class TextEntity;
}

namespace based::graphics
{
	class Mesh;
	class Model;
	class Sprite;
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

		glm::vec3 LocalPosition = { 0.0f, 0.0f, 0.0f };
		glm::vec3 LocalRotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 LocalScale = { 1.0f, 1.0f, 1.0f };

		Transform() = default;
		Transform(const Transform&) = default;
		Transform(const glm::vec3& pos) : Position(pos) {}
		Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, bool local = false)
		{
			if (local)
			{
				LocalPosition = pos;
				LocalRotation = rot;
				LocalScale = scale;
			} else
			{
				Position = pos;
				Rotation = rot;
				Scale = scale;
			}
		}
		Transform(const glm::vec3& pos, const glm::vec3& localPos, 
			const glm::vec3& rot, const glm::vec3& localRot,
			const glm::vec3& scale, const glm::vec3& localScale)
		{
			LocalPosition = localPos;
			LocalRotation = localRot;
			LocalScale = localScale;
			Position = pos;
			Rotation = rot;
			Scale = scale;
		}
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
		graphics::Sprite* sprite;

		SpriteRenderer() = default;
		SpriteRenderer(graphics::Sprite* spritePtr) : sprite(spritePtr) {}
		SpriteRenderer(const SpriteRenderer&) = default;
	};

	struct ModelRenderer
	{
		graphics::Model* model;

		ModelRenderer() = default;
		ModelRenderer(graphics::Model* modelPtr) : model(modelPtr) {}
		ModelRenderer(const ModelRenderer&) = default;
	};

	struct MeshRenderer
	{
		graphics::Mesh* mesh;
		graphics::Model* model;

		MeshRenderer() = default;
		MeshRenderer(graphics::Mesh* meshPtr) : mesh(meshPtr) {}
		MeshRenderer(graphics::Mesh* meshPtr, graphics::Model* modelPtr) : mesh(meshPtr), model(modelPtr) {}
		MeshRenderer(const MeshRenderer&) = default;
	};

	struct TextRenderer
	{
		ui::TextEntity* text;

		TextRenderer() = default;
		TextRenderer(ui::TextEntity* textPtr) : text(textPtr) {}
	};

	struct Enabled
	{
		Enabled() = default;
	};
}
