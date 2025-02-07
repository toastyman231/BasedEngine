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
		mJobSystem = new JPH::JobSystemThreadPool(2048, 8,
			std::thread::hardware_concurrency() - 1);
		JPH::RegisterTypes();

		mPhysicsSystem = new JPH::PhysicsSystem();
		mPhysicsSystem->Init(10240, 0, 65536, 20480,
			mBroadphaseLayerInterface, mObjBroadphaseLayerFilter, mObjLayerPairFilter);
		mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings);
		mPhysicsSystem->SetGravity(JPH::Vec3(0, -9.81f, 0));

		mDebugRenderer = new physics::JoltDebugRendererImpl();
	}

	void PhysicsManager::Update(float deltaTime)
	{
		PROFILE_FUNCTION();

		if (core::Time::TimeScale() == 0.f) return;

		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		auto physicsEntityView = registry.view<scene::EntityReference, scene::RigidbodyComponent>();
		auto& bodyInterface = mPhysicsSystem->GetBodyInterface();

		for (auto& e : physicsEntityView)
		{
			if (auto& entity = registry.get<scene::EntityReference>(e).entity.lock())
			{
				auto& rigidbody = registry.get<scene::RigidbodyComponent>(e);
				auto id = rigidbody.rigidbodyID;
				entity->SetTransform(convert(bodyInterface.GetPosition(id)),
					convert(bodyInterface.GetRotation(id).GetEulerAngles()) * math::Rad2Deg,
					entity->GetTransform().Scale);
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
	                                    JPH::EMotionType type, uint16_t layer, JPH::EActivation activation) const
	{
		return mPhysicsSystem->GetBodyInterface().CreateAndAddBody(
			JPH::BodyCreationSettings(
				shape, convert(position), 
				JPH::Quat::sEulerAngles(convert(rotation)),
				type,
				layer
				), activation
		);
	}

	void PhysicsManager::DrawDebugBodies()
	{
		if (mRenderDebug)
		{
			JPH::BodyManager::DrawSettings settings;
			settings.mDrawShapeWireframe = true;

			mPhysicsSystem->DrawBodies(settings, mDebugRenderer);
		}
	}
}
