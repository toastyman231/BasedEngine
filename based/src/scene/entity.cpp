#include "scene/entity.h"

namespace based::scene
{
	Entity::Entity(entt::registry& registry)
		: mRegistry(registry)
	{
		mEntity = mRegistry.create();
		mEntityName = "New Entity "; //+ mEntityCount++;
	}

	void Entity::DestroyEntity(Entity ent)
	{
		ent.mRegistry.destroy(ent.mEntity);
		delete(&ent);
	}
}