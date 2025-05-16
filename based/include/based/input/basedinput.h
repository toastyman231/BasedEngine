#pragma once

namespace based::managers
{
	class InputManager;
}

namespace based::input
{
	enum class InputActionType
	{
		Boolean,
		Axis1D,
		Axis2D,
		Axis3D
	};

	struct InputActionValue
	{
		InputActionType type = InputActionType::Boolean;
		bool triggered = false;
		float axis1DValue = 0.f;
		glm::vec2 axis2DValue = glm::vec2(0.f);
		glm::vec3 axis3DValue = glm::vec3(0.f);

		bool IsValueZero() const
		{
			switch (type)
			{
			case InputActionType::Boolean:
				return !triggered;
			case InputActionType::Axis1D:
				return axis1DValue == 0.f;
			case InputActionType::Axis2D:
				return axis2DValue == glm::vec2(0.f);
			case InputActionType::Axis3D:
				return axis3DValue == glm::vec3(0.f);
			default:
				return true; // Should never happen
			}
		}

		InputActionValue operator+(const InputActionValue& other) const
		{
			InputActionValue value;
			value.type = type;
			value.triggered = triggered || other.triggered;
			value.axis1DValue = std::clamp(axis1DValue + other.axis1DValue, -1.f, 1.f);
			value.axis2DValue = glm::clamp(axis2DValue + other.axis2DValue, -1.f, 1.f);
			value.axis3DValue = glm::clamp(axis3DValue + other.axis3DValue, -1.f, 1.f);
			return value;
		}
	};

	class InputActionTriggerBase
	{
	public:
		enum class TriggerType : uint8_t // Replicating Unreal Engine 5 functionality
		{
			Implicit,
			Explicit,
			Blocker
		};

		InputActionTriggerBase(const std::string& name, TriggerType type);
		virtual ~InputActionTriggerBase() = default;

		virtual bool Evaluate(const InputActionValue& value) = 0;
		virtual InputActionTriggerBase* Copy() const = 0;

		std::string GetName() const { return mTriggerName; }
		TriggerType GetType() const { return mTriggerType; }
	private:
		std::string mTriggerName;
		TriggerType mTriggerType;
	};

	class InputActionModifierBase
	{
	public:
		InputActionModifierBase(const std::string& name);
		virtual ~InputActionModifierBase() = default;

		virtual void Evaluate(InputActionValue& value) = 0;

		std::string GetName() const { return mModifierName; }
	private:
		std::string mModifierName;
	};

	enum class InputState
	{
		Inactive = -1,
		Triggered,
		Started,
		Ongoing,
		Completed,
		Canceled
	};

	struct InputMappingConfig;
	struct InputAction
	{
		InputAction() = default;
		InputAction(const std::string& actionName, InputActionType actionType, bool triggerPaused = false)
			: name(actionName), type(actionType), triggerWhenPaused(triggerPaused) {}

		std::string name;
		InputActionType type;
		bool consumeInput = true;
		bool triggerWhenPaused;
		InputState state = InputState::Inactive;
		std::list<InputActionTriggerBase*> mTriggers = {};
		std::list<InputActionModifierBase*> mModifiers = {};

		InputActionValue GetValue() const { return mCurrentValue; }
		InputActionValue GetRawValue() const { return mRawValue; }

		friend class managers::InputManager;
	private:
		InputActionValue mRawValue = {};
		InputActionValue mCurrentValue = {};
		InputMappingConfig* mConsumedConfig = nullptr;
	};

	struct BasedKey
	{
		int key;
		enum KeyType
		{
			Keyboard,
			Mouse,
			Controller
		} type;

		std::list<InputActionTriggerBase*> mTriggerOverrides = {};
		std::list<InputActionModifierBase*> mModifierOverrides = {};
	};

	struct InputMappingConfig
	{
		std::string name;
		int priority = 0;
		std::unordered_map<std::string, std::list<BasedKey>> mMappings;
	};

	struct InputConfigComparator
	{
		bool operator()(const InputMappingConfig* l, const InputMappingConfig* r) const
		{
			return l->priority > r->priority;
		}
	};

	struct InputComponent
	{
		int controllerID = 0;
		std::priority_queue<InputMappingConfig*, std::vector<InputMappingConfig*>, InputConfigComparator> mActiveMappings;

		entt::delegate<void(const InputAction&)> mStartedEvent;
		entt::delegate<void(const InputAction&)> mTriggeredEvent;
		entt::delegate<void(const InputAction&)> mOngoingEvent;
		entt::delegate<void(const InputAction&)> mCompletedEvent;
		entt::delegate<void(const InputAction&)> mCanceledEvent;
	};
}
