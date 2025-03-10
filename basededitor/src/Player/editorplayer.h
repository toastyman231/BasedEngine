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
		float speed = 6.f;
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

			if (based::input::Mouse::Button(BASED_INPUT_MOUSE_RIGHT) || 
				based::input::Mouse::Button(BASED_INPUT_MOUSE_LEFT))
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
				based::input::Mouse::SetCursorMode(based::input::CursorMode::Confined);

				player.pitch += static_cast<float>(based::input::Mouse::DY())
					* player.sensitivity * based::core::Time::UnscaledDeltaTime();
				player.yaw += static_cast<float>(based::input::Mouse::DX())
					* player.sensitivity * based::core::Time::UnscaledDeltaTime();

				player.pitch = based::math::Clamp(player.pitch, -89.f, 89.f);

				//BASED_TRACE("Pitch {} Yaw {}", player.pitch, player.yaw);

				/*glm::quat yawQuat = glm::angleAxis(glm::radians(player.yaw), glm::vec3(0, 1, 0)); // Rotate around Y
				glm::quat pitchQuat = glm::angleAxis(glm::radians(player.pitch), glm::vec3(1, 0, 0)); // Rotate around X
				glm::quat rollQuat = glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)); // Rotate around Z

				glm::quat finalQuat = yawQuat * pitchQuat * rollQuat; // Apply rotations in correct order*/

				//transform.SetRotation(finalQuat);

				transform.SetEulerAngles({ player.pitch, player.yaw, 0.f });
				/*transform.SetGlobalTransform(
					transform.Position(),
					{ player.pitch, player.yaw, 0.f },
					transform.Scale());*/
				//BASED_TRACE("Rotation: {} {} {}", transform.Rotation().x, transform.Rotation().y, transform.Rotation().z);
			} else
			{
				based::input::Mouse::SetCursorMode(based::input::CursorMode::Free);
			}

			if (entity && camera && based::input::Mouse::Button(BASED_INPUT_MOUSE_RIGHT))
			{
				if (based::input::Keyboard::Key(BASED_INPUT_KEY_W))
				{
					entity->SetPosition(transform.Position() +
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetForward());
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_S))
				{
					entity->SetPosition(transform.Position() -
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetForward());
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_D))
				{
					entity->SetPosition(transform.Position() +
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetRight());
				}

				if (based::input::Keyboard::Key(BASED_INPUT_KEY_A))
				{
					entity->SetPosition(transform.Position() -
						player.speed * based::core::Time::UnscaledDeltaTime() * camera->GetRight());
				}
			}
		}
	}
}
