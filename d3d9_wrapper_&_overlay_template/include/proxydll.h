// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#pragma once

#ifndef _SP_D3D9_OVERLAY_TEMPLATE_H_
	#define _SP_D3D9_OVERLAY_TEMPLATE_H_

#include "SP_IO.hpp"


//////////////////////// WRAPPER DLL FUNCTIONS ////////////////////////
// Exported function
IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion);
// Regular functions
void InitInstance(HANDLE hModule);
int InitSettings(); // Parses settings file (.ini) for intialization settings
void ExitInstance(void); // Unloads DLL when exiting
void LoadOriginalDll(void); // Loads the original d3d9.dll from the system directory


//////////////////////// CONSTANTS & ENUMS ////////////////////////
// Settings file data:
#define _SP_DS_SETTINGS_FILE_ ".\\DS_d3d9_Mod.ini"	// Settings file name
//	Settings file sections
#define _SP_DS_SETTINGS_SECTION_KEYBINDS_ "Keybinds"
#define _SP_DS_SETTINGS_SECTION_PREFS_ "Preferences"
#define _SP_DS_SETTINGS_SECTION_ADV_SETTINGS_ "Advanced Settings"
#define _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_ "Developer Keybinds"
#define _SP_DS_SETTINGS_SECTION_DEV_PREFS_ "Developer Preferences"
//	Advanced settings section keys
#define _SP_DS_DLL_CHAIN_KEY_ "d3d9Chain"
#define _SP_DS_DSPW_ADJUSTMENT_KEY_ "DspwOverlayAdjustment"
//	Keybinds section keys
#define _SP_DS_HOTKEY_TOGGLE_OL_TXT_KEY_ "ToggleOverlay"
#define _SP_DS_HOTKEY_NEXT_OL_TXT_POS_KEY_ "ChangeOverlayTextPosition"
#define _SP_DS_HOTKEY_NEXT_OL_TXT_STYLE_KEY_ "ChangeOverlayTextStyle"
#define _SP_DS_HOTKEY_TOGGLE_TEXT_WATERMARK_KEY_ "ToggleInfoWatermark"
//	User preferences section keys
#define _SP_DS_OL_TXT_ENABLED_KEY_ "EnableOverlay"
#define _SP_DS_OL_TXT_SIZE_KEY_ "OverlayTextSize"
#define _SP_DS_OL_TXT_HORIZONTAL_POS_KEY_ "OverlayTextHorizontalPosition"
#define _SP_DS_OL_TXT_VERTICAL_POS_KEY_ "OverlayTextVerticalPosition"
#define _SP_DS_OL_TXT_STYLE_KEY_ "OverlayTextStyle"
#define _SP_DS_OL_TXT_AUDIO_ENABLED_KEY_ "EnableAudioFeedback"
#define _SP_DS_OL_TXT_ENABLE_FPS_KEY_ "DisplayFPS"
#define _SP_DS_OL_TXT_ENABLE_TIME_KEY_ "DisplayTime"
#define _SP_DS_OL_TXT_ENABLE_DATE_KEY_ "DisplayDate"
//	Developer keybinds section keys
#define _SP_DS_HOTKEY_PRINT_OL_TXT_TEST_MSG_KEY_ "PrintTestMessage"
#define _SP_DS_HOTKEY_TOGGLE_AUDIO_FEEDBACK_KEY_ "ToggleAudioFeedback"
#define _SP_DS_HOTKEY_TOGGLE_VERBOSE_OUTPUT_KEY_ "ToggleVerboseOutput"
#define _SP_DS_HOTKEY_INCREASE_TXT_SIZE_KEY_ "IncreaseTextSize"
#define _SP_DS_HOTKEY_DECREASE_TXT_SIZE_KEY_ "DecreaseTextSize"
#define _SP_DS_HOTKEY_RESET_TXT_SIZE_KEY_ "ResetTextSize"
#define _SP_DS_HOTKEY_TOGGLE_MULTICOLOR_FEED_KEY_ "ToggleMultiColorFeed"
//	Developer preferences section keys
#define _SP_DS_OL_TXT_VERBOSE_OUTPUT_ENABLED_KEY_ "EnableVerboseOutput"
#define _SP_DS_OL_TXT_MULTICOLOR_FEED_ENABLED_KEY_ "EnableMultiColorTextFeed"
#define _SP_DS_OL_LOAD_DINPUT8_EARLY_KEY_ "PreLoadDinput8DLL"

