// Author: Sean Pesce

#pragma once

#ifndef _SP_DS_D3D9_TEMPLATE_H_
	#define _SP_DS_D3D9_TEMPLATE_H_

#include "SP_IO.hpp"
#include <stdio.h>


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

void mod_loop();	// Main loop for the mod thread


#endif // _SP_DS_D3D9_TEMPLATE_H_