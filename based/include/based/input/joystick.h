#pragma once

struct SDL_ControllerDeviceEvent;
typedef struct _SDL_GameController SDL_GameController;
namespace based::input
{
	class Joystick
	{
	public:
		// Adapted from SDL - see SDL_GameControllerAxis
		enum class Axis
		{
			LeftStickHorizontal,
			LeftStickVertical,
			RightStickHorizontal,
			RightStickVertical,
			LeftTrigger,
			RightTrigger,
			Count
		};

		// Adapted from SDL - see SDL_GameControllerButton
		enum class Button
		{
			Face_Bottom,
			Face_Right,
			Face_Left,
			Face_Top,
			Back,
			Guide,
			Start,
			LS,
			RS,
			LB,
			RB,
			DPAD_Up,
			DPAD_Down,
			DPAD_Left,
			DPAD_Right,
			Count
		};

		// Adapted from SDL - See SDL_GameControllerType
		enum class ControllerType
		{
			Unknown = 0,
			Xbox360,
			XboxOne,
			PS3,
			PS4,
			SwitchPro,
			PS5 = 7,
			JoyconLeft = 11,
			JoyconRight,
			JoyconPair
		};

		static void OnJoystickConnected(SDL_ControllerDeviceEvent& e);
		static void OnJoystickDisconnected(SDL_ControllerDeviceEvent& e);
		static void Shutdown();
		static void Update();

		static bool IsJoystickAvailable(int joystickId);

		static bool GetButton(int joystickId, Button button);
		static bool GetButton(int joystickId, int button);
		static bool GetButtonDown(int joystickId, Button button);
		static bool GetButtonUp(int joystickId, Button button);
		static float GetAxis(int joystickId, Axis axis, bool useDeadzone = false);
		static float GetAxis(int joystickId, int axis);

		static ControllerType GetControllerType(int joystickId);
		static std::string GetDeviceName(int joystickId);
	private:
		static int GetNextFreeIndex();

		struct Controller
		{
			int joystickIndex = -1;
			SDL_GameController* gc = nullptr;

			std::array<bool, (int)Button::Count> buttons;
			std::array<bool, (int)Button::Count> buttonsLast;
			std::array<float, (int)Axis::Count> axes;
			std::array<float, (int)Axis::Count> axesLast;
		};

		static std::unordered_map<int, std::unique_ptr<Controller>> availableJoysticks;
		static float deadzone;
	};
}

