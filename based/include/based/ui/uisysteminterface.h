#pragma once

#ifndef RMLUI_BACKENDS_PLATFORM_SDL_H
#define RMLUI_BACKENDS_PLATFORM_SDL_H

#include <SDL2/SDL.h>

namespace based::ui
{
	class SystemInterface_SDL : public Rml::SystemInterface {
	public:
		SystemInterface_SDL();
		~SystemInterface_SDL();

		// Optionally, provide or change the window to be used for setting the mouse cursors.
		void SetWindow(SDL_Window* window);

		// -- Inherited from Rml::SystemInterface  --

		double GetElapsedTime() override;

		void SetMouseCursor(const Rml::String& cursor_name) override;

		void SetClipboardText(const Rml::String& text) override;
		void GetClipboardText(Rml::String& text) override;

	private:
		SDL_Window* window = nullptr;
	};
}

namespace RmlSDL {

	// Applies input on the context based on the given SDL event.
	// @return True if the event is still propagating, false if it was handled by the context.
	bool InputEventHandler(Rml::Context* context, SDL_Event& ev);

	// Converts the SDL key to RmlUi key.
	Rml::Input::KeyIdentifier ConvertKey(int sdl_key);

	// Converts the SDL mouse button to RmlUi mouse button.
	int ConvertMouseButton(int sdl_mouse_button);

	// Returns the active RmlUi key modifier state.
	int GetKeyModifierState();

} // namespace RmlSDL

#endif