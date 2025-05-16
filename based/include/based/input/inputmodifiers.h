#pragma once

#include "basedinput.h"
#include "basedtime.h"
#include "input/basedinput.h"

namespace based::input
{
	class NegateModifier : public InputActionModifierBase
	{
	public:
		explicit NegateModifier()
			: InputActionModifierBase("Negate")
		{
		}

		void Evaluate(InputActionValue& value) override
		{
			switch (value.type) {
			case InputActionType::Boolean:
				value.triggered = !value.triggered;
				break;
			case InputActionType::Axis1D:
				value.axis1DValue *= -1.f;
				break;
			case InputActionType::Axis2D:
				value.axis2DValue *= -1.f;
				break;
			case InputActionType::Axis3D:
				value.axis3DValue *= -1.f;
				break;
			}
		}
	};

	class SwizzleYXModifier : public InputActionModifierBase
	{
	public:
		explicit SwizzleYXModifier()
			: InputActionModifierBase("SwizzleYX")
		{
		}

		void Evaluate(InputActionValue& value) override
		{
			switch (value.type)
			{
			case InputActionType::Boolean:
			case InputActionType::Axis1D:
				break;
			case InputActionType::Axis2D:
				value.axis2DValue = glm::vec2(value.axis2DValue.y, value.axis2DValue.x);
				break;
			case InputActionType::Axis3D:
				value.axis3DValue = glm::vec3(value.axis3DValue.y, value.axis3DValue.x, value.axis3DValue.z);
				break;
			}
		}
	};

	class DeadzoneModifier : public InputActionModifierBase
	{
	public:
		explicit DeadzoneModifier(float deadzone)
			: InputActionModifierBase("Deadzone")
			, mDeadzone(deadzone)
		{
		}

		void Evaluate(InputActionValue& value) override
		{
			if (abs(value.axis1DValue) < mDeadzone)
			{
				value.axis1DValue = 0.f;
			}

			if (abs(value.axis2DValue.x) < mDeadzone)
			{
				value.axis2DValue.x = 0.f;
			}
			if (abs(value.axis2DValue.y) < mDeadzone)
			{
				value.axis2DValue.y = 0.f;
			}

			if (abs(value.axis3DValue.x) < mDeadzone)
			{
				value.axis3DValue.x = 0.f;
			}
			if (abs(value.axis3DValue.y) < mDeadzone)
			{
				value.axis3DValue.y = 0.f;
			}
			if (abs(value.axis3DValue.z) < mDeadzone)
			{
				value.axis3DValue.z = 0.f;
			}
		}

	private:
		float mDeadzone = 0.2f;
	};

	class HoldTrigger : public InputActionTriggerBase
	{
	public:
		HoldTrigger(float holdTime = 1.f)
			: InputActionTriggerBase("Hold", TriggerType::Explicit)
			, mHoldTime(holdTime)
		{}

		bool Evaluate(const InputActionValue& value) override
		{
			if (!value.IsValueZero())
				mHoldTimer += core::Time::DeltaTime();
			else mHoldTimer = 0.f;

			return mHoldTimer > mHoldTime;
		}

		InputActionTriggerBase* Copy() const override
		{
			return new HoldTrigger(mHoldTime);
		}

	private:
		float mHoldTime = 1.f;

		float mHoldTimer = 0.f;
	};
}