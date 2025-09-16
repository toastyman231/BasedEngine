#include "pch.h"
#include "managers/inputmanager.h"

#include "app.h"
#include "basedtypes.h"
#include "engine.h"
#include "core/serializer.h"
#include "input/inputmodifiers.h"
#include "input/joystick.h"
#include "input/keyboard.h"
#include "input/mouse.h"

namespace based::managers
{
	void InputManager::Initialize()
	{
		PROFILE_FUNCTION();
		RegisterModifier(new input::DeadzoneModifier(0.2f));
		RegisterModifier(new input::NegateModifier());
		RegisterModifier(new input::SwizzleYXModifier());

		RegisterTrigger(new input::HoldTrigger());

		input::Keys::AddKey(input::Keys::MouseX.keyName, input::Keys::MouseX);
		input::Keys::AddKey(input::Keys::MouseY.keyName, input::Keys::MouseY);
		input::Keys::AddKey(input::Keys::Mouse2D.keyName, input::Keys::Mouse2D);
		input::Keys::AddKey(input::Keys::MouseScrollUp.keyName, input::Keys::MouseScrollUp);
		input::Keys::AddKey(input::Keys::MouseScrollDown.keyName, input::Keys::MouseScrollDown);
		input::Keys::AddKey(input::Keys::MouseWheelAxis.keyName, input::Keys::MouseWheelAxis);

		input::Keys::AddKey(input::Keys::LeftMouseButton.keyName, input::Keys::LeftMouseButton);
		input::Keys::AddKey(input::Keys::RightMouseButton.keyName, input::Keys::RightMouseButton);
		input::Keys::AddKey(input::Keys::MiddleMouseButton.keyName, input::Keys::MiddleMouseButton);
		input::Keys::AddKey(input::Keys::ThumbMouseButton.keyName, input::Keys::ThumbMouseButton);
		input::Keys::AddKey(input::Keys::ThumbMouseButton2.keyName, input::Keys::ThumbMouseButton2);

		input::Keys::AddKey(input::Keys::BackSpace.keyName, input::Keys::BackSpace);
		input::Keys::AddKey(input::Keys::Tab.keyName, input::Keys::Tab);
		input::Keys::AddKey(input::Keys::Enter.keyName, input::Keys::Enter);
		input::Keys::AddKey(input::Keys::Pause.keyName, input::Keys::Pause);

		input::Keys::AddKey(input::Keys::CapsLock.keyName, input::Keys::CapsLock);
		input::Keys::AddKey(input::Keys::Escape.keyName, input::Keys::Escape);
		input::Keys::AddKey(input::Keys::SpaceBar.keyName, input::Keys::SpaceBar);
		input::Keys::AddKey(input::Keys::PageUp.keyName, input::Keys::PageUp);
		input::Keys::AddKey(input::Keys::PageDown.keyName, input::Keys::PageDown);
		input::Keys::AddKey(input::Keys::End.keyName, input::Keys::End);
		input::Keys::AddKey(input::Keys::Home.keyName, input::Keys::Home);

		input::Keys::AddKey(input::Keys::Left.keyName, input::Keys::Left);
		input::Keys::AddKey(input::Keys::Up.keyName, input::Keys::Up);
		input::Keys::AddKey(input::Keys::Right.keyName, input::Keys::Right);
		input::Keys::AddKey(input::Keys::Down.keyName, input::Keys::Down);

		input::Keys::AddKey(input::Keys::Insert.keyName, input::Keys::Insert);
		input::Keys::AddKey(input::Keys::Delete.keyName, input::Keys::Delete);

		input::Keys::AddKey(input::Keys::Zero.keyName, input::Keys::Zero);
		input::Keys::AddKey(input::Keys::One.keyName, input::Keys::One);
		input::Keys::AddKey(input::Keys::Two.keyName, input::Keys::Two);
		input::Keys::AddKey(input::Keys::Three.keyName, input::Keys::Three);
		input::Keys::AddKey(input::Keys::Four.keyName, input::Keys::Four);
		input::Keys::AddKey(input::Keys::Five.keyName, input::Keys::Five);
		input::Keys::AddKey(input::Keys::Six.keyName, input::Keys::Six);
		input::Keys::AddKey(input::Keys::Seven.keyName, input::Keys::Seven);
		input::Keys::AddKey(input::Keys::Eight.keyName, input::Keys::Eight);
		input::Keys::AddKey(input::Keys::Nine.keyName, input::Keys::Nine);

		input::Keys::AddKey(input::Keys::A.keyName, input::Keys::A);
		input::Keys::AddKey(input::Keys::B.keyName, input::Keys::B);
		input::Keys::AddKey(input::Keys::C.keyName, input::Keys::C);
		input::Keys::AddKey(input::Keys::D.keyName, input::Keys::D);
		input::Keys::AddKey(input::Keys::E.keyName, input::Keys::E);
		input::Keys::AddKey(input::Keys::F.keyName, input::Keys::F);
		input::Keys::AddKey(input::Keys::G.keyName, input::Keys::G);
		input::Keys::AddKey(input::Keys::H.keyName, input::Keys::H);
		input::Keys::AddKey(input::Keys::I.keyName, input::Keys::I);
		input::Keys::AddKey(input::Keys::J.keyName, input::Keys::J);
		input::Keys::AddKey(input::Keys::K.keyName, input::Keys::K);
		input::Keys::AddKey(input::Keys::L.keyName, input::Keys::L);
		input::Keys::AddKey(input::Keys::M.keyName, input::Keys::M);
		input::Keys::AddKey(input::Keys::N.keyName, input::Keys::N);
		input::Keys::AddKey(input::Keys::O.keyName, input::Keys::O);
		input::Keys::AddKey(input::Keys::P.keyName, input::Keys::P);
		input::Keys::AddKey(input::Keys::Q.keyName, input::Keys::Q);
		input::Keys::AddKey(input::Keys::R.keyName, input::Keys::R);
		input::Keys::AddKey(input::Keys::S.keyName, input::Keys::S);
		input::Keys::AddKey(input::Keys::T.keyName, input::Keys::T);
		input::Keys::AddKey(input::Keys::U.keyName, input::Keys::U);
		input::Keys::AddKey(input::Keys::V.keyName, input::Keys::V);
		input::Keys::AddKey(input::Keys::W.keyName, input::Keys::W);
		input::Keys::AddKey(input::Keys::X.keyName, input::Keys::X);
		input::Keys::AddKey(input::Keys::Y.keyName, input::Keys::Y);
		input::Keys::AddKey(input::Keys::Z.keyName, input::Keys::Z);

		input::Keys::AddKey(input::Keys::NumPadZero.keyName, input::Keys::NumPadZero);
		input::Keys::AddKey(input::Keys::NumPadOne.keyName, input::Keys::NumPadOne);
		input::Keys::AddKey(input::Keys::NumPadTwo.keyName, input::Keys::NumPadTwo);
		input::Keys::AddKey(input::Keys::NumPadThree.keyName, input::Keys::NumPadThree);
		input::Keys::AddKey(input::Keys::NumPadFour.keyName, input::Keys::NumPadFour);
		input::Keys::AddKey(input::Keys::NumPadFive.keyName, input::Keys::NumPadFive);
		input::Keys::AddKey(input::Keys::NumPadSix.keyName, input::Keys::NumPadSix);
		input::Keys::AddKey(input::Keys::NumPadSeven.keyName, input::Keys::NumPadSeven);
		input::Keys::AddKey(input::Keys::NumPadEight.keyName, input::Keys::NumPadEight);
		input::Keys::AddKey(input::Keys::NumPadNine.keyName, input::Keys::NumPadNine);

		input::Keys::AddKey(input::Keys::Multiply.keyName, input::Keys::Multiply);
		input::Keys::AddKey(input::Keys::Add.keyName, input::Keys::Add);
		input::Keys::AddKey(input::Keys::Subtract.keyName, input::Keys::Subtract);
		input::Keys::AddKey(input::Keys::Decimal.keyName, input::Keys::Decimal);
		input::Keys::AddKey(input::Keys::Divide.keyName, input::Keys::Divide);

		input::Keys::AddKey(input::Keys::F1.keyName, input::Keys::F1);
		input::Keys::AddKey(input::Keys::F2.keyName, input::Keys::F2);
		input::Keys::AddKey(input::Keys::F3.keyName, input::Keys::F3);
		input::Keys::AddKey(input::Keys::F4.keyName, input::Keys::F4);
		input::Keys::AddKey(input::Keys::F5.keyName, input::Keys::F5);
		input::Keys::AddKey(input::Keys::F6.keyName, input::Keys::F6);
		input::Keys::AddKey(input::Keys::F7.keyName, input::Keys::F7);
		input::Keys::AddKey(input::Keys::F8.keyName, input::Keys::F8);
		input::Keys::AddKey(input::Keys::F9.keyName, input::Keys::F9);
		input::Keys::AddKey(input::Keys::F10.keyName, input::Keys::F10);
		input::Keys::AddKey(input::Keys::F11.keyName, input::Keys::F11);
		input::Keys::AddKey(input::Keys::F12.keyName, input::Keys::F12);

		input::Keys::AddKey(input::Keys::NumLock.keyName, input::Keys::NumLock);
		input::Keys::AddKey(input::Keys::ScrollLock.keyName, input::Keys::ScrollLock);

		input::Keys::AddKey(input::Keys::LeftShift.keyName, input::Keys::LeftShift);
		input::Keys::AddKey(input::Keys::RightShift.keyName, input::Keys::RightShift);
		input::Keys::AddKey(input::Keys::LeftControl.keyName, input::Keys::LeftControl);
		input::Keys::AddKey(input::Keys::RightControl.keyName, input::Keys::RightControl);
		input::Keys::AddKey(input::Keys::LeftAlt.keyName, input::Keys::LeftAlt);
		input::Keys::AddKey(input::Keys::RightAlt.keyName, input::Keys::RightAlt);
		input::Keys::AddKey(input::Keys::LeftCommand.keyName, input::Keys::LeftCommand);
		input::Keys::AddKey(input::Keys::RightCommand.keyName, input::Keys::RightCommand);

		input::Keys::AddKey(input::Keys::Semicolon.keyName, input::Keys::Semicolon);
		input::Keys::AddKey(input::Keys::Equals.keyName, input::Keys::Equals);
		input::Keys::AddKey(input::Keys::Comma.keyName, input::Keys::Comma);
		input::Keys::AddKey(input::Keys::Hyphen.keyName, input::Keys::Hyphen);
		input::Keys::AddKey(input::Keys::Period.keyName, input::Keys::Period);
		input::Keys::AddKey(input::Keys::Slash.keyName, input::Keys::Slash);
		input::Keys::AddKey(input::Keys::Grave.keyName, input::Keys::Grave);
		input::Keys::AddKey(input::Keys::LeftBracket.keyName, input::Keys::LeftBracket);
		input::Keys::AddKey(input::Keys::Backslash.keyName, input::Keys::Backslash);
		input::Keys::AddKey(input::Keys::RightBracket.keyName, input::Keys::RightBracket);
		input::Keys::AddKey(input::Keys::Apostrophe.keyName, input::Keys::Apostrophe);

		input::Keys::AddKey(input::Keys::Ampersand.keyName, input::Keys::Ampersand);
		input::Keys::AddKey(input::Keys::Colon.keyName, input::Keys::Colon);
		input::Keys::AddKey(input::Keys::Exclamation.keyName, input::Keys::Exclamation);
		input::Keys::AddKey(input::Keys::LeftParantheses.keyName, input::Keys::LeftParantheses);
		input::Keys::AddKey(input::Keys::RightParantheses.keyName, input::Keys::RightParantheses);

		input::Keys::AddKey(input::Keys::Gamepad_Left2D.keyName, input::Keys::Gamepad_Left2D);
		input::Keys::AddKey(input::Keys::Gamepad_LeftX.keyName, input::Keys::Gamepad_LeftX);
		input::Keys::AddKey(input::Keys::Gamepad_LeftY.keyName, input::Keys::Gamepad_LeftY);
		input::Keys::AddKey(input::Keys::Gamepad_Right2D.keyName, input::Keys::Gamepad_Right2D);
		input::Keys::AddKey(input::Keys::Gamepad_RightX.keyName, input::Keys::Gamepad_RightX);
		input::Keys::AddKey(input::Keys::Gamepad_RightY.keyName, input::Keys::Gamepad_RightY);
		input::Keys::AddKey(input::Keys::Gamepad_LeftTriggerAxis.keyName, input::Keys::Gamepad_LeftTriggerAxis);
		input::Keys::AddKey(input::Keys::Gamepad_RightTriggerAxis.keyName, input::Keys::Gamepad_RightTriggerAxis);

		input::Keys::AddKey(input::Keys::Gamepad_LeftThumbstick.keyName, input::Keys::Gamepad_LeftThumbstick);
		input::Keys::AddKey(input::Keys::Gamepad_RightThumbstick.keyName, input::Keys::Gamepad_RightThumbstick);
		input::Keys::AddKey(input::Keys::Gamepad_Special_Left.keyName, input::Keys::Gamepad_Special_Left);
		input::Keys::AddKey(input::Keys::Gamepad_Special_Right.keyName, input::Keys::Gamepad_Special_Right);
		input::Keys::AddKey(input::Keys::Gamepad_FaceButton_Bottom.keyName, input::Keys::Gamepad_FaceButton_Bottom);
		input::Keys::AddKey(input::Keys::Gamepad_FaceButton_Right.keyName, input::Keys::Gamepad_FaceButton_Right);
		input::Keys::AddKey(input::Keys::Gamepad_FaceButton_Left.keyName, input::Keys::Gamepad_FaceButton_Left);
		input::Keys::AddKey(input::Keys::Gamepad_FaceButton_Top.keyName, input::Keys::Gamepad_FaceButton_Top);
		input::Keys::AddKey(input::Keys::Gamepad_LeftShoulder.keyName, input::Keys::Gamepad_LeftShoulder);
		input::Keys::AddKey(input::Keys::Gamepad_RightShoulder.keyName, input::Keys::Gamepad_RightShoulder);
		input::Keys::AddKey(input::Keys::Gamepad_LeftTriggerButton.keyName, input::Keys::Gamepad_LeftTriggerButton);
		input::Keys::AddKey(input::Keys::Gamepad_RightTriggerButton.keyName, input::Keys::Gamepad_RightTriggerButton);
		input::Keys::AddKey(input::Keys::Gamepad_DPad_Up.keyName, input::Keys::Gamepad_DPad_Up);
		input::Keys::AddKey(input::Keys::Gamepad_DPad_Down.keyName, input::Keys::Gamepad_DPad_Down);
		input::Keys::AddKey(input::Keys::Gamepad_DPad_Right.keyName, input::Keys::Gamepad_DPad_Right);
		input::Keys::AddKey(input::Keys::Gamepad_DPad_Left.keyName, input::Keys::Gamepad_DPad_Left);

		input::Keys::AddKey(input::Keys::InvalidKey.keyName, input::Keys::InvalidKey);

		std::queue<std::filesystem::path> dirs;
		if (!mPrimarySearchPath.empty())
		{
			dirs.emplace(mPrimarySearchPath);
		} else
		{
			dirs.emplace("Assets/");
		}

		while (!dirs.empty())
		{
			auto currentDir = dirs.front();
			dirs.pop();
			if (!std::filesystem::exists(currentDir)) continue;

			for (const auto& dir : std::filesystem::directory_iterator(currentDir))
			{
				if (dir.is_directory())
				{
					dirs.push(dir);
					continue;
				}

				if (dir.path().extension().string() == ".bactions") LoadActionFile(dir.path().string());
			}
		}
	}

