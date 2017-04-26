// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#pragma once


#define WIN32_LEAN_AND_MEAN		

#include <Windows.h>
#include <algorithm>
#include "d3d9.h"
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"

#define _SP_DS_SETTINGS_FILE_ ".\\DS_d3d9_Mod.ini"
#define _SP_DS_SETTINGS_SECTION_KEYBINDS_ "Keybinds"
#define _SP_DS_SETTINGS_SECTION_SETTINGS_ "Settings"
#define _SP_DS_SETTINGS_SECTION_PREFS_ "Preferences"
#define _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_ "Developer Keybinds"
#define _SP_DS_SETTINGS_SECTION_DEV_PREFS_ "Developer Preferences"
// Setting keys
#define _SP_DS_DLL_CHAIN_KEY_ "DLL_Chain"
// Keybind keys
#define _SP_DS_HOTKEY_TOGGLE_OL_TXT_KEY_ "ToggleOverlay"
#define _SP_DS_HOTKEY_NEXT_OL_TXT_POS_KEY_ "ChangeOverlayTextPosition"
#define _SP_DS_HOTKEY_NEXT_OL_TXT_STYLE_KEY_ "ChangeOverlayTextStyle"
#define _SP_DS_HOTKEY_PRINT_OL_TXT_TEST_MSG_KEY_ "PrintTestMessage"
#define _SP_DS_HOTKEY_TOGGLE_AUDIO_FEEDBACK_KEY_ "ToggleAudioFeedback"
#define _SP_DS_HOTKEY_TOGGLE_VERBOSE_OUTPUT_KEY_ "ToggleVerboseOutput"
#define _SP_DS_HOTKEY_INCREASE_TXT_SIZE_KEY_ "IncreaseTextSize"
#define _SP_DS_HOTKEY_DECREASE_TXT_SIZE_KEY_ "DecreaseTextSize"
#define _SP_DS_HOTKEY_RESET_TXT_SIZE_KEY_ "ResetTextSize"
#define _SP_DS_HOTKEY_TOGGLE_MULTICOLOR_FEED_KEY_ "ToggleMultiColorFeed"
// User preference keys
#define _SP_DS_OL_TXT_ENABLED_KEY_ "EnableOverlay"
#define _SP_DS_OL_TXT_SIZE_KEY_ "OverlayTextSize"
#define _SP_DS_OL_TXT_HORIZONTAL_POS_KEY_ "OverlayTextHorizontalPosition"
#define _SP_DS_OL_TXT_VERTICAL_POS_KEY_ "OverlayTextVerticalPosition"
#define _SP_DS_OL_TXT_STYLE_KEY_ "OverlayTextStyle"
#define _SP_DS_OL_TXT_AUDIO_ENABLED_KEY_ "EnableAudioFeedback"
#define _SP_DS_OL_TXT_VERBOSE_OUTPUT_ENABLED_KEY_ "EnableVerboseOutput"
#define _SP_DS_OL_TXT_MULTICOLOR_FEED_ENABLED_KEY_ "EnableMultiColorTextFeed"
//

