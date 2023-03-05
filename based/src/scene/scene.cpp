#include "engine.h"

#include "based/scene/scene.h"

#include "external/glm/ext/matrix_transform.hpp"
#include "scene/components.h"
#include "ui/textentity.h"
#include "graphics/sprite.h"

namespace based::scene
{
	void Scene::InitializeScene() const
	{
		const auto entityView = mRegistry.view<EntityReference>();

		for (const auto entity : entityView)
		{
			scene::Entity* entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (!entityPtr->IsActive()) continue;
			entityPtr->Initialize();
		}
	}

	void Scene::RenderScene() const
	{
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, mActiveCamera));
		const auto view = mRegistry.view<Enabled, Transform, SpriteRenderer>();

		for (const auto entity : view)
		{
			graphics::Sprite* sprite = mRegistry.get<SpriteRenderer>(entity).sprite;
			graphics::Sprite::DrawSprite(sprite);
		}

		const auto textView = mRegistry.view<Enabled, Transform, TextRenderer>();

		for (const auto entity : textView)
		{
			ui::TextEntity* text = mRegistry.get<TextRenderer>(entity).text;
			ui::TextEntity::DrawFont(text);
		}

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void Scene::UpdateScene(float deltaTime) const
	{
		const auto entityView = mRegistry.view<Enabled, EntityReference>();

		for (const auto entity : entityView)
		{
			scene::Entity* entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (entityPtr)
			{
				if (!entityPtr->IsActive()) continue;
				entityPtr->Update(deltaTime);
			}
		}
	}

	void Scene::ShutdownScene() const
	{
		const auto entityView = mRegistry.view<EntityReference>();

		for (const auto entity : entityView)
		{
			scene::Entity* entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (!entityPtr->IsActive()) continue;
			entityPtr->Shutdown();
			//delete(entityPtr);
		}
	}
}
