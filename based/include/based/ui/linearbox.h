#pragma once
#include "uielement.h"

namespace based::ui
{
	class LinearBox: public UiElement
	{
	public:
		enum LinearBoxMode
		{
			VERTICAL, HORIZONTAL
		};

		LinearBox(float x, float y, float width, float height, LinearBoxMode mode);

		void OnCreate() override {}
		void OnShow() override {}
		void OnHide() override {}
		void OnDestroy() override {}

		void Update(float deltaTime) override;

		LinearBoxMode BoxType;
	private:
		bool mIsDirty = false;

		void UpdateContents();
	};
}
