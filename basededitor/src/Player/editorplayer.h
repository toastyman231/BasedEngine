#pragma once
#include "Panels/gameview.h"
#include "based/core/basedtime.h"
#include "based/input/keyboard.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "external/entt/entity/registry.hpp"

namespace editor
{
	struct EditorPlayer
	{
		float speed = 2.5f;
	};

	inline void EditorPlayerUpdateSystem(entt::registry& registry, panels::GameView& sceneView)
	{
		if (!sceneView.IsFocused()) return;

		auto view = registry.view<
			based::scene::EntityReference,
			based::scene::Transform,
			based::scene::CameraComponent,
			EditorPlayer>();

		for (auto& ent : view)
		{
			auto& entPtr = registry.get<based::scene::EntityReference>(ent).entity;
			auto& transform = registry.get<based::scene::Transform>(ent);
			auto& cameraComp = registry.get<based::scene::CameraComponent>(ent).camera;
			auto& player = registry.get<EditorPlayer>(ent);

			auto entity = entPtr.lock();
			auto camera = cameraComp.lock();

			if (entity && camera)
			{
				if (based::input::Keyboard::Key(BASED_INPUT_KEY_W))
				{
					entity->SetPosition(transform.Position + 
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetForward());
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_S))
				{
					entity->SetPosition(transform.Position -
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetForward());
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_D))
				{
					entity->SetPosition(transform.Position +
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetRight());
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_A))
				{
					entity->SetPosition(transform.Position -
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetRight());
				}
			}
		}
	}
}
