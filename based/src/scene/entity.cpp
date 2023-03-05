#include "scene/entity.h"

namespace based::scene
{
	Entity::Entity() : Entity(Engine::Instance().GetApp().GetCurrentScene()->GetRegistry()) {}

	Entity::Entity(entt::registry& registry)
		: mRegistry(registry), mIsEnabled(true)
	{
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
		//BASED_TRACE("New active value: {}", mIsEnabled);
		if (active) { AddComponent<scene::Enabled>(); OnEnable(); }
		else { RemoveComponent<scene::Enabled>(); OnDisable(); }
	}

	void Entity::SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		AddOrReplaceComponent<scene::Transform>(pos, rot, scale);
	}

	void Entity::SetPosition(glm::vec3 pos)
	{
		Transform transform = GetComponent<scene::Transform>();
		AddOrReplaceComponent<scene::Transform>(pos, transform.Rotation, transform.Scale);
	}

	void Entity::SetRotation(glm::vec3 rot)
	{
		Transform transform = GetComponent<scene::Transform>();
		AddOrReplaceComponent<scene::Transform>(transform.Position, rot, transform.Scale);
	}

	void Entity::SetScale(glm::vec3 scale)
	{
		Transform transform = GetComponent<scene::Transform>();
		AddOrReplaceComponent<scene::Transform>(transform.Position, transform.Rotation, scale);
	}
}
