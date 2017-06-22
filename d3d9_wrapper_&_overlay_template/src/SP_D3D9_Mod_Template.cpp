// Author: Sean Pesce

#include "stdafx.h"
#include "SP_D3D9_Mod_Template.h"


// Main loop for the mod thread
void mod_loop()
{
	initialize_mod(true);

	while (mod_loop_enabled)
	{
		
		if (mod_loop_paused)
		{
			initialize_mod(false);
		}
		else if (gl_pSpD3D9Device != NULL && 
			(GetForegroundWindow() == gl_pSpD3D9Device->overlay->focus_window || GetForegroundWindow() == gl_pSpD3D9Device->overlay->device_window // Check if game window is active	// @TODO: This doesn't always detect the window as active (Example: Oblivion)
			|| GetActiveWindow() == gl_pSpD3D9Device->overlay->focus_window || GetActiveWindow() == gl_pSpD3D9Device->overlay->device_window
			|| GetFocus() == gl_pSpD3D9Device->overlay->focus_window || GetFocus() == gl_pSpD3D9Device->overlay->device_window
			/*|| IsWindowEnabled(gl_pSpD3D9Device->focus_window) || IsWindowEnabled(gl_pSpD3D9Device->device_window)*/))
		{

			get_async_keyboard_state(key_state); // Capture all current async key states

			std::list<SP_KEY_FUNCTION>::const_iterator key_func_iterator;
			for (key_func_iterator = keybinds.begin(); key_func_iterator != keybinds.end(); key_func_iterator++)
			{
				if (hotkey_is_down(key_func_iterator->key))
				{
					key_func_iterator->function();
					break;
				}
			}

			Sleep(1);
		}
		else
		{
			// Game window is not   
			Sleep(100);
		}
	}
}

