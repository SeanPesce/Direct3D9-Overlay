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
			(GetForegroundWindow() == gl_pSpD3D9Device->focus_window || GetForegroundWindow() == gl_pSpD3D9Device->device_window // Check if game window is active	// @TODO: This doesn't always detect the window as active (Example: Oblivion)
			|| GetActiveWindow() == gl_pSpD3D9Device->focus_window || GetActiveWindow() == gl_pSpD3D9Device->device_window
			|| GetFocus() == gl_pSpD3D9Device->focus_window || GetFocus() == gl_pSpD3D9Device->device_window
			/*|| IsWindowEnabled(gl_pSpD3D9Device->focus_window) || IsWindowEnabled(gl_pSpD3D9Device->device_window)*/))
		{

			get_async_keyboard_state(key_state); // Capture all current async key states

			if (hotkey_is_down(hotkey_toggle_overlay_text_feed))
			{
				// Toggle overlay text feed
				gl_pSpD3D9Device->text_overlay.enabled = !gl_pSpD3D9Device->text_overlay.enabled;
				if (gl_pSpD3D9Device->text_overlay.enabled && user_pref_verbose_output_enabled)
				{
					print_ol_feed(_SP_DS_OL_TXT_OL_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (hotkey_is_down(hotkey_toggle_audio_feedback))
			{
				// Toggle audio feedback
				user_pref_audio_feedback_enabled = !user_pref_audio_feedback_enabled;
				if (user_pref_audio_feedback_enabled)
				{
					print_ol_feed(_SP_DS_OL_TXT_AUDIO_FEEDBACK_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				else
				{
					print_ol_feed(_SP_DS_OL_TXT_AUDIO_FEEDBACK_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_toggle_text_feed_info_bar))
			{
				// Toggle info line (date, time, FPS, etc)
				if (!gl_pSpD3D9Device->show_text_feed_info_bar)
				{
					if (user_pref_show_text_feed_info_bar)
					{
						gl_pSpD3D9Device->show_text_feed_info_bar = user_pref_show_text_feed_info_bar;
					}
					else
					{
						gl_pSpD3D9Device->show_text_feed_info_bar = SP_D3D9_INFO_BAR_TITLE;
					}
					if (user_pref_verbose_output_enabled)
					{
						print_ol_feed(_SP_DS_OL_TXT_OL_TEXT_FEED_INFO_STRING_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
					}
				}
				else
				{
					gl_pSpD3D9Device->show_text_feed_info_bar = 0;
					if (user_pref_verbose_output_enabled)
					{
						print_ol_feed(_SP_DS_OL_TXT_OL_TEXT_FEED_INFO_STRING_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
					}
				}
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_next_overlay_text_pos))
			{
				// Change to next overlay text feed position preset
				next_overlay_text_position(gl_pSpD3D9Device->text_overlay.text_format);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_next_overlay_text_style))
			{
				// Change to next overlay text style
				next_overlay_text_style(gl_pSpD3D9Device->text_overlay.text_style);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_toggle_verbose_output))
			{
				// Toggle verbose text output
				user_pref_verbose_output_enabled = !user_pref_verbose_output_enabled;
				if (user_pref_verbose_output_enabled)
				{
					print_ol_feed(_SP_DS_OL_TXT_VERBOSE_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				else
				{
					print_ol_feed(_SP_DS_OL_TXT_VERBOSE_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_toggle_multicolor_feed))
			{
				// Toggle multicolor text feed
				gl_pSpD3D9Device->multicolor_overlay_text_feed_enabled = !gl_pSpD3D9Device->multicolor_overlay_text_feed_enabled;
				if (user_pref_verbose_output_enabled && gl_pSpD3D9Device->multicolor_overlay_text_feed_enabled)
				{
					print_ol_feed(_SP_DS_OL_TXT_MULTICOLOR_FEED_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				else if (user_pref_verbose_output_enabled)
				{
					print_ol_feed(_SP_DS_OL_TXT_MULTICOLOR_FEED_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_reset_overlay_text_size))
			{
				// Restore default overlay text size (defined in user preferences)
				current_overlay_text_size = user_pref_overlay_text_size;
				gl_pSpD3D9Device->text_overlay_new_font_size = current_overlay_text_size;
				if (user_pref_verbose_output_enabled)
				{
					print_ol_feed(std::string(_SP_DS_OL_TXT_SIZE_RESET_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_increase_overlay_text_size))
			{
				// Increase overlay text size
				gl_pSpD3D9Device->text_overlay_new_font_size = ++current_overlay_text_size;
				if (user_pref_verbose_output_enabled)
				{
					print_ol_feed(std::string(_SP_DS_OL_TXT_SIZE_INCREASED_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_decrease_overlay_text_size))
			{
				if (current_overlay_text_size > 1) // Check if current font size is already the smallest supported
				{
					// Decrease overlay text feed font size
					gl_pSpD3D9Device->text_overlay_new_font_size = --current_overlay_text_size;
					if (user_pref_verbose_output_enabled)
					{
						print_ol_feed(std::string(_SP_DS_OL_TXT_SIZE_DECREASED_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
					}
				}
				else if (user_pref_verbose_output_enabled)
				{
					// Current overlay text feed font size is already the smallest supported; can't decrease it
					print_ol_feed(_SP_DS_OL_TXT_SIZE_CANT_DECREASE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, SP_D3D9_TEXT_COLOR_YELLOW);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pSpD3D9Device->text_overlay.enabled && hotkey_is_down(hotkey_print_overlay_test_message))
			{
				// Print test message to text overlay feed
				print_ol_feed(_SP_DS_OL_TXT_TEST_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, test_message_color);
				if (test_message_color >= 0 && test_message_color < _SP_D3D9_TEXT_COLOR_COUNT_-1)
				{
					test_message_color++; // Get the next text color
				}
				else
				{
					test_message_color = 0; // Last text color reached, reset to first color
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}

			// @TODO: Other hotkeys should also be checked and handled here

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
	while (mod_loop_enabled && (mod_loop_paused || gl_pSpD3D9Device == NULL || gl_pSpD3D9Device->game_window == NULL || !gl_pSpD3D9Device->initialized) /* || gl_pSpD3D9Device->focus_window == NULL || gl_pSpD3D9Device->device_window == NULL*/)
	{
		// Wait for the IDirect3DDevice9 wrapper object to be initialized
		Sleep(500);
	}

	if (!mod_loop_enabled)
	{
		return;
	}

	// Enable/disable multicolor overlay text
	gl_pSpD3D9Device->multicolor_overlay_text_feed_enabled = user_pref_multicolor_feed_enabled;

	// Set overlay text feed position, style, and font size
	gl_pSpD3D9Device->text_overlay.text_format = user_pref_overlay_text_pos;
	gl_pSpD3D9Device->text_overlay.text_style = user_pref_overlay_text_style;
	if (first_time_setup)
	{
		gl_pSpD3D9Device->text_overlay_new_font_size = user_pref_overlay_text_size;
		current_overlay_text_size = user_pref_overlay_text_size;
		test_message_color = SP_D3D9_TEXT_COLOR_WHITE; // Initialize test message text color to white (color changes every time the message is printed
	}
	else
	{
		gl_pSpD3D9Device->text_overlay_new_font_size = current_overlay_text_size;
	}

	// Enable/disable overlay text
	gl_pSpD3D9Device->text_overlay.enabled = user_pref_overlay_text_feed_enabled;

	// Enable/disable text feed info line
	gl_pSpD3D9Device->show_text_feed_info_bar = user_pref_show_text_feed_info_bar;

	#ifdef D3D_DEBUG_INFO
	print_ol_feed("DEBUG: Direct3D debugging is enabled", 0, false, SP_D3D9_TEXT_COLOR_ORANGE);
	#endif // D3D_DEBUG_INFO

	print_ol_feed("--------------------------------------------------------", 0, false, SP_D3D9_TEXT_COLOR_CYCLE_ALL);
	
	
	if (user_pref_verbose_output_enabled && first_time_setup)
	{
		// Print whether a d3d9.dll wrapper was chained
		extern std::string d3d9_dll_chain;
		extern bool d3d9_dll_chain_failed;
		if (d3d9_dll_chain.length() > 0)
		{
			if (!d3d9_dll_chain_failed)
			{
				print_ol_feed(std::string("DEBUG: \"").append(d3d9_dll_chain).append("\" successfully loaded as Direct3D 9 wrapper").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
			}
			else
			{
				print_ol_feed(std::string("DEBUG: Failed to load \"").append(d3d9_dll_chain).append("\" as Direct3D 9 wrapper").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
			}
		}
		else
		{
			print_ol_feed("DEBUG: No Direct3D 9 DLL chain specified", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
		}
		// Print number of generic DLLs that were loaded at runtime
		extern unsigned int generic_dll_count;
		if (generic_dll_count == 1)
		{
			print_ol_feed("DEBUG: 1 generic DLL loaded at runtime", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
		}
		else if (generic_dll_count)
		{
			print_ol_feed(std::string("DEBUG: ").append(std::to_string(generic_dll_count)).append(" generic DLLs loaded at runtime").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
		}
		else
		{
			print_ol_feed("DEBUG: No generic DLLs were loaded at runtime", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
		}
		// Print the PvP Watchdog font size, if DSPWSteam.ini was found
		if (dspw_pref_font_size)
		{
			print_ol_feed(std::string("DEBUG: PvP Watchdog overlay font size = ").append(std::to_string(dspw_pref_font_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
		}
		else
		{
			print_ol_feed("DEBUG: PvP Watchdog overlay font size not found (assuming zero)", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 10, true, SP_D3D9_TEXT_COLOR_BLUE);
		}
	}
}

// Switches the overlay text feed to the next preset position
void next_overlay_text_position(DWORD current_position)
{
	switch (current_position)
	{
	case _SP_TEXT_TOP_LEFT_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_TOP_CENTER_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_TOP_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_TOP_CENTER_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_TOP_RIGHT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_TOP_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_TOP_RIGHT_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_CENTER_LEFT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_MID_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_CENTER_LEFT_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_CENTER_CENTER_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_MID_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_CENTER_CENTER_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_CENTER_RIGHT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_MID_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_CENTER_RIGHT_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_BOTTOM_LEFT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_BOTTOM_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_BOTTOM_LEFT_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_BOTTOM_CENTER_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_BOTTOM_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_BOTTOM_CENTER_:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_BOTTOM_RIGHT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_BOTTOM_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_BOTTOM_RIGHT_:
	default:
		gl_pSpD3D9Device->text_overlay.text_format = _SP_TEXT_TOP_LEFT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_TOP_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	}
	SP_beep(800, _SP_DS_DEFAULT_BEEP_DURATION_);
}

// Switches to the next text overlay style (outlined, shadowed, or plain)
void next_overlay_text_style(int current_style)
{
	switch (current_style)
	{
	case SP_D3D9_OUTLINED_TEXT:
		gl_pSpD3D9Device->text_overlay.text_style = SP_D3D9_SHADOWED_TEXT;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_SHADOW_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case SP_D3D9_SHADOWED_TEXT:
		gl_pSpD3D9Device->text_overlay.text_style = SP_D3D9_PLAIN_TEXT;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_PLAIN_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case SP_D3D9_PLAIN_TEXT:
	default:
		gl_pSpD3D9Device->text_overlay.text_style = SP_D3D9_OUTLINED_TEXT;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_OUTLINE_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	}
	SP_beep(600, _SP_DS_DEFAULT_BEEP_DURATION_);
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