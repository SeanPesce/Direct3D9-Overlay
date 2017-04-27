// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#pragma once

#ifndef _SP_DS_D3D9_OVERLAY_TEMPLATE_H_
	#define _SP_DS_D3D9_OVERLAY_TEMPLATE_H_

#include "SP_IO.hpp"

// Exported function
IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion);

// Regular functions
void InitInstance(HANDLE hModule);
int InitSettings();
void ExitInstance(void);
void LoadOriginalDll(void);


//////////////////////// CONSTANTS ////////////////////////
enum SP_OL_TXT_ENABLED_ENUM {
	OL_TXT_DISABLED,
	OL_TXT_ENABLED
};
enum SP_OL_TXT_POS_ENUM {
	OL_TXT_POS_LEFT,
	OL_TXT_POS_HCENTER,
	OL_TXT_POS_RIGHT,
	OL_TXT_POS_TOP,
	OL_TXT_POS_VCENTER,
	OL_TXT_POS_BOTTOM
};
const char *SP_OL_TXT_POS_VALS[6] = { "LEFT", "CENTER", "RIGHT", "TOP", "CENTER", "BOTTOM" };
//
const char *SP_OL_TXT_STYLE_VALS[3] = { "OUTLINE", "SHADOW", "PLAIN" };
// Default initialization setting values
#define _SP_DS_DEFAULT_VAL_OL_TXT_ENABLED_ OL_TXT_ENABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_AUDIO_ENABLED_ OL_TXT_DISABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_VERBOSE_OUTPUT_ENABLED_ OL_TXT_DISABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_MULTICOLOR_FEED_ENABLED_ OL_TXT_ENABLED
#define _SP_DS_DEFAULT_VAL_OL_TXT_HORIZONTAL_POS_ SP_OL_TXT_POS_VALS[OL_TXT_POS_HCENTER]
#define _SP_DS_DEFAULT_VAL_OL_TXT_VERTICAL_POS_ SP_OL_TXT_POS_VALS[OL_TXT_POS_BOTTOM]
#define _SP_DS_DEFAULT_VAL_OL_TXT_STYLE_ SP_OL_TXT_STYLE_VALS[SP_DX9_BORDERED_TEXT]

//////////////////////// VARIABLES & DATA ////////////////////////

HANDLE mod_thread;
DWORD mod_thread_id;
HWND ds_game_window;	// Game window handle
SHORT key_state[256];	// Buffer for async key states
bool mod_loop_enabled;	// Enables/disables the main loop
// Keybinds
unsigned int hotkey_toggle_overlay_text;
unsigned int hotkey_next_overlay_text_pos;
unsigned int hotkey_next_overlay_text_style;
unsigned int hotkey_print_overlay_test_message;
unsigned int hotkey_toggle_audio_feedback;
unsigned int hotkey_toggle_verbose_output;
unsigned int hotkey_increase_overlay_text_size;
unsigned int hotkey_decrease_overlay_text_size;
unsigned int hotkey_reset_overlay_text_size;
unsigned int hotkey_toggle_multicolor_feed;
// User preferences
bool user_pref_overlay_text_enabled;
bool user_pref_audio_feedback_enabled;
bool user_pref_verbose_output_enabled;
bool user_pref_multicolor_feed_enabled;
int user_pref_overlay_text_size;
DWORD user_pref_overlay_text_pos;
int user_pref_overlay_text_style;


//////////////////////// FUNCTION PROTOTYPES ////////////////////////

DWORD WINAPI init_mod_thread(LPVOID lpParam);
void mod_loop();	// Main loop for the mod thread
void get_user_preferences();
void load_dinput8();


#endif // _SP_DS_D3D9_OVERLAY_TEMPLATE_H_