// Initializes mod data and settings based on user preferences
void initialize_mod(bool first_time_setup)
{
	while (mod_loop_enabled && (mod_loop_paused || gl_pSpD3D9Device == NULL || gl_pSpD3D9Device->overlay->game_window == NULL) /* || gl_pSpD3D9Device->focus_window == NULL || gl_pSpD3D9Device->device_window == NULL*/)
	{
		// Wait for the IDirect3DDevice9 wrapper object to be initialized
		Sleep(500);
	}

	if (!mod_loop_enabled)
	{
		return;
	}

	// Set overlay text feed position, style, and font size
	gl_pSpD3D9Device->overlay->text_feed->position = user_pref_overlay_text_pos;
	gl_pSpD3D9Device->overlay->text_feed->style = user_pref_overlay_text_style;
	if (first_time_setup)
	{
		gl_pSpD3D9Device->overlay->text_feed->set_font_height(user_pref_overlay_text_size);
		current_overlay_text_size = user_pref_overlay_text_size;
		test_message_color = _SP_D3D9O_TF_DEFAULT_COLOR_; // Initialize test message text color to white (color changes every time the message is printed
	}
	else
	{
		gl_pSpD3D9Device->overlay->text_feed->set_font_height(current_overlay_text_size);
	}

	// Enable/disable overlay text
	gl_pSpD3D9Device->overlay->text_feed->set_enabled(user_pref_overlay_text_feed_enabled);

	// Enable/disable text feed info line
	gl_pSpD3D9Device->overlay->text_feed->show_info_bar = user_pref_show_text_feed_info_bar;

	#ifdef D3D_DEBUG_INFO
	print_ol_feed("DEBUG: Direct3D debugging is enabled", 0, false, SP_D3D9O_TEXT_COLOR_ORANGE);
	#endif // D3D_DEBUG_INFO

	#ifdef _SP_D3D9O_TF_USE_ID3DX_FONT_
	print_ol_feed("--------------------------------------------------------", 0, false, SP_D3D9O_TEXT_COLOR_CYCLE_ALL);
	#else
	print_ol_feed("--------------------------------------------------------", 0, false);
	#endif // _SP_D3D9O_TF_USE_ID3DX_FONT_
	
	if (user_pref_verbose_output_enabled && first_time_setup)
	{
		// Print whether a d3d9.dll wrapper was chained
		extern std::string d3d9_dll_chain;
		extern bool d3d9_dll_chain_failed;
		if (d3d9_dll_chain.length() > 0)
		{
			if (!d3d9_dll_chain_failed)
			{
				print_ol_feed(std::string("DEBUG: \"").append(d3d9_dll_chain).append("\" successfully loaded as Direct3D 9 wrapper").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
			}
			else
			{
				print_ol_feed(std::string("DEBUG: Failed to load \"").append(d3d9_dll_chain).append("\" as Direct3D 9 wrapper").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
			}
		}
		else
		{
			print_ol_feed("DEBUG: No Direct3D 9 DLL chain specified", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
		}
		// Print number of generic DLLs that were loaded at runtime
		extern unsigned int generic_dll_count;
		if (generic_dll_count == 1)
		{
			print_ol_feed("DEBUG: 1 generic DLL loaded at runtime", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
		}
		else if (generic_dll_count)
		{
			print_ol_feed(std::string("DEBUG: ").append(std::to_string(generic_dll_count)).append(" generic DLLs loaded at runtime").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
		}
		else
		{
			print_ol_feed("DEBUG: No generic DLLs were loaded at runtime", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
		}
#ifdef _SP_DARK_SOULS_1_
		// Print the PvP Watchdog font size, if DSPWSteam.ini was found
		if (dspw_pref_font_size)
		{
			print_ol_feed(std::string("DEBUG: PvP Watchdog overlay font size = ").append(std::to_string(dspw_pref_font_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
		}
		else
		{
			print_ol_feed("DEBUG: PvP Watchdog overlay font size not found (assuming zero)", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9O_TEXT_COLOR_BLUE);
		}
#endif // _SP_DARK_SOULS_1_
	}
}

// Switches the overlay text feed to the next preset position
int next_overlay_text_position()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		switch (gl_pSpD3D9Device->overlay->text_feed->position)
		{
		case _SP_TEXT_TOP_LEFT_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_TOP_CENTER_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_TOP_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_TOP_CENTER_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_TOP_RIGHT_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_TOP_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_TOP_RIGHT_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_CENTER_LEFT_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_MID_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_CENTER_LEFT_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_CENTER_CENTER_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_MID_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_CENTER_CENTER_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_CENTER_RIGHT_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_MID_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_CENTER_RIGHT_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_BOTTOM_LEFT_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_BOTTOM_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_BOTTOM_LEFT_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_BOTTOM_CENTER_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_BOTTOM_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_BOTTOM_CENTER_:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_BOTTOM_RIGHT_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_BOTTOM_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case _SP_TEXT_BOTTOM_RIGHT_:
		default:
			gl_pSpD3D9Device->overlay->text_feed->position = _SP_TEXT_TOP_LEFT_;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_TOP_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		}
		SP_beep(800, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

		return 0;
	}

	return -1; // Text feed not enabled
}

// Switches to the next text overlay style (outlined, shadowed, or plain)
int next_overlay_text_style()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		switch (gl_pSpD3D9Device->overlay->text_feed->style)
		{
		case SP_D3D9O_OUTLINED_TEXT:
			gl_pSpD3D9Device->overlay->text_feed->style = SP_D3D9O_SHADOWED_TEXT;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_SHADOW_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case SP_D3D9O_SHADOWED_TEXT:
			gl_pSpD3D9Device->overlay->text_feed->style = SP_D3D9O_PLAIN_TEXT;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_PLAIN_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		case SP_D3D9O_PLAIN_TEXT:
		default:
			gl_pSpD3D9Device->overlay->text_feed->style = SP_D3D9O_OUTLINED_TEXT;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_OUTLINE_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			break;
		}
		SP_beep(600, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);
		return 0;
	}

	return -1; // Text feed not enabled
}


// Toggles overlay text feed module
int toggle_text_feed()
{
	gl_pSpD3D9Device->overlay->text_feed->set_enabled(!gl_pSpD3D9Device->overlay->text_feed->is_enabled());
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled() && user_pref_verbose_output_enabled)
	{
		print_ol_feed(_SP_D3D9_OL_TXT_OL_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
	}
	SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
	Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

	return 0;
}

// Toggle audio feedback (when hotkeys are pressed)
int toggle_audio_feedback()
{
	user_pref_audio_feedback_enabled = !user_pref_audio_feedback_enabled;
	if (user_pref_audio_feedback_enabled)
	{
		print_ol_feed(_SP_D3D9_OL_TXT_AUDIO_FEEDBACK_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
	}
	else
	{
		print_ol_feed(_SP_D3D9_OL_TXT_AUDIO_FEEDBACK_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
	}
	SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
	Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

	return 0;
}

// Toggle text feed info line (date, time, FPS, etc)
int toggle_info_bar()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		if (!gl_pSpD3D9Device->overlay->text_feed->show_info_bar)
		{
			if (user_pref_show_text_feed_info_bar)
			{
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar = user_pref_show_text_feed_info_bar;
			}
			else
			{
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar = SP_D3D9O_INFO_BAR_TITLE;
			}
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_OL_TEXT_FEED_INFO_STRING_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
		}
		else
		{
			gl_pSpD3D9Device->overlay->text_feed->show_info_bar = SP_D3D9O_INFO_BAR_DISABLED;
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(_SP_D3D9_OL_TXT_OL_TEXT_FEED_INFO_STRING_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
		}
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

		return 0;
	}
	
	return -1; // Text feed not enabled
}

// Toggle verbose text feed output
int toggle_verbose_output()
{
	if (gl_pSpD3D9Device->overlay->enabled_modules)
	{
		user_pref_verbose_output_enabled = !user_pref_verbose_output_enabled;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_D3D9_OL_TXT_VERBOSE_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		else
		{
			print_ol_feed(_SP_D3D9_OL_TXT_VERBOSE_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);

		return 0;
	}

	return -1; // Overlay not enabled
}

// Restore default overlay text feed font size (defined in user preferences)
int reset_text_feed_font_size()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		current_overlay_text_size = user_pref_overlay_text_size;
		gl_pSpD3D9Device->overlay->text_feed->set_font_height(current_overlay_text_size);
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(std::string(_SP_D3D9_OL_TXT_SIZE_RESET_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);
		return 0;
	}

	return -1; // Text feed not enabled
}

// Increase overlay text feed font size
int increase_text_feed_font_size()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		gl_pSpD3D9Device->overlay->text_feed->set_font_height(++current_overlay_text_size);
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(std::string(_SP_D3D9_OL_TXT_SIZE_INCREASED_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);
		return 0;
	}

	return -1; // Text feed not enabled
}

// Decrease overlay text feed font size
int decrease_text_feed_font_size()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		if (current_overlay_text_size > 1) // Check if current font size is already the smallest supported
		{
			// Decrease overlay text feed font size
			gl_pSpD3D9Device->overlay->text_feed->set_font_height(--current_overlay_text_size);
			if (user_pref_verbose_output_enabled)
			{
				print_ol_feed(std::string(_SP_D3D9_OL_TXT_SIZE_DECREASED_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
			}
			SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
			Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);
			return 0;
		}
		else if (user_pref_verbose_output_enabled)
		{
			// Current overlay text feed font size is already the smallest supported; can't decrease it
			print_ol_feed(_SP_D3D9_OL_TXT_SIZE_CANT_DECREASE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, SP_D3D9O_TEXT_COLOR_YELLOW);
			SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
			Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);
			return 1;
		}
	}

	return -1; // Text feed not enabled
}

// Print test message to overlay text feed
int print_overlay_test_message()
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		print_ol_feed(_SP_D3D9_OL_TXT_TEST_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, test_message_color);
		switch (test_message_color)
		{
		case SP_D3D9O_TEXT_COLOR_WHITE:
			test_message_color = SP_D3D9O_TEXT_COLOR_BLACK;
			break;
		case SP_D3D9O_TEXT_COLOR_BLACK:
			test_message_color = SP_D3D9O_TEXT_COLOR_RED;
			break;
		case SP_D3D9O_TEXT_COLOR_RED:
			test_message_color = SP_D3D9O_TEXT_COLOR_ORANGE;
			break;
		case SP_D3D9O_TEXT_COLOR_ORANGE:
			test_message_color = SP_D3D9O_TEXT_COLOR_YELLOW;
			break;
		case SP_D3D9O_TEXT_COLOR_YELLOW:
			test_message_color = SP_D3D9O_TEXT_COLOR_GREEN;
			break;
		case SP_D3D9O_TEXT_COLOR_GREEN:
			test_message_color = SP_D3D9O_TEXT_COLOR_CYAN;
			break;
		case SP_D3D9O_TEXT_COLOR_CYAN:
			test_message_color = SP_D3D9O_TEXT_COLOR_BLUE;
			break;
		case SP_D3D9O_TEXT_COLOR_BLUE:
			test_message_color = SP_D3D9O_TEXT_COLOR_PURPLE;
			break;
		case SP_D3D9O_TEXT_COLOR_PURPLE:
			test_message_color = SP_D3D9O_TEXT_COLOR_PINK;
			break;
#ifdef _SP_D3D9O_TF_USE_ID3DX_FONT_
		case SP_D3D9O_TEXT_COLOR_PINK:
			test_message_color = SP_D3D9O_TEXT_COLOR_CYCLE_ALL;
			break;
		case SP_D3D9O_TEXT_COLOR_CYCLE_ALL:
			test_message_color = SP_D3D9O_TEXT_COLOR_WHITE;
			break;
#else
		case SP_D3D9O_TEXT_COLOR_PINK:
			test_message_color = SP_D3D9O_TEXT_COLOR_WHITE;
			break;
#endif // _SP_D3D9O_TF_USE_ID3DX_FONT_
		default:
			test_message_color = _SP_D3D9O_TF_DEFAULT_COLOR_;
			break;
		}

		SP_beep(500, _SP_D3D9_DEFAULT_BEEP_DURATION_);
		Sleep(_SP_D3D9_KEYPRESS_WAIT_TIME_);
		return 0;
	}

	return -1; // Text feed not enabled
}


// Beeps at the specified frequency for a specified duration (in milliseconds),
//		if audio feedback is enabled. If audio is disabled and wait==true, the 
//		thread is put to sleep for the specified duration instead.
void SP_beep(DWORD frequency, DWORD duration, bool wait)
{
	if (user_pref_audio_feedback_enabled)
	{
		Beep(frequency, duration);
	}
	else if(wait)
	{
		Sleep(duration);
	}
}

// Beeps at the specified frequency for a specified duration (in milliseconds),
//		if audio feedback is enabled. If audio is disabled, the thread is put
//		to sleep for the specified duration instead.
// Functionality is the same as calling SP_beep(frequency, duration, true);
void SP_beep(DWORD frequency, DWORD duration)
{
	if (user_pref_audio_feedback_enabled)
	{
		Beep(frequency, duration);
	}
	else
	{
		Sleep(duration);
	}
}