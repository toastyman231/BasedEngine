#include "engine.h"

#include "based/scene/scene.h"

#include "external/glm/ext/matrix_transform.hpp"
#include "scene/components.h"

namespace based::scene
{
	void Scene::RenderScene() const
	{
		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, mActiveCamera));
		const auto view = mRegistry.view<Transform, SpriteRenderer>();

		for (const auto entity : view)
		{
			//if (!mRegistry.all_of<scene::Enabled>(entity)) continue;

			// TODO: maybe move this to an Entity class?
			// TODO: figure out rotation :(
			const std::shared_ptr<graphics::VertexArray> va = mRegistry.get<SpriteRenderer>(entity).vertexArray;
			const std::shared_ptr<graphics::Material> mat = mRegistry.get<SpriteRenderer>(entity).material;
			auto model = glm::mat4(1.f);
			model = glm::translate(model, mRegistry.get<Transform>(entity).Position);
			model = glm::scale(model, mRegistry.get<Transform>(entity).Scale);
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, mat, model));
		}

		Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));

		// TODO: find a better way to render all this shit
		//Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PushCamera, mActiveCamera));
		/*const auto textView = mRegistry.view<Transform, TextRenderer>();

		for (const auto entity : textView)
		{
			const auto color = mRegistry.get<TextRenderer>(entity).text->GetColor();
			const auto transform = mRegistry.get<Transform>(entity);
			mRegistry.get<TextRenderer>(entity).text->RenderText(transform.Position, color, transform.Scale.x);
		}*/

		//Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(PopCamera));
	}
}
