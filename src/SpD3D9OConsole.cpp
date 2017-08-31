// Author: Sean Pesce

#include "stdafx.h"
#include "SpD3D9OConsole.h"
#include "VersionHelpers.h"  // To determine which fonts are available
#include "SpD3D9OConsoleTextSelection.h"
#include "Resource.h" // For Windows mouse cursor


// Static class data
seqan::Index<seqan::StringSet<seqan::String<char>>> *SpD3D9OConsole::commands_index = NULL;
std::vector<SP_D3D9O_CONSOLE_COMMAND> SpD3D9OConsole::commands;		// Set of available console commands and corresponding functions
seqan::StringSet<seqan::String<char>> SpD3D9OConsole::commands_set;	// Set of available console command strings


SpD3D9OConsole::SpD3D9OConsole(SpD3D9Overlay *new_overlay)
{
	overlay = new_overlay;

	if (overlay == NULL)
	{
		// Handle error?
	}

	// Get user-preferred console font size
	extern int user_pref_console_text_size;
	font_height = user_pref_console_text_size;


	// Initialize empty command log
	command_log.push_back("");
	command_log_position = 1;

	if (output_log_capacity < output_log_displayed_lines)
	{
		output_log_capacity = output_log_displayed_lines;
	}

	clear_selection();
	clear();

	// Inititalize font interface
	font = new CD3DFont(font_family.c_str(), font_height, _SP_D3D9O_C_DEFAULT_FONT_FLAGS_);
	font->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	font->RestoreDeviceObjects();

	// Initialize text cursor
	if (!IsWindowsVistaOrGreater())
	{
		// Use older (uglier) font
		cursor_font_family = _SP_D3D9O_C_DEFAULT_OLD_OS_CURSOR_FONT_FAMILY_;
	}
	cursor = new CD3DFont(cursor_font_family.c_str(), cursor_size, 0);
	cursor->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	cursor->RestoreDeviceObjects();
}



SpD3D9OConsole::~SpD3D9OConsole()
{
	if (commands_index != NULL)
	{
		delete commands_index;
		commands_index = NULL;
	}

	if (win_cursor_sprite != NULL)
	{
		win_cursor_sprite->Release();
		win_cursor_sprite = NULL;
	}
	if (win_cursor_tex != NULL)
	{
		win_cursor_tex->Release();
		win_cursor_tex = NULL;
	}
}



void SpD3D9OConsole::get_input()
{
	extern SHORT key_state[256];	// Buffer for async key states

	#ifdef _SP_USE_ASYNC_KEY_STATE_INPUT_
		get_async_keyboard_state(key_state); // Capture all current async key states

		if (hotkey_is_down(_CLOSE_CONSOLE_KEY_))
		{
			toggle();
			return;
		}
	#endif // _SP_USE_ASYNC_KEY_STATE_INPUT_

	if (SpD3D9OInputHandler::get() == NULL
		#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
			|| (SpD3D9OInputHandler::get()->oGetRawInputData == NULL)
		#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
		)
	{
		get_async_keyboard_state(key_state); // Capture all current async key states
		if (hotkey_is_down(_CLOSE_CONSOLE_KEY_))
		{
			toggle();
			return;
		}
	}

	#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		SpD3D9OInputHandler::get()->get_dinput_data();
	#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


	#if (defined _SP_USE_DETOUR_DISPATCH_MSG_INPUT_ || defined _SP_USE_WIN_HOOK_EX_INPUT_)
		MSG msg;
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_ || _SP_USE_WIN_HOOK_EX_INPUT_
}


