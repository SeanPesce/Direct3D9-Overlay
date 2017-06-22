// Author: Sean Pesce

#pragma once

#ifndef _SP_MOD_DARK_SOULS_1_H_
	#define _SP_MOD_DARK_SOULS_1_H_

//////////////////////// CONSTANTS & ENUMS ////////////////////////

// Configuration setting key names
#define _SP_DS1_MOD_HOTKEY_BONFIRE_INPUT_FIX_ "BonfireInputFix"

// Overlay messages
#define _SP_DS1_MOD_MSG_BONFIRE_INPUT_FIX_ "Applied bonfire input fix"


enum SP_DS1_PLAYER_STATUS_ENUM {
	SP_DS1_PLAYER_STATUS_LOADING = -1,
	SP_DS1_PLAYER_STATUS_HUMAN = 0,
	SP_DS1_PLAYER_STATUS_COOP = 1,
	SP_DS1_PLAYER_STATUS_INVADER = 2,
	SP_DS1_PLAYER_STATUS_HOLLOW = 8
};



//////////////////////// MOD DATA ////////////////////////

void *ds1_base; // Base address of Dark Souls game process
void *player_char_base; // Base address for player character data

SpPointer player_char_status; // Player character status (loading, human, co-op, invader, hollow)


//////////////////////// EXTERNAL SYMBOLS ////////////////////////
extern SpD3D9Device *gl_pSpD3D9Device;
extern void SP_beep(DWORD frequency, DWORD duration);



//////////////////////// FUNCTION PROTOTYPES ////////////////////////
void initialize_ds1_mods();
void load_ds1_mod_keybinds();
int fix_bonfire_input(); // Fixes input bug that causes players to be stuck at a bonfire

#ifdef D3D_DEBUG_INFO
int print_player_character_status();
#endif // D3D_DEBUG_INFO


#endif // _SP_MOD_DARK_SOULS_1_H_