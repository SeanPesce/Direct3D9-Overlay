// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#pragma once

#ifndef _SP_DS_D3D9_OVERLAY_TEMPLATE_H_
	#define _SP_DS_D3D9_OVERLAY_TEMPLATE_H_

#include "SP_IO.hpp"

// Constants & Variables:
extern const char *example_overlay_text;
const char *example_overlay_text = "Dark Souls mod template by Sean Pesce";

// Exported function
IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion);

// Regular functions
void InitInstance(HANDLE hModule);
int InitSettings();
void ExitInstance(void);
void LoadOriginalDll(void);


//////////////////////// CONSTANTS ////////////////////////
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
unsigned int hotkey_next_overlay_text_pos;
unsigned int hotkey_next_overlay_text_style;
// User preferences
int user_pref_overlay_text_size;
DWORD user_pref_overlay_text_pos;
int user_pref_overlay_text_style;


//////////////////////// FUNCTION PROTOTYPES ////////////////////////

DWORD WINAPI init_mod_thread(LPVOID lpParam);
void mod_loop();	// Main loop for the mod thread
void get_user_preferences();


#endif // _SP_DS_D3D9_OVERLAY_TEMPLATE_H_