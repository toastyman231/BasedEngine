#pragma once
#include "Panels/gameview.h"
#include "based/core/basedtime.h"
#include "based/input/keyboard.h"
#include "based/input/mouse.h"
#include "based/scene/components.h"
#include "based/scene/entity.h"
#include "external/entt/entity/registry.hpp"

namespace editor
{
	struct EditorPlayer
	{
		float speed = 2.5f;
		float pitch = 0.f;
		float yaw = 0.f;
		float sensitivity = 100.f;
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
			auto& entPtr = view.get<based::scene::EntityReference>(ent).entity;
			auto& transform = view.get<based::scene::Transform>(ent);
			auto& cameraComp = view.get<based::scene::CameraComponent>(ent).camera;
			auto& player = view.get<EditorPlayer>(ent);

			auto entity = entPtr.lock();
			auto camera = cameraComp.lock();

			BASED_TRACE("Entity player exists!");

			if (based::input::Mouse::Button(BASED_INPUT_MOUSE_RIGHT) || 
				based::input::Mouse::Button(BASED_INPUT_MOUSE_LEFT))
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
				based::input::Mouse::SetCursorMode(based::input::CursorMode::Confined);

				player.pitch += static_cast<float>(based::input::Mouse::DX())
					* player.sensitivity * based::core::Time::UnscaledDeltaTime();
				player.yaw += static_cast<float>(based::input::Mouse::DY())
					* player.sensitivity * based::core::Time::UnscaledDeltaTime();

				player.yaw = based::math::Clamp(player.yaw, -89.f, 89.f);

				if (camera)
					entity->SetRotation({ player.yaw, player.pitch, camera->GetTransform().Rotation.z });
			} else
			{
				based::input::Mouse::SetCursorMode(based::input::CursorMode::Free);
			}

			if (entity && camera && based::input::Mouse::Button(BASED_INPUT_MOUSE_RIGHT))
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
