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
			AddComponent<scene::Transform>(newEntity);

			// Black magic iterator over the given args
			([&]
				{
					AddComponent(newEntity, args);
				} (), ...);

			return newEntity;
		}

		static void DestroyEntity(Entity ent);

		// TODO: Maybe make this not static?
		template<typename Type, typename ...Args>
		static inline void AddComponent(Entity entity, Args &&... args)
		{
			entity.mRegistry.emplace<Type>(entity.mEntity, args...);
		}

		template<typename Type>
		inline Type GetComponent()
		{
			return mRegistry.get<Type>(mEntity);
		}

	private:
		entt::registry& mRegistry;
		entt::entity mEntity;
		std::string mEntityName;

		//static const int mEntityCount = 0;
	};
}