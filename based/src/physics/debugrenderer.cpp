#include "pch.h"
#include "based/physics/debugrenderer.h"

namespace based::physics
{
	void JoltDebugRendererImpl::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
	{
	}

	void JoltDebugRendererImpl::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
		JPH::ColorArg inColor, ECastShadow inCastShadow)
	{
	}

	JPH::DebugRenderer::Batch JoltDebugRendererImpl::CreateTriangleBatch(const Triangle* inTriangles,
		int inTriangleCount)
	{
		return Batch();
	}

	JPH::DebugRenderer::Batch JoltDebugRendererImpl::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount,
		const JPH::uint32* inIndices, int inIndexCount)
	{
		return Batch();
	}

	void JoltDebugRendererImpl::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds,
		float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode,
		ECastShadow inCastShadow, EDrawMode inDrawMode)
	{
	}

	void JoltDebugRendererImpl::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString,
		JPH::ColorArg inColor, float inHeight)
	{
	}
}