void SpD3D9OConsole::draw()
{
	// Check if cursor texture was initialized
	if (win_cursor_tex == NULL)
	{
		init_win_cursor();
	}

	// Check if font size was changed
	if ((unsigned int)font->GetFontHeight() != font_height || (unsigned int)cursor->GetFontHeight() != cursor_size)
	{
		update_fonts_and_cursor();
	}

	// Get screenspace values
	RECT window_rect/*, autocomplete_lims*/; // Window dimensions
	SIZE char_size;
	long	max_chars, // Maximum characters per line
			max_input_chars; // Maximum input characters per line
	std::string full_prompt;
	std::vector<std::string> autocomplete_matches;
	int longest_autocomplete/*, autocomplete_hover*/;
	get_screenspace_values(&window_rect, &char_size, NULL, &max_chars, NULL, NULL, &full_prompt, &max_input_chars, &autocomplete_matches, &longest_autocomplete, /*&autocomplete_lims*/NULL, /*&autocomplete_hover*/NULL, 7);


	// Create background/border rectangles
	long console_height = (long)(char_size.cy  * (output_log_displayed_lines + 1));
	//long console_height = (long)((font_height  * (output_log_displayed_lines + 1))*1.5);
	
	D3DRECT border = { 0, 0, window_rect.right - window_rect.left, console_height+(2* (int)border_width) };
	D3DRECT background = { (int)border_width, (int)border_width, (window_rect.right - window_rect.left)- (int)border_width, console_height + (int)border_width };


	// Calculate maximum number of characters & lines that can be displayed on-screen
	//unsigned int max_lines = (unsigned int)((/*(float)*/console_height / 1.5f) / /*(float)*/char_size.cx);

	
	// Concatenate prompt if it's too long
	if ((int)prompt.length() > (max_chars - 1))
	{
		prompt = prompt.substr(0, max_chars - 1);
	}

	// Set the displayable substring of the current input
	set_input_string_display_limits(max_input_chars);
	

	// Build console output log string
	std::string output_string = "";
	for (int i = output_log_displayed_lines - 1; i >= 0; i--)
	{
		std::string *current_line = &output_log.at(output_log.size() - (1 + i));
		if ((int)current_line->length() > max_chars)
		{
			output_string.append(std::string(*current_line).substr(0, max_chars)).append("\n");
		}
		else
		{
			output_string.append(*current_line).append("\n");
		}
	}

	// Determine whether to draw caret
	DWORD current_time = GetTickCount();
	if (caret_blink_delay != 0 && current_time >= next_caret_blink)
	{
		show_caret = !show_caret;
		next_caret_blink = current_time + caret_blink_delay;
	}

	// Build input line (prompt and current command)
	std::string cur_cmd = command;
	if (show_caret && (caret_position < command.length()))
	{
		cur_cmd.erase(caret_position, 1);
		cur_cmd.insert(caret_position, 1, caret);
	}
	if ((int)cur_cmd.length() > max_input_chars)
	{
		cur_cmd = cur_cmd.substr(input_display_start, max_input_chars);
	}
	if (show_caret && (caret_position >= command.length()))
	{
		cur_cmd += caret;
	}

	cur_cmd.insert(0, full_prompt.c_str());
	output_string.append(cur_cmd);
	std::string input_line = cur_cmd;

	// Get autocomplete options
	std::string prompt_alignment_spaces;
	for (int i = 0; i < (int)full_prompt.length(); i++)
	{
		prompt_alignment_spaces += ' ';
	}
	for (auto match : autocomplete_matches)
	{
		output_string.append("\n").append(prompt_alignment_spaces).append(match);
	}


	// Draw console background & border
	overlay->device->Clear(1, &border, D3DCLEAR_TARGET, color.border, 0, 0);
	overlay->device->Clear(1, &background, D3DCLEAR_TARGET, color.background, 0, 0);

	// Draw background for autocomplete dropdown
	if (autocomplete_matches.size() > 0)
	{
		long prompt_width = (long)(char_size.cx * full_prompt.length());
		background = { prompt_width + (int)border_width, console_height + (int)border_width, prompt_width + (int)border_width + (char_size.cx * longest_autocomplete), (long)(char_size.cy  * (output_log_displayed_lines + 1 + autocomplete_matches.size())) + (int)border_width };
		border = { background.x1 - (int)autocomplete_border_width, background.y1, background.x2 + (int)autocomplete_border_width, background.y2 + (int)autocomplete_border_width };
		overlay->device->Clear(1, &border, D3DCLEAR_TARGET, color.autocomplete_border, 0, 0);
		overlay->device->Clear(1, &background, D3DCLEAR_TARGET, color.autocomplete_bg, 0, 0);
		// Draw selected/hover background
		background.y1 += (char_size.cy * selection.autocomplete_selection);
		background.y2 = background.y1 + char_size.cy;
		if (selection.focus == SP_D3D9O_SELECT_AUTOCOMPLETE)
		{
			overlay->device->Clear(1, &background, D3DCLEAR_TARGET, color.autocomplete_bg_select, 0, 0);
		}
		else
		{
			overlay->device->Clear(1, &background, D3DCLEAR_TARGET, color.autocomplete_bg_hover, 0, 0);
		}
	}


	// Render the console text
	font->BeginDrawing();
	font->DrawText((float)border_width, (float)border_width, color.text, output_string.c_str(), D3DFONT_COLORTABLE, 0);
	if (selection.focus == SP_D3D9O_SELECT_TEXT)
	{
		draw_highlighted_text(selection, &input_line);
	}
	font->EndDrawing();


	// Get cursor screenspace size
	cursor->GetTextExtent("|", &char_size);

	if (show_cursor)
	{
		// Render the cursor
		if (win_cursor_tex == NULL ||
			((SpD3D9OInputHandler::get()->cursor_position.y <= (console_height + (int)border_width))
				&& (SpD3D9OInputHandler::get()->cursor_position.y > (int)border_width)
				&& (SpD3D9OInputHandler::get()->cursor_position.x > (int)border_width)
				&& (SpD3D9OInputHandler::get()->cursor_position.x <= (window_rect.right - (int)border_width))))
		{
			// Render text cursor
			cursor->BeginDrawing();
			cursor->DrawText((float)(SpD3D9OInputHandler::get()->cursor_position.x - (char_size.cx / 2)), (float)(SpD3D9OInputHandler::get()->cursor_position.y - (char_size.cy / 2)), color.text_cursor, "I", 0, 0);
			cursor->EndDrawing();
		}
		else
		{
			// Render windows cursor
			RECT win_cursor_rect = { 0, 1, (LONG)(cursor_size / 1.625), cursor_size };
			if (cursor_size > 120)
			{
				win_cursor_rect.right -= 2;
			}
			else if (cursor_size > 25)
			{
				win_cursor_rect.right--;
			}
			if (cursor_size > 130)
			{
				win_cursor_rect.top += 2;
			}
			win_cursor_sprite->Begin(D3DXSPRITE_ALPHABLEND);
			win_cursor_sprite->Draw(win_cursor_tex, (const RECT*)&win_cursor_rect, NULL, &D3DXVECTOR3((FLOAT)SpD3D9OInputHandler::get()->cursor_position.x, (FLOAT)SpD3D9OInputHandler::get()->cursor_position.y, 0), 0xFFFFFFFF);
			win_cursor_sprite->End();
		}
	}
}


void SpD3D9OConsole::add_prompt_elements(std::string *full_prompt, int *max_chars)
{
	if (prompt_elements & SP_D3D9O_PROMPT_USER)
	{
		extern std::string local_username;
		full_prompt->append(local_username);
		if (prompt_elements & SP_D3D9O_PROMPT_HOSTNAME)
		{
			full_prompt->append("@");
		}
		else if (prompt_elements & SP_D3D9O_PROMPT_CWD)
		{
			full_prompt->append(" | ");
		}
	}
	if (prompt_elements & SP_D3D9O_PROMPT_HOSTNAME)
	{
		extern std::string hostname;
		full_prompt->append(hostname);
		if (prompt_elements & SP_D3D9O_PROMPT_CWD)
		{
			full_prompt->append(" | ");
		}
	}
	if (prompt_elements & SP_D3D9O_PROMPT_CWD)
	{
		extern std::string game_exe_dir;
		full_prompt->append(game_exe_dir);
	}
	full_prompt->append(prompt);

	// Concatenate extended prompt if it's too long
	if (max_chars != NULL && (int)full_prompt->length() >= *max_chars)
	{
		(*full_prompt) = full_prompt->substr(0, (*max_chars) - 1);
	}
}



// Clears console by pushing blank messages to output
void SpD3D9OConsole::clear()
{
	for (int i = 0; i <= (int)output_log_displayed_lines; i++)
	{
		output_log.push_back("");
	}
}



