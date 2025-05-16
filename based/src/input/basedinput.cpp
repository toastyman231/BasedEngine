#include "pch.h"
#include "input/basedinput.h"

namespace based::input
{
	InputActionTriggerBase::InputActionTriggerBase(const std::string& name, TriggerType type)
		: mTriggerName(name)
		, mTriggerType(type)
	{}

	InputActionModifierBase::InputActionModifierBase(const std::string& name)
		: mModifierName(name)
	{}
}
