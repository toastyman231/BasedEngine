#include "scene/entity.h"

#include "based/core/profiler.h"

namespace based::scene
{
	Entity::Entity() : Entity(Engine::Instance().GetApp().GetCurrentScene()->GetRegistry()) {}

	Entity::Entity(entt::registry& registry)
		: mRegistry(registry), mIsEnabled(true)
	{
		PROFILE_FUNCTION();
		mEntity = mRegistry.create();
		mEntityName = "New Entity";
		AddComponent<Transform>();
		AddComponent<EntityReference>(this);
		SetActive(true);
	}

	Entity::~Entity()
	{
		if (mRegistry.valid(mEntity)) DestroyEntity(this);
	}

	void Entity::DestroyEntity(Entity* ent)
	{
		if (!ent) return;
		ent->OnDestroy();

		ent->mRegistry.destroy(ent->mEntity);
		delete(ent);
	}

	void Entity::SetActive(bool active)
	{
		if (active) mIsEnabled = true;
		else mIsEnabled = false;
		if (active) { AddComponent<scene::Enabled>(); OnEnable(); }
		else { RemoveComponent<scene::Enabled>(); OnDisable(); }
	}

	void Entity::SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		PROFILE_FUNCTION();
		SetPosition(pos);
		SetRotation(rot);
		SetScale(scale);
	}

	void Entity::SetPosition(glm::vec3 pos)
	{
		Transform transform = GetComponent<scene::Transform>();
		AddOrReplaceComponent<scene::Transform>(pos, transform.LocalPosition,
			transform.Rotation, transform.LocalRotation,
			transform.Scale, transform.LocalScale);

		for (auto const& child : Children)
		{
			glm::vec3 newAbsolutePosition = pos + child->GetComponent<scene::Transform>().LocalPosition;
			child->SetPosition(newAbsolutePosition);
		}
	}

	void Entity::SetRotation(glm::vec3 rot)
	{
		Transform transform = GetComponent<scene::Transform>();

		AddOrReplaceComponent<scene::Transform>(transform.Position, transform.LocalPosition,
			rot, transform.LocalRotation,
			transform.Scale, transform.LocalScale);

		for (auto const& child : Children)
		{
			glm::vec3 newAbsoluteRotation = rot + child->GetComponent<scene::Transform>().LocalRotation;
			child->SetRotation(newAbsoluteRotation);
		}
	}

	void Entity::SetScale(glm::vec3 scale)
	{
		Transform transform = GetComponent<scene::Transform>();

		AddOrReplaceComponent<scene::Transform>(transform.Position, transform.LocalPosition,
			transform.Rotation, transform.LocalRotation,
			scale, transform.LocalScale);

		for (auto const& child : Children)
		{
			glm::vec3 newAbsoluteScale = scale * child->GetComponent<scene::Transform>().LocalScale;
			child->SetScale(newAbsoluteScale);
		}
	}

	void Entity::SetLocalTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		PROFILE_FUNCTION();
		SetLocalPosition(pos);
		SetLocalRotation(rot);
		SetLocalScale(scale);
	}

	void Entity::SetLocalPosition(glm::vec3 pos)
	{
		Transform transform = GetComponent<scene::Transform>();
		glm::vec3 newPosition;

		if (Parent)
		{
			newPosition = Parent->GetComponent<scene::Transform>().Position + pos;
		} else
		{
			newPosition = pos;
			pos = glm::vec3(0.f);
		}

		AddOrReplaceComponent<scene::Transform>(newPosition, pos,
			transform.Rotation, transform.LocalRotation,
			transform.Scale, transform.LocalScale);

		for (auto const& child : Children)
		{
			glm::vec3 newAbsolutePosition = newPosition + child->GetComponent<scene::Transform>().LocalPosition;
			child->SetPosition(newAbsolutePosition);
		}
	}

	void Entity::SetLocalRotation(glm::vec3 rot)
	{
		Transform transform = GetComponent<scene::Transform>();
		glm::vec3 newRotation;

		if (Parent)
		{
			newRotation = Parent->GetComponent<scene::Transform>().Rotation + rot;
		} else
		{
			newRotation = rot;
			rot = glm::vec3(0.f);
		}

		AddOrReplaceComponent<scene::Transform>(transform.Position, transform.LocalPosition,
			newRotation, rot,
			transform.Scale, transform.LocalScale);

		for (auto const& child : Children)
		{
			glm::vec3 newAbsoluteRotation = newRotation + child->GetComponent<scene::Transform>().LocalRotation;
			child->SetRotation(newAbsoluteRotation);
		}
	}

	void Entity::SetLocalScale(glm::vec3 scale)
	{
		Transform transform = GetComponent<scene::Transform>();

		glm::vec3 newScale;

		if (Parent)
		{
			newScale = Parent->GetComponent<scene::Transform>().Scale * scale;
		}
		else
		{
			newScale = scale;
			scale = glm::vec3(1.f);
		}

		AddOrReplaceComponent<scene::Transform>(transform.Position, transform.LocalPosition,
			transform.Rotation, transform.LocalRotation,
			newScale, scale);

		for (auto const& child : Children)
		{
			glm::vec3 newAbsoluteScale = newScale * child->GetComponent<scene::Transform>().LocalScale;
			child->SetScale(newAbsoluteScale);
		}
	}
}
