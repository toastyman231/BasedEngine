#pragma once

#include "external/entt/entt.hpp"

#include "components.h"
#include "based/engine.h"
#include "based/app.h"

#include <string>

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

		scene::Transform& GetTransform() { return GetComponent<scene::Transform>(); }
		void SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		void SetPosition(glm::vec3 pos);
		void SetRotation(glm::vec3 rot);
		void SetScale(glm::vec3 scale);

		virtual void Initialize() {}
		virtual void Update(float deltaTime) {}
		virtual void Shutdown() {}

		virtual void OnStart() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}

	private:
		entt::registry& mRegistry;
		entt::entity mEntity;
		std::string mEntityName;

		bool mIsEnabled;
	};
}
