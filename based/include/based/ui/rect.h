#pragma once

#include "external/glm/vec2.hpp"

namespace based::ui {
	struct Padding
	{
		float top;
		float right;
		float bottom;
		float left;

		Padding(float padding) : Padding(padding, padding, padding, padding) {}

		Padding(float t, float r, float b, float l)
		{
			top = t;
			right = r;
			bottom = b;
			left = l;
		}
	};

	struct RectTransform
	{
		float x;
		float y;
		float width;
		float height;

		glm::vec2 anchorPoint;

		glm::vec2 alignment;

		ui::Padding* Padding;

		RectTransform(float x, float y, float width, float height): x(x), y(y), width(width), height(height)
		{
			anchorPoint = glm::vec2{ 0.5f, 0.5f };
			alignment = glm::vec2{ 0.f, 0.f };
			Padding = new ui::Padding(0.f);
		}

		glm::vec3 GetCenter() const
		{
			return glm::vec3{ (width - Padding->right * 2.f) / 2.f + x, (height - Padding->bottom * 2.f) / 2.f - y, 0.f };
		}

		glm::vec3 GetSize() const
		{
			return glm::vec3{ width, height, 0 };
		}

		void SetPadding(float padding) { Padding = new ui::Padding(padding); }
		void SetPadding(float t, float r, float b, float l) { Padding = new ui::Padding(t, r, b, l); }
	};
}
