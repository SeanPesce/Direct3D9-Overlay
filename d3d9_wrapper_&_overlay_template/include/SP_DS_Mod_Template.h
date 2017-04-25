// Author: Sean Pesce

#pragma once

#ifndef _SP_DS_MOD_TEMPLATE_H_
	#define _SP_DS_MOD_TEMPLATE_H_

#include <Windows.h>
#include <string>
#include "SP_IO.hpp"
#include "myIDirect3DDevice9.h"


#define hotkey_is_down(hotkey) (hotkey != 0 && (key_state[hotkey] & _SP_KEY_DOWN_)) // Checks if a hotkey is currently being pressed, but only if it's a valid virtual key

#define _SP_TEXT_TOP_LEFT_ (DT_NOCLIP | DT_TOP | DT_LEFT)
#define _SP_TEXT_TOP_CENTER_ (DT_NOCLIP | DT_TOP | DT_CENTER)
#define _SP_TEXT_TOP_RIGHT_ (DT_NOCLIP | DT_TOP | DT_RIGHT)
#define _SP_TEXT_CENTER_LEFT_ (DT_NOCLIP | DT_VCENTER | DT_LEFT)
#define _SP_TEXT_CENTER_CENTER_ (DT_NOCLIP | DT_VCENTER | DT_CENTER)
#define _SP_TEXT_CENTER_RIGHT_ (DT_NOCLIP | DT_VCENTER | DT_RIGHT)
#define _SP_TEXT_BOTTOM_LEFT_ (DT_NOCLIP | DT_BOTTOM | DT_LEFT)
#define _SP_TEXT_BOTTOM_CENTER_ (DT_NOCLIP | DT_BOTTOM | DT_CENTER)
#define _SP_TEXT_BOTTOM_RIGHT_ (DT_NOCLIP | DT_BOTTOM | DT_RIGHT)

#define _SP_DS_WINDOW_CLASS_ "DARK SOULS"
#define _SP_DS_KEYPRESS_WAIT_TIME_ 100


// Output strings
#define _SP_DS_OL_TXT_INTRO_MESSAGE_ "Dark Souls mod template by Sean Pesce"
#define _SP_DS_OL_TXT_OUTLINE_STYLE_MESSAGE_ "Text style changed to outlined"
#define _SP_DS_OL_TXT_SHADOW_STYLE_MESSAGE_ "Text style changed to shadowed"
#define _SP_DS_OL_TXT_PLAIN_STYLE_MESSAGE_ "Text style changed to plain"

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
extern HWND ds_game_window;
extern SHORT key_state[256];
extern bool mod_loop_enabled;
extern unsigned int hotkey_toggle_overlay_text;
extern unsigned int hotkey_next_overlay_text_pos;
extern unsigned int hotkey_next_overlay_text_style;
extern bool user_pref_overlay_text_enabled;
extern int user_pref_overlay_text_size;
extern DWORD user_pref_overlay_text_pos;
extern int user_pref_overlay_text_style;


void get_ds_window();
BOOL CALLBACK try_ds_window(HWND hwnd, LPARAM lParam);
void next_overlay_text_position(DWORD current_position);
void next_overlay_text_style(int current_style);


#endif // _SP_DS_MOD_TEMPLATE_H_