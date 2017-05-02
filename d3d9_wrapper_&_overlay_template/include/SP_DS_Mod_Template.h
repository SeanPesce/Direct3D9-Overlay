// Author: Sean Pesce

#pragma once

#ifndef _SP_DS_MOD_TEMPLATE_H_
	#define _SP_DS_MOD_TEMPLATE_H_

#include <Windows.h>
#include "SP_IO.hpp"
#include "myIDirect3DDevice9.h"

// Macro to determine if a hotkey is enabled and currently being pressed
#define hotkey_is_down(hotkey) (hotkey != 0 && (key_state[hotkey] & _SP_KEY_DOWN_))
#define print_ol_feed gl_pmyIDirect3DDevice9->print_to_overlay_feed
// Overlay text feed position presets
#define _SP_TEXT_TOP_LEFT_ (DT_NOCLIP | DT_TOP | DT_LEFT)
#define _SP_TEXT_TOP_CENTER_ (DT_NOCLIP | DT_TOP | DT_CENTER)
#define _SP_TEXT_TOP_RIGHT_ (DT_NOCLIP | DT_TOP | DT_RIGHT)
#define _SP_TEXT_CENTER_LEFT_ (DT_NOCLIP | DT_VCENTER | DT_LEFT)
#define _SP_TEXT_CENTER_CENTER_ (DT_NOCLIP | DT_VCENTER | DT_CENTER)
#define _SP_TEXT_CENTER_RIGHT_ (DT_NOCLIP | DT_VCENTER | DT_RIGHT)
#define _SP_TEXT_BOTTOM_LEFT_ (DT_NOCLIP | DT_BOTTOM | DT_LEFT)
#define _SP_TEXT_BOTTOM_CENTER_ (DT_NOCLIP | DT_BOTTOM | DT_CENTER)
#define _SP_TEXT_BOTTOM_RIGHT_ (DT_NOCLIP | DT_BOTTOM | DT_RIGHT)
// Dark Souls game window class returned by RealGetWindowClass()
#define _SP_DS_WINDOW_CLASS_ "DARK SOULS"
// Delay, in milliseconds, after a mod keybind is pressed, before the mod will continue listening for future keypresses
#define _SP_DS_KEYPRESS_WAIT_TIME_ 100
// Standard lifetime (in milliseconds) of an overlay text feed message
#define _SP_DS_OL_TEXT_FEED_MSG_LIFESPAN_ 2000


// Output
#define _SP_DS_DEFAULT_BEEP_DURATION_ 100
#define _SP_DS_OL_TXT_TITLE_MESSAGE_ "Dark Souls mod template by Sean Pesce"
#define _SP_DS_OL_TXT_DINPUT8_LOADED_EARLY_MESSAGE_ "DEBUG: dinput8.dll loaded early"
#define _SP_DS_OL_TXT_DINPUT8_NOT_LOADED_EARLY_MESSAGE_ "DEBUG: dinput8.dll was not preloaded (default option)"
#define _SP_DS_OL_TXT_OL_ENABLED_MESSAGE_ "Overlay enabled"
#define _SP_DS_OL_TXT_OL_TEXT_WATERMARK_ENABLED_MESSAGE_ "Info watermark enabled"
#define _SP_DS_OL_TXT_OL_TEXT_WATERMARK_DISABLED_MESSAGE_ "Info watermark disabled"
#define _SP_DS_OL_TXT_OUTLINE_STYLE_MESSAGE_ "Text style changed to outlined"
#define _SP_DS_OL_TXT_SHADOW_STYLE_MESSAGE_ "Text style changed to shadowed"
#define _SP_DS_OL_TXT_PLAIN_STYLE_MESSAGE_ "Text style changed to plain"
#define _SP_DS_OL_TXT_TOP_LEFT_POS_MESSAGE_ "Text position set to top left"
#define _SP_DS_OL_TXT_TOP_CENTER_POS_MESSAGE_ "Text position set to top center"
#define _SP_DS_OL_TXT_TOP_RIGHT_POS_MESSAGE_ "Text position set to top right"
#define _SP_DS_OL_TXT_MID_LEFT_POS_MESSAGE_ "Text position set to center left"
#define _SP_DS_OL_TXT_MID_CENTER_POS_MESSAGE_ "Text position set to center"
#define _SP_DS_OL_TXT_MID_RIGHT_POS_MESSAGE_ "Text position set to center right"
#define _SP_DS_OL_TXT_BOTTOM_LEFT_POS_MESSAGE_ "Text position set to bottom left"
#define _SP_DS_OL_TXT_BOTTOM_CENTER_POS_MESSAGE_ "Text position set to bottom center"
#define _SP_DS_OL_TXT_BOTTOM_RIGHT_POS_MESSAGE_ "Text position set to bottom right"
#define _SP_DS_OL_TXT_VERBOSE_ENABLED_MESSAGE_ "Verbose output enabled"
#define _SP_DS_OL_TXT_VERBOSE_DISABLED_MESSAGE_ "Verbose output disabled"
#define _SP_DS_OL_TXT_AUDIO_FEEDBACK_ENABLED_MESSAGE_ "Audio feedback enabled"
#define _SP_DS_OL_TXT_AUDIO_FEEDBACK_DISABLED_MESSAGE_ "Audio feedback disabled"
#define _SP_DS_OL_TXT_MULTICOLOR_FEED_ENABLED_MESSAGE_ "Multi-color text feed enabled"
#define _SP_DS_OL_TXT_MULTICOLOR_FEED_DISABLED_MESSAGE_ "Multi-color text feed disabled"
#define _SP_DS_OL_TXT_SIZE_RESET_MESSAGE_ "Font size reset to "
#define _SP_DS_OL_TXT_SIZE_INCREASED_MESSAGE_ "Font size increased to "
#define _SP_DS_OL_TXT_SIZE_DECREASED_MESSAGE_ "Font size decreased to "
#define _SP_DS_OL_TXT_SIZE_CANT_DECREASE_MESSAGE_ "WARNING: Font is too small; can't decrease font size"
#define _SP_DS_OL_TXT_TEST_MESSAGE_ "TEST MESSAGE"


