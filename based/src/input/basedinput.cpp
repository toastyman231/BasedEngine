#include "pch.h"
#include "input/basedinput.h"
#include "input/basedinput.h"
#include "input/basedinput.h"
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
					/*switch (mInputMethod) {
					case InputMethod::KeyboardMouse:
						switch (key->type) {
						case BasedKey::Keyboard:
							if (result.size() < keyCount) 
								result.emplace_back(SDL_GetKeyName(SDL_SCANCODE_TO_KEYCODE(key->key)));
							break;
						case BasedKey::MouseButton:
							if (result.size() < keyCount)
							{
								if (key->key == 1) result.emplace_back("LeftMouse");
								if (key->key == 2) result.emplace_back("MiddleMouse");
								if (key->key == 3) result.emplace_back("RightMouse");
								if (key->key == 4) result.emplace_back("MouseX1");
								if (key->key == 5) result.emplace_back("MouseX2");
							}
							break;
						case BasedKey::MouseAxis:
							if (result.size() < keyCount)
							{
								if (key->key == 0) result.emplace_back("MouseX");
								if (key->key == 1) result.emplace_back("MouseY");
								if (key->key == 2) result.emplace_back("MouseXY");
							}
							break;
						case BasedKey::ControllerButton:
						case BasedKey::ControllerAxis:
							continue;
						}
						break;
					case InputMethod::Controller:
						switch (key->type) {
						case BasedKey::Keyboard:
						case BasedKey::MouseButton:
						case BasedKey::MouseAxis:
							continue;
						case BasedKey::ControllerButton:
							if (result.size() < keyCount)
								result.emplace_back(
									SDL_GameControllerGetStringForButton(
										static_cast<SDL_GameControllerButton>(key->key)));
							break;
						case BasedKey::ControllerAxis:
							if (result.size() < keyCount)
								result.emplace_back(SDL_GameControllerGetStringForAxis(
									static_cast<SDL_GameControllerAxis>(key->key)));
							break;
						}
						break;
					case InputMethod::Other:
						break;
					}*/
					result.emplace_back(key->keyName);
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

	void Keys::AddKey(const std::string& keyName, const BasedKey& key)
	{
		if (mInputKeys.find(keyName) != mInputKeys.end())
		{
			BASED_WARN("Cannot add duplicate key {}!", keyName);
			return;
		}

		mInputKeys[keyName] = std::make_shared<BasedKey>(key);
	}

	std::shared_ptr<BasedKey> Keys::GetKey(const std::string& keyName)
	{
		if (auto keyIter = mInputKeys.find(keyName);
			keyIter != mInputKeys.end() && keyIter->second)
		{
			return keyIter->second;
		}

		return mInputKeys.find("INVALID_KEY")->second;
	}

	const BasedKey Keys::MouseX{ 0, "MouseX", BasedKey::KeyType::MouseAxis };
	const BasedKey Keys::MouseY{ 1, "MouseY", BasedKey::KeyType::MouseAxis };
	const BasedKey Keys::Mouse2D{ 2, "Mouse2D", BasedKey::KeyType::MouseAxis };
	const BasedKey Keys::MouseScrollUp{3, "MouseScrollUp", BasedKey::KeyType::MouseAxis };
	const BasedKey Keys::MouseScrollDown{4, "MouseScrollDown", BasedKey::KeyType::MouseAxis };
	const BasedKey Keys::MouseWheelAxis{ 5, "MouseWheelAxis", BasedKey::KeyType::MouseAxis };

	const BasedKey Keys::LeftMouseButton{ 1, "LeftMouseButton", BasedKey::KeyType::MouseButton };
	const BasedKey Keys::RightMouseButton{ 2, "RightMouseButton", BasedKey::KeyType::MouseButton };
	const BasedKey Keys::MiddleMouseButton{ 3, "MiddleMouseButton", BasedKey::KeyType::MouseButton };
	const BasedKey Keys::ThumbMouseButton{ 4, "ThumbMouseButton", BasedKey::KeyType::MouseButton };
	const BasedKey Keys::ThumbMouseButton2{ 5, "ThumbMouseButton2", BasedKey::KeyType::MouseButton };

	const BasedKey Keys::BackSpace{ 42, "BackSpace", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Tab{ 43, "Tab", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Enter{ 40, "Enter", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Pause{ 72, "Pause", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::CapsLock{ 57, "CapsLock", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Escape{ 41, "Escape", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::SpaceBar{ 44, "SpaceBar", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::PageUp{ 75, "PageUp", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::PageDown{ 78, "PageDown", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::End{ 77, "End", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Home{ 74, "Home", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::Left{ 80, "Left", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Up{ 82, "Up", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Right{ 79, "Right", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Down{ 81, "Down", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::Insert{ 73, "Insert", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Delete{ 76, "Delete", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::One{ 30, "One", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Two{ 31, "Two", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Three{ 32, "Three", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Four{ 33, "Four", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Five{ 34, "Five", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Six{ 35, "Six", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Seven{ 36, "Seven", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Eight{ 37, "Eight", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Nine{ 38, "Nine", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Zero{ 39, "Zero", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::A{ 4, "A", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::B{ 5, "B", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::C{ 6, "C", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::D{ 7, "D", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::E{ 8, "E", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F{ 9, "F", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::G{ 10, "G", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::H{ 11, "H", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::I{ 12, "I", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::J{ 13, "J", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::K{ 14, "K", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::L{ 15, "L", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::M{ 16, "M", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::N{ 17, "N", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::O{ 18, "O", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::P{ 19, "P", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Q{ 20, "Q", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::R{ 21, "R", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::S{ 22, "S", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::T{ 23, "T", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::U{ 24, "U", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::V{ 25, "V", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::W{ 26, "W", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::X{ 27, "X", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Y{ 28, "Y", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Z{ 29, "Z", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::NumPadOne{ 89, "NumPadOne", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadTwo{ 90, "NumPadTwo", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadThree{ 91, "NumPadThree", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadFour{ 92, "NumPadFour", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadFive{ 93, "NumPadFive", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadSix{ 94, "NumPadSix", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadSeven{ 95, "NumPadSeven", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadEight{ 96, "NumPadEight", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadNine{ 97, "NumPadNine", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::NumPadZero{ 98, "NumPadZero", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::Multiply{ 85, "Multiply", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Add{ 87, "Add", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Subtract{ 86, "Subtract", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Decimal{ 99, "Decimal", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Divide{ 84, "Divide", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::F1{ 58, "F1", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F2{ 59, "F2", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F3{ 60, "F3", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F4{ 61, "F4", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F5{ 62, "F5", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F6{ 63, "F6", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F7{ 64, "F7", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F8{ 65, "F8", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F9{ 66, "F9", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F10{ 67, "F10", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F11{ 68, "F11", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::F12{ 69, "F12", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::NumLock{ 83, "NumLock", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::ScrollLock{ 71, "ScrollLock", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::LeftShift{ 225, "LeftShift", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::RightShift{ 229, "RightShift", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::LeftControl{ 224, "LeftControl", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::RightControl{ 228, "RightControl", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::LeftAlt{ 226, "LeftAlt", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::RightAlt{ 230, "RightAlt", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::LeftCommand{ 227, "LeftCommand", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::RightCommand{ 231, "RightCommand", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::Semicolon{ 51, "Semicolon", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Equals{ 46, "Equals", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Comma{ 54, "Comma", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Hyphen{ 45, "Hyphen", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Period{ 55, "Period", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Slash{ 56, "Slash", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Grave{ 53, "Grave", BasedKey::KeyType::Keyboard }; // Unlike Unreal, this is not a tilde
	const BasedKey Keys::LeftBracket{ 47, "LeftBracket", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Backslash{ 49, "Backslash", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::RightBracket{ 48, "RightBracket", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Apostrophe{ 52, "Apostrophe", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::Ampersand{ 199, "Ampersand", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Colon{ 203, "Colon", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::Exclamation{ 207, "Exclamation", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::LeftParantheses{ 182, "LeftParantheses", BasedKey::KeyType::Keyboard };
	const BasedKey Keys::RightParantheses{ 183, "RightParantheses", BasedKey::KeyType::Keyboard };

	const BasedKey Keys::Gamepad_Left2D{ 6, "Gamepad_Left2D", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_LeftX{ 0, "Gamepad_LeftX", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_LeftY{ 1, "Gamepad_LeftY", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_Right2D{ 7, "Gamepad_Right2D", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_RightX{ 2, "Gamepad_RightX", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_RightY{ 3, "Gamepad_RightY", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_LeftTriggerAxis{ 4, "Gamepad_LeftTriggerAxis", BasedKey::KeyType::ControllerAxis };
	const BasedKey Keys::Gamepad_RightTriggerAxis{ 5, "Gamepad_RightTriggerAxis", BasedKey::KeyType::ControllerAxis };

	const BasedKey Keys::Gamepad_LeftThumbstick{ 7, "Gamepad_LeftThumbstick", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_RightThumbstick{ 8, "Gamepad_RightThumbstick", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_Special_Left{ 4, "Gamepad_Special_Left", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_Guide{ 5, "Gamepad_Guide", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_Special_Right{ 6, "Gamepad_Special_Right", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_FaceButton_Bottom{ 0, "Gamepad_FaceButton_Bottom", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_FaceButton_Right{ 1, "Gamepad_FaceButton_Right", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_FaceButton_Left{ 2, "Gamepad_FaceButton_Left", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_FaceButton_Top{ 3, "Gamepad_FaceButton_Top", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_LeftShoulder{ 9, "Gamepad_LeftShoulder", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_RightShoulder{ 10, "Gamepad_RightShoulder", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_LeftTriggerButton{ 15, "Gamepad_LeftTriggerButton", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_RightTriggerButton{ 16, "Gamepad_RightTriggerButton", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_DPad_Up{ 11, "Gamepad_DPad_Up", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_DPad_Down{ 12, "Gamepad_DPad_Down", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_DPad_Right{ 13, "Gamepad_DPad_Right", BasedKey::KeyType::ControllerButton };
	const BasedKey Keys::Gamepad_DPad_Left{ 14, "Gamepad_DPad_Left", BasedKey::KeyType::ControllerButton };

	const BasedKey Keys::InvalidKey{ 9999, "INVALID_KEY", BasedKey::KeyType::Keyboard };

	std::unordered_map<std::string, std::shared_ptr<BasedKey>> Keys::mInputKeys = {};
}
