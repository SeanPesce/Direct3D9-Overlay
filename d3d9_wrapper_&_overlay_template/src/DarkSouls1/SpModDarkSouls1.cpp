// Author: Sean Pesce

#include "stdafx.h"
#include "SpModDarkSouls1.h"


void initialize_ds1_mods()
{
	load_ds1_mod_keybinds();

	//ds1_base = get_process_base();
	ds1_base = GetModuleHandle(NULL); // Obtain base address of Dark Souls game process
	player_char_base = (void*)((unsigned int)ds1_base + 0xF7E204); // Obtain base address for player character data

	player_char_status = SpPointer(player_char_base, { 0xA28 }); // Player character status (loading, human, co-op, invader, hollow)
}



void load_ds1_mod_keybinds()
{
	unsigned int key = 0; // Temporarily holds keybinds before they're assigned to functions
	extern std::list<SP_KEY_FUNCTION> keybinds; // List of hotkey-to-function bindings

	if (key = get_vk_hotkey(_SP_D3D9_SETTINGS_FILE_, _SP_D3D9_SETTINGS_SECTION_KEYBINDS_, _SP_DS1_MOD_HOTKEY_BONFIRE_INPUT_FIX_))
	{
		add_function_keybind(key, fix_bonfire_input, &keybinds);
	}
	key = 0;
}


// Fixes input bug that causes players to be stuck at a bonfire
int fix_bonfire_input()
{
	int status = -1;
	player_char_status.read(&status);

	if (status == SP_DS1_PLAYER_STATUS_HOLLOW || status == SP_DS1_PLAYER_STATUS_HUMAN) // Check if player is hollow/human
	{
		SpPointer bonfire_anim_fix = SpPointer((void*)0x12E29E8, { 0x0, 0xFC });

		bonfire_anim_fix.write((uint32_t)0); // Write zero to bonfire animation status address

		gl_pSpD3D9Device->overlay->text_feed->print(_SP_DS1_MOD_MSG_BONFIRE_INPUT_FIX_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);

		SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

		return 0;
	}
	else
	{
		// Player is not hollow/human, so can't be at a bonfire
		return -1;
	}
}



#ifdef D3D_DEBUG_INFO
int print_player_character_status()
{
	std::string msg = "Player character status: ";

	msg.append(std::to_string(*(int*)player_char_status.resolve()));

	gl_pSpD3D9Device->overlay->text_feed->print(msg.c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);

	SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
	Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

	return 0;
}
#endif // D3D_DEBUG_INFO