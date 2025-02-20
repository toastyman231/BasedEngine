#pragma once

#include <memory>
#include <external/glm/fwd.hpp>
#include <external/glm/vec2.hpp>
#include <external/glm/vec3.hpp>

#include "based/core/uuid.h"
#include "based/physics/physicsconversions.h"
#include "based/engine.h"

#include <Jolt/Jolt.h>
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>

#include "based/math/basedmath.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

namespace JPH
{
	class CharacterVirtual;
}

namespace based::ui
{
	class TextEntity;
}

namespace based::animation
{
	class Animator;
}

namespace based::graphics
{
	class Mesh;
	class Model;
	class Sprite;
	class Camera;
}

namespace based::scene
{
	class Entity;

	struct EntityReference
	{
		std::weak_ptr<Entity> entity;

		EntityReference() = default;
		EntityReference(const EntityReference&) = default;
		EntityReference(const std::shared_ptr<Entity>& entityPtr) : entity(entityPtr) {}
	};

	struct IDComponent
	{
		based::core::UUID uuid;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct Transform
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::vec3 LocalPosition = { 0.0f, 0.0f, 0.0f };
		glm::vec3 LocalRotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 LocalScale = { 1.0f, 1.0f, 1.0f };

		Transform() = default;
		Transform(const Transform&) = default;
		Transform(const glm::vec3& pos) : Position(pos) {}
		Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, bool local = false)
		{
			if (local)
			{
				LocalPosition = pos;
				LocalRotation = rot;
				LocalScale = scale;
			} else
			{
				Position = pos;
				Rotation = rot;
				Scale = scale;
			}
		}
		Transform(const glm::vec3& pos, const glm::vec3& localPos, 
			const glm::vec3& rot, const glm::vec3& localRot,
			const glm::vec3& scale, const glm::vec3& localScale)
		{
			LocalPosition = localPos;
			LocalRotation = localRot;
			LocalScale = localScale;
			Position = pos;
			Rotation = rot;
			Scale = scale;
		}
	};

	struct Velocity
	{
		float dx;
		float dy;

		Velocity() = default;
		Velocity(float xVel, float yVel) : dx(xVel), dy(yVel) {}
		Velocity(const Velocity&) = default;
		Velocity(const glm::vec2& vel) : dx(vel.x), dy(vel.y) {}
	};

	struct SpriteRenderer
	{
		std::weak_ptr<graphics::Sprite> sprite;

		SpriteRenderer() = delete;
		SpriteRenderer(const std::shared_ptr<graphics::Sprite>& spritePtr) : sprite(spritePtr) {}
	};

	struct ModelRenderer
	{
		std::weak_ptr<graphics::Model> model;

		ModelRenderer() = delete;
		ModelRenderer(const std::shared_ptr<graphics::Model>& modelPtr) : model(modelPtr) {}
		ModelRenderer(const ModelRenderer&) = default;
	};

	struct AnimatorComponent
	{
		std::weak_ptr<animation::Animator> animator;

		AnimatorComponent() = default;
		AnimatorComponent(const std::shared_ptr<animation::Animator>& anim) : animator(anim) {}
	};

	struct MeshRenderer
	{
		std::weak_ptr<graphics::Mesh> mesh;

		MeshRenderer() = delete;
		MeshRenderer(const std::shared_ptr<graphics::Mesh>& meshPtr) : mesh(meshPtr){}
		MeshRenderer(const MeshRenderer&) = default;
	};

	struct PointLight {
		glm::vec3 position;

		float constant;
		float linear;
		float quadratic;
		float intensity;

		glm::vec3 color;

		PointLight(float c, float l, float q, glm::vec3 col) : constant(c), linear(l), quadratic(q), intensity(0.5f), color(col) {}
		PointLight(float c, float l, float q, float i, glm::vec3 col) : constant(c), linear(l), quadratic(q), intensity(i), color(col) {}
	};

	struct DirectionalLight
	{
		glm::vec3 direction;
		glm::vec3 color;
		float intensity = 1.f;

		DirectionalLight(glm::vec3 dir, glm::vec3 col, float i) : direction(dir), color(col), intensity(i) {}
		DirectionalLight(glm::vec3 dir, glm::vec3 col) : direction(dir), color(col) {}
		DirectionalLight(glm::vec3 col) : direction(glm::vec3(60.f, 60.f, 0.f)), color(col) {}
	};

	struct CameraComponent
	{
		std::weak_ptr<graphics::Camera> camera;

		CameraComponent(const std::shared_ptr<graphics::Camera>& cam) : camera(cam) {}
	};

	// Kind of only exists to signal to the serializer which components
	// need to be serialized using reflected data
	struct ScriptComponent
	{
		ScriptComponent() = default;
		virtual ~ScriptComponent() = default;

		bool Enabled = true;
	};

	struct PhysicsShapeComponent
	{
		JPH::Ref<JPH::Shape> shape;
		glm::vec3 center;
		glm::vec3 rotation;
	};

	struct BoxShapeComponent : public PhysicsShapeComponent
	{
		glm::vec3 halfExtent;

		BoxShapeComponent(glm::vec3 hExtent) : PhysicsShapeComponent()
		{
			halfExtent = hExtent;
			shape = new JPH::BoxShape(convert(hExtent));
		}

		BoxShapeComponent(glm::vec3 hExtent, glm::vec3 position, glm::vec3 rot)
			: PhysicsShapeComponent()
		{
			center = position;
			rotation = rot;
			halfExtent = hExtent;
			shape = new JPH::BoxShape(convert(hExtent));
		}
	};

	struct CapsuleShapeComponent: public PhysicsShapeComponent
	{
		float HalfHeight;
		float Radius;

		CapsuleShapeComponent(float halfHeight, float radius)
		{
			HalfHeight = halfHeight;
			Radius = radius;
			shape = new JPH::CapsuleShape(halfHeight, radius);
		}
	};

	struct RigidbodyComponent
	{
		JPH::BodyID rigidbodyID;

		RigidbodyComponent(PhysicsShapeComponent shape, 
			JPH::EMotionType type, uint16_t layer, JPH::EActivation activation = JPH::EActivation::Activate)
		{
			rigidbodyID = Engine::Instance().GetPhysicsManager().AddBody(
				shape.shape, shape.center, shape.rotation * based::math::Deg2Rad, type, layer, activation
			);
		}

		void RegisterBody(const entt::entity entity)
		{
			Engine::Instance()
				.GetPhysicsManager()
				.GetPhysicsSystem()
				.GetBodyInterface()
				.SetUserData(rigidbodyID, static_cast<uint32_t>(entity));
		}
	};

	struct CharacterController
	{
		JPH::CharacterVirtual* Character;
		JPH::CharacterVirtualSettings Settings;

		float Speed = 6.0f;
		float JumpForce = 4.0f;
		bool AllowAirControl = false;
		bool ControlEnabled = true;

		CharacterController(JPH::CharacterVirtualSettings settings, Transform transform)
		{
			Settings = settings;
			Character = new JPH::CharacterVirtual(&settings, convert(transform.Position),
				JPH::Quat::sEulerAngles(convert(transform.Rotation)), 0,
				&Engine::Instance().GetPhysicsManager().GetPhysicsSystem());
		}

		CharacterController(Transform transform, JPH::RefConst<JPH::Shape> shape)
		{
			JPH::CharacterVirtualSettings settings;
			settings.mMaxSlopeAngle = math::Deg2Rad * 45.f;
			settings.mMaxStrength = 100.f;
			settings.mShape = shape;
			settings.mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
			settings.mCharacterPadding = 0.02f;
			settings.mPenetrationRecoverySpeed = 1.f;
			settings.mPredictiveContactDistance = 0.1f;
			settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -0.3f); // Accept contacts that touch the lower sphere of the capsule
			settings.mEnhancedInternalEdgeRemoval = false;
			/*settings.mInnerBodyShape = sCreateInnerBody ? mInnerStandingShape : nullptr;*/
			settings.mInnerBodyLayer = physics::Layers::MOVING;

			Settings = settings;
			Character = new JPH::CharacterVirtual(&settings, convert(transform.Position),
				JPH::Quat::sEulerAngles(convert(transform.Rotation)), 0,
				&Engine::Instance().GetPhysicsManager().GetPhysicsSystem());
		}
	};

	struct TextRenderer
	{
		ui::TextEntity* text;

		TextRenderer() = default;
		TextRenderer(ui::TextEntity* textPtr) : text(textPtr) {}
	};

	struct Enabled
	{
		Enabled() = default;
	};
}
