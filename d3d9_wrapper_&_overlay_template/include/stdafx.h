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
// Setting keys
#define _SP_DS_DLL_CHAIN_KEY_ "DLL_Chain"
// Keybind keys
#define _SP_DS_HOTKEY_NEXT_OL_TXT_POS_KEY_ "ChangeOverlayTextPosition"
#define _SP_DS_HOTKEY_NEXT_OL_TXT_STYLE_KEY_ "ChangeOverlayTextStyle"
// User preference keys
#define _SP_DS_OL_TXT_SIZE_KEY_ "OverlayTextSize"
#define _SP_DS_OL_TXT_HORIZONTAL_POS_KEY_ "OverlayTextHorizontalPosition"
#define _SP_DS_OL_TXT_VERTICAL_POS_KEY_ "OverlayTextVerticalPosition"
#define _SP_DS_OL_TXT_STYLE_KEY_ "OverlayTextStyle"
//

