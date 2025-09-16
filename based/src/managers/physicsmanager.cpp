#include "pch.h"
#include "based/managers/physicsmanager.h"

#include "app.h"
#include "basedtime.h"
#include "engine.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/JobSystemThreadPool.h"

#include "based/physics/physicsconversions.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "math/basedmath.h"
#include "physics/debugrenderer.h"
#include "scene/entity.h"

namespace based::managers
{
	void PhysicsManager::Initialize()
	{
		PROFILE_FUNCTION();
		JPH::RegisterDefaultAllocator();
		JPH::Factory::sInstance = new JPH::Factory();
		mTempAllocator = new JPH::TempAllocatorImpl(32 * 1024 * 1024);
		// TODO: Switch over to my own job manager
		mJobSystem = new JPH::JobSystemThreadPool(2048, 8,
			std::thread::hardware_concurrency() - 1u);
		JPH::RegisterTypes();

		mPhysicsSystem = new JPH::PhysicsSystem();
		mPhysicsSystem->Init(10240, 0, 65536, 20480,
			mBroadphaseLayerInterface, mObjBroadphaseLayerFilter, mObjLayerPairFilter);
		mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings);
		mPhysicsSystem->SetGravity(JPH::Vec3(0, -9.81f, 0));

#ifdef BASED_CONFIG_DEBUG
		mDebugRenderer = new physics::JoltDebugRendererImpl();
#endif
	}

	void PhysicsManager::Update(float deltaTime)
	{
		PROFILE_FUNCTION();

		if (core::Time::TimeScale() == 0.f) return;

		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		auto physicsEntityView = registry.view<scene::Enabled, scene::EntityReference, scene::RigidbodyComponent>();
		auto& bodyInterface = mPhysicsSystem->GetBodyInterface();

		for (auto& e : physicsEntityView)
		{
			if (auto entity = registry.get<scene::EntityReference>(e).entity.lock())
			{
				auto& rigidbody = registry.get<scene::RigidbodyComponent>(e);
				auto id = rigidbody.rigidbodyID;
				entity->SetTransform(convert(bodyInterface.GetPosition(id)),
					convert(bodyInterface.GetRotation(id).GetEulerAngles()) * math::Rad2Deg,
					entity->GetTransform().Scale());
			}
		}

		mPhysicsSystem->Update((1.f / mPhysicsStepFrequency) * (mUseUnscaledTime ? 1.f : core::Time::TimeScale()), 
			1, mTempAllocator, mJobSystem);
	}

	void PhysicsManager::Shutdown()
	{
		delete mPhysicsSystem;
		delete mTempAllocator;
		delete mJobSystem;
	}

	void PhysicsManager::SetGravity(glm::vec3 gravity) const
	{
		mPhysicsSystem->SetGravity(convert(gravity));
	}

	JPH::BodyID PhysicsManager::AddBody(const JPH::Shape* shape, glm::vec3 position, glm::vec3 rotation, 
	                                    JPH::EMotionType type, uint16_t layer, JPH::EActivation activation,
										bool isTrigger) const
	{
		PROFILE_FUNCTION();
		auto settings = JPH::BodyCreationSettings(
			shape, convert(position),
			JPH::Quat::sEulerAngles(convert(rotation)),
			type,
			layer
		);
		settings.mIsSensor = isTrigger;
		return mPhysicsSystem->GetBodyInterface().CreateAndAddBody(settings, activation);
	}

	void PhysicsManager::DrawDebugBodies()
	{
#ifdef BASED_CONFIG_DEBUG
		if (mRenderDebug)
		{
			auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
			auto view = registry.view<scene::Enabled, scene::RigidbodyComponent>();

			for (auto& e : view)
			{
				auto& rb = view.get<scene::RigidbodyComponent>(e);
				auto shape = mPhysicsSystem->GetBodyInterface().GetTransformedShape(rb.rigidbodyID);
				
				shape.mShape->Draw(mDebugRenderer,
					mPhysicsSystem->GetBodyInterface().GetCenterOfMassTransform(rb.rigidbodyID),
					shape.GetShapeScale(),
					JPH::Color::sGreen, false, true);
			}

			auto charView = registry.view<scene::Enabled, scene::CharacterController>();

			for (auto& e : charView)
			{
				scene::CharacterController& character = charView.get<scene::CharacterController>(e);
				auto& trans = registry.get<scene::Transform>(e);

				/*character.Character->GetShape()->Draw(
					mDebugRenderer,
					character.Character->GetCenterOfMassTransform(),
					character.Character->GetTransformedShape().GetShapeScale(),
					JPH::Color::sGreen, false, true
					);*/

				auto& lockInterface = mPhysicsSystem->GetBodyLockInterface();
				JPH::BodyLockRead lock(lockInterface, character.Character->GetInnerBodyID());

				if (lock.Succeeded())
				{
					auto& body = lock.GetBody();
					auto shape = body.GetShape();
					shape->Draw(mDebugRenderer, body.GetCenterOfMassTransform(), body.GetTransformedShape().GetShapeScale(),
						JPH::Color::sBlue, false, true);
				}
			}
		}
#endif
	}
}
