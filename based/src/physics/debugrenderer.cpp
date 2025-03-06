#include "pch.h"
#include "based/physics/debugrenderer.h"

#include "app.h"
#include "engine.h"
#include "based/graphics/linerenderer.h"
#include "based/physics/physicsconversions.h"
#include "graphics/rendercommands.h"
#include "managers/rendermanager.h"

namespace based::physics
{
	JoltDebugRendererImpl::JoltDebugRendererImpl()
	{
		mGeometryMaterial = graphics::Material::LoadFileMaterialWithoutSaving(
			ASSET_PATH("Materials/DebugGeometry.bmat"));
		Initialize();
	}

	void JoltDebugRendererImpl::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
	{
		graphics::DebugLineRenderer::DrawDebugLine(convert(inFrom), convert(inTo), 5,
			glm::vec4(inColor.r, inColor.g, inColor.b, inColor.a));
	}

	void JoltDebugRendererImpl::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
		JPH::ColorArg inColor, ECastShadow inCastShadow)
	{
		static auto va = std::make_shared<graphics::VertexArray>();

		static std::shared_ptr<graphics::Material> mat = graphics::Material::LoadMaterialFromFile(
			ASSET_PATH("Materials/Unlit.bmat"),
			Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage());
		mat->SetUniformValue("color", glm::vec4{inColor.r, inColor.g, inColor.b, inColor.a});

		BASED_CREATE_VERTEX_BUFFER(line, float);
		va->ClearElements();
		line->PushVertex({ inV1.GetX(), inV1.GetY(), inV1.GetZ() });
		line->PushVertex({ inV2.GetX(), inV2.GetY(), inV2.GetZ() });
		line->PushVertex({ inV3.GetX(), inV3.GetY(), inV3.GetZ() });
		line->SetLayout({ 3 });
		va->PushBuffer(std::move(line));
		va->Upload();

		Engine::Instance().GetRenderManager().Submit(
			BASED_SUBMIT_RC(RenderVertexArrayMaterial, va, mat));
	}

	JPH::DebugRenderer::Batch JoltDebugRendererImpl::CreateTriangleBatch(const Triangle* inTriangles,
		int inTriangleCount)
	{
		int index = static_cast<int>(mTriangleBatches.size());
		auto va = mTriangleBatches.emplace_back(std::make_shared<graphics::VertexArray>());

		BASED_CREATE_VERTEX_BUFFER(pos, float);
		BASED_CREATE_VERTEX_BUFFER(norm, float);
		BASED_CREATE_VERTEX_BUFFER(uv, float);
		BASED_CREATE_VERTEX_BUFFER(color, int);

		for (int i = 0; i < inTriangleCount; i++)
		{
			auto triangle = inTriangles + i;
			auto position = triangle->mV->mPosition;
			auto normal = triangle->mV->mNormal;
			auto uvCoord = triangle->mV->mUV;
			auto col = triangle->mV->mColor;

			pos->PushVertex({ position.x, position.y, position.z });
			norm->PushVertex({ normal.x, normal.y, normal.z });
			uv->PushVertex({ uvCoord.x, uvCoord.y });
			color->PushVertex({ col.r, col.g, col.b, col.a });
		}

		pos->SetLayout({ 3 });
		norm->SetLayout({ 3 });
		uv->SetLayout({ 2 });
		color->SetLayout({ 4 });
		va->PushBuffer(std::move(pos));
		va->PushBuffer(std::move(uv));
		va->PushBuffer(std::move(norm));
		va->PushBuffer(std::move(color));
		va->Upload();

		TriangleBatchImpl* batch = new TriangleBatchImpl(index);
		Batch triangleBatch = JPH::Ref(batch);
		return triangleBatch;
	}

	JPH::DebugRenderer::Batch JoltDebugRendererImpl::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount,
		const JPH::uint32* inIndices, int inIndexCount)
	{
		int index = static_cast<int>(mTriangleBatches.size());
		auto va = mTriangleBatches.emplace_back(std::make_shared<graphics::VertexArray>());

		BASED_CREATE_VERTEX_BUFFER(pos, float);
		BASED_CREATE_VERTEX_BUFFER(norm, float);
		BASED_CREATE_VERTEX_BUFFER(uv, float);
		BASED_CREATE_VERTEX_BUFFER(color, int);

		for (int i = 0; i < inVertexCount; i++)
		{
			auto vertex = inVertices + i;
			auto position = vertex->mPosition;
			auto normal = vertex->mNormal;
			auto uvCoord = vertex->mUV;
			auto col = vertex->mColor;

			pos->PushVertex({ position.x, position.y, position.z });
			norm->PushVertex({ normal.x, normal.y, normal.z });
			uv->PushVertex({ uvCoord.x, uvCoord.y });
			color->PushVertex({ col.r, col.g, col.b, col.a });
		}

		pos->SetLayout({ 3 });
		norm->SetLayout({ 3 });
		uv->SetLayout({ 2 });
		color->SetLayout({ 4 });
		va->SetElements(std::vector(inIndices, inIndices + inIndexCount));
		va->PushBuffer(std::move(pos));
		va->PushBuffer(std::move(uv));
		va->PushBuffer(std::move(norm));
		va->PushBuffer(std::move(color));
		va->Upload();

		TriangleBatchImpl* batch = new TriangleBatchImpl(index);
		Batch triangleBatch = JPH::Ref(batch);
		return triangleBatch;
	}

	void JoltDebugRendererImpl::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds,
		float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode,
		ECastShadow inCastShadow, EDrawMode inDrawMode)
	{
		auto& rm = Engine::Instance().GetRenderManager();
		auto cameraPos = convert(Engine::Instance().GetApp().GetCurrentScene()->GetActiveCamera()->GetTransform().Position());
		LOD lod = inGeometry->GetLOD(cameraPos, inWorldSpaceBounds, inLODScaleSq);

		if (inDrawMode == EDrawMode::Wireframe)
			rm.Submit(BASED_SUBMIT_RC(SetWireframe, true));
		rm.Submit(BASED_SUBMIT_RC(RenderVertexArrayMaterial,
			mTriangleBatches[dynamic_cast<TriangleBatchImpl*>(lod.mTriangleBatch.GetPtr())->index],
			mGeometryMaterial, convert(inModelMatrix)));
		if (inDrawMode == EDrawMode::Wireframe)
			rm.Submit(BASED_SUBMIT_RC(SetWireframe, false));
	}

	void JoltDebugRendererImpl::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString,
		JPH::ColorArg inColor, float inHeight)
	{
	}
}
