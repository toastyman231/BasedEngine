#include "scene/entity.h"

namespace based::scene
{
	Entity::Entity(entt::registry& registry)
		: mRegistry(registry), mIsEnabled(true)
	{
		mEntity = mRegistry.create();
		mEntityName = "New Entity";
	}

	void Entity::DestroyEntity(Entity ent)
	{
		ent.mRegistry.destroy(ent.mEntity);
		delete(&ent);
	}

	void Entity::SetActive(bool active)
	{
		if (active) mIsEnabled = true;
		else mIsEnabled = false;
		//BASED_TRACE("New active value: {}", mIsEnabled);
		if (active) AddComponent<scene::Enabled>();
		else RemoveComponent<scene::Enabled>();
	}
}