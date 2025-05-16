#include "pch.h"
#include "input/joystick.h"

#include "SDL2/SDL_events.h"
#include "SDL2/SDL_gamecontroller.h"

namespace based::input
{
	std::unordered_map<int, std::unique_ptr<Joystick::Controller>> Joystick::availableJoysticks;
	float Joystick::deadzone = 0.2f;

	void Joystick::OnJoystickConnected(SDL_ControllerDeviceEvent& e)
	{
		int deviceIndex = e.which;
		if (SDL_IsGameController(deviceIndex))
		{
			auto c = std::make_unique<Controller>();
			c->gc = SDL_GameControllerOpen(deviceIndex);
			if (c->gc)
			{
				c->joystickIndex = deviceIndex;
				std::fill(c->buttons.begin(), c->buttons.end(), false);
				std::fill(c->buttonsLast.begin(), c->buttonsLast.end(), false);
				std::fill(c->axes.begin(), c->axes.end(), 0.f);
				std::fill(c->axesLast.begin(), c->axesLast.end(), 0.f);

				int mapIndex = GetNextFreeIndex();
				BASED_TRACE("Joystick connected: mapIndex({}), deviceIndex({})", mapIndex, deviceIndex);
				availableJoysticks[mapIndex] = std::move(c);
			}
			else
			{
				BASED_ERROR("Error opening game controller with device index {}: {}", deviceIndex, SDL_GetError());
			}
		}
	}

	void Joystick::OnJoystickDisconnected(SDL_ControllerDeviceEvent& e)
	{
		int deviceIndex = e.which;
		for (auto it = availableJoysticks.begin(); it != availableJoysticks.end(); it++)
		{
			Controller* c = it->second.get();
			if (c->joystickIndex == deviceIndex)
			{
				BASED_TRACE("Joystick disconnected {}", deviceIndex);
				SDL_GameControllerClose(c->gc);
				availableJoysticks.erase(it);
				break;
			}
		}
	}

	void Joystick::Shutdown()
	{
		for (auto it = availableJoysticks.begin(); it != availableJoysticks.end();)
		{
			Controller* c = it->second.get();
			SDL_GameControllerClose(c->gc);
			it = availableJoysticks.erase(it);
		}
	}

	void Joystick::Update()
	{
		for (auto it = availableJoysticks.begin(); it != availableJoysticks.end(); it++)
		{
			Controller* c = it->second.get();
			BASED_ASSERT(c && c->gc, "Invalid game controller in available joysticks map");
			if (c && c->gc)
			{
				c->buttonsLast = c->buttons;
				c->axesLast = c->axes;

				for (unsigned int i = 0; i < static_cast<int>(Button::Count); i++)
				{
					c->buttons[i] = SDL_GameControllerGetButton(c->gc, static_cast<SDL_GameControllerButton>(i));
				}

				for (unsigned int i = 0; i < static_cast<int>(Axis::Count); i++)
				{
					// SDL ranges axes from -32768 to 32767
					c->axes[i] = std::clamp(
						SDL_GameControllerGetAxis(c->gc, static_cast<SDL_GameControllerAxis>(i)) / 32767.f, 
						-1.f, 1.f);
					//if (c->axes[i] > -deadzone && c->axes[i] < deadzone) c->axes[i] = 0.f;
				}
			}
		}
	}

	bool Joystick::IsJoystickAvailable(int joystickId)
	{
		return availableJoysticks.count(joystickId) > 0;
	}

	bool Joystick::GetButton(int joystickId, Button button)
	{
		static bool sentWarning = false;
		auto it = availableJoysticks.find(joystickId);
		if (it != availableJoysticks.end())
		{
			sentWarning = false;
			return it->second->buttons[static_cast<int>(button)];
		}
		if (!sentWarning)
		{
			sentWarning = true;
			BASED_ERROR("Joystick with id {} is not available!", joystickId);
		}
		return false;
	}

	bool Joystick::GetButton(int joystickId, int button)
	{
		return GetButton(joystickId, (Button)button);
	}

	bool Joystick::GetButtonDown(int joystickId, Button button)
	{
		static bool sentWarning = false;
		auto it = availableJoysticks.find(joystickId);
		if (it != availableJoysticks.end())
		{
			sentWarning = false;
			return it->second->buttons[static_cast<int>(button)] && !it->second->buttonsLast[static_cast<int>(button)];
		}
		if (!sentWarning)
		{
			sentWarning = true;
			BASED_ERROR("Joystick with id {} is not available!", joystickId);
		}
		return false;
	}

	bool Joystick::GetButtonUp(int joystickId, Button button)
	{
		static bool sentWarning = false;
		auto it = availableJoysticks.find(joystickId);
		if (it != availableJoysticks.end())
		{
			sentWarning = false;
			return !it->second->buttons[static_cast<int>(button)] && it->second->buttonsLast[static_cast<int>(button)];
		}
		if (!sentWarning)
		{
			sentWarning = true;
			BASED_ERROR("Joystick with id {} is not available!", joystickId);
		}
		return false;
	}

	float Joystick::GetAxis(int joystickId, Axis axis, bool useDeadzone)
	{
		static bool sentWarning = false;
		auto it = availableJoysticks.find(joystickId);
		if (it != availableJoysticks.end())
		{
			sentWarning = false;
			float val = it->second->axes[static_cast<int>(axis)];
			if (useDeadzone) return abs(val) > deadzone ? val : 0.f;
			return val;
		}
		if (!sentWarning)
		{
			sentWarning = true;
			BASED_ERROR("Joystick with id {} is not available!", joystickId);
		}
		return 0.f;
	}

	float Joystick::GetAxis(int joystickId, int axis)
	{
		return GetAxis(joystickId, (Axis)axis);
	}

	int Joystick::GetNextFreeIndex()
	{
		int ret = -1;

		for (int i = 0; i < SDL_NumJoysticks(); i++)
		{
			if (availableJoysticks.count(i) == 0)
			{
				ret = i;
				break;
			}
		}

		return ret;
	}
}