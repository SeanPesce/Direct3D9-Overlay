// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#pragma once


#define WIN32_LEAN_AND_MEAN		

#include <Windows.h>
#include "d3d9.h"
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"

#define _SP_DS_SETTINGS_FILE_ ".\\DS_d3d9_Mod.ini"
#define _SP_DS_SETTINGS_SECTION_KEYBINDS_ "Keybinds"
#define _SP_DS_SETTINGS_SECTION_SETTINGS_ "Settings"
#define _SP_DS_DLL_CHAIN_KEY_ "DLL_Chain"
#define _SP_DS_HOTKEY_STR_NEXT_OL_TXT_POS_KEY_ "ChangeOverlayTextPosition"
#define _SP_DS_HOTKEY_STR_NEXT_OL_TXT_STYLE_KEY_ "ChangeOverlayTextStyle"

