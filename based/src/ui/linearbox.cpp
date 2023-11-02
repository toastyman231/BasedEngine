#include "ui/linearbox.h"

namespace based::ui
{
	LinearBox::LinearBox(float x, float y, float width, float height, LinearBoxMode mode) : BoxType(mode), UiElement(x, y, width, height)
	{
	}

	void LinearBox::Update(float deltaTime)
	{
		UpdateContents();
	}

	void LinearBox::UpdateContents()
	{
		//if (!mIsDirty) return;

		int index = 0;
		for (const UiElement* child : GetAllChildren())
		{
			constexpr float align1 = 0.5f;
			const float align2 = (static_cast<float>(GetChildCount()) / 2.f) - static_cast<float>(index);

			const glm::vec2 align = (BoxType == LinearBoxMode::HORIZONTAL) ? glm::vec2{ align2, align1 } : glm::vec2{ align1, align2 };
			child->GetTransform()->alignment = align;
			child->GetTransform()->width = (BoxType == LinearBoxMode::HORIZONTAL) ? GetTransform()->width / static_cast<float>(GetChildCount()) : GetTransform()->width;
			child->GetTransform()->height = (BoxType == LinearBoxMode::HORIZONTAL) ? GetTransform()->height : GetTransform()->height / static_cast<float>(GetChildCount());

			index++;
		}

		mIsDirty = false;
	}
}
