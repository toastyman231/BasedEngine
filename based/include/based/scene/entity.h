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

		template<typename... Args>
		static inline Entity* CreateEntity(Args &&...args)
		{
			Entity* newEntity = new Entity(Engine::Instance().GetApp().GetCurrentScene()->GetRegistry());

			// Black magic iterator over the given args
			([&]
				{
					AddComponent(newEntity, args);
				} (), ...);

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

		void SetActive(bool active);

		virtual void Initialize() {};
		virtual void Update() {}
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