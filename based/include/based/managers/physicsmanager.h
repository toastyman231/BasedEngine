#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "physics/physicslayers.h"

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

		void SetGravity(glm::vec3 gravity) const;

		JPH::BodyID AddBody(const JPH::Shape* shape, glm::vec3 position, glm::vec3 rotation,
		                    JPH::EMotionType type, uint16_t layer, JPH::EActivation activation = JPH::EActivation::Activate) const;

	protected:
		JPH::PhysicsSystem* mPhysicsSystem = nullptr;
		JPH::JobSystem* mJobSystem = nullptr;
		JPH::BodyInterface* mBodyInterface = nullptr;
		JPH::TempAllocator* mTempAllocator = nullptr;

		JPH::PhysicsSettings mPhysicsSettings;

		physics::BPLayerInterfaceImpl mBroadphaseLayerInterface;
		physics::ObjectVsBroadPhaseLayerFilterImpl mObjBroadphaseLayerFilter;
		physics::ObjectLayerPairFilterImpl mObjLayerPairFilter;

	};
}
