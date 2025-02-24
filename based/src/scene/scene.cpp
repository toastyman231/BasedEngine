#include "pch.h"
#include "engine.h"

#include "based/scene/scene.h"

#include "based/scene/components.h"
#include "basedtime.h"
#include "animation/animator.h"
#include "graphics/model.h"
#include "ui/textentity.h"
#include "graphics/sprite.h"

namespace based::scene
{
	void Scene::InitializeScene() const
	{
		PROFILE_FUNCTION();
		const auto entityView = mRegistry.view<EntityReference>();

		for (const auto entity : entityView)
		{
			auto entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (auto ent = entityPtr.lock())
			{
				if (!ent->IsActive()) continue;
				ent->Initialize();
			}
		}
	}

	void Scene::AnimateScene() const
	{
		PROFILE_FUNCTION();
		const auto view = mRegistry.view<Enabled, AnimatorComponent, ModelRenderer>();

		for (const auto entity : view)
		{
			scene::AnimatorComponent anim = mRegistry.get<scene::AnimatorComponent>(entity);
			scene::ModelRenderer model = mRegistry.get<scene::ModelRenderer>(entity);

			auto m = model.model.lock();
			auto a = anim.animator.lock();
			if (m && a)
			{
				a->UpdateAnimation(a->GetTimeMode() == animation::TimeMode::Scaled ? 
					core::Time::DeltaTime() : 
					core::Time::UnscaledDeltaTime());

				auto transforms = a->GetFinalBoneMatrices();
				auto mat = m->GetMaterial();
				for (int i = 0; i < transforms.size(); ++i)
					mat->SetUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
			}
			
		}
	}

	void Scene::RenderScene() const
	{
		PROFILE_FUNCTION();
		
		const auto view = mRegistry.view<Enabled, Transform, SpriteRenderer>();
		std::vector<std::shared_ptr<graphics::Sprite>> sprites;
		sprites.reserve(view.size_hint());

		// TODO: Find a better way to sort entities
		for (const auto entity : view)
		{
			auto sprite = mRegistry.get<SpriteRenderer>(entity);
			if (auto spr = sprite.sprite.lock())
				sprites.emplace_back(spr);
		}

		std::sort(sprites.begin(), sprites.end(), [](const auto& ent1, const auto& ent2)
			{
				return static_cast<std::shared_ptr<graphics::Sprite>>(ent1)->GetSortOrder() < 
					static_cast<std::shared_ptr<graphics::Sprite>>(ent2)->GetSortOrder();
			});

		for (auto& sprite : sprites)
		{
			sprite->Draw();
		}

		const auto modelView = mRegistry.view<Enabled, Transform, ModelRenderer, EntityReference>();

		for (const auto entity : modelView)
		{
			scene::ModelRenderer renderer = mRegistry.get<ModelRenderer>(entity);
			scene::EntityReference ent = mRegistry.get<EntityReference>(entity);
			scene::Transform trans = ent.entity.lock()->GetTransform();//mRegistry.get<Transform>(entity);

			auto m = renderer.model.lock();
			auto e = ent.entity.lock();
			if (m)
			{
				m->Draw(trans);
			}
			else
			{
				BASED_WARN("Could not lock mesh for entity {}", e->GetEntityName());
			}
		}

		const auto meshView = mRegistry.view<Enabled, Transform, MeshRenderer, EntityReference>();

		for (const auto entity : meshView)
		{
			//scene::Transform trans = mRegistry.get<Transform>(entity);
			scene::MeshRenderer renderer = mRegistry.get<MeshRenderer>(entity);
			scene::EntityReference ent = mRegistry.get<EntityReference>(entity);
			scene::Transform trans = ent.entity.lock()->GetTransform();//mRegistry.get<Transform>(entity);

			if (auto m = renderer.mesh.lock())
			{
				m->Draw(trans);
			} else
			{
				BASED_WARN("Could not lock mesh for entity {}", ent.entity.lock()->GetEntityName());
			}
		}

		const auto textView = mRegistry.view<Enabled, Transform, TextRenderer>();

		for (const auto entity : textView)
		{
			ui::TextEntity* text = mRegistry.get<TextRenderer>(entity).text;
			ui::TextEntity::DrawFont(text);
		}
	}

	void Scene::UpdateScene(float deltaTime)
	{
		PROFILE_FUNCTION();

		const auto cameraView = mRegistry.view<Enabled, Transform, CameraComponent>();

		for (const auto& camera : cameraView)
		{
			auto camPtr = mRegistry.get<CameraComponent>(camera);
			auto trans = mRegistry.get<Transform>(camera);
			if (auto cam = camPtr.camera.lock())
			{
				cam->SetTransform(trans.Position, trans.Rotation, trans.Scale);
			}
		}

		const auto entityView = mRegistry.view<Enabled, EntityReference>();

		for (const auto& entity : entityView)
		{
			auto entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (auto ent = entityPtr.lock())
			{
				if (!ent->IsActive()) continue;
				ent->Update(deltaTime);
			}
		}

		const auto lightView = mRegistry.view<Enabled, PointLight, Transform>();

		for (const auto& entity : lightView)
		{
			auto& trans = mRegistry.get<Transform>(entity);

			mRegistry.patch<PointLight>(entity, 
				[this, trans](auto& l)
				{
					l.position = trans.Position;
				});
		}

		const auto dirView = mRegistry.view<Enabled, DirectionalLight, Transform>();

		for (const auto& entity : dirView)
		{
			auto& trans = mRegistry.get<Transform>(entity);

			mRegistry.patch<DirectionalLight>(entity,
				[this, trans](auto& l)
				{
					l.direction = trans.Rotation;
				});
		}

		AnimateScene();
	}

	void Scene::ShutdownScene() const
	{
		PROFILE_FUNCTION();

		const auto entityView = mRegistry.view<EntityReference>();

		for (const auto entity : entityView)
		{
			auto entityPtr = mRegistry.get<EntityReference>(entity).entity;
			if (auto ent = entityPtr.lock())
			{
				if (!ent->IsActive()) continue;
				ent->Shutdown();
			}
		}
	}

	void Scene::SetActiveCamera(std::shared_ptr<graphics::Camera> cam)
	{
		if (mActiveCamera) mActiveCamera->main = false;
		mActiveCamera = std::move(cam);
		mActiveCamera->main = true;
	}

	bool Scene::LoadScene(const std::string& path)
	{
		auto scene = std::make_shared<Scene>();
		Engine::Instance().GetApp().LoadScene(scene);

		auto serializer = SceneSerializer(scene);
		return serializer.Deserialize(path);
	}

	bool Scene::LoadSceneAdditive(const std::string& path)
	{
		auto serializer = SceneSerializer(Engine::Instance().GetApp().GetCurrentScene());
		return serializer.Deserialize(path);
	}
}
