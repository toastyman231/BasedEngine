#pragma once
#include "based/engine.h"

namespace based::graphics
{
	class Material;
	class VertexArray;

	struct Line
	{
		Line(
			std::shared_ptr<VertexArray> inVA,
			std::shared_ptr<Material> inMat,
			float inWidth,
			float inDuration,
			glm::vec4 inCol) :
			va(inVA), mat(inMat), width(inWidth), duration(inDuration), color(inCol) {}

		bool operator==(const Line& other) const
		{
			return other.va == va && other.mat == mat && other.width == width && other.duration == duration && other.color == color;
		}

		std::shared_ptr<VertexArray> va;
		std::shared_ptr<Material> mat;
		float width = 1;
		float duration = -1;
		glm::vec4 color = { 0, 0, 0, 1 };
	};

	class DebugLineRenderer
	{
		friend class Engine;

	public:
		static void DrawDebugLine(glm::vec3 startPos, glm::vec3 endPos, float width = 1, 
			glm::vec4 color = {0, 0, 0, 1}, float duration = -1.f);

	private:
		static void DrawLines();

		inline static std::vector<Line> mLines;
		inline static std::vector<Line> mLinesToRemoveNextFrame;
	};
}