// Keybind & user preference-related variables
extern unsigned int hotkey_toggle_overlay_text_feed;
extern unsigned int hotkey_next_overlay_text_pos;
extern unsigned int hotkey_next_overlay_text_style;
extern unsigned int hotkey_print_overlay_test_message;
extern unsigned int hotkey_toggle_audio_feedback;
extern unsigned int hotkey_toggle_verbose_output;
extern unsigned int hotkey_increase_overlay_text_size;
extern unsigned int hotkey_decrease_overlay_text_size;
extern unsigned int hotkey_reset_overlay_text_size;
extern unsigned int hotkey_toggle_multicolor_feed;
extern unsigned int hotkey_toggle_info_watermark;
extern bool user_pref_overlay_text_feed_enabled;
extern bool user_pref_audio_feedback_enabled;
extern bool user_pref_verbose_output_enabled;
extern bool user_pref_multicolor_feed_enabled;
extern int user_pref_overlay_text_size;
extern bool user_pref_load_dinput8_early;
extern DWORD user_pref_overlay_text_pos;
extern int user_pref_overlay_text_style;
extern int user_pref_show_text_watermark;

// Overlay-related variables
extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9; // Pointer to the IDirect3DDevice9 wrapper that contains the overlay
int current_overlay_text_size;
int test_message_color;

// Mod-related variables
extern HWND ds_game_window; // Dark Souls game window handle
extern SHORT key_state[256]; // Key state buffer to hold the states of all 256 virtual keys
bool mod_loop_enabled; // Controls whether the main loop for the mod is enabled/disabled

// Function definitions
void mod_loop();	// Main loop for the mod thread
void initialize_mod(); // Initializes mod data and settings based on user preferences
void get_ds_window(); // Obtains the handle for the Dark Souls game window and stores it in ds_game_window
BOOL CALLBACK try_ds_window(HWND hwnd, LPARAM lParam); // Tests whether a window is the Dark Souls game window
void next_overlay_text_position(DWORD current_position); // Cycles through the 9 overlay text feed position presets
void next_overlay_text_style(int current_style); // Cycles through the overlay text styles (outlined, shadowed, or plain)
void SP_beep(DWORD frequency, DWORD duration, bool wait); // Beeps at the specified frequency for a specified duration(in milliseconds) if audio feedback is enabled. If audio is disabled and wait==true, the thread is put to sleep for the specified duration instead.
void SP_beep(DWORD frequency, DWORD duration); // Beeps at the specified frequency for a specified duration(in milliseconds) if audio feedback is enabled. If audio is disabled, the thread is put to sleep for the specified duration instead.

#endif // _SP_DS_MOD_TEMPLATE_H_