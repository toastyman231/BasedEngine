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
#include "external/glm/gtx/euler_angles.hpp"
#include "external/glm/gtx/orthonormalize.hpp"
#include "Jolt/Physics/Character/CharacterVirtual.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Geometry/IndexedTriangle.h"

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
		EntityReference(const EntityReference& other)
		{
			entity = other.entity;
		}
		EntityReference& operator= (const EntityReference& other)
		{
			if (this != &other)
			{
				entity = other.entity;
			}
			return *this;
		}
		EntityReference(const std::shared_ptr<Entity>& entityPtr) : entity(entityPtr) {}
	};

	struct DontDestroyOnLoad {};

	struct IDComponent
	{
		core::UUID uuid;

		IDComponent() = default;
		IDComponent(core::UUID id) : uuid(id) {}
		IDComponent& operator= (const IDComponent& other)
		{
			if (this != &other)
			{
				uuid = core::UUID(); // IDs should always be unique
			}
			return *this;
		}
		IDComponent(const IDComponent& other)
		{
			uuid = core::UUID();
		}
	};

	struct Transform
	{
		Transform* Parent = nullptr;

		Transform()
		{
			mCachedMatrix = glm::mat4(1.f);
		}
		Transform(const glm::vec3& pos)
		{
			mCachedMatrix = glm::mat4(1.f);
			SetPosition(pos);
		}
		Transform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
		{
			mCachedMatrix = glm::mat4(1.f);
			SetLocalTransform(pos, rot, scale);
		}

		Transform(const Transform& other)
		{
			Parent = other.Parent;
			mLocalPosition = other.mLocalPosition;
			mLocalRotation = other.mLocalRotation;
			mLocalScale = other.mLocalScale;
			mCachedEuler = other.mCachedEuler;
			mUseCachedEuler = other.mUseCachedEuler;
			mCachedMatrix = other.mCachedMatrix;
		}
		Transform& operator= (const Transform& other)
		{
			if (this != &other)
			{
				Parent = other.Parent;
				mLocalPosition = other.mLocalPosition;
				mLocalRotation = other.mLocalRotation;
				mLocalScale = other.mLocalScale;
				mCachedEuler = other.mCachedEuler;
				mUseCachedEuler = other.mUseCachedEuler;
				mCachedMatrix = other.mCachedMatrix;
			}
			return *this;
		}

		glm::vec3 Position() const
		{
			if (!Parent)
			{
				return LocalPosition();
			}
			return glm::vec3(Parent->GetGlobalMatrix() * glm::vec4(mLocalPosition, 1.f));
		}
		glm::vec3 LocalPosition() const
		{
			return mLocalPosition;
		}

		glm::quat Rotation() const
		{
			if (!Parent)
			{
				return LocalRotation();
			}
			return Parent->Rotation() * LocalRotation();
		}
		glm::quat LocalRotation() const
		{ 
			return mLocalRotation;
		}

		glm::vec3 EulerAngles() const
		{
			if (!Parent)
			{
				return LocalEulerAngles();
			}

			if (mUseCachedEuler) return Parent->EulerAngles() + LocalEulerAngles();

			return glm::degrees(glm::eulerAngles((glm::normalize(Rotation()))));
		}
		glm::vec3 LocalEulerAngles() const
		{
			if (mUseCachedEuler) return mCachedEuler;
			return glm::degrees(glm::eulerAngles(glm::normalize(mLocalRotation)));
		}

		glm::vec3 Scale() const
		{
			if (!Parent)
			{
				return LocalScale();
			}
			return Parent->Scale() * LocalScale();
		}
		glm::vec3 LocalScale() const
		{
			return mLocalScale;
		}

		glm::mat4 GetGlobalMatrix()
		{
			if (Parent)
			{
				return Parent->GetGlobalMatrix() * GetLocalMatrix();
			}
			else return GetLocalMatrix();
		}

		glm::mat4 GetLocalMatrix()
		{
			return mCachedMatrix;
		}

		glm::vec3 InverseTransformPoint(const glm::vec3& point) const
		{
			glm::vec3 localPosition;
			if (Parent)
				localPosition = Parent->InverseTransformPoint(point);
			else
				localPosition = point;

			localPosition -= mLocalPosition;
			glm::vec3 newPosition = glm::inverse(mLocalRotation) * localPosition;
			return newPosition;
		}

		void SetPosition(const glm::vec3& position)
		{
			if (Parent)
			{
				SetLocalPosition(Parent->InverseTransformPoint(position));
			}
			else SetLocalPosition(position);
		}
		void SetLocalPosition(const glm::vec3& position)
		{
			mLocalPosition = position;
			UpdateCachedMatrix();
		}

		void SetEulerAngles(const glm::vec3& eulerAngles)
		{
			if (Parent)
			{
				SetLocalEulerAngles(Parent->EulerAngles() - eulerAngles);
			}
			else SetLocalEulerAngles(eulerAngles);
		}
		void SetLocalEulerAngles(const glm::vec3& eulerAngles)
		{
			mCachedEuler = eulerAngles;
			mUseCachedEuler = true;
			mLocalRotation = EulerToQuaternion(eulerAngles);
			UpdateCachedMatrix();
		}

		void SetRotation(const glm::quat& q)
		{
			if (Parent)
			{
				SetLocalRotation(glm::inverse(Parent->Rotation()) * q);
			}
			else SetLocalRotation(q);
		}
		void SetLocalRotation(const glm::quat& q)
		{
			mLocalRotation = glm::normalize(q);
			mUseCachedEuler = false;
			UpdateCachedMatrix();
		}

		void SetScale(const glm::vec3& scale)
		{
			if (Parent)
			{
				glm::vec3 parentScale = Parent->Scale();
				if (parentScale.x == 0.f) parentScale.x = 0.0001f;
				if (parentScale.y == 0.f) parentScale.y = 0.0001f;
				if (parentScale.z == 0.f) parentScale.z = 0.0001f;

				SetLocalScale(scale / parentScale);
			}
			else SetLocalScale(scale);
		}
		void SetLocalScale(const glm::vec3& scale)
		{
			mLocalScale = scale;
			UpdateCachedMatrix();
		}

		void SetLocalTransform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
		{
			SetLocalPosition(pos);

			SetLocalEulerAngles(rot);

			SetLocalScale(scale);
		}

		void SetLocalTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
		{
			SetLocalPosition(pos);

			SetLocalRotation(rot);

			SetLocalScale(scale);
		}

		void SetGlobalTransform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
		{
			SetPosition(pos);

			SetEulerAngles(rot);

			SetScale(scale);
		}

		void SetGlobalTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
		{
			SetPosition(pos);

			SetRotation(rot);

			SetScale(scale);
		}

		void SetGlobalTransformFromMatrix(const glm::mat4& matrix)
		{
			if (!Parent)
			{
				SetLocalTransformFromMatrix(matrix);
				return;
			}

			auto parentMatrix = Parent->GetGlobalMatrix();

			mCachedMatrix = glm::inverse(parentMatrix) * matrix;
			mUseCachedEuler = false;

			mLocalPosition = glm::vec3(mCachedMatrix[3]);

			glm::vec3 scale;
			scale.x = glm::length(glm::vec3(mCachedMatrix[0]));
			scale.y = glm::length(glm::vec3(mCachedMatrix[1]));
			scale.z = glm::length(glm::vec3(mCachedMatrix[2]));

			mLocalScale = scale;

			glm::mat3 rotationMatrix;
			rotationMatrix[0] = glm::normalize(glm::vec3(mCachedMatrix[0]) / scale.x);
			rotationMatrix[1] = glm::normalize(glm::vec3(mCachedMatrix[1]) / scale.y);
			rotationMatrix[2] = glm::normalize(glm::vec3(mCachedMatrix[2]) / scale.z);

			rotationMatrix = glm::orthonormalize(rotationMatrix);

			mLocalRotation = glm::quat_cast(rotationMatrix);
		}

		void SetLocalTransformFromMatrix(const glm::mat4& matrix)
		{
			mCachedMatrix = matrix;
			mUseCachedEuler = false;

			mLocalPosition = glm::vec3(mCachedMatrix[3]);

			glm::vec3 scale;
			scale.x = glm::length(glm::vec3(mCachedMatrix[0]));
			scale.y = glm::length(glm::vec3(mCachedMatrix[1]));
			scale.z = glm::length(glm::vec3(mCachedMatrix[2]));

			mLocalScale = scale;

			glm::mat3 rotationMatrix;
			rotationMatrix[0] = glm::normalize(glm::vec3(mCachedMatrix[0]) / scale.x);
			rotationMatrix[1] = glm::normalize(glm::vec3(mCachedMatrix[1]) / scale.y);
			rotationMatrix[2] = glm::normalize(glm::vec3(mCachedMatrix[2]) / scale.z);

			rotationMatrix = glm::orthonormalize(rotationMatrix);

			mLocalRotation = glm::quat_cast(rotationMatrix);
		}

		static glm::quat EulerToQuaternion(const glm::vec3& eulerAngles)
		{
			glm::quat yawQuat = glm::angleAxis(glm::radians(eulerAngles.y), glm::vec3(0, 1, 0)); // Rotate around Y
			glm::quat pitchQuat = glm::angleAxis(glm::radians(eulerAngles.x), glm::vec3(1, 0, 0)); // Rotate around X
			glm::quat rollQuat = glm::angleAxis(glm::radians(eulerAngles.z), glm::vec3(0, 0, 1)); // Rotate around Z

			return yawQuat * pitchQuat * rollQuat; // Apply rotations in correct order
		}

	private:
		void UpdateCachedMatrix()
		{
			if (mUseCachedEuler)
			{
				glm::mat4 rotation =
					glm::rotate(glm::mat4(1.f), glm::radians(mCachedEuler.y), glm::vec3(0.f, 1.f, 0.f)) *
					glm::rotate(glm::mat4(1.f), glm::radians(mCachedEuler.x), glm::vec3(1.f, 0.f, 0.f)) *
					glm::rotate(glm::mat4(1.f), glm::radians(mCachedEuler.z), glm::vec3(0.f, 0.f, 1.f));

				mCachedMatrix = glm::translate(glm::mat4(1.f), mLocalPosition)
					* rotation * glm::scale(glm::mat4(1.f), mLocalScale);
			}
			else
			{
				mCachedMatrix = glm::translate(glm::mat4(1.f), mLocalPosition)
					* glm::mat4_cast(mLocalRotation) * glm::scale(glm::mat4(1.f), mLocalScale);
			}
		}

		glm::vec3 mLocalPosition = glm::vec3(0.f, 0.f, 0.f);
		glm::quat mLocalRotation = glm::quat(1.f, 0.f, 0.f, 0.f);
		glm::vec3 mCachedEuler = glm::vec3(0.f);
		glm::vec3 mLocalScale = glm::vec3(1.f, 1.f, 1.f);
		bool mUseCachedEuler = false;

		glm::mat4 mCachedMatrix;
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
		SpriteRenderer(const SpriteRenderer& other)
		{
			sprite = other.sprite;
		}
		SpriteRenderer& operator= (const SpriteRenderer& other)
		{
			if (this != &other)
			{
				sprite = other.sprite;
			}
			return *this;
		}
	};

	struct ModelRenderer
	{
		std::weak_ptr<graphics::Model> model;

		ModelRenderer() = default;
		ModelRenderer(const std::shared_ptr<graphics::Model>& modelPtr) : model(modelPtr) {}
		ModelRenderer(const ModelRenderer& other)
		{
			model = other.model;
		}
		ModelRenderer& operator= (const ModelRenderer& other)
		{
			if (this != &other)
			{
				model = other.model;
			}
			return *this;
		}
	};

	struct AnimatorComponent
	{
		std::weak_ptr<animation::Animator> animator;

		AnimatorComponent() = default;
		AnimatorComponent(const std::shared_ptr<animation::Animator>& anim) : animator(anim) {}
		AnimatorComponent(const AnimatorComponent& other)
		{
			animator = other.animator;
		}
		AnimatorComponent& operator= (const AnimatorComponent& other)
		{
			if (this != &other)
			{
				animator = other.animator;
			}
			return *this;
		}
	};

	struct MeshRenderer
	{
		std::weak_ptr<graphics::Mesh> mesh;
		std::weak_ptr<graphics::Material> material;
		std::vector<std::string> excludedPasses;

		MeshRenderer();

		MeshRenderer(const std::shared_ptr<graphics::Mesh>& meshPtr) : MeshRenderer()
		{
			mesh = meshPtr;
		}

		MeshRenderer(const std::shared_ptr<graphics::Mesh>& meshPtr, 
			const std::shared_ptr<graphics::Material>& mat)
		{
			mesh = meshPtr;
			material = mat;
		}

		MeshRenderer(const MeshRenderer& other)
		{
			mesh = other.mesh;
			material = other.material;
			excludedPasses = other.excludedPasses;
		}
		MeshRenderer& operator= (const MeshRenderer& other)
		{
			if (this != &other)
			{
				mesh = other.mesh;
				material = other.material;
				excludedPasses = other.excludedPasses;
			}
			return *this;
		}
	};

	struct PointLight {
		glm::vec3 position;

		float constant;
		float linear;
		float quadratic;
		float intensity;

		glm::vec3 color;

		PointLight() = default;
		PointLight(float c, float l, float q, glm::vec3 col) : constant(c), linear(l), quadratic(q), intensity(0.5f), color(col) {}
		PointLight(float c, float l, float q, float i, glm::vec3 col) : constant(c), linear(l), quadratic(q), intensity(i), color(col) {}
		PointLight(const PointLight& other)
		{
			position = other.position;
			constant = other.constant;
			linear = other.linear;
			quadratic = other.quadratic;
			intensity = other.intensity;
			color = other.color;
		}
		PointLight& operator= (const PointLight& other)
		{
			if (this != &other)
			{
				position = other.position;
				constant = other.constant;
				linear = other.linear;
				quadratic = other.quadratic;
				intensity = other.intensity;
				color = other.color;
			}
			return *this;
		}
	};

	struct DirectionalLight
	{
		glm::vec3 direction;
		glm::vec3 color;
		float intensity = 1.f;

		DirectionalLight()
			: direction(glm::vec3(60.f, -60.f, 0.f))
			, color(glm::vec3(1.f)) {}
		DirectionalLight(glm::vec3 dir, glm::vec3 col, float i) : direction(dir), color(col), intensity(i) {}
		DirectionalLight(glm::vec3 dir, glm::vec3 col) : direction(dir), color(col) {}
		DirectionalLight(glm::vec3 col) : direction(glm::vec3(60.f, 60.f, 0.f)), color(col) {}
		DirectionalLight(const DirectionalLight& other)
		{
			direction = other.direction;
			color = other.color;
			intensity = other.intensity;
		}
		DirectionalLight& operator= (const DirectionalLight& other)
		{
			if (this != &other)
			{
				direction = other.direction;
				color = other.color;
				intensity = other.intensity;
			}
			return *this;
		}
	};

	struct CameraComponent
	{
		std::weak_ptr<graphics::Camera> camera;

		CameraComponent();
		CameraComponent(const std::shared_ptr<graphics::Camera>& cam) : camera(cam) {}
		CameraComponent(const CameraComponent& other)
		{
			camera = other.camera;
		}
		CameraComponent& operator= (const CameraComponent& other)
		{
			if (this != &other)
			{
				camera = other.camera;
			}
			return *this;
		}
	};

	// Kind of only exists to signal to the serializer which components
	// need to be serialized using reflected data
	struct ScriptComponent
	{
		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other) = default;
		ScriptComponent& operator= (const ScriptComponent& other) = default;
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

		BoxShapeComponent(const BoxShapeComponent& other)
		{
			shape = other.shape;
			center = other.center;
			rotation = other.rotation;
			halfExtent = other.halfExtent;
		}
		BoxShapeComponent operator= (const BoxShapeComponent& other)
		{
			if (this != &other)
			{
				shape = other.shape;
				center = other.center;
				rotation = other.rotation;
				halfExtent = other.halfExtent;
			}
			return *this;
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

		CapsuleShapeComponent(const CapsuleShapeComponent& other)
		{
			shape = other.shape;
			center = other.center;
			rotation = other.rotation;
			HalfHeight = other.HalfHeight;
			Radius = other.Radius;
		}
		CapsuleShapeComponent& operator= (const CapsuleShapeComponent& other)
		{
			if (this != &other)
			{
				shape = other.shape;
				center = other.center;
				rotation = other.rotation;
				HalfHeight = other.HalfHeight;
				Radius = other.Radius;
			}
			return *this;
		}
	};

	struct MeshShapeComponent : public PhysicsShapeComponent
	{
		MeshShapeComponent(std::shared_ptr<graphics::Mesh> mesh);
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

		RigidbodyComponent(JPH::RefConst<JPH::Shape> shape,
			JPH::EMotionType type, uint16_t layer, glm::vec3 rotation,
			JPH::EActivation activation = JPH::EActivation::Activate)
		{
			rigidbodyID = Engine::Instance().GetPhysicsManager().AddBody(
				shape, convert(shape->GetCenterOfMass()), 
				rotation * based::math::Deg2Rad, type, layer, activation
			);
		}

		RigidbodyComponent(PhysicsShapeComponent shape,
			JPH::EMotionType type, uint16_t layer, glm::vec3 position, glm::vec3 rotation,
			JPH::EActivation activation = JPH::EActivation::Activate)
		{
			rigidbodyID = Engine::Instance().GetPhysicsManager().AddBody(
				shape.shape, position, rotation * based::math::Deg2Rad, type, layer, activation
			);
		}

		RigidbodyComponent(const RigidbodyComponent& other)
		{
			rigidbodyID = other.rigidbodyID;
		}
		RigidbodyComponent& operator= (const RigidbodyComponent& other)
		{
			if (this != &other)
			{
				rigidbodyID = other.rigidbodyID;
			}
			return *this;
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

	struct TriggerComponent : RigidbodyComponent
	{
		TriggerComponent(const PhysicsShapeComponent& shape, JPH::EMotionType type,
			JPH::EActivation activation = JPH::EActivation::Activate)
			: RigidbodyComponent(shape, type, physics::Layers::SENSOR, activation)
		{
			auto& lock_interface = 
				Engine::Instance().GetPhysicsManager().GetPhysicsSystem().GetBodyLockInterface();

			JPH::BodyLockWrite lock(lock_interface, rigidbodyID);
			if (lock.Succeeded()) // body id may no longer be valid (unlikely given this is a constructor, but still possible)
			{
				auto& body = lock.GetBody();
				body.SetIsSensor(true);
			}
			else BASED_WARN("Could not lock body to create sensor!");
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

		CharacterController(JPH::CharacterVirtualSettings settings, const Transform& transform)
		{
			Settings = settings;
			Character = new JPH::CharacterVirtual(&settings, convert(transform.Position()),
				JPH::Quat::sEulerAngles(convert(transform.EulerAngles())), 0,
				&Engine::Instance().GetPhysicsManager().GetPhysicsSystem());
		}

		CharacterController(Transform transform, JPH::RefConst<JPH::Shape> shape, 
			JPH::RefConst<JPH::Shape> innerShape = nullptr)
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
			settings.mInnerBodyShape = innerShape != nullptr ? innerShape : nullptr;
			settings.mInnerBodyLayer = physics::Layers::CHARACTER;

			Settings = settings;
			Character = new JPH::CharacterVirtual(&settings, convert(transform.Position()),
				JPH::Quat::sEulerAngles(convert(transform.EulerAngles())), 0,
				&Engine::Instance().GetPhysicsManager().GetPhysicsSystem());
		}

		CharacterController(const CharacterController& other)
		{
			Character = other.Character;
			Settings = other.Settings;
			Speed = other.Speed;
			JumpForce = other.JumpForce;
			AllowAirControl = other.AllowAirControl;
			ControlEnabled = other.ControlEnabled;
		}
		CharacterController& operator= (const CharacterController& other)
		{
			if (this != &other)
			{
				Character = other.Character;
				Settings = other.Settings;
				Speed = other.Speed;
				JumpForce = other.JumpForce;
				AllowAirControl = other.AllowAirControl;
				ControlEnabled = other.ControlEnabled;
			}
			return *this;
		}
	};

	struct TextRenderer
	{
		ui::TextEntity* text;

		TextRenderer() = default;
		TextRenderer(ui::TextEntity* textPtr) : text(textPtr) {}
		TextRenderer(const TextRenderer& other)
		{
			text = other.text;
		}
	};

	struct Enabled
	{
		Enabled() = default;
		Enabled(const Enabled& other) = default;
	};
}
