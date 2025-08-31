#pragma once

namespace based::input
{
	enum class CursorMode : uint8_t
	{
		Free,
		Confined
	};

	class Mouse
	{
	public:
		static void Initialize();
		static void Update();

		inline static int X() { return x; }
		inline static int Y() { return y; }

		inline static int DX() { return xLast; }//return x - xLast; }
		inline static int DY() { return yLast; }//return y - yLast; }

		static bool Button(int button);
		static bool ButtonDown(int button);
		static bool ButtonUp(int button);

		static void SetCursorMode(CursorMode mode);
		static void SetCursorVisible(bool visible);

		static void SetMousePosition(glm::ivec2 position, bool zeroDelta = false);
		static glm::ivec2 GetMousePosition();

		inline static glm::vec2 xBounds = {-999, -999};
		inline static glm::vec2 yBounds = { -999, -999 };

	private:
		constexpr static int ButtonCount = 5; //SDL supports up to 5 mouse buttons

		static int x, xLast;
		static int y, yLast;
		static CursorMode mCursorMode;

		static std::array<bool, ButtonCount> buttons;
		static std::array<bool, ButtonCount> buttonsLast;

		inline static bool mDebugTabbedOut;
	};
}

// Adapted from SDL
enum : uint8_t
{
	BASED_INPUT_MOUSE_FIRST = 1,
	BASED_INPUT_MOUSE_LEFT = BASED_INPUT_MOUSE_FIRST,
	BASED_INPUT_MOUSE_MIDDLE = 2,
	BASED_INPUT_MOUSE_RIGHT = 3,
	BASED_INPUT_MOUSE_X1 = 4,
	BASED_INPUT_MOUSE_X2 = 5,
	BASED_INPUT_MOUSE_LAST = 5
};