// Prints a message to the console output
void SpD3D9OConsole::print(const char *new_message)
{
	if (output_stream)
	{
		std::string message = new_message;
		std::string line;

		int newline_pos;

		if ((newline_pos = message.find('\n')) != std::string::npos)
		{
			do
			{
				line = message.substr(0, newline_pos);
				output_log.push_back(line.c_str());
				message.erase(0, newline_pos + 1);
			} while ((newline_pos = message.find('\n')) != std::string::npos);
			output_log.push_back(message.c_str());
		}
		else
		{
			// No newlines appear in the string
			output_log.push_back(message.c_str());
		}
	}
}



// Checks if console is currently open
bool SpD3D9OConsole::is_open()
{
	return ((overlay->enabled_elements & SP_D3D9O_CONSOLE_ENABLED) != 0); // Weirdness to avoid compiler warnings
}



// Opens or closes the console. Returns true if console is open after function executes; false otherwise
bool SpD3D9OConsole::toggle()
{
	clear_selection();

	if (overlay->enabled_elements & SP_D3D9O_CONSOLE_ENABLED)
	{
		// Console is currently open
		overlay->enabled_elements &= SP_D3D9O_CONSOLE_DISABLED; // Close console
	}
	else
	{
		// Console is currently hidden

		// Set cursor position
		SpD3D9OInputHandler::get()->cursor_position.x = 100;
		SpD3D9OInputHandler::get()->cursor_position.y = 100;

		// Open console
		overlay->enabled_elements |= SP_D3D9O_CONSOLE_ENABLED;

		#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
			// Flush the keyboard input buffer before the user starts typing
			SpD3D9OInputHandler::get()->flush_keyboard_input_buffer();
		#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
	}

	// Send update message to overlay to reposition text feed
	overlay->needs_update = true;

	Sleep(200);

	return ((overlay->enabled_elements & SP_D3D9O_CONSOLE_ENABLED) != 0); // Weirdness to avoid compiler warnings
}



// Opens the console
int open_console()
{
	extern SpD3D9Device *gl_pSpD3D9Device;

	if (!gl_pSpD3D9Device->overlay->console->is_open() && SpD3D9OInputHandler::get() != NULL)
	{
		gl_pSpD3D9Device->overlay->console->toggle();
	}

	return 0;
}



