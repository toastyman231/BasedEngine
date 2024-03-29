#pragma once

#include "external/entt/entt.hpp"

#include "components.h"
#include "based/engine.h"
#include "based/app.h"

#include <string>

#include "based/log.h"

namespace based::scene
{
	class Entity
	{
	public:
		Entity();
		Entity(entt::registry& registry);
		virtual ~Entity();

		template<typename Type, typename... Args>
		static inline Type* CreateEntity(glm::vec3 pos = glm::vec3(0.f),
			glm::vec3 rot = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f), Args &&...args)
		{
			Type* newEntity = new Type(args...);

			newEntity->SetTransform(pos, rot, scale);
			newEntity->Initialize();

			return newEntity;
		}

		static void DestroyEntity(Entity* ent);

		// TODO: Do some testing to make sure adding/removing works even if you already have component or don't have component
		template<typename Type, typename ...Args>
		inline void AddComponent(Args &&... args)
		{
			if (HasComponent<Type>()) return;
			mRegistry.emplace<Type>(mEntity, args...);
		}

		template<typename Type, typename ...Args>
		inline void AddOrReplaceComponent(Args &&... args)
		{
			mRegistry.emplace_or_replace<Type>(mEntity, args...);
		}

		template<typename Type>
		inline void RemoveComponent() const
		{
			mRegistry.remove<Type>(mEntity);
		}

		template<typename Type>
		inline bool HasComponent() const
		{
			return mRegistry.all_of<Type>(mEntity);
		}

		template<typename Type>
		inline Type GetComponent()
		{
			return mRegistry.get<Type>(mEntity);
		}

		inline bool IsActive() const { return mIsEnabled; }

		inline entt::entity& GetEntityHandle() { return mEntity; }

		template<typename ...Types>
		static void EntityForEach(std::function<void(Entity*)> func)
		{
			const auto view = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().view<EntityReference, Types...>();

			for (const auto entity : view)
			{
				Entity* ent = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().get<EntityReference>(entity).entity;
				func(ent);
			}
		}

		void SetActive(bool active);

		scene::Transform GetTransform() { return GetComponent<scene::Transform>(); }
		std::string& GetEntityName() { return mEntityName; }
		void SetEntityName(const std::string& name) { mEntityName = name; }
		virtual void SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		virtual void SetPosition(glm::vec3 pos);
		virtual void SetRotation(glm::vec3 rot);
		virtual void SetScale(glm::vec3 scale);
		virtual void SetLocalTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		virtual void SetLocalPosition(glm::vec3 pos);
		virtual void SetLocalRotation(glm::vec3 rot);
		virtual void SetLocalScale(glm::vec3 scale);

		virtual void Initialize() {}
		virtual void Update(float deltaTime) {}
		virtual void Shutdown() {}

		virtual void OnStart() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}

		void SetParent(Entity* parentEntity)
		{
			if (parentEntity == nullptr && Parent)
			{
				Parent->RemoveChild(this);
				return;
			}

			Parent = parentEntity;
			Parent->Children.emplace_back(this);
		}

		bool RemoveChild(const Entity* childEntity)
		{
			int i = 0;
			for (const auto child : Children)
			{
				if (child->mEntity == childEntity->mEntity)
				{
					Children.erase(Children.begin() + i);//.remove(child);
					child->Parent = nullptr;
					return true;
				}
				i++;
			}

			return false;
		}

		Entity* Parent = nullptr;

		std::vector<Entity*> Children;

		bool operator==(const Entity& other) const
		{
			return mEntity == other.mEntity;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	private:
		entt::registry& mRegistry;
		entt::entity mEntity;
		std::string mEntityName;

		bool mIsEnabled;
	};
}
