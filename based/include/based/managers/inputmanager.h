#pragma once

#include "input/basedinput.h"

namespace based::managers
{
	class InputManager
	{
	public:
		void Initialize();
		void Update();
		void Shutdown();

		void LoadActionFile(const std::string& path);

		bool AddInputMapping(input::InputComponent& input, const std::string& name, int priority = 0);
		bool RemoveInputMapping(input::InputComponent& input, const std::string& name);

		void RegisterTrigger(input::InputActionTriggerBase* trigger);
		void RegisterModifier(input::InputActionModifierBase* modifier);

		static input::InputComponent* GetInputComponentForPlayer(int playerId);

		const input::InputAction& GetAction(const std::string& name) const;

	private:
		std::string mPrimarySearchPath = "Assets/Input/";
		std::unordered_map<std::string, input::InputAction> mInputActions;
		std::unordered_map<std::string, input::InputMappingConfig> mMappingConfigs;
		std::unordered_map<std::string, input::InputActionTriggerBase*> mTriggers;
		std::unordered_map<std::string, input::InputActionModifierBase*> mModifiers;
	};
}