/*
	Authors of significant portions of code below:

	-----------------------------------------
	* Game hacking QTS ( Quickie Tip Series )
	* no. 16 - Callback based keyboard and mouse input
	-----------------------------------------
	* Author: SEGnosis      - GHAnon.net
	* Thanks to:
	* bitterbanana          - No known site
	* Drunken Cheetah       - No known site
	* fatboy88              - No known site
	* Geek4Ever             - No known site
	* learn_more            - www.uc-forum.com
	* Novocaine             - http://ilsken.net/blog/?page_id=64
	* Philly0494            - No known site
	* Roverturbo            - www.uc-forum.com
	* SilentKarma           - www.halocoders.com - offline
	* Strife                - www.uc-forum.com
	* Wieter20              - No known site
*/
#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
void SpD3D9OConsole::handle_key_event(DIDEVICEOBJECTDATA *key_event)
{
	SpD3D9OInputHandler::get()->handled = false;

	if (_SP_DI8_KEY_DOWN_(key_event))
	{
		switch (key_event->dwOfs)
		{
			case DIK_ESCAPE:
				toggle();
				break;
			case DIK_RETURN:
			case DIK_NUMPADENTER:
				command_log.push_back(command);
				command_log_position = (unsigned int)command_log.size();

				//Send(command); // @TODO

				command.clear();
				caret_position = 0;
				break;
			case DIK_LEFT:
				if (caret_position > 0)
				{
					caret_position--;    // Move the caret back 1 if it's not already at 0
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case DIK_RIGHT:
				if (caret_position < (int)command.length())
				{
					caret_position++;    // Move the caret forward one if it's not already at the end of the string
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case DIK_UP:
				if (command_log_position <= 1)
				{
					command_log_position = 0;
				}
				else
				{
					command_log_position--;
				}
				caret_position = 0;
				command = command_log.at(command_log_position);
				caret_position = command.length();
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case DIK_DOWN:
				if (command_log_position < (unsigned int)command_log.size() - 1)
				{
					caret_position = 0;
					command_log_position++;
					command = command_log.at(command_log_position);
					caret_position = command.length();
				}
				else if(command_log_position != (unsigned int)command_log.size())
				{
					caret_position = 0;
					command_log_position = (int)command_log.size();
					command.clear();
				}
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case DIK_BACKSPACE:
				if (caret_position > 0)
				{
					command.erase(caret_position - 1, 1);
					caret_position--;
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				break;
			case DIK_DELETE:
				if (SpD3D9OInputHandler::get()->shift) // Shift+DEL are pressed
				{
					command.clear(); // Empty the string
					caret_position = 0; // Reset caret
				}
				else
				{
					// Delete the character in front of the caret if it's not at the end of the string
					// (Note that the caret stays in the same position)
					if (caret_position < (int)command.length())
					{
						command.erase(caret_position, 1);
					}
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case DIK_LSHIFT:
			case DIK_RSHIFT:
				SpD3D9OInputHandler::get()->shift = true;
				break;
			case DIK_LCONTROL:
			case DIK_RCONTROL:
				SpD3D9OInputHandler::get()->ctrl = true;
				break;
			case DIK_LALT:
			case DIK_RALT:
				SpD3D9OInputHandler::get()->alt = true;
				break;
			case DIK_LWIN:
			case DIK_RWIN:
				SpD3D9OInputHandler::get()->win = true;
				break;
			case DIK_END:
			case DIK_NEXT: // Page down
				caret_position = (int)command.length();
				break;
			case DIK_HOME:
			case DIK_PRIOR: // Page up
				caret_position = 0;
				break;
			default:
				char c;
				if (SpD3D9OInputHandler::get()->shift)
				{
					c = SpD3D9OInputHandler::get()->convert_shift_char[key_event->dwOfs];
				}
				else
				{
					c = SpD3D9OInputHandler::get()->convert_char[key_event->dwOfs];
				}

				if (c != '\0')
				{
					command.insert(caret_position, 1, c);
					caret_position++;
					show_caret = true;
					next_caret_blink = GetTickCount() + caret_blink_delay;
				}
				break;
		}
	}
	else
	{
		// Key was released
		switch (key_event->dwOfs)
		{
			case DIK_LSHIFT:
			case DIK_RSHIFT:
				SpD3D9OInputHandler::get()->shift = false;
				break;
			case DIK_LCONTROL:
			case DIK_RCONTROL:
				SpD3D9OInputHandler::get()->ctrl = false;
				break;
			case DIK_LALT:
			case DIK_RALT:
				SpD3D9OInputHandler::get()->alt = false;
				break;
			case DIK_LWIN:
			case DIK_RWIN:
				SpD3D9OInputHandler::get()->win = false;
				break;
			default:
				break;
		}
	}
}
#else // !_SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
void SpD3D9OConsole::handle_key_press(WPARAM wParam)
{
	DWORD last_err;
	std::string str; // Used when a temp string is needed
	std::vector<std::string> matches; // Only used if tab key is pressed
	int input_sel_start, input_sel_end; // Only used when obtaining selected input

	if (is_open() && !SpD3D9OInputHandler::get()->handled)        // If the console is visible, take input
	{
		switch (wParam)
		{
			case _CLOSE_CONSOLE_KEY_:
				toggle();
				clear_selection();
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_LEFT:
				clear_selection();
				if (caret_position > 0)
				{
					caret_position--;    // Move the caret back 1 if it's not already at 0
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_RIGHT:
				clear_selection();
				if (caret_position < (int)command.length())
				{
					caret_position++;    // Move the caret forward one if it's not already at the end of the string
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_UP:
				clear_selection();
				if (command_log_position <= 1)
				{
					command_log_position = 0;
				}
				else
				{
					command_log_position--;
				}
				caret_position = 0;
				command = command_log.at(command_log_position);
				caret_position = command.length();
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_DOWN:
				clear_selection();
				if (command_log_position < (unsigned int)command_log.size() - 1)
				{
					caret_position = 0;
					command_log_position++;
					command = command_log.at(command_log_position);
					caret_position = command.length();
				}
				else if (command_log_position != (unsigned int)command_log.size())
				{
					caret_position = 0;
					command_log_position = (int)command_log.size();
					command.clear();
				}
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_DELETE:
				if (SpD3D9OInputHandler::get()->shift) // Shift+DEL are pressed
				{
					clear_selection();
					command.clear(); // Empty the string
					caret_position = 0; // Reset caret
				}
				else
				{
					// Delete the character in front of the caret if it's not at the end of the string
					// (Note that the caret stays in the same position)
					get_input_selection(&input_sel_start, &input_sel_end);
					clear_selection();
					if (input_sel_start > -1 && input_sel_end > -1)
					{
						caret_position = input_sel_start;
						int input_sel_len = input_sel_end - input_sel_start;
						command.erase(caret_position, input_sel_len);
					}
					else if (caret_position < (int)command.length())
					{
						command.erase(caret_position, 1);
					}
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_TAB:
				if (command.length() > 0)
				{
					// If input isn't blank, use autocomplete
					get_autocomplete_options(command.c_str(), selection.autocomplete_selection+1, &matches);
					if ((int)matches.size() > selection.autocomplete_selection)
					{
						command = matches.at(selection.autocomplete_selection);
						caret_position = command.length();
					}
				}
				else
				{
					// If input is blank, get last command
					if (command_log_position <= 1)
					{
						command_log_position = 0;
					}
					else
					{
						command_log_position--;
					}
					caret_position = 0;
					command = command_log.at(command_log_position);
					caret_position = command.length();
				}
				clear_selection();
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			
			case VK_END:
			case VK_NEXT: // Page down
				clear_selection();
				caret_position = (int)command.length();
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_HOME:
			case VK_PRIOR: // Page up
				clear_selection();
				caret_position = 0;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_RETURN:
				clear_selection();
				command_log.push_back(command);
				command_log_position = (unsigned int)command_log.size();
				str = command;
				if (echo)
				{
					// Add elements to prompt
					std::string full_prompt;
					add_prompt_elements(&full_prompt);
					print(command.insert(0, full_prompt).c_str());
				}
				command.clear();
				caret_position = 0;

				execute_command(str.c_str());
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_BACK:
				get_input_selection(&input_sel_start, &input_sel_end);
				clear_selection();
				if (input_sel_start > -1 && input_sel_end > -1)
				{
					caret_position = input_sel_start;
					int input_sel_len = input_sel_end - input_sel_start;
					command.erase(caret_position, input_sel_len);
				}
				else if (caret_position > 0)
				{
					command.erase(caret_position - 1, 1);
					caret_position--;
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
			default:
				char c;
				if (SpD3D9OInputHandler::get()->shift)
				{
					c = SpD3D9OInputHandler::get()->convert_shift_char[wParam];
				}
				else
				{
					c = SpD3D9OInputHandler::get()->convert_char[wParam];
				}

				if (SpD3D9OInputHandler::get()->ctrl)
				{
					switch (wParam)
					{
						case 0x41: // Ctrl+A
							// @TODO
							break;
						case 0x43: // Ctrl+C
							build_highlighted_text(selection, &str);
							last_err = copy(&str);
							if (last_err != ERROR_SUCCESS)
							{
								print("ERROR: Unable to copy current selection");
							}
							break;
						case 0x56: // Ctrl+V
							// Delete selection
							get_input_selection(&input_sel_start, &input_sel_end);
							clear_selection();
							if (input_sel_start > -1 && input_sel_end > -1)
							{
								caret_position = input_sel_start;
								int input_sel_len = input_sel_end - input_sel_start;
								command.erase(caret_position, input_sel_len);
							}
							// Paste clipboard instead of typing
							last_err = paste();
							if (last_err != ERROR_SUCCESS)
							{
								print("ERROR: Unable to paste clipboard data (clipboard might be holding non-text data)");
							}
							else
							{
								show_caret = true;
								next_caret_blink = GetTickCount() + caret_blink_delay;
							}
							break;
						case 0x58: // Ctrl+X
							get_input_selection(&input_sel_start, &input_sel_end);
							if (input_sel_start > -1 && input_sel_end > -1)
							{
								build_highlighted_text(selection, &str);
								last_err = copy(&str);
								if (last_err != ERROR_SUCCESS)
								{
									print("ERROR: Unable to copy current selection");
								}
								else
								{
									clear_selection();
									caret_position = input_sel_start;
									int input_sel_len = input_sel_end - input_sel_start;
									command.erase(caret_position, input_sel_len);
									show_caret = true;
									next_caret_blink = GetTickCount() + caret_blink_delay;
								}
							}
							break;
						default:
							break;
					}

				}
				else if (c != '\0')
				{
					if (SpD3D9OInputHandler::get()->capslock && wParam >= 0x41 && wParam <= 0x5A) // Capslock is on and key is a letter
					{
						if (SpD3D9OInputHandler::get()->shift)
						{
							c = SpD3D9OInputHandler::get()->convert_char[wParam];
						}
						else
						{
							c = SpD3D9OInputHandler::get()->convert_shift_char[wParam];
						}
					}
					get_input_selection(&input_sel_start, &input_sel_end);
					clear_selection();
					if (input_sel_start > -1 && input_sel_end > -1)
					{
						caret_position = input_sel_start;
						int input_sel_len = input_sel_end - input_sel_start;
						command.erase(caret_position, input_sel_len);
					}
					command.insert(caret_position, 1, c);
					caret_position++;
					show_caret = true;
					next_caret_blink = GetTickCount() + caret_blink_delay;
				}
				SpD3D9OInputHandler::get()->handled = true;
				break;
			#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
		} // switch(wParam)
		//set_input_string_display_limits();
	} // if(is_open())
}



void SpD3D9OConsole::handle_mouse_input(RAWMOUSE *mouse_input)
{
	if (!is_open())
	{
		return;
	}

	// Mouse button press
	switch (mouse_input->usButtonFlags)
	{
		case RI_MOUSE_LEFT_BUTTON_DOWN:
			start_selection();
			break;

		case RI_MOUSE_LEFT_BUTTON_UP:
			continue_text_selection();
			if (selection.focus == SP_D3D9O_SELECT_TEXT && selection.line1 == selection.line2 && selection.i1 == selection.i2)
			{
				clear_selection();
			}
			
			continue_autocomplete_selection();
			break;

		case RI_MOUSE_RIGHT_BUTTON_DOWN:
			break;

		case RI_MOUSE_RIGHT_BUTTON_UP:
			break;

		case RI_MOUSE_MIDDLE_BUTTON_DOWN:
			break;

		case RI_MOUSE_MIDDLE_BUTTON_UP:
			break;

		case RI_MOUSE_WHEEL:
			if (mouse_input->usButtonData == 120)
			{
				// Scrolling up
			}
			else if (mouse_input->usButtonData == 65416)
			{
				// Scrolling down
			}
			break;

		// Additional mouse buttons
		case RI_MOUSE_BUTTON_4_DOWN:
		case RI_MOUSE_BUTTON_4_UP:
		case RI_MOUSE_BUTTON_5_DOWN:
		case RI_MOUSE_BUTTON_5_UP:
			break;

	} // switch (mouse_input->usButtonFlags)

	// Mouse movement
	switch (mouse_input->usFlags & MOUSE_MOVE_ABSOLUTE)
	{
		case MOUSE_MOVE_ABSOLUTE:
			// Mouse movement data is based on absolute position
		case MOUSE_MOVE_RELATIVE:
			// Mouse movement data is relative (based on last known position)

			if (SpD3D9OInputHandler::get()->mouse_button_down[0] && selection.focus == SP_D3D9O_SELECT_TEXT)
			{
				continue_text_selection();
			}
			
			continue_autocomplete_selection();
			break;
	}
}

#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


/*
	Parses a string into a console command and list of arguments.

	@return CONSOLE_COMMAND_SUCCESS if successful, or CONSOLE_COMMAND_NOT_FOUND_ERROR if the specified console command does not exist.

	@error ERROR_PROC_NOT_FOUND is set if the specified console command does not exist.
*/
int SpD3D9OConsole::execute_command(const char *new_command, int *return_code, std::string *output)
{
	int return_val = CONSOLE_COMMAND_SUCCESS;

	std::string command = new_command;
	trim(&command);

	if (command.length() == 0)
	{
		return return_val;
	}

	std::string command_name;
	std::string command_args = "";
	unsigned int space;
	if ((space = command.find_first_of(' ')) == std::string::npos)
	{
		// No spaces
		command_name = command;
	}
	else
	{
		command_name = command.substr(0, space);
		command_args = command.substr(space+1);
	}
	to_lower((char *)command_name.c_str());
	std::vector<std::string> args;
	std::string output_file;
	char output_action = parse_args(command_args.c_str(), &args, &output_file);

	int command_index = -1;
	seqan::String<char> cmd(command_name);
	seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> commands_finder;
	seqan::setHaystack(commands_finder, *commands_index);
	while (seqan::find(commands_finder, cmd))
	{
		if (seqan::position(commands_finder).i2 == 0 && (seqan::length(seqan::value(commands_set, seqan::position(commands_finder).i1)) == std::string(command_name).length()))
		{
			command_index = seqan::position(commands_finder).i1;
			break;
		}
	}
	seqan::clear(commands_finder);

	if (command_index > -1)
	{
		int command_return_code = CONSOLE_COMMAND_SUCCESS;
		std::string command_output = "";
		
		if (commands.at(command_index).alias_for.length() > 0 && commands.at(command_index).macro_args.size() > 0)
		{
			// Command is a macro
			std::vector<std::string> macro_args = commands.at(command_index).macro_args;
			for (auto arg : args)
			{
				macro_args.push_back(arg);
			}
			if (output_action == 0)
			{
				output_action = check_args_output_redirect(&macro_args, &output_file);
			}
			// Execute the command
			command_return_code = commands.at(command_index).function(macro_args, &command_output);
		}
		else
		{
			// Not a macro

			// Execute the command
			command_return_code = commands.at(command_index).function(args, &command_output);
		}

		if (output_action != 0)
		{
			switch (output_action)
			{
				case 'a':
					// Append file with output
					if (file_append_text(output_file.c_str(), command_output.c_str()))
					{
						// Failed to append file
						std::string out_str = std::string("ERROR: Failed to append output file \"").append(output_file).append("\"");
						if (output == NULL)
						{
							print(out_str.c_str());
						}
						else
						{
							output->append(out_str);
						}
					}
					break;
				case 'o':
					// Overwrite file with output
					if (file_write_text(output_file.c_str(), command_output.c_str()))
					{
						// Failed to write file
						std::string out_str = std::string("ERROR: Failed to write output file \"").append(output_file).append("\"");
						if (output == NULL)
						{
							print(out_str.c_str());
						}
						else
						{
							output->append(out_str);
						}
					}
					break;
				case 'd':
					// Discard output
					command_output.clear();
					break;
				default:
					break;
			}
		}
		else if(command_output.size() > 0)
		{
			if (output == NULL)
			{
				print(command_output.c_str());
			}
			else
			{
				output->append(command_output);
			}
		}

		if (return_code != NULL)
		{
			// Store return code
			(*return_code) = command_return_code;
		}
	}
	else
	{
		std::string out_str = std::string(_SP_D3D9O_C_ERROR_UNKNOWN_COMMAND_" \"").append(command_name).append("\"");
		return_val = CONSOLE_COMMAND_NOT_FOUND_ERROR;
		SetLastError(ERROR_PROC_NOT_FOUND);
		if (output == NULL)
		{
			// Send output to override output stream
			print(out_str.c_str());
		}
		else
		{
			output->append(out_str);
		}

		if (return_code != NULL)
		{
			// Store error code in return_code buffer
			(*return_code) = CONSOLE_COMMAND_NOT_FOUND_ERROR;
		}
	}

	return return_val;
}


int SpD3D9OConsole::register_command(const char *new_command, int(*function)(std::vector<std::string>, std::string *), const char *help_message, const char *alias_for, std::vector<std::string> macro_args) // Static function
{
	if (new_command == NULL || help_message == NULL || function == NULL || alias_for == NULL)
	{
		// Arguments can't be NULL
		SetLastError(ERROR_INVALID_ADDRESS);
		return (int)ERROR_INVALID_ADDRESS;
	}
	else if (new_command[0] == '\0')
	{
		// Command can't be an empty string
		SetLastError(ERROR_INVALID_PARAMETER);
		return (int)ERROR_INVALID_PARAMETER;
	}

	std::string cmd_str = new_command;
	trim(&cmd_str);
	char *command = (char *)cmd_str.c_str();
	to_lower(command);

	if (command[0] == '\0')
	{
		// Check for empty string again after trimming
		SetLastError(ERROR_INVALID_PARAMETER);
		return (int)ERROR_INVALID_PARAMETER;
	}
	
	std::string invalid_command_chars = _SP_D3D9O_C_INVALID_CONSOLE_COMMAND_CHARS_;
	for(int c = 0; c < (int)invalid_command_chars.length(); c++)
	{
		if (cmd_str.find(invalid_command_chars.c_str()[c]) != std::string::npos)
		{
			SetLastError(ERROR_SXS_XML_E_BADCHARINSTRING);
			return (int)ERROR_SXS_XML_E_BADCHARINSTRING;
		}
	}

	// Check if the command is already registered
	if (commands.size() > 0)
	{
		bool found_command = false;
		seqan::String<char> cmd(command);
		seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> commands_finder;
		seqan::setHaystack(commands_finder, *commands_index);
		while (seqan::find(commands_finder, cmd))
		{
			if (seqan::position(commands_finder).i2 == 0 && (seqan::length(seqan::value(commands_set, seqan::position(commands_finder).i1)) == std::string(command).length()))
			{
				found_command = true;
				break;
			}
		}
		seqan::clear(commands_finder);
		if (found_command)
		{
			// Command was already registered
			SetLastError(ERROR_DUP_NAME);
			return (int)ERROR_DUP_NAME;
		}
	}

	seqan::appendValue(commands_set, command);
	SP_D3D9O_CONSOLE_COMMAND new_cmd;
	new_cmd.command = command;
	new_cmd.function = function;
	new_cmd.help_message = help_message;
	new_cmd.id = seqan::positionToId(commands_set, seqan::length(commands_set) - 1);
	new_cmd.alias_for = alias_for;
	new_cmd.macro_args = macro_args;

	commands.push_back(new_cmd);
	if (commands_index != NULL)
	{
		delete commands_index;
		commands_index = NULL;
	}
	commands_index = new seqan::Index<seqan::StringSet<seqan::String<char>>>(commands_set);

	return 0;
}


int SpD3D9OConsole::register_alias(const char *new_alias, const char *existing_command, std::vector<std::string> macro_args)
{
	if (existing_command == NULL)
	{
		// Arguments can't be NULL
		SetLastError(ERROR_INVALID_ADDRESS);
		return (int)ERROR_INVALID_ADDRESS;
	}
	
	// Trim whitespace from existing command name
	std::string existing_cmd(existing_command);
	trim(&existing_cmd);
	
	if (existing_cmd.c_str()[0] == '\0')
	{
		// Arguments can't be empty string
		SetLastError(ERROR_INVALID_PARAMETER);
		return (int)ERROR_INVALID_PARAMETER;
	}

	// Convert existing command name to lowercase
	to_lower((char *)existing_cmd.c_str());

	int index = SpD3D9OConsole::get_console_command_index(existing_cmd.c_str());
	if (index == -1)
	{
		// Couldn't find the pre-existing command specified
		SetLastError(ERROR_PROC_NOT_FOUND);
		return (int)ERROR_PROC_NOT_FOUND;
	}

	return SpD3D9OConsole::register_command(new_alias, SpD3D9OConsole::commands.at(index).function, SpD3D9OConsole::commands.at(index).help_message.c_str(), existing_cmd.c_str(), macro_args);
}



void SpD3D9OConsole::update_fonts_and_cursor()
{

	if (font != NULL)
	{
		delete font;
		font = NULL;
	}

	font = new CD3DFont(_SP_D3D9O_C_DEFAULT_FONT_FAMILY_, font_height, _SP_D3D9O_C_DEFAULT_FONT_FLAGS_);

	font->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	font->RestoreDeviceObjects();


	if (cursor != NULL)
	{
		delete cursor;
		cursor = NULL;
	}
	cursor = new CD3DFont(cursor_font_family.c_str(), cursor_size, 0);
	cursor->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	cursor->RestoreDeviceObjects();

	if (win_cursor_tex != NULL)
	{
		win_cursor_tex->Release();
		win_cursor_tex = NULL;
	}
	init_win_cursor();

	overlay->needs_update = true;
}


// Pastes clipboard data into console input
DWORD SpD3D9OConsole::paste()
{
	DWORD err;

	if (!OpenClipboard(NULL))
	{
		// Try game window
		if (!OpenClipboard(*gl_pSpD3D9Device->overlay->game_window))
		{
			err = GetLastError();
			return err;
		}
	}

	HANDLE clipboard_data = GetClipboardData(CF_TEXT); // ANSI text format
	if (clipboard_data == NULL)
	{
		err = GetLastError();
		CloseClipboard();
		return err;
	}

	char *clipboard_text = (char*)GlobalLock(clipboard_data);
	if (clipboard_text == NULL)
	{
		err = GetLastError();
		CloseClipboard();
		return err;
	}

	std::string clipboard_str = clipboard_text;
	GlobalUnlock(clipboard_data);
	CloseClipboard();

	// Remove newline, return-feed, and indent (tab) characters
	for (int c = 0; c < (int)clipboard_str.length(); c++)
	{
		if (clipboard_str.c_str()[c] == '\n' || clipboard_str.c_str()[c] == '\r' || clipboard_str.c_str()[c] == '\t')
		{
			((char *)clipboard_str.c_str())[c] = ' ';
		}
	}

	command.insert(caret_position, clipboard_str);
	caret_position += clipboard_str.length();

	return 0;
}


// Copies current un-submitted console input to the clipboard
DWORD SpD3D9OConsole::copy(std::string *str)
{
	DWORD err;

	if (!OpenClipboard(NULL))
	{
		// Try game window
		if (!OpenClipboard(*gl_pSpD3D9Device->overlay->game_window))
		{
			err = GetLastError();
			return err;
		}
	}

	if (!EmptyClipboard())
	{
		err = GetLastError();
		CloseClipboard();
		return err;
	}

	HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE, str->length()+1);
	if (!hglob) {
		err = GetLastError();
		CloseClipboard();
		return err;
	}

	memcpy(GlobalLock(hglob), str->c_str(), str->length()+1);
	GlobalUnlock(hglob);
	SetClipboardData(CF_TEXT, hglob);
	CloseClipboard();
	GlobalFree(hglob);

	return 0;
}


HRESULT SpD3D9OConsole::init_win_cursor()
{
	extern HINSTANCE gl_hThisInstance;
	HRSRC cursor_hrsrc = FindResource(gl_hThisInstance, MAKEINTRESOURCE(ID_NUM_WIN_CURSOR_PNG), "ID_WIN_CURSOR_PNG");
	HGLOBAL cursor_hg = LoadResource(gl_hThisInstance, cursor_hrsrc);
	DWORD cursor_rsrc_size = SizeofResource(gl_hThisInstance, cursor_hrsrc);

	BYTE *cursor_rsrc_data = (BYTE*)LockResource(cursor_hg);

	//HRESULT hres = D3DXCreateTextureFromFileInMemory(gl_pSpD3D9Device->m_pIDirect3DDevice9, cursor_rsrc_data, cursor_rsrc_size, &win_cursor_tex);
	HRESULT hres = D3DXCreateTextureFromFileInMemoryEx(
		gl_pSpD3D9Device->m_pIDirect3DDevice9, cursor_rsrc_data,
		cursor_rsrc_size, (UINT)(cursor_size / 1.625), cursor_size,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &win_cursor_tex);

	FreeResource(cursor_hg);

	D3DXCreateSprite(gl_pSpD3D9Device->m_pIDirect3DDevice9, &win_cursor_sprite);

	return hres;
}


// Obtains the position (index) of a command, (Note: not the ID)
int SpD3D9OConsole::get_console_command_index(const char *command)
{
	int command_index = -1;
	seqan::String<char> cmd(command);
	seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> commands_finder;
	seqan::setHaystack(commands_finder, *commands_index);
	while (seqan::find(commands_finder, cmd))
	{
		if (seqan::position(commands_finder).i2 == 0 && (seqan::length(seqan::value(commands_set, seqan::position(commands_finder).i1)) == std::string(command).length()))
		{
			command_index = seqan::position(commands_finder).i1;
			break;
		}
	}
	seqan::clear(commands_finder);

	return command_index;
}



void SpD3D9OConsole::get_autocomplete_options(const char *str, unsigned int max_matches, std::vector<std::string> *matches, int *longest)
{
	matches->clear();

	int longest_match = -1; // Length of longest match

	if (max_matches == 0 || std::string(str).length() == 0)
	{
		if (longest != NULL)
		{
			*longest = longest_match;
		}
		return;
	}

	int found = 0; // Number of matches found thus far
	std::string lower_str = str;
	to_lower((char *)lower_str.c_str());
	seqan::String<char> search_string(lower_str.c_str());
	seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> commands_finder;
	seqan::setHaystack(commands_finder, *commands_index);
	while (seqan::find(commands_finder, search_string) && found < (int)max_matches)
	{
		if (seqan::position(commands_finder).i2 == 0 && (seqan::length(seqan::value(commands_set, seqan::position(commands_finder).i1)) != std::string(str).length()))
		{
			std::string match = commands.at(seqan::position(commands_finder).i1).command;
			matches->push_back(match);
			found++;
			if ((int)match.length() > longest_match)
			{
				longest_match = match.length();
			}
		}
	}
	seqan::clear(commands_finder);

	if (longest != NULL)
	{
		*longest = longest_match;
	}
}



void SpD3D9OConsole::set_input_string_display_limits(unsigned int max_input_chars)
{
	if (command.length() > max_input_chars)
	{
		if (caret_position > input_display_end)
		{
			if (caret_position == command.length())
			{
				input_display_end = command.length() - 1;
				input_display_start = input_display_end - (max_input_chars - 1);
			}
			else
			{
				input_display_start += (caret_position - input_display_end);
				input_display_end = caret_position;
			}
		}
		else if (caret_position < input_display_start)
		{
			input_display_end -= (input_display_start - caret_position);
			input_display_start = caret_position;
		}
		
		if ((caret_position == command.length()) || (((command.length() - 1) - input_display_start) < (max_input_chars - 1)))
		{
			input_display_end = command.length() - 1;
			input_display_start = input_display_end - (max_input_chars - 1);
		}
	}
	else
	{
		input_display_start = 0;
		if (command.length() == 0)
		{
			input_display_end = 0;
		}
		else
		{
			input_display_end = command.length() - 1;
		}
	}
}


void to_lower(char *string)
{
	unsigned int index = 0;
	while (string[index] != '\0')
	{
		if (string[index] >= 'A' && string[index] <= 'Z')
		{
			string[index] -= ('A' - 'a');
		}
		index++;
	}
}

void trim(std::string *string, const char *new_mask)
{
	// If no mask is specified, only whitespace is trimmed
	std::string mask = new_mask;
	int start = string->find_first_not_of(mask.c_str());
	string->erase(0, start);

	while (mask.find(*(string->end()-1)) != std::string::npos)
	{
		string->erase(string->length()-1, 1);
	}
}


// Parses command arguments and, if output should be redirected, returns 'o' (overwrite file), 'a' (append file), or 'd' (discard output)
char parse_args(const char *args_c_str, std::vector<std::string> *args, std::string *output_file)
{
	args->clear();
	int c = 0; // Index of character being parsed
	std::string arg;
	std::vector<bool> is_string_arg;

	while (args_c_str[c] != '\0')
	{
		switch (args_c_str[c])
		{
			case '\0':
				break;
			// Whitespace chars:
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				c++; // Ignore character
				break;
			default:
				is_string_arg.push_back(resolve_arg(args_c_str, &c, &arg));
				args->push_back(arg.c_str());
				arg.clear();
				break;
		}
	}

	if (args->size() > 0)
	{
		/*if ((!is_string_arg.at(args->size() - 1)) && (args->at(args->size() - 1).length() == 1) && (args->at(args->size() - 1).c_str()[0] == '&'))
		{
			// Run command in separate thread ("&")
			is_string_arg.pop_back();
			args->pop_back();

		}*/
		
		if ((args->size() > 1) && (!is_string_arg.at(args->size() - 2)) && (args->at(args->size() - 2).length() >= 1) && (args->at(args->size() - 2).c_str()[0] == '>'))
		{
			if (args->at(args->size() - 2).length() == 1)
			{
				// Pipe output to file with filename args->at(args->size() - 1) (">" = overwrite file)
				output_file->clear();
				output_file->append(args->at(args->size() - 1));
				is_string_arg.pop_back();
				is_string_arg.pop_back();
				args->pop_back();
				args->pop_back();
				return 'o';
			}
			else if ((args->at(args->size() - 2).length() == 2) && (args->at(args->size() - 2).c_str()[1] == '>'))
			{
				// Pipe output to file with filename args->at(args->size() - 1) (">>" = append file)
				output_file->clear();
				output_file->append(args->at(args->size() - 1));
				is_string_arg.pop_back();
				is_string_arg.pop_back();
				args->pop_back();
				args->pop_back();
				return 'a';
			}
			
		}
		else if(((args->size() > 1) && (!is_string_arg.at(args->size() - 1)) && (args->at(args->size() - 1).length() == 1) && (args->at(args->size() - 1).c_str()[0] == '>'))
				|| ((args->size() > 1) && (!is_string_arg.at(args->size() - 1)) && (args->at(args->size() - 1).length() == 2) && (args->at(args->size() - 1).c_str()[0] == '>') && (args->at(args->size() - 1).c_str()[1] == '>')))
		{
			// Discard output
			is_string_arg.pop_back();
			args->pop_back();
			return 'd';
		}
	}

	return 0;
}


bool resolve_arg(const char *args_c_str, int *index, std::string *arg)
{
	char quote; // Only used for string arguments
	bool escape = false;  // Only used for string arguments

	bool break_loop = false;

	switch (args_c_str[*index])
	{
		case '\'':
		case '"':
			// Build string argument
			quote = args_c_str[*index];
			(*index)++;
			while (args_c_str[*index] != '\0' && !break_loop)
			{
				switch (args_c_str[*index])
				{
					case '\0':
						break_loop = true;
						break;
					case '\\':
						// Escape character
						if (!escape)
						{
							escape = true;
						}
						else
						{
							escape = false;
							(*arg) += args_c_str[*index];
						}
						(*index)++;
						break;
					default:
						if (args_c_str[*index] == quote && !escape)
						{
							// End of argument
							(*index)++;
							break_loop = true;
						}
						else
						{
							escape = false;
							(*arg) += args_c_str[*index];
							(*index)++;
						}
						break;
				}
			}
			return true; // Return value indicates that this is a string argument
			break;
		default:
			// Build non-string argument
			while (args_c_str[*index] != '\0' && !break_loop)
			{
				switch (args_c_str[*index])
				{
					case '\0':
					// Whitespace chars:
					case ' ':
					case '\t':
					case '\n':
					case '\r':
					// Quote chars:
					case '\'':
					case '"':
						break_loop = true;
						break;
					default:
						(*arg) += args_c_str[*index];
						(*index)++;
						break;
				}
			}
			return false; // Return value indicates that this is not a string argument
			break;
	}
}


char check_args_output_redirect(std::vector<std::string> *args, std::string *output_file)
{
	if ((args->size() > 1) && (args->at(args->size() - 2).length() >= 1) && (args->at(args->size() - 2).c_str()[0] == '>'))
	{
		if (args->at(args->size() - 2).length() == 1)
		{
			// Pipe output to file with filename args->at(args->size() - 1) (">" = overwrite file)
			output_file->clear();
			output_file->append(args->at(args->size() - 1));
			args->pop_back();
			args->pop_back();
			return 'o';
		}
		else if ((args->at(args->size() - 2).length() == 2) && (args->at(args->size() - 2).c_str()[1] == '>'))
		{
			// Pipe output to file with filename args->at(args->size() - 1) (">>" = append file)
			output_file->clear();
			output_file->append(args->at(args->size() - 1));
			args->pop_back();
			args->pop_back();
			return 'a';
		}

	}
	else if (((args->size() > 1) && (args->at(args->size() - 1).length() == 1) && (args->at(args->size() - 1).c_str()[0] == '>'))
		|| ((args->size() > 1) && (args->at(args->size() - 1).length() == 2) && (args->at(args->size() - 1).c_str()[0] == '>') && (args->at(args->size() - 1).c_str()[1] == '>')))
	{
		// Discard output (">" or ">>" with no output file specified)
		args->pop_back();
		return 'd';
	}

	return 0;
}