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
#define _SP_DS_OL_TEXT_FEED_MSG_LIFESPAN_ 2000


// Output
#define _SP_DS_DEFAULT_BEEP_DURATION_ 100
#define _SP_DS_OL_TXT_INTRO_MESSAGE_ "Dark Souls mod template by Sean Pesce"
#define _SP_DS_OL_TXT_OL_ENABLED_MESSAGE_ "Overlay enabled"
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
#define _SP_DS_OL_TXT_SIZE_RESET_MESSAGE_ "Font size reset to "
#define _SP_DS_OL_TXT_SIZE_INCREASED_MESSAGE_ "Font size increased to "
#define _SP_DS_OL_TXT_SIZE_DECREASED_MESSAGE_ "Font size decreased to "
#define _SP_DS_OL_TXT_TEST_MESSAGE_ "TEST MESSAGE"

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
extern HWND ds_game_window;
extern SHORT key_state[256];
extern bool mod_loop_enabled;
extern unsigned int hotkey_toggle_overlay_text;
extern unsigned int hotkey_next_overlay_text_pos;
extern unsigned int hotkey_next_overlay_text_style;
extern unsigned int hotkey_print_overlay_test_message;
extern unsigned int hotkey_toggle_audio_feedback;
extern unsigned int hotkey_toggle_verbose_output;
extern unsigned int hotkey_increase_overlay_text_size;
extern unsigned int hotkey_decrease_overlay_text_size;
extern unsigned int hotkey_reset_overlay_text_size;
extern bool user_pref_overlay_text_enabled;
extern bool user_pref_audio_feedback_enabled;
extern bool user_pref_verbose_output_enabled;
extern int user_pref_overlay_text_size;
extern DWORD user_pref_overlay_text_pos;
extern int user_pref_overlay_text_style;

// Mod variables
int current_overlay_text_size;

void get_ds_window();
BOOL CALLBACK try_ds_window(HWND hwnd, LPARAM lParam);
void next_overlay_text_position(DWORD current_position);
void next_overlay_text_style(int current_style);
void SP_beep(DWORD frequency, DWORD duration);


#endif // _SP_DS_MOD_TEMPLATE_H_