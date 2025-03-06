#include "pch.h"
#include "scene/entity.h"

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
		SetActive(true);
	}

	Entity::Entity(entt::entity handle, entt::registry& registry)
		: mRegistry(registry), mEntity(handle), mIsEnabled(true)
	{
	}

	Entity& Entity::operator=(const Entity& other)
	{
		if (this != &other)
		{
			mRegistry = std::move(other.mRegistry);
			mEntity = other.mEntity;
			mEntityName = other.mEntityName;
			mIsEnabled = other.mIsEnabled;
		}
		return *this;
	}

	Entity::~Entity()
	{
		BASED_TRACE("Destroying entity {}", GetEntityName());
		if (mRegistry.valid(mEntity)) mRegistry.destroy(mEntity);
	}

	std::shared_ptr<Entity> Entity::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(name, core::UUID());
	}

	std::shared_ptr<Entity> Entity::CreateEntityWithUUID(const std::string& name, core::UUID uuid)
	{
		auto newEntity = std::make_shared<Entity>();
		newEntity->SetEntityName(name);
		newEntity->AddComponent<EntityReference>(newEntity);
		newEntity->AddComponent<IDComponent>(uuid);

		newEntity->Initialize();

		return newEntity;
	}

	void Entity::DestroyEntity(std::shared_ptr<Entity> ent)
	{
		if (!ent) return;
		ent->OnDestroy();

		if (auto parent = ent->Parent.lock())
			parent->RemoveChild(ent);

		auto scene = Engine::Instance().GetApp().GetCurrentScene();
		scene->GetEntityStorage().Delete(ent->GetEntityName());

		if (ent->mRegistry.valid(ent->mEntity)) 
			ent->mRegistry.destroy(ent->mEntity);
		ent.reset();
	}

	void Entity::SetActive(bool active)
	{
		if (active) mIsEnabled = true;
		else mIsEnabled = false;
		if (active) { AddComponent<scene::Enabled>(); OnEnable(); }
		else { RemoveComponent<scene::Enabled>(); OnDisable(); }
	}

	Transform& Entity::GetTransform()
	{
		return GetComponent<Transform>();
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
		Transform& transform = GetComponent<scene::Transform>();

		transform.SetGlobalTransform(pos, transform.Rotation(), transform.Scale());
	}

	void Entity::SetRotation(glm::vec3 rot)
	{
		Transform& transform = GetComponent<scene::Transform>();

		transform.SetGlobalTransform(transform.Position(), rot, transform.Scale());
	}

	void Entity::SetScale(glm::vec3 scale)
	{
		Transform& transform = GetComponent<scene::Transform>();

		transform.SetGlobalTransform(transform.Position(), transform.Rotation(), scale);
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
		Transform& transform = GetComponent<Transform>();

		transform.SetLocalTransform(pos, transform.LocalRotation(), transform.LocalScale());
	}

	void Entity::SetLocalRotation(glm::vec3 rot)
	{
		Transform& transform = GetComponent<Transform>();

		transform.SetLocalTransform(transform.LocalPosition(), rot, transform.LocalScale());
	}

	void Entity::SetLocalScale(glm::vec3 scale)
	{
		Transform& transform = GetComponent<Transform>();

		transform.SetLocalTransform(transform.LocalPosition(), transform.LocalRotation(), scale);
	}
}
