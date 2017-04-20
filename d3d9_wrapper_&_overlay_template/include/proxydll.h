// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#pragma once

#ifndef _SP_DS_D3D9_OVERLAY_TEMPLATE_H_
	#define _SP_DS_D3D9_OVERLAY_TEMPLATE_H_

#include "SP_IO.hpp"
#include <d3dx9core.h>

// Data & structures for the text overlay:
D3DRECT text_background;
RECT text_box;
RECT text_shadow_box;
RECT text_outline_boxes[8];
ID3DXFont* SP_font = NULL;
TCHAR *SP_font_name = "Arial";

// Constants & Variables:
bool font_initialized = false;
int overlay_text_type;
extern const char *example_overlay_text;
const char *example_overlay_text = "Example overlay by Sean Pesce";

// Exported function
IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion);

// Regular functions
void InitInstance(HANDLE hModule);
int InitSettings();
void ExitInstance(void);
void LoadOriginalDll(void);


//////////////////////// CONSTANTS ////////////////////////

extern const char *SETTINGS_FILE;
const char *SETTINGS_FILE = ".\\DS_d3d9_Mod.ini";
extern const char *SETTINGS_FILE_SUBSECTION;
const char *SETTINGS_FILE_SUBSECTION = "DS_d3d9_Mod";
extern const char *DLL_CHAIN_KEY;
const char *DLL_CHAIN_KEY = "DLL_Chain";
extern const char *HOTKEY1_KEY;
const char *HOTKEY1_KEY = "Hotkey1";
extern const char *DS_WINDOW_CLASS;
const char *DS_WINDOW_CLASS = "DARK SOULS";


//////////////////////// VARIABLES & DATA ////////////////////////

HANDLE mod_thread;
DWORD mod_thread_id;
HWND ds_game_window;	// Game window handle
SHORT key_state[256];	// Buffer for async key states
bool mod_loop_enabled;	// Enables/disables the main loop
unsigned int hotkey1;


//////////////////////// FUNCTION PROTOTYPES ////////////////////////

DWORD WINAPI init_mod_thread(LPVOID lpParam);
void mod_loop();	// Main loop for the mod thread


#endif // _SP_DS_D3D9_OVERLAY_TEMPLATE_H_