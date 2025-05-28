#include "pch.h"
#include "input/basedinput.h"

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keyboard.h>

#include "basedtypes.h"
#include "input/joystick.h"

namespace based::input
{
	InputActionTriggerBase::InputActionTriggerBase(const std::string& name, TriggerType type)
		: mTriggerName(name)
		, mTriggerType(type)
	{}

	InputActionModifierBase::InputActionModifierBase(const std::string& name)
		: mModifierName(name)
	{}

	std::string InputComponent::GetMappedKeyForAction(const std::string& action)
	{
		return GetMappedKeysForAction(action, 1).front();
	}

	std::vector<std::string> InputComponent::GetMappedKeysForAction(const std::string& action, int keyCount)
	{
		std::vector<InputMappingConfig*>& configs = core::Container(mActiveMappings);
		std::vector<std::string> result;
		
		for (const auto config : configs)
		{
			for (const auto& [actionName, mappings] : config->mMappings)
			{
				if (actionName != action) continue;

				
				for (const auto& key : mappings)
				{
					switch (mInputMethod) {
					case InputMethod::KeyboardMouse:
						switch (key.type) {
						case BasedKey::Keyboard:
							if (result.size() < keyCount) 
								result.emplace_back(SDL_GetKeyName(SDL_SCANCODE_TO_KEYCODE(key.key)));
							break;
						case BasedKey::MouseButton:
							if (result.size() < keyCount)
							{
								if (key.key == 1) result.emplace_back("LeftMouse");
								if (key.key == 2) result.emplace_back("MiddleMouse");
								if (key.key == 3) result.emplace_back("RightMouse");
								if (key.key == 4) result.emplace_back("MouseX1");
								if (key.key == 5) result.emplace_back("MouseX2");
							}
							break;
						case BasedKey::MouseAxis:
							if (result.size() < keyCount)
							{
								if (key.key == 0) result.emplace_back("MouseX");
								if (key.key == 1) result.emplace_back("MouseY");
								if (key.key == 2) result.emplace_back("MouseXY");
							}
							break;
						case BasedKey::ControllerButton:
						case BasedKey::ControllerAxis:
							continue;
						}
						break;
					case InputMethod::Controller:
						switch (key.type) {
						case BasedKey::Keyboard:
						case BasedKey::MouseButton:
						case BasedKey::MouseAxis:
							continue;
						case BasedKey::ControllerButton:
							if (result.size() < keyCount)
								result.emplace_back(
									SDL_GameControllerGetStringForButton(
										static_cast<SDL_GameControllerButton>(key.key)));
							break;
						case BasedKey::ControllerAxis:
							if (result.size() < keyCount)
								result.emplace_back(SDL_GameControllerGetStringForAxis(
									static_cast<SDL_GameControllerAxis>(key.key)));
							break;
						}
						break;
					case InputMethod::Other:
						break;
					}
				}

				if (result.size() == keyCount) return result;
			}
		}

		if (result.empty()) result = { "" };
		return result;
	}

	std::string InputComponent::GetKeyImageForAction(const std::string& action,
		const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& keyMaps)
	{
		return GetKeyImagesForAction(action, 1, keyMaps).front();
	}

	std::vector<std::string> InputComponent::GetKeyImagesForAction(const std::string& action, int keyCount,
		const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& keyMaps)
	{
		auto keys = GetMappedKeysForAction(action, keyCount);
		std::vector<std::string> result;

		for (const auto& key : keys)
		{
			switch (mInputMethod) {
			case InputMethod::KeyboardMouse:
				if (keyMaps.find("KBM") != keyMaps.end())
				{
					auto& kbmMap = keyMaps.at("KBM");
					if (kbmMap.find(key) != kbmMap.end())
						result.emplace_back(kbmMap.at(key));
				}
				break;
			case InputMethod::Controller:
				{
					auto controllerType = Joystick::GetControllerType(controllerID);
					std::string mapName;
					switch (controllerType)
					{
					case Joystick::ControllerType::Xbox360:
						if (keyMaps.find("Xbox360") != keyMaps.end())
						{
							mapName = "Xbox360";
							break;
						}
					case Joystick::ControllerType::XboxOne:
						if (keyMaps.find("XboxOne") != keyMaps.end())
						{
							mapName = "XboxOne";
							break;
						}
						break;
					case Joystick::ControllerType::PS3:
						if (keyMaps.find("PS3") != keyMaps.end())
						{
							mapName = "PS3";
							break;
						}
					case Joystick::ControllerType::PS4:
						if (keyMaps.find("PS4") != keyMaps.end())
						{
							mapName = "PS4";
							break;
						}
					case Joystick::ControllerType::PS5:
						if (keyMaps.find("PS5") != keyMaps.end())
						{
							mapName = "PS5";
							break;
						}
						break;
					case Joystick::ControllerType::JoyconLeft:
						if (keyMaps.find("JoyconLeft") != keyMaps.end())
						{
							mapName = "JoyconLeft";
							break;
						}
					case Joystick::ControllerType::JoyconRight:
						if (keyMaps.find("JoyconRight") != keyMaps.end())
						{
							mapName = "JoyconRight";
							break;
						}
					case Joystick::ControllerType::JoyconPair:
						if (keyMaps.find("JoyconPair") != keyMaps.end())
						{
							mapName = "JoyconPair";
							break;
						}
					case Joystick::ControllerType::SwitchPro:
						if (keyMaps.find("SwitchPro") != keyMaps.end())
						{
							mapName = "SwitchPro";
							break;
						}
						break;
					default:
						if (keyMaps.find("Custom") != keyMaps.end())
						{
							mapName = "Custom";
							break;
						}
						break;
					}

					if (mapName.empty()) continue;

					auto& keyMap = keyMaps.at(mapName);
					if (keyMap.find(key) != keyMap.end())
						result.emplace_back(keyMap.at(key));
				}
				break;
			case InputMethod::Other:
				break;
			}
		}

		if (result.empty())
		{
			result = std::vector<std::string>(keyCount);
			std::fill(result.begin(), result.end(), "");
		}
		return result;
	}
}
