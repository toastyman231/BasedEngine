#pragma once

#include "components.h"
#include "based/engine.h"
#include "based/app.h"

#include "based/graphics/defaultassetlibraries.h"

namespace based::scene
{
	class Entity : public std::enable_shared_from_this<Entity>
	{
	public:
		Entity();
		Entity(entt::registry& registry);
		Entity(entt::entity handle, entt::registry& registry);
		Entity& operator= (const Entity& other);
		virtual ~Entity();

		static std::shared_ptr<Entity> CreateEntity(const std::string& name);

		static std::shared_ptr<Entity> CreateEntityWithUUID(const std::string& name, core::UUID uuid);

		template<typename Type, typename... Args>
		static inline std::shared_ptr<Type> CreateEntity(const std::string& name = "New Entity", 
			glm::vec3 pos = glm::vec3(0.f), glm::vec3 rot = glm::vec3(0.f), 
			glm::vec3 scale = glm::vec3(1.f), Args &&...args)
		{
			auto newEntity = std::make_shared<Type>(args...);
			newEntity->SetEntityName(name);
			newEntity->template AddComponent<EntityReference>(newEntity);
			newEntity->template AddComponent<IDComponent>(core::UUID());

			newEntity->SetTransform(pos, rot, scale);
			newEntity->Initialize();

			return newEntity;
		}

		static void DestroyEntity(std::shared_ptr<Entity> ent);

		// TODO: Do some testing to make sure adding/removing works even if you already have component or don't have component
		template<typename Type, typename ...Args>
		inline void AddComponent(Args &&... args)
		{
			if (HasComponent<Type>()) return;

			if constexpr (std::is_same_v<Type, EntityReference>)
			{
				mRegistry.emplace<Type>(mEntity, shared_from_this());
			}
			else if constexpr (std::is_same_v<Type, RigidbodyComponent>)
			{
				auto component = mRegistry.emplace<RigidbodyComponent>(mEntity, args...);
				component.RegisterBody(GetEntityHandle());
			}
			else mRegistry.emplace<Type>(mEntity, args...);
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
		inline Type& GetComponent()
		{
			return mRegistry.get<Type>(mEntity);
		}

		inline core::UUID GetUUID() { return GetComponent<IDComponent>().uuid; }

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

		entt::registry& GetRegistry() { return mRegistry; }

		void SetActive(bool active);

		scene::Transform& GetTransform();
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

		void SetParent(const std::shared_ptr<Entity>& parentEntity, bool keepRelativeTransform = true)
		{
			auto parent = Parent.lock();

			if (parentEntity == nullptr)
			{
				if (parent)
				{
					// INTENTIONAL COPY - caching global position
					auto transform = GetTransform();
					Parent = std::weak_ptr<Entity>();
					GetTransform().Parent = nullptr;
					GetTransform().SetGlobalTransform(transform.Position(), transform.Rotation(), transform.Scale());
				}
				else Parent = std::weak_ptr<Entity>();
			}

			if (parent && parent != parentEntity)
			{
				parent->RemoveChild(shared_from_this());
			}

			if (parentEntity == nullptr || 
				parentEntity == parent || 
				parentEntity->GetEntityHandle() == mEntity) 
				return;

			Parent = parentEntity;
			parentEntity->Children.emplace_back(shared_from_this());
			if (keepRelativeTransform)
			{
				mRegistry.patch<Transform>(mEntity,
					[parentEntity](Transform& tr)
					{
						tr.SetLocalTransform(
							tr.Position() - parentEntity->GetTransform().Position(),
							tr.EulerAngles() - parentEntity->GetTransform().EulerAngles(),
							tr.Scale() / parentEntity->GetTransform().Scale()
						);
					});
			} else
			{
				SetLocalTransform({ 0, 0, 0 }, { 0, 0, 0 }, { 1, 1, 1 });
			}

			mRegistry.patch<Transform>(mEntity,
				[parentEntity](Transform& tr)
				{
					tr.Parent = &parentEntity->GetComponent<Transform>();
				});
		}

		bool RemoveChild(const std::shared_ptr<Entity>& childEntity)
		{
			auto it = std::find_if(Children.begin(), Children.end(),
				[childEntity](std::weak_ptr<Entity> e)
				{
					return e.lock() == childEntity;
				});

			if (it != Children.end())
			{
				childEntity->GetComponent<Transform>().Parent = nullptr;
				Children.erase(it);
				return true;
			}
			else return false;
		}

		std::weak_ptr<Entity> Parent;

		std::vector<std::weak_ptr<Entity>> Children;

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
