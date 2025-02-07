#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>

#include "based/core/assetlibrary.h"

namespace based::graphics
{
	class Material;
	class VertexArray;
}

namespace based::physics 
{
	class TriangleBatchImpl : public JPH::RefTargetVirtual
	{
	public:
		TriangleBatchImpl(int i) : index(i) {}
		void AddRef() override { ++mRefCount; }
		void Release() override { if (--mRefCount == 0) delete this; }

		int index;

	private:
		JPH::atomic<uint32_t> mRefCount = 0;
	};

	class JoltDebugRendererImpl : public JPH::DebugRenderer 
	{
	public:
		JoltDebugRendererImpl();

		void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
		void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor,
			ECastShadow inCastShadow) override;
		Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
		Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices,
			int inIndexCount) override;
		void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq,
			JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow,
			EDrawMode inDrawMode) override;
		void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor,
			float inHeight) override;

	private:
		std::vector<std::shared_ptr<graphics::VertexArray>> mTriangleBatches;
		std::shared_ptr<graphics::Material> mGeometryMaterial;
	};
}
