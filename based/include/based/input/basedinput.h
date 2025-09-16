#pragma once

#include "external/entt/entt.hpp"

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

	enum class InputMethod
	{
		KeyboardMouse,
		Controller,
		Other
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
			// TODO: Try switching this to max(abs)
			value.axis1DValue = axis1DValue + other.axis1DValue;
			value.axis2DValue = axis2DValue + other.axis2DValue;
			value.axis3DValue = axis3DValue + other.axis3DValue;
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

	enum class InputState : int8_t
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
		std::string keyName;
		enum KeyType : uint8_t
		{
			Keyboard,
			MouseButton,
			MouseAxis,
			ControllerButton,
			ControllerAxis
		} type;

		bool operator==(const BasedKey& other) const
		{
			return key == other.key && type == other.type;
		}

		bool IsValid() const { return keyName != "INVALID_KEY"; }
	};

	struct InputMappingConfig
	{
		std::string name;
		int priority = 0;
		std::unordered_map<std::string, std::vector<std::shared_ptr<BasedKey>>> mMappings;
		std::unordered_map<std::shared_ptr<BasedKey>, std::vector<InputActionTriggerBase*>> mTriggerOverrides = {};
		std::unordered_map<std::shared_ptr<BasedKey>, std::vector<InputActionModifierBase*>> mModifierOverrides = {};
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
		int playerID = 0;
		std::priority_queue<InputMappingConfig*, std::vector<InputMappingConfig*>, InputConfigComparator> mActiveMappings;
		InputMethod mInputMethod = InputMethod::KeyboardMouse;

		std::string GetMappedKeyForAction(const std::string& action);
		std::vector<std::string> GetMappedKeysForAction(const std::string& action, int keyCount);
		std::string GetKeyImageForAction(const std::string& action, 
			const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& keyMaps);
		std::vector<std::string> GetKeyImagesForAction(const std::string& action, int keyCount,
			const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& keyMaps);

		entt::dispatcher mStartedEvent;
		entt::dispatcher mTriggeredEvent;
		entt::dispatcher mOngoingEvent;
		entt::dispatcher mCompletedEvent;
		entt::dispatcher mCanceledEvent;
	};

	struct Keys
	{
		// Mostly ripped entirely from Unreal EKeys struct lmao

		// Mouse Axes
		static const BasedKey MouseX;
		static const BasedKey MouseY;
		static const BasedKey Mouse2D;
		static const BasedKey MouseScrollUp;
		static const BasedKey MouseScrollDown;
		static const BasedKey MouseWheelAxis;

		// Mouse buttons
		static const BasedKey LeftMouseButton;
		static const BasedKey RightMouseButton;
		static const BasedKey MiddleMouseButton;
		static const BasedKey ThumbMouseButton;
		static const BasedKey ThumbMouseButton2;

		// Keyboard keys
		static const BasedKey BackSpace;
		static const BasedKey Tab;
		static const BasedKey Enter;
		static const BasedKey Pause;

		static const BasedKey CapsLock;
		static const BasedKey Escape;
		static const BasedKey SpaceBar;
		static const BasedKey PageUp;
		static const BasedKey PageDown;
		static const BasedKey End;
		static const BasedKey Home;

		static const BasedKey Left;
		static const BasedKey Up;
		static const BasedKey Right;
		static const BasedKey Down;

		static const BasedKey Insert;
		static const BasedKey Delete;

		static const BasedKey Zero;
		static const BasedKey One;
		static const BasedKey Two;
		static const BasedKey Three;
		static const BasedKey Four;
		static const BasedKey Five;
		static const BasedKey Six;
		static const BasedKey Seven;
		static const BasedKey Eight;
		static const BasedKey Nine;

		static const BasedKey A;
		static const BasedKey B;
		static const BasedKey C;
		static const BasedKey D;
		static const BasedKey E;
		static const BasedKey F;
		static const BasedKey G;
		static const BasedKey H;
		static const BasedKey I;
		static const BasedKey J;
		static const BasedKey K;
		static const BasedKey L;
		static const BasedKey M;
		static const BasedKey N;
		static const BasedKey O;
		static const BasedKey P;
		static const BasedKey Q;
		static const BasedKey R;
		static const BasedKey S;
		static const BasedKey T;
		static const BasedKey U;
		static const BasedKey V;
		static const BasedKey W;
		static const BasedKey X;
		static const BasedKey Y;
		static const BasedKey Z;

		static const BasedKey NumPadZero;
		static const BasedKey NumPadOne;
		static const BasedKey NumPadTwo;
		static const BasedKey NumPadThree;
		static const BasedKey NumPadFour;
		static const BasedKey NumPadFive;
		static const BasedKey NumPadSix;
		static const BasedKey NumPadSeven;
		static const BasedKey NumPadEight;
		static const BasedKey NumPadNine;

		static const BasedKey Multiply;
		static const BasedKey Add;
		static const BasedKey Subtract;
		static const BasedKey Decimal;
		static const BasedKey Divide;

		static const BasedKey F1;
		static const BasedKey F2;
		static const BasedKey F3;
		static const BasedKey F4;
		static const BasedKey F5;
		static const BasedKey F6;
		static const BasedKey F7;
		static const BasedKey F8;
		static const BasedKey F9;
		static const BasedKey F10;
		static const BasedKey F11;
		static const BasedKey F12;

		static const BasedKey NumLock;
		static const BasedKey ScrollLock;

		static const BasedKey LeftShift;
		static const BasedKey RightShift;
		static const BasedKey LeftControl;
		static const BasedKey RightControl;
		static const BasedKey LeftAlt;
		static const BasedKey RightAlt;
		static const BasedKey LeftCommand;
		static const BasedKey RightCommand;

		static const BasedKey Semicolon;
		static const BasedKey Equals;
		static const BasedKey Comma;
		static const BasedKey Hyphen;
		static const BasedKey Period;
		static const BasedKey Slash;
		static const BasedKey Grave;
		static const BasedKey LeftBracket;
		static const BasedKey Backslash;
		static const BasedKey RightBracket;
		static const BasedKey Apostrophe;

		static const BasedKey Ampersand;
		static const BasedKey Colon;
		static const BasedKey Exclamation;
		static const BasedKey LeftParantheses;
		static const BasedKey RightParantheses;

		//Gamepad keys
		static const BasedKey Gamepad_Left2D;
		static const BasedKey Gamepad_LeftX;
		static const BasedKey Gamepad_LeftY;
		static const BasedKey Gamepad_Right2D;
		static const BasedKey Gamepad_RightX;
		static const BasedKey Gamepad_RightY;
		static const BasedKey Gamepad_LeftTriggerAxis;
		static const BasedKey Gamepad_RightTriggerAxis;

		static const BasedKey Gamepad_LeftThumbstick;
		static const BasedKey Gamepad_RightThumbstick;
		static const BasedKey Gamepad_Special_Left;
		static const BasedKey Gamepad_Guide;
		static const BasedKey Gamepad_Special_Right;
		static const BasedKey Gamepad_FaceButton_Bottom;
		static const BasedKey Gamepad_FaceButton_Right;
		static const BasedKey Gamepad_FaceButton_Left;
		static const BasedKey Gamepad_FaceButton_Top;
		static const BasedKey Gamepad_LeftShoulder;
		static const BasedKey Gamepad_RightShoulder;
		static const BasedKey Gamepad_LeftTriggerButton;
		static const BasedKey Gamepad_RightTriggerButton;
		static const BasedKey Gamepad_DPad_Up;
		static const BasedKey Gamepad_DPad_Down;
		static const BasedKey Gamepad_DPad_Right;
		static const BasedKey Gamepad_DPad_Left;

		static const BasedKey InvalidKey;

		static void AddKey(const std::string& keyName, const BasedKey& key);
		static std::shared_ptr<BasedKey> GetKey(const std::string& keyName);

		static std::unordered_map<std::string, std::shared_ptr<BasedKey>> mInputKeys;
	};
}
