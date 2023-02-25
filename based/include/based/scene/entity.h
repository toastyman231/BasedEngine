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
		Entity() = default;
		Entity(entt::registry& registry);
		~Entity() = default;

		template<typename... Args>
		static inline Entity CreateEntity(Args &&...args)
		{
			Entity newEntity = Entity(Engine::Instance().GetApp().GetCurrentScene()->GetRegistry());
			newEntity.AddComponent<scene::Transform>();
			newEntity.SetActive(true);

			// Black magic iterator over the given args
			([&]
				{
					AddComponent(newEntity, args);
				} (), ...);

			return newEntity;
		}

		static void DestroyEntity(Entity ent);

		// TODO: Do some testing to make sure adding/removing works even if you already have component or don't have component
		template<typename Type, typename ...Args>
		inline void AddComponent(Args &&... args)
		{
			// TODO: Should decide whether to make Replace it's own function
			if (HasComponent<Type>()) return;
			mRegistry.emplace_or_replace<Type>(mEntity, args...);
		}

		template<typename Type>
		inline void RemoveComponent()
		{
			mRegistry.remove<Type>(mEntity);
		}

		template<typename Type>
		inline bool HasComponent()
		{
			return mRegistry.all_of<Type>(mEntity);
		}

		template<typename Type>
		inline Type GetComponent()
		{
			return mRegistry.get<Type>(mEntity);
		}

		inline bool IsActive() { return mIsEnabled; }

		void SetActive(bool active);

	private:
		entt::registry& mRegistry;
		entt::entity mEntity;
		std::string mEntityName;

		bool mIsEnabled;
		//static const int mEntityCount = 0;
	};
}