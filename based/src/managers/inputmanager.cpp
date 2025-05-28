#include "pch.h"
#include "managers/inputmanager.h"

#include "app.h"
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
		RegisterModifier(new input::DeadzoneModifier(0.2f));
		RegisterModifier(new input::NegateModifier());
		RegisterModifier(new input::SwizzleYXModifier());

		RegisterTrigger(new input::HoldTrigger());

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

				LoadActionFile(dir.path().string());
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
				value.triggered = input::Joystick::GetButton(controllerID, key.key);
				break;
			}
			break;
		case input::InputActionType::Axis1D:
			switch (key.type)
			{
			case input::BasedKey::Keyboard:
				value.axis1DValue = input::Keyboard::Key(key.key) ? 1.f : 0.f;
				break;
			case input::BasedKey::MouseButton:
			case input::BasedKey::MouseAxis:
				if (key.key == 0) value.axis1DValue = (float)(input::Mouse::DX());
				else if (key.key == 1) value.axis1DValue = (float)(-input::Mouse::DY());
				else value.axis1DValue = 0.f;
				break;
			case input::BasedKey::ControllerButton:
			case input::BasedKey::ControllerAxis:
				value.axis1DValue = input::Joystick::GetAxis(controllerID, key.key);
				break;
			}
			break;
		case input::InputActionType::Axis2D:
			switch (key.type)
			{
			case input::BasedKey::Keyboard:
				value.axis2DValue = glm::vec2(input::Keyboard::Key(key.key) ? 1.f : 0.f, 0.f);
				break;
			case input::BasedKey::MouseButton:
			case input::BasedKey::MouseAxis:
				value.axis2DValue = glm::vec2(input::Mouse::DX(), -input::Mouse::DY());
				break;
			case input::BasedKey::ControllerButton:
			case input::BasedKey::ControllerAxis:
				if (key.key == (int)input::Joystick::Axis::LeftStickHorizontal 
					|| key.key == (int)input::Joystick::Axis::LeftStickVertical)
				{
					value.axis2DValue = glm::vec2(
						input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickHorizontal),
						-input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickVertical));
				} else if (key.key == (int)input::Joystick::Axis::RightStickHorizontal
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
			case input::BasedKey::MouseButton:
			case input::BasedKey::MouseAxis:
				value.axis3DValue = glm::vec3(input::Mouse::DX(), -input::Mouse::DY(), 0.f);
				break;
			case input::BasedKey::ControllerButton:
			case input::BasedKey::ControllerAxis:
				if (key.key == (int)input::Joystick::Axis::LeftStickHorizontal
					|| key.key == (int)input::Joystick::Axis::LeftStickVertical)
				{
					value.axis3DValue = glm::vec3(
						input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickHorizontal),
						-input::Joystick::GetAxis(controllerID, input::Joystick::Axis::LeftStickVertical),
						0.f);
				}
				else if (key.key == (int)input::Joystick::Axis::RightStickHorizontal
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

	bool ShouldActionTrigger(input::InputAction& inputAction, const input::InputActionValue& value, const input::BasedKey& key)
	{
		bool shouldTrigger = !key.mTriggerOverrides.empty() || !inputAction.mTriggers.empty() ? false : !value.IsValueZero();
		int implicitCount = 0;
		int successfulImplicits = 0;
		if (!key.mTriggerOverrides.empty())
		{
			for (auto& trigger : key.mTriggerOverrides)
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
		auto& registry = Engine::Instance().GetApp().GetCurrentScene()->GetRegistry();
		auto inputView = registry.view<input::InputComponent>();

		for (const auto& e : inputView)
		{
			auto& inputComp = inputView.get<input::InputComponent>(e);
			auto& activeMappings = inputComp.mActiveMappings;
			std::vector<input::InputMappingConfig*> tempStorage;
			std::vector <input::BasedKey> consumedKeys;

			while (!activeMappings.empty())
			{
				auto config = activeMappings.top();
				activeMappings.pop();
				if (!config) continue;

				tempStorage.push_back(config);
			}

			for (auto& config : tempStorage)
			{
				activeMappings.push(config);
			}

			for (auto& config : tempStorage)
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

					for (auto& key : mapping)
					{
						input::InputActionValue keyRaw;
						keyRaw.type = accumulatedUnmodified.type;
						bool didConsume = GetRawKeyValue(keyRaw, key, consumedKeys, 
							inputAction.type, inputComp.controllerID, 
							inputAction.consumeInput);
						accumulatedUnmodified = accumulatedUnmodified + keyRaw;

						input::InputActionValue value = keyRaw;

						if (!key.mModifierOverrides.empty() && !keyRaw.IsValueZero())
						{
							for (auto& modifier : key.mModifierOverrides)
							{
								if (!modifier) continue;

								modifier->Evaluate(value);
							}
						} else if (!keyRaw.IsValueZero())
						{
							for (auto& modifier : inputAction.mModifiers)
							{
								if (!modifier) continue;

								modifier->Evaluate(value);
							}
						}

						accumulatedModified = accumulatedModified + value;

						shouldTrigger = ShouldActionTrigger(inputAction, accumulatedModified, key);

						if (!value.IsValueZero())
						{
							switch (key.type) {
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
			auto type = (input::InputActionType)action["Type"].as<int>();
			auto triggerPaused = action["TriggerWhenPaused"].as<bool>();
			auto consumeInput = action["ConsumeInput"].as<bool>();

			if (mInputActions.find(name) != mInputActions.end()) continue;

			input::InputAction inputAction(name, type, triggerPaused);
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

		auto configs = data["Configs"];
		if (configs)
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
							input::BasedKey key;
							key.key = mappedKey["Key"].as<uint32_t>();
							key.type = (input::BasedKey::KeyType)mappedKey["Type"].as<int>();

							for (const auto& trigger : mappedKey["Triggers"])
							{
								auto triggerName = trigger.as<std::string>();
								auto iter = mTriggers.find(triggerName);
								if (iter != mTriggers.end())
								{
									auto actionTrigger = mTriggers[triggerName];
									key.mTriggerOverrides.push_back(actionTrigger->Copy());
								}
							}

							for (const auto& modifier : mappedKey["Modifiers"])
							{
								auto modifierName = modifier.as<std::string>();
								auto iter = mModifiers.find(modifierName);
								if (iter != mModifiers.end())
								{
									auto actionModifier = mModifiers[modifierName];
									key.mModifierOverrides.push_back(actionModifier);
								}
							}

							if (inputConfig.mMappings.find(actionName) == inputConfig.mMappings.end())
							{
								inputConfig.mMappings.emplace(actionName, std::list{ key });
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
