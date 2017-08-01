// Author: Sean Pesce

#pragma once

#ifndef _SP_D3D9_MOD_TEMPLATE_H_
	#define _SP_D3D9_MOD_TEMPLATE_H_

#include <Windows.h>
#include "SP_IO.hpp"
#include "SpD3D9Device.h"

// Macro to determine if a hotkey is enabled and currently being pressed
#define hotkey_is_down(hotkey) (hotkey != 0 && (key_state[hotkey] & _SP_KEY_DOWN_))
#define print_ol_feed gl_pSpD3D9Device->overlay->text_feed->print
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
#define _SP_D3D9_WINDOW_CLASS_ "DARK SOULS"


// Output
#define _SP_D3D9_OL_TXT_OL_ENABLED_MESSAGE_ "Overlay enabled"
#define _SP_D3D9_OL_TXT_OL_TEXT_FEED_INFO_STRING_ENABLED_MESSAGE_ "Info bar enabled"
#define _SP_D3D9_OL_TXT_OL_TEXT_FEED_INFO_STRING_DISABLED_MESSAGE_ "Info bar disabled"
#define _SP_D3D9_OL_TXT_OUTLINE_STYLE_MESSAGE_ "Text style changed to outlined"
#define _SP_D3D9_OL_TXT_SHADOW_STYLE_MESSAGE_ "Text style changed to shadowed"
#define _SP_D3D9_OL_TXT_PLAIN_STYLE_MESSAGE_ "Text style changed to plain"
#define _SP_D3D9_OL_TXT_TOP_LEFT_POS_MESSAGE_ "Text position set to top left"
#define _SP_D3D9_OL_TXT_TOP_CENTER_POS_MESSAGE_ "Text position set to top center"
#define _SP_D3D9_OL_TXT_TOP_RIGHT_POS_MESSAGE_ "Text position set to top right"
#define _SP_D3D9_OL_TXT_MID_LEFT_POS_MESSAGE_ "Text position set to center left"
#define _SP_D3D9_OL_TXT_MID_CENTER_POS_MESSAGE_ "Text position set to center"
#define _SP_D3D9_OL_TXT_MID_RIGHT_POS_MESSAGE_ "Text position set to center right"
#define _SP_D3D9_OL_TXT_BOTTOM_LEFT_POS_MESSAGE_ "Text position set to bottom left"
#define _SP_D3D9_OL_TXT_BOTTOM_CENTER_POS_MESSAGE_ "Text position set to bottom center"
#define _SP_D3D9_OL_TXT_BOTTOM_RIGHT_POS_MESSAGE_ "Text position set to bottom right"
#define _SP_D3D9_OL_TXT_VERBOSE_ENABLED_MESSAGE_ "Verbose output enabled"
#define _SP_D3D9_OL_TXT_VERBOSE_DISABLED_MESSAGE_ "Verbose output disabled"
#define _SP_D3D9_OL_TXT_AUDIO_FEEDBACK_ENABLED_MESSAGE_ "Audio feedback enabled"
#define _SP_D3D9_OL_TXT_AUDIO_FEEDBACK_DISABLED_MESSAGE_ "Audio feedback disabled"
#define _SP_D3D9_OL_TXT_MULTICOLOR_FEED_ENABLED_MESSAGE_ "Multi-color text feed enabled"
#define _SP_D3D9_OL_TXT_MULTICOLOR_FEED_DISABLED_MESSAGE_ "Multi-color text feed disabled"
#define _SP_D3D9_OL_TXT_SIZE_RESET_MESSAGE_ "Font size reset to "
#define _SP_D3D9_OL_TXT_SIZE_INCREASED_MESSAGE_ "Font size increased to "
#define _SP_D3D9_OL_TXT_SIZE_DECREASED_MESSAGE_ "Font size decreased to "
#define _SP_D3D9_OL_TXT_SIZE_CANT_DECREASE_MESSAGE_ "WARNING: Font is too small; can't decrease font size"
#define _SP_D3D9_OL_TXT_TEST_MESSAGE_ "TEST MESSAGE"


// Keybind & user preference-related variables
//	More info and a reference for virtual key codes can be found at:
//	https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
std::list<SP_KEY_FUNCTION> keybinds; // Stores all function/keybind mappings
extern bool user_pref_overlay_text_feed_enabled;
extern bool user_pref_audio_feedback_enabled;
extern bool user_pref_verbose_output_enabled;
extern int user_pref_overlay_text_size;
extern DWORD user_pref_overlay_text_pos;
extern SP_D3D9O_TEXT_FEED_STYLE_ENUM user_pref_overlay_text_style;
extern int user_pref_show_text_feed_info_bar;

#ifdef _SP_DARK_SOULS_1_
// Dark Souls PvP Watchdog Settings
extern int dspw_pref_font_size;
extern int user_pref_dspw_ol_offset;
#endif // _SP_DARK_SOULS_1_

// Overlay-related variables
extern SpD3D9Device* gl_pSpD3D9Device; // Pointer to the IDirect3DDevice9 wrapper that contains the overlay
int current_overlay_text_size;
SP_D3D9O_TEXT_COLOR_ENUM test_message_color;
int dspw_overlay_adjustment; // Used to adjust the overlay to avoid clipping with the PvP Watchdog overlay

// Input-related variables
extern SHORT key_state[256]; // Key state buffer to hold the states of all 256 virtual keys
typedef void(__stdcall *initialization_func_T)();
std::vector<initialization_func_T> dll_init_funcs; // Initialization functions for loaded DLLs
typedef void(__stdcall *info_bar_func_T)(std::string *);
std::vector<info_bar_func_T> dll_info_bar_funcs; // Functions for adding live-updating strings to the text feed info bar from loaded DLLs
bool input_loop_paused;
bool input_loop_enabled; // Controls whether the main loop that detects player input is enabled/disabled

// Function definitions
void input_loop();	// Main loop for the thread that detects player input
void initialize_mod(bool first_time_setup); // Initializes mod data and settings based on user preferences
int toggle_text_feed(); // Toggle overlay text feed
int toggle_audio_feedback(); // Toggle audio feedback (when hotkeys are pressed)
int toggle_info_bar(); // Toggle text feed info line (date, time, FPS, etc)
int next_overlay_text_position(); // Cycles through the 9 overlay text feed position presets
int next_overlay_text_style(); // Cycles through the overlay text styles (outlined, shadowed, or plain)
int toggle_verbose_output(); // Toggle verbose text feed output
int reset_text_feed_font_size(); // Restore default overlay text feed font size (defined in user preferences)
int increase_text_feed_font_size(); // Increase overlay text feed font size
int decrease_text_feed_font_size(); // Decrease overlay text feed font size
int print_overlay_test_message(); // Print test message to overlay text feed
void SP_beep(DWORD frequency, DWORD duration, bool wait); // Beeps at the specified frequency for a specified duration(in milliseconds) if audio feedback is enabled. If audio is disabled and wait==true, the thread is put to sleep for the specified duration instead.
void SP_beep(DWORD frequency, DWORD duration); // Beeps at the specified frequency for a specified duration(in milliseconds) if audio feedback is enabled. If audio is disabled, the thread is put to sleep for the specified duration instead.

#endif // _SP_D3D9_MOD_TEMPLATE_H_