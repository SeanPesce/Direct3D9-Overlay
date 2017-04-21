// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#pragma once

#ifndef _SP_DS_D3D9_OVERLAY_TEMPLATE_H_
	#define _SP_DS_D3D9_OVERLAY_TEMPLATE_H_

#include "SP_IO.hpp"

// Constants & Variables:
extern const char *example_overlay_text;
const char *example_overlay_text = "Example overlay by Sean Pesce";

// Exported function
IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion);

// Regular functions
void InitInstance(HANDLE hModule);
int InitSettings();
void ExitInstance(void);
void LoadOriginalDll(void);


//////////////////////// VARIABLES & DATA ////////////////////////

HANDLE mod_thread;
DWORD mod_thread_id;
HWND ds_game_window;	// Game window handle
SHORT key_state[256];	// Buffer for async key states
bool mod_loop_enabled;	// Enables/disables the main loop
unsigned int hotkey_next_overlay_text_pos;
unsigned int hotkey_next_overlay_text_style;


//////////////////////// FUNCTION PROTOTYPES ////////////////////////

DWORD WINAPI init_mod_thread(LPVOID lpParam);
void mod_loop();	// Main loop for the mod thread


#endif // _SP_DS_D3D9_OVERLAY_TEMPLATE_H_