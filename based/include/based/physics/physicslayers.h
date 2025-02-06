#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace based::physics
{
	namespace Layers
	{
		static constexpr JPH::ObjectLayer UNUSED1 = 0; // 4 unused values so that broadphase layers values don't match with object layer values (for testing purposes)
		static constexpr JPH::ObjectLayer UNUSED2 = 1;
		static constexpr JPH::ObjectLayer UNUSED3 = 2;
		static constexpr JPH::ObjectLayer UNUSED4 = 3;
		static constexpr JPH::ObjectLayer STATIC = 4;
		static constexpr JPH::ObjectLayer MOVING = 5;
		static constexpr JPH::ObjectLayer NUM_LAYERS = 6;
	};

	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer STATIC(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::BroadPhaseLayer UNUSED(2);
		static constexpr JPH::uint NUM_LAYERS(3);
	};

	/// BroadPhaseLayerInterface implementation
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::UNUSED1] = BroadPhaseLayers::UNUSED;
			mObjectToBroadPhase[Layers::UNUSED2] = BroadPhaseLayers::UNUSED;
			mObjectToBroadPhase[Layers::UNUSED3] = BroadPhaseLayers::UNUSED;
			mObjectToBroadPhase[Layers::UNUSED4] = BroadPhaseLayers::UNUSED;
			mObjectToBroadPhase[Layers::STATIC] = BroadPhaseLayers::STATIC;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual JPH::uint GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
			case Layers::STATIC:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return inLayer2 == BroadPhaseLayers::STATIC || inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::UNUSED1:
			case Layers::UNUSED2:
			case Layers::UNUSED3:
				return false;
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
			case Layers::UNUSED1:
			case Layers::UNUSED2:
			case Layers::UNUSED3:
			case Layers::UNUSED4:
				return false;
			case Layers::STATIC:
				return inObject2 == Layers::MOVING;
			case Layers::MOVING:
				return inObject2 == Layers::STATIC || inObject2 == Layers::MOVING;
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};
}