	bool GetRawKeyValue(
		input::InputActionValue& value,
		const input::BasedKey& key, 
		std::vector<input::BasedKey>& consumedKeys,
		input::InputActionType type, int controllerID,
		bool consumeInput)
	{
		if (std::find(consumedKeys.begin(), consumedKeys.end(), key) != consumedKeys.end()) 
			return true;

		switch (type) {
		case input::InputActionType::Boolean:
			switch (key.type)
			{
			case input::BasedKey::Keyboard:
				value.triggered = input::Keyboard::Key(key.key);
				break;
			case input::BasedKey::MouseAxis:
			case input::BasedKey::MouseButton:
				value.triggered = input::Mouse::Button(key.key);
				break;
			case input::BasedKey::ControllerAxis:
			case input::BasedKey::ControllerButton:
				if (key.key == 15)
					value.triggered = input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftTrigger) > 0.6f;
				else if (key.key == 16)
					value.triggered = input::Joystick::GetAxis(controllerID, input::Joystick::Axis::RightTrigger) > 0.6f;
				else value.triggered = input::Joystick::GetButton(controllerID, key.key);
				break;
			}
			break;
		case input::InputActionType::Axis1D:
			switch (key.type)
			{
			case input::BasedKey::Keyboard:
				value.axis1DValue = input::Keyboard::Key(key.key) ? 1.f : 0.f;
				break;
			case input::BasedKey::ControllerButton:
			case input::BasedKey::MouseButton:
				value.axis1DValue = 0.f;
				break;
			case input::BasedKey::MouseAxis:
				if (key.key == 0) value.axis1DValue = (float)(input::Mouse::DX());
				else if (key.key == 1) value.axis1DValue = (float)(-input::Mouse::DY());
				// TODO: Add scroll wheel support
				else value.axis1DValue = 0.f;
				break;
			case input::BasedKey::ControllerAxis:
				if (key.key <= 5)
					value.axis1DValue = input::Joystick::GetAxis(controllerID, key.key);
				else value.axis1DValue = 0.f;
				break;
			}
			break;
		case input::InputActionType::Axis2D:
			switch (key.type)
			{
			case input::BasedKey::Keyboard:
				value.axis2DValue = glm::vec2(input::Keyboard::Key(key.key) ? 1.f : 0.f, 0.f);
				break;
			case input::BasedKey::ControllerButton:
			case input::BasedKey::MouseButton:
				value.axis2DValue = glm::vec2(0.f);
				break;
			case input::BasedKey::MouseAxis:
				value.axis2DValue = glm::vec2(input::Mouse::DX(), -input::Mouse::DY());
				break;
		case input::BasedKey::ControllerAxis:
				if (key.key == 6
					|| key.key == (int)input::Joystick::Axis::LeftStickHorizontal 
					|| key.key == (int)input::Joystick::Axis::LeftStickVertical)
				{
					value.axis2DValue = glm::vec2(
						input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickHorizontal),
						-input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickVertical));
				} else if (key.key == 7
					|| key.key == (int)input::Joystick::Axis::RightStickHorizontal
					|| key.key == (int)input::Joystick::Axis::RightStickVertical)
				{
					value.axis2DValue = glm::vec2(
						input::Joystick::GetAxis(controllerID, input::Joystick::Axis::RightStickHorizontal),
						-input::Joystick::GetAxis(controllerID, input::Joystick::Axis::RightStickVertical));
				} else
				{
					value.axis2DValue = glm::vec2(
						input::Joystick::GetAxis(controllerID, key.key),
						0.f);
				}
				break;
			}
			break;
		case input::InputActionType::Axis3D:
			switch (key.type)
			{
			case input::BasedKey::Keyboard:
				value.axis3DValue = glm::vec3(input::Keyboard::Key(key.key) ? 1.f : 0.f, 0.f, 0.f);
				break;
			case input::BasedKey::ControllerButton:
			case input::BasedKey::MouseButton:
				value.axis3DValue = glm::vec3(0.f);
				break;
			case input::BasedKey::MouseAxis:
				value.axis3DValue = glm::vec3(input::Mouse::DX(), -input::Mouse::DY(), 0.f);
				break;
			case input::BasedKey::ControllerAxis:
				if (key.key == 6
					|| key.key == (int)input::Joystick::Axis::LeftStickHorizontal
					|| key.key == (int)input::Joystick::Axis::LeftStickVertical)
				{
					value.axis3DValue = glm::vec3(
						input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickHorizontal),
						-input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickVertical),
						0.f);
				}
				else if (key.key == 7
					|| key.key == (int)input::Joystick::Axis::RightStickHorizontal
					|| key.key == (int)input::Joystick::Axis::RightStickVertical)
				{
					value.axis3DValue = glm::vec3(
						input::Joystick::GetAxis(controllerID, input::Joystick::Axis::RightStickHorizontal),
					-	input::Joystick::GetAxis(controllerID, input::Joystick::Axis::RightStickVertical),
						0.f);
				}
				else
				{
					value.axis3DValue = glm::vec3(
						input::Joystick::GetAxis(controllerID, key.key),
						0.f, 0.f);
				}
				break;
			}
			break;
		}

		if (consumeInput && !value.IsValueZero())
		{
			consumedKeys.emplace_back(key);
			//BASED_TRACE("CONSUMED KEY {}", key.key);
			return true;
		}
		return false;
	}

	bool ShouldActionTrigger(
		const input::InputMappingConfig* inputConfig,
		input::InputAction& inputAction, 
		const input::InputActionValue& value, 
		const std::shared_ptr<input::BasedKey>& key)
	{
		auto overrides = inputConfig->mTriggerOverrides.find(key);
		bool hasTriggerOverrides = overrides != inputConfig->mTriggerOverrides.end() && !overrides->second.empty();
		bool shouldTrigger = hasTriggerOverrides || !inputAction.mTriggers.empty() ? false : !value.IsValueZero();
		int implicitCount = 0;
		int successfulImplicits = 0;
		if (hasTriggerOverrides)
		{
			for (input::InputActionTriggerBase* trigger : overrides->second)
			{
				if (!trigger) continue;

				bool res = trigger->Evaluate(value);
				if (res && trigger->GetType() == input::InputActionTriggerBase::TriggerType::Explicit)
				{
					shouldTrigger = true;
					break;
				}
				if (res && trigger->GetType() == input::InputActionTriggerBase::TriggerType::Blocker)
				{
					shouldTrigger = false;
					break;
				}
				if (trigger->GetType() == input::InputActionTriggerBase::TriggerType::Implicit)
				{
					implicitCount++;
					if (res) successfulImplicits++;
				}
			}

			if (implicitCount > 0 && successfulImplicits >= implicitCount) shouldTrigger = true;
		}
		else
		{
			for (auto& trigger : inputAction.mTriggers)
			{
				if (!trigger) continue;

				bool res = trigger->Evaluate(value);
				if (res && trigger->GetType() == input::InputActionTriggerBase::TriggerType::Explicit)
				{
					shouldTrigger = true;
					break;
				}
				if (res && trigger->GetType() == input::InputActionTriggerBase::TriggerType::Blocker)
				{
					shouldTrigger = false;
					break;
				}
				if (trigger->GetType() == input::InputActionTriggerBase::TriggerType::Implicit)
				{
					implicitCount++;
					if (res) successfulImplicits++;
				}
			}

			if (implicitCount > 0 && successfulImplicits >= implicitCount) shouldTrigger = true;
		}

		return shouldTrigger;
	}

	void InputManager::Update()
	{
		PROFILE_FUNCTION();
		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		auto inputView = registry.view<input::InputComponent>();

		for (const auto& e : inputView)
		{
			auto& inputComp = inputView.get<input::InputComponent>(e);
			auto& activeMappings = inputComp.mActiveMappings;
			std::vector<input::InputMappingConfig*> configs = core::Container(activeMappings);
			std::vector <input::BasedKey> consumedKeys;

			for (input::InputMappingConfig* config : configs)
			{
				for (auto& [actionName, mapping] : config->mMappings)
				{
					auto& inputAction = mInputActions[actionName];
					if (!inputAction.triggerWhenPaused && core::Time::TimeScale() == 0.f)
					{
						inputAction.mCurrentValue = inputAction.mRawValue = input::InputActionValue();
						inputAction.state = input::InputState::Inactive;
						continue;
					}

					input::InputActionValue accumulatedUnmodified;
					input::InputActionValue accumulatedModified;
					accumulatedUnmodified.type = inputAction.type;
					accumulatedModified.type = inputAction.type;

					bool shouldTrigger = false;

					for (auto key : mapping)
					{
						if (!key) continue;

						input::InputActionValue keyRaw;
						keyRaw.type = accumulatedUnmodified.type;
						bool didConsume = GetRawKeyValue(keyRaw, *key, consumedKeys, 
							inputAction.type, inputComp.controllerID, 
							inputAction.consumeInput);
						accumulatedUnmodified = accumulatedUnmodified + keyRaw;

						input::InputActionValue value = keyRaw;

						if (auto overrides = config->mModifierOverrides.find(key);
							overrides != config->mModifierOverrides.end()
							&& !overrides->second.empty() && !keyRaw.IsValueZero())
						{
							for (auto modifier : overrides->second)
							{
								if (!modifier) continue;

								modifier->Evaluate(value);
							}
						} else if (!keyRaw.IsValueZero())
						{
							for (auto modifier : inputAction.mModifiers)
							{
								if (!modifier) continue;

								modifier->Evaluate(value);
							}
						}

						accumulatedModified = accumulatedModified + value;

						shouldTrigger = ShouldActionTrigger(config, inputAction, accumulatedModified, key);

						if (!value.IsValueZero())
						{
							switch (key->type) {
							case input::BasedKey::Keyboard:
							case input::BasedKey::MouseButton:
							case input::BasedKey::MouseAxis:
								inputComp.mInputMethod = input::InputMethod::KeyboardMouse;
								break;
							case input::BasedKey::ControllerButton:
							case input::BasedKey::ControllerAxis:
								inputComp.mInputMethod = input::InputMethod::Controller;
								break;
							}
						}

						inputAction.mCurrentValue = accumulatedModified;
						inputAction.mRawValue = accumulatedUnmodified;
					}

					switch (inputAction.state)
					{
					case input::InputState::Inactive:
						if (inputAction.GetValue().IsValueZero()) break;
						inputAction.state = input::InputState::Started;
						inputComp.mStartedEvent.trigger(inputAction);
						if (shouldTrigger)
						{
							inputAction.state = input::InputState::Triggered;
						}
						else
						{
							inputAction.state = input::InputState::Ongoing;
						}
						break;
					case input::InputState::Triggered:
						if (!shouldTrigger)
						{
							inputAction.state = input::InputState::Completed;
							inputComp.mCompletedEvent.trigger(inputAction);
							break;
						}
						inputComp.mTriggeredEvent.trigger(inputAction);
						break;
					case input::InputState::Started:
						inputAction.state = input::InputState::Ongoing;
					case input::InputState::Ongoing:
						if (inputAction.GetValue().IsValueZero())
						{
							inputAction.state = input::InputState::Canceled;
							inputComp.mCanceledEvent.trigger(inputAction);
							break;
						}
						inputComp.mOngoingEvent.trigger(inputAction);
						if (shouldTrigger)
						{
							inputAction.state = input::InputState::Triggered;
						}
						break;
					case input::InputState::Completed:
					case input::InputState::Canceled:
						inputAction.state = input::InputState::Inactive;
						break;
					}
				}
			}
		}
	}

	void InputManager::Shutdown()
	{
		PROFILE_FUNCTION();
		for (auto& [id, val] : mTriggers)
		{
			delete val;
		}

		for (auto& [id, val] : mModifiers)
		{
			delete val;
		}
	}

	void InputManager::LoadActionFile(const std::string& path)
	{
		std::ifstream stream(path);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		auto actions = data["Actions"];
		if (!actions)
		{
			BASED_ERROR("Could not load actions at {}", path);
			return;
		}

		for (const auto& action : actions)
		{
			auto name = action["ActionName"].as<std::string>();
			auto type = action["Type"].as<std::string>();
			input::InputActionType actionType;
			if (type == "Boolean" || type == "boolean" || type == "Bool" || type == "bool" || type == "Button" || type == "button")
			{
				actionType = input::InputActionType::Boolean;
			}
			else if (type == "Axis1D" || type == "axis1d" || type == "axis1D" || type == "Axis1d" || type == "axis" || type == "Axis" || type == "axis1" || type == "Axis1")
			{
				actionType = input::InputActionType::Axis1D;
			}
			else if (type == "Axis2D" || type == "axis2d" || type == "axis2D" || type == "Axis2d" || type == "axis2" || type == "Axis2")
			{
				actionType = input::InputActionType::Axis2D;
			}
			else if (type == "Axis3D" || type == "axis3d" || type == "axis3D" || type == "Axis3d" || type == "axis3" || type == "Axis3")
			{
				actionType = input::InputActionType::Axis3D;
			} else
			{
				BASED_WARN("Action type {} for action {} is not a valid action type!", type, name);
				continue;
			}

			auto triggerPaused = action["TriggerWhenPaused"].as<bool>();
			auto consumeInput = action["ConsumeInput"].as<bool>();

			if (mInputActions.find(name) != mInputActions.end()) continue;

			input::InputAction inputAction(name, actionType, triggerPaused);
			inputAction.consumeInput = consumeInput;

			for (const auto& trigger : action["Triggers"])
			{
				auto triggerName = trigger.as<std::string>();
				auto iter = mTriggers.find(triggerName);
				if (iter != mTriggers.end())
				{
					auto actionTrigger = mTriggers[triggerName];
					inputAction.mTriggers.push_back(actionTrigger->Copy());
				}
			}

			for (const auto& modifier : action["Modifiers"])
			{
				auto modifierName = modifier.as<std::string>();
				auto iter = mModifiers.find(modifierName);
				if (iter != mModifiers.end())
				{
					auto actionModifier = mModifiers[modifierName];
					inputAction.mModifiers.push_back(actionModifier);
				}
			}

			mInputActions.emplace(name, inputAction);
		}

		if (auto configs = data["Configs"])
		{
			for (const auto& config : configs)
			{
				auto name = config["ConfigName"].as<std::string>();
				if (auto mappings = config["Mappings"])
				{
					input::InputMappingConfig inputConfig;
					inputConfig.name = name;

					for (const auto& mapping : mappings)
					{
						auto actionName = mapping["ActionName"].as<std::string>();

						for (const auto& mappedKey : mapping["MappedKeys"])
						{
							auto key = input::Keys::GetKey(mappedKey["Key"].as<std::string>());
							if (!key->IsValid()) continue;

							for (const auto& trigger : mappedKey["Triggers"])
							{
								auto triggerName = trigger.as<std::string>();
								auto iter = mTriggers.find(triggerName);
								if (iter != mTriggers.end())
								{
									auto actionTrigger = mTriggers[triggerName];
									if (auto overrides = inputConfig.mTriggerOverrides.find(key);
										overrides != inputConfig.mTriggerOverrides.end())
										overrides->second.emplace_back(actionTrigger->Copy());
									else inputConfig.mTriggerOverrides.emplace(key, std::vector{actionTrigger->Copy()});
								}
							}

							for (const auto& modifier : mappedKey["Modifiers"])
							{
								auto modifierName = modifier.as<std::string>();
								auto iter = mModifiers.find(modifierName);
								if (iter != mModifiers.end())
								{
									auto actionModifier = mModifiers[modifierName];
									if (auto overrides = inputConfig.mModifierOverrides.find(key);
										overrides != inputConfig.mModifierOverrides.end())
										overrides->second.emplace_back(actionModifier);
									else inputConfig.mModifierOverrides.emplace(key, std::vector{ actionModifier });
								}
							}

							if (inputConfig.mMappings.find(actionName) == inputConfig.mMappings.end())
							{
								inputConfig.mMappings.emplace(actionName, std::vector{ key });
							} else
							{
								inputConfig.mMappings[actionName].push_back(key);
							}
						}
					}

					mMappingConfigs.emplace(name, inputConfig);
				}
			}
		}
	}

	bool InputManager::AddInputMapping(input::InputComponent& input, const std::string& name, int priority)
	{
		auto iter = mMappingConfigs.find(name);
		if (iter == mMappingConfigs.end()) return false;

		auto& config = mMappingConfigs[name];
		config.priority = priority;

		input.mActiveMappings.push(&config);
		return true;
	}

	bool InputManager::RemoveInputMapping(input::InputComponent& input, const std::string& name)
	{
		std::vector<input::InputMappingConfig*> temp;
		bool result = false;
		while (!input.mActiveMappings.empty())
		{
			auto config = input.mActiveMappings.top();
			input.mActiveMappings.pop();

			if (config->name != name) temp.push_back(config);
			else result = true;
		}

		for (auto& config : temp)
		{
			input.mActiveMappings.push(config);
		}

		return result;
	}

	void InputManager::RegisterTrigger(input::InputActionTriggerBase* trigger)
	{
		mTriggers.emplace(trigger->GetName(), trigger);
	}

	void InputManager::RegisterModifier(input::InputActionModifierBase* modifier)
	{
		mModifiers.emplace(modifier->GetName(), modifier);
	}

	input::InputComponent* InputManager::GetInputComponentForPlayer(int playerId)
	{
		auto view = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry().view<scene::Enabled, input::InputComponent>();

		for (const auto& e : view)
		{
			auto& inputComp = view.get<input::InputComponent>(e);

			if (inputComp.playerID == playerId) return &inputComp;
		}

		return nullptr;
	}

	const input::InputAction& InputManager::GetAction(const std::string& name) const
	{
		// TODO: Need a safety check here
		return mInputActions.at(name);
	}
}
