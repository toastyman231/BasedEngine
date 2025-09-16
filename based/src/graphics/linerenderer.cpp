#include "pch.h"
#include "based/graphics/linerenderer.h"

#include "app.h"
#include "basedtime.h"
#include "engine.h"
#include "graphics/material.h"
#include "graphics/vertex.h"

namespace based::graphics
{
#ifdef BASED_CONFIG_DEBUG
	void DebugLineRenderer::DrawDebugLine(glm::vec3 startPos, glm::vec3 endPos, float width, glm::vec4 color, float duration)
	{
		auto va = std::make_shared<VertexArray>();

		static std::shared_ptr<Material> mat = Material::LoadMaterialFromFile(
			ASSET_PATH("Materials/DefaultLine.bmat"),  
			Engine::Instance().GetApp().GetCurrentScene()->GetMaterialStorage());
		mat->SetUniformValue("color", color);
		mat->SetUniformValue("width", width);

		BASED_CREATE_VERTEX_BUFFER(line, float);
		line->PushVertex({startPos.x, startPos.y, startPos.z});
		line->PushVertex({endPos.x, endPos.y, endPos.z});
		line->SetLayout({ 3 });
		va->PushBuffer(std::move(line));
		va->Upload();

		mLines.emplace_back(va, mat, width, duration, color);
	}

	// SHOULD ONLY BE CALLED ONCE PER FRAME BY ENGINE
	void DebugLineRenderer::DrawLines()
	{
		std::vector<size_t> linesToRemove;
		for (size_t i = 0; i < mLines.size(); ++i)
		{
			auto& line = mLines[i];
			if (line.duration != -1.f)
			{
				line.duration -= core::Time::DeltaTime();
				if (line.duration <= 0)
				{
					linesToRemove.push_back(i);
					continue;
				}
			}
			Engine::Instance().GetRenderManager().Submit(BASED_SUBMIT_RC(RenderLineMaterial, line.va, line.mat));
		}

		if (linesToRemove.empty()) return;
		for (auto line : linesToRemove)
		{
			mLines[line] = mLines.back();
			mLines.pop_back();
		}
	}
#else
	void DebugLineRenderer::DrawDebugLine(glm::vec3 startPos, glm::vec3 endPos, float width, glm::vec4 color, float duration)
	{
		return;
	}

	void DebugLineRenderer::DrawLines() { return; }
#endif
}
