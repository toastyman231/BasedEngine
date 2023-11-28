#include "engine.h"

#include "based/scene/scene.h"

#include "external/glm/ext/matrix_transform.hpp"
#include "graphics/model.h"
#include "scene/components.h"
#include "ui/textentity.h"
#include "graphics/sprite.h"

#include "based/core/profiler.h"

namespace based::scene
{
	void Scene::InitializeScene() const
	{
		PROFILE_FUNCTION();
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
		PROFILE_FUNCTION();
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, mActiveCamera));
		const auto view = mRegistry.view<Enabled, Transform, SpriteRenderer>();
		std::vector<graphics::Sprite*> sprites;

		// TODO: Find a better way to sort entities
		for (const auto entity : view)
		{
			sprites.push_back(mRegistry.get<SpriteRenderer>(entity).sprite);
		}

		std::sort(sprites.begin(), sprites.end(), [](const auto& ent1, const auto& ent2)
			{
				return static_cast<graphics::Sprite*>(ent1)->GetSortOrder() < static_cast<graphics::Sprite*>(ent2)->GetSortOrder();
			});

		for (const auto sprite : sprites)
		{
			graphics::Sprite::DrawSprite(sprite);
		}

		const auto modelView = mRegistry.view<Enabled, Transform, ModelRenderer>();

		for (const auto entity : modelView)
		{
			scene::Transform trans = mRegistry.get<Transform>(entity);
			mRegistry.get<ModelRenderer>(entity).model->Draw(trans.Position, trans.Rotation, trans.Scale);
		}

		const auto meshView = mRegistry.view<Enabled, Transform, MeshRenderer>();

		for (const auto entity : meshView)
		{
			scene::Transform trans = mRegistry.get<Transform>(entity);
			mRegistry.get<MeshRenderer>(entity).mesh->Draw(trans.Position, trans.Rotation, trans.Scale);
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
		PROFILE_FUNCTION();
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
		PROFILE_FUNCTION();
		const auto entityView = mRegistry.view<EntityReference>();

		for (const auto entity : entityView)
		{
			scene::Entity* entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (!entityPtr->IsActive()) continue;
			entityPtr->Shutdown();
		}
	}
}