// Enumerator whose values specify whether a toggleable setting is enabled or disabled:
enum SP_OL_TXT_ENABLED_ENUM {
	OL_TXT_DISABLED,
	OL_TXT_ENABLED
};
// Enumerator whose values specify horizontal and vertical positions of overlay feed text:
enum SP_OL_TXT_POS_ENUM {
	OL_TXT_POS_LEFT,
	OL_TXT_POS_HCENTER,
	OL_TXT_POS_RIGHT,
	OL_TXT_POS_TOP,
	OL_TXT_POS_VCENTER,
	OL_TXT_POS_BOTTOM
};

// Acceptable values that a user can specify for text position in the settings file:
const char *SP_OL_TXT_POS_VALS[6] = { "LEFT", "CENTER", "RIGHT", "TOP", "CENTER", "BOTTOM" }; // If specified string isn't one of these values, the default value is used
// Acceptable values that a user can specify for text style in the settings file:
const char *SP_OL_TXT_STYLE_VALS[3] = { "OUTLINE", "SHADOW", "PLAIN" }; // If specified string isn't one of these values, the default value is used

// Default initialization setting values:
#define _SP_DS_DEFAULT_VAL_OL_TXT_ENABLED_ OL_TXT_ENABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_AUDIO_ENABLED_ OL_TXT_DISABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_VERBOSE_OUTPUT_ENABLED_ OL_TXT_DISABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_MULTICOLOR_FEED_ENABLED_ OL_TXT_ENABLED
#define _SP_DS_DEFAULT_VAL_OL_LOAD_DINPUT8_EARLY_ OL_TXT_DISABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_HORIZONTAL_POS_ SP_OL_TXT_POS_VALS[OL_TXT_POS_HCENTER]
#define _SP_DS_DEFAULT_VAL_OL_TXT_VERTICAL_POS_ SP_OL_TXT_POS_VALS[OL_TXT_POS_BOTTOM]
#define _SP_DS_DEFAULT_VAL_OL_TXT_STYLE_ SP_OL_TXT_STYLE_VALS[SP_DX9_BORDERED_TEXT]


//////////////////////// VARIABLES & DATA ////////////////////////

HANDLE mod_thread;		// Mod thread handle
DWORD mod_thread_id;	// Mod thread ID
HWND ds_game_window;	// Game window handle
SHORT key_state[256];	// Buffer for async key states
extern bool mod_loop_enabled; // Controls whether the main loop for the mod is enabled/disabled

// Keybinds (stored as virtual key codes)
//	More info and a reference for virtual key codes can be found at:
//	https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
unsigned int hotkey_toggle_overlay_text_feed;
unsigned int hotkey_next_overlay_text_pos;
unsigned int hotkey_next_overlay_text_style;
unsigned int hotkey_print_overlay_test_message;
unsigned int hotkey_toggle_audio_feedback;
unsigned int hotkey_toggle_verbose_output;
unsigned int hotkey_increase_overlay_text_size;
unsigned int hotkey_decrease_overlay_text_size;
unsigned int hotkey_reset_overlay_text_size;
unsigned int hotkey_toggle_multicolor_feed;
unsigned int hotkey_toggle_info_watermark;
// User preferences
bool user_pref_overlay_text_feed_enabled;
bool user_pref_audio_feedback_enabled;
bool user_pref_verbose_output_enabled;
bool user_pref_multicolor_feed_enabled;
bool user_pref_load_dinput8_early;
int user_pref_overlay_text_size;
DWORD user_pref_overlay_text_pos;
int user_pref_overlay_text_style;
int user_pref_show_text_watermark;
// Dark Souls PvP Watchdog Settings
// (These values will be used to adjust the overlay to avoid clipping with the DSPW overlay)
int dspw_pref_font_size;
int user_pref_dspw_ol_offset;


//////////////////////// MOD FUNCTION PROTOTYPES ////////////////////////
DWORD WINAPI init_mod_thread(LPVOID lpParam); // Determines whether mod is enabled and calls the main loop for the mod
void get_user_preferences(); // Reads in user preferences as specified in the settings file (.ini)
void load_dinput8(); // Loads dinput8.dll
int get_dspw_font_size(); // Reads the PvP Watchdog settings file (DSPWSteam.ini) to obtain the DSPW font size in case user wants to adjust this overlay to avoid clipping with the PvP Watchdog overlay


#endif // _SP_D3D9_OVERLAY_TEMPLATE_H_