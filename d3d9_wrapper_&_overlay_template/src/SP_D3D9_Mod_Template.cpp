// Author: Sean Pesce

#include "stdafx.h"
#include "SP_D3D9_Mod_Template.h"


// Main loop for the mod thread
void mod_loop()
{
	initialize_mod();

	while (mod_loop_enabled)
	{
		// Check if game window is active
		// @TODO: This doesn't always detect the window as active (Example: Oblivion)
		if ((GetForegroundWindow() == gl_pmyIDirect3DDevice9->focus_window || GetForegroundWindow() == gl_pmyIDirect3DDevice9->device_window
			|| GetActiveWindow() == gl_pmyIDirect3DDevice9->focus_window || GetActiveWindow() == gl_pmyIDirect3DDevice9->device_window
			|| GetFocus() == gl_pmyIDirect3DDevice9->focus_window || GetFocus() == gl_pmyIDirect3DDevice9->device_window
			/*|| IsWindowEnabled(gl_pmyIDirect3DDevice9->focus_window) || IsWindowEnabled(gl_pmyIDirect3DDevice9->device_window)*/)) {

			get_async_keyboard_state(key_state); // Capture all current async key states

			if (hotkey_is_down(hotkey_toggle_overlay_text_feed))
			{
				// Toggle overlay text feed
				gl_pmyIDirect3DDevice9->text_overlay.enabled = !gl_pmyIDirect3DDevice9->text_overlay.enabled;
				if (gl_pmyIDirect3DDevice9->text_overlay.enabled && user_pref_verbose_output_enabled)
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
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_toggle_info_watermark))
			{
				// Toggle info watermark (date, time, FPS, etc)
				if (!gl_pmyIDirect3DDevice9->show_text_watermark)
				{
					if (user_pref_show_text_watermark)
					{
						gl_pmyIDirect3DDevice9->show_text_watermark = user_pref_show_text_watermark;
					}
					else
					{
						gl_pmyIDirect3DDevice9->show_text_watermark = SP_DX9_WATERMARK_TITLE;
					}
					if (user_pref_verbose_output_enabled)
					{
						print_ol_feed(_SP_DS_OL_TXT_OL_TEXT_WATERMARK_ENABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
					}
				}
				else
				{
					gl_pmyIDirect3DDevice9->show_text_watermark = 0;
					if (user_pref_verbose_output_enabled)
					{
						print_ol_feed(_SP_DS_OL_TXT_OL_TEXT_WATERMARK_DISABLED_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
					}
				}
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_next_overlay_text_pos))
			{
				// Change to next overlay text feed position preset
				next_overlay_text_position(gl_pmyIDirect3DDevice9->text_overlay.text_format);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_next_overlay_text_style))
			{
				// Change to next overlay text style
				next_overlay_text_style(gl_pmyIDirect3DDevice9->text_overlay.text_style);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_toggle_verbose_output))
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
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_toggle_multicolor_feed))
			{
				// Toggle multicolor text feed
				gl_pmyIDirect3DDevice9->multicolor_overlay_text_feed_enabled = !gl_pmyIDirect3DDevice9->multicolor_overlay_text_feed_enabled;
				if (user_pref_verbose_output_enabled && gl_pmyIDirect3DDevice9->multicolor_overlay_text_feed_enabled)
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
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_reset_overlay_text_size))
			{
				// Restore default overlay text size (defined in user preferences)
				current_overlay_text_size = user_pref_overlay_text_size;
				gl_pmyIDirect3DDevice9->SP_DX9_set_text_height(current_overlay_text_size);
				if (user_pref_verbose_output_enabled)
				{
					print_ol_feed(std::string(_SP_DS_OL_TXT_SIZE_RESET_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_increase_overlay_text_size))
			{
				// Increase overlay text size
				gl_pmyIDirect3DDevice9->SP_DX9_set_text_height(++current_overlay_text_size);
				if (user_pref_verbose_output_enabled)
				{
					print_ol_feed(std::string(_SP_DS_OL_TXT_SIZE_INCREASED_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_decrease_overlay_text_size))
			{
				if (current_overlay_text_size > 1) // Check if current font size is already the smallest supported
				{
					// Decrease overlay text feed font size
					gl_pmyIDirect3DDevice9->SP_DX9_set_text_height(--current_overlay_text_size);
					if (user_pref_verbose_output_enabled)
					{
						print_ol_feed(std::string(_SP_DS_OL_TXT_SIZE_DECREASED_MESSAGE_).append(std::to_string(current_overlay_text_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
					}
				}
				else if (user_pref_verbose_output_enabled)
				{
					// Current overlay text feed font size is already the smallest supported; can't decrease it
					print_ol_feed(_SP_DS_OL_TXT_SIZE_CANT_DECREASE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, SP_DX9_TEXT_COLOR_YELLOW);
				}
				SP_beep(500, _SP_DS_DEFAULT_BEEP_DURATION_);
				Sleep(_SP_DS_KEYPRESS_WAIT_TIME_);
			}
			else if (gl_pmyIDirect3DDevice9->text_overlay.enabled && hotkey_is_down(hotkey_print_overlay_test_message))
			{
				// Print test message to text overlay feed
				print_ol_feed(_SP_DS_OL_TXT_TEST_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, test_message_color);
				if (test_message_color >= 0 && test_message_color < _SP_DX9_TEXT_COLOR_COUNT_-1)
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
			// Game window is not active
			Sleep(100);
		}

		
	}
}

// Initializes mod data and settings based on user preferences
void initialize_mod()
{
	while (gl_pmyIDirect3DDevice9 == NULL || gl_pmyIDirect3DDevice9->game_window == NULL /* || gl_pmyIDirect3DDevice9->focus_window == NULL || gl_pmyIDirect3DDevice9->device_window == NULL*/)
	{
		// Wait for the IDirect3DDevice9 wrapper object to be initialized
		Sleep(500);
	}

	// Enable/disable multicolor overlay text
	gl_pmyIDirect3DDevice9->multicolor_overlay_text_feed_enabled = user_pref_multicolor_feed_enabled;

	// Initialize test message text color to white (color changes every time the message is printed
	test_message_color = 0;

	// Set overlay text feed position, style, and font size
	gl_pmyIDirect3DDevice9->text_overlay.text_format = user_pref_overlay_text_pos;
	gl_pmyIDirect3DDevice9->text_overlay.text_style = user_pref_overlay_text_style;
	gl_pmyIDirect3DDevice9->SP_DX9_set_text_height(user_pref_overlay_text_size);
	current_overlay_text_size = user_pref_overlay_text_size;

	// Enable/disable overlay text
	gl_pmyIDirect3DDevice9->text_overlay.enabled = user_pref_overlay_text_feed_enabled;

	// Enable/disable overlay text watermark
	gl_pmyIDirect3DDevice9->show_text_watermark = user_pref_show_text_watermark;

	print_ol_feed("--------------------------------------------------------", 0, false, SP_DX9_TEXT_COLOR_CYCLE_ALL);
	
	if (user_pref_verbose_output_enabled)
	{
		if (dspw_pref_font_size)
		{
			print_ol_feed(std::string("DEBUG: PvP Watchdog overlay font size = ").append(std::to_string(dspw_pref_font_size)).c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 8, true, SP_DX9_TEXT_COLOR_BLUE);
		}
		else
		{
			print_ol_feed("DEBUG: PvP Watchdog overlay font size not found (assuming zero)", _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 8, true, SP_DX9_TEXT_COLOR_BLUE);
		}
	}
	
	if (user_pref_load_dinput8_early)
	{
		// Notify user that dinput8.dll was preloaded
		print_ol_feed(_SP_DS_OL_TXT_DINPUT8_LOADED_EARLY_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 8, true, SP_DX9_TEXT_COLOR_BLUE);
	}
	else if (user_pref_verbose_output_enabled)
	{
		// Notify user that dinput8.dll was not preloaded (but only if verbose output is enabled)
		print_ol_feed(_SP_DS_OL_TXT_DINPUT8_NOT_LOADED_EARLY_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ * 8, true, SP_DX9_TEXT_COLOR_BLUE);
	}
}

// Switches the overlay text feed to the next preset position
void next_overlay_text_position(DWORD current_position)
{
	switch (current_position)
	{
	case _SP_TEXT_TOP_LEFT_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_TOP_CENTER_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_TOP_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_TOP_CENTER_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_TOP_RIGHT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_TOP_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_TOP_RIGHT_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_CENTER_LEFT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_MID_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_CENTER_LEFT_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_CENTER_CENTER_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_MID_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_CENTER_CENTER_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_CENTER_RIGHT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_MID_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_CENTER_RIGHT_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_BOTTOM_LEFT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_BOTTOM_LEFT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_BOTTOM_LEFT_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_BOTTOM_CENTER_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_BOTTOM_CENTER_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_BOTTOM_CENTER_:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_BOTTOM_RIGHT_;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_BOTTOM_RIGHT_POS_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case _SP_TEXT_BOTTOM_RIGHT_:
	default:
		gl_pmyIDirect3DDevice9->text_overlay.text_format = _SP_TEXT_TOP_LEFT_;
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
	case SP_DX9_BORDERED_TEXT:
		gl_pmyIDirect3DDevice9->text_overlay.text_style = SP_DX9_SHADOWED_TEXT;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_SHADOW_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case SP_DX9_SHADOWED_TEXT:
		gl_pmyIDirect3DDevice9->text_overlay.text_style = SP_DX9_PLAIN_TEXT;
		if (user_pref_verbose_output_enabled)
		{
			print_ol_feed(_SP_DS_OL_TXT_PLAIN_STYLE_MESSAGE_, _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true);
		}
		break;
	case SP_DX9_PLAIN_TEXT:
	default:
		gl_pmyIDirect3DDevice9->text_overlay.text_style = SP_DX9_BORDERED_TEXT;
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