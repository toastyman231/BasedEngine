#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "physics/physicslayers.h"

namespace based::physics
{
	class JoltDebugRendererImpl;
}

namespace based::managers
{
	class PhysicsManager
	{
	public:
		PhysicsManager() = default;
		~PhysicsManager() = default;

		void Initialize();
		void Update(float deltaTime);
		void Shutdown();

		JPH::PhysicsSystem& GetPhysicsSystem() const { return *mPhysicsSystem; }
		JPH::DebugRenderer* GetDebugRenderer() const { return (JPH::DebugRenderer*)mDebugRenderer; }
		JPH::TempAllocator* GetTempAllocator() const { return mTempAllocator; }
		bool ShouldRenderDebug() const { return mRenderDebug; }

		void SetGravity(glm::vec3 gravity) const;
		void SetStepFrequency(float freq) { mPhysicsStepFrequency = freq; }
		void SetTimeMode(bool useUnscaled) { mUseUnscaledTime = useUnscaled; }
		void SetRenderDebug(bool renderDebug) { mRenderDebug = renderDebug; }

		JPH::BodyID AddBody(const JPH::Shape* shape, glm::vec3 position, glm::vec3 rotation,
		                    JPH::EMotionType type, uint16_t layer, JPH::EActivation activation = JPH::EActivation::Activate,
							bool isTrigger = false) const;
		void DrawDebugBodies();

	protected:
		float mPhysicsStepFrequency = 60.f;
		bool mUseUnscaledTime = false;
		bool mRenderDebug = false;

		JPH::PhysicsSystem* mPhysicsSystem = nullptr;
		JPH::JobSystem* mJobSystem = nullptr;
		JPH::BodyInterface* mBodyInterface = nullptr;
		JPH::TempAllocator* mTempAllocator = nullptr;
		physics::JoltDebugRendererImpl* mDebugRenderer = nullptr;

		JPH::PhysicsSettings mPhysicsSettings;

		physics::BPLayerInterfaceImpl mBroadphaseLayerInterface;
		physics::ObjectVsBroadPhaseLayerFilterImpl mObjBroadphaseLayerFilter;
		physics::ObjectLayerPairFilterImpl mObjLayerPairFilter;

	};
}
