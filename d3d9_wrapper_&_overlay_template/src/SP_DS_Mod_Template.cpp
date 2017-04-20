// Author: Sean Pesce

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include "SP_IO.hpp"

// Checks if a hotkey is currently being pressed, but only if it's a valid virtual key:
#define hotkey_is_down(hotkey) (hotkey != 0 && (key_state[hotkey] & _SP_KEY_DOWN_))

enum overlay_text_types {
	SP_TEXT_OUTLINED,
	SP_TEXT_SHADOWED,
	SP_TEXT_SOLID_BACKGROUND
};
extern const char *DS_WINDOW_CLASS;

extern HWND ds_game_window;
extern SHORT key_state[256];
extern int overlay_text_type;
extern bool mod_loop_enabled;
extern unsigned int hotkey1;

void get_ds_window();
BOOL CALLBACK try_ds_window(HWND hwnd, LPARAM lParam);



// Main loop for the mod thread
void mod_loop()
{
	get_ds_window(); // Get the Dark Souls window handle

	while (mod_loop_enabled)
	{
		if (GetForegroundWindow() == ds_game_window) { // Check if Dark Souls game window is active

			get_async_keyboard_state(key_state); // Capture all current async key states

			if (hotkey_is_down(hotkey1))
			{
				// @TODO: Replace this block of code with mod functionality
				Beep(800, 100);
				switch (overlay_text_type) {
					case SP_TEXT_OUTLINED:
					case SP_TEXT_SHADOWED:
						overlay_text_type++;
						break;
					case SP_TEXT_SOLID_BACKGROUND:
					default:
						overlay_text_type = 0;
						break;
				}
			}

			// @TODO: Other hotkeys should also be checked and handled here

			Sleep(1);
		}
		else
		{
			// Dark Souls game window is not active
			Sleep(100);
		}

		
	}
}

// Obtains the window handle for the Dark Souls game window
void get_ds_window()
{
	bool found_ds_window = false;

	while (!found_ds_window)
	{
		if (!EnumWindows(try_ds_window, (LPARAM)&found_ds_window))
		{
			// Handle error
		}
		Sleep(30);
	}
}

// Checks if a given window is the Dark Souls game window, and if so, saves the window handle
BOOL CALLBACK try_ds_window(HWND hwnd, LPARAM lParam)
{
	bool *found_ds_window = (bool*)lParam;
	DWORD pid;

	GetWindowThreadProcessId(hwnd, &pid); // Get Dark Souls process ID

	if (pid == GetCurrentProcessId())
	{
		// hwnd was created by Dark Souls process
		char window_class[128];
		
		if (!RealGetWindowClass(hwnd, window_class, 128))
		{
			// Handle error
		}
		window_class[127] = '\0';

		if (strcmp(window_class, DS_WINDOW_CLASS) == 0)
		{
			// hwnd is the Dark Souls game window
			ds_game_window = hwnd;
			*found_ds_window = true;
		}
	}
	return 1;
}