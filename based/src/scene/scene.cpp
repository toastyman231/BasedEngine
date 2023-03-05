#include "engine.h"

#include "based/scene/scene.h"

#include "external/glm/ext/matrix_transform.hpp"
#include "scene/components.h"
#include "ui/textentity.h"

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
		const auto view = mRegistry.view<Transform, SpriteRenderer>();

		for (const auto entity : view)
		{
			if (!mRegistry.all_of<scene::Enabled>(entity)) continue;

			// TODO: maybe move this to an Entity class?
			// TODO: figure out rotation :(
			const std::shared_ptr<graphics::VertexArray> va = mRegistry.get<SpriteRenderer>(entity).vertexArray;
			const std::shared_ptr<graphics::Material> mat = mRegistry.get<SpriteRenderer>(entity).material;
			auto model = glm::mat4(1.f);
			model = glm::translate(model, mRegistry.get<Transform>(entity).Position);
			model = glm::scale(model, mRegistry.get<Transform>(entity).Scale);
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, mat, model));
		}

		const auto textView = mRegistry.view<Transform, TextRenderer>();

		for (const auto entity : textView)
		{
			ui::TextEntity* text = mRegistry.get<TextRenderer>(entity).text;
			ui::TextEntity::DrawFont(text);
		}

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}

	void Scene::UpdateScene(float deltaTime) const
	{
		const auto entityView = mRegistry.view<EntityReference>();

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
