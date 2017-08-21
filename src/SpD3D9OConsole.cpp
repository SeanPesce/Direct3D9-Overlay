// Author: Sean Pesce

#include "stdafx.h"
#include "SpD3D9OConsole.h"


// Static class data
seqan::Index<seqan::StringSet<seqan::String<char>>> *SpD3D9OConsole::commands_index = NULL;
std::vector<SP_D3D9O_CONSOLE_COMMAND> SpD3D9OConsole::commands;		// Set of available console commands and corresponding functions
seqan::StringSet<seqan::String<char>> SpD3D9OConsole::commands_set;	// Set of available console command strings
seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> SpD3D9OConsole::commands_finder;


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

	clear();

	// Inititalize font interface
	font = new CD3DFont(font_family.c_str(), font_height, _SP_D3D9O_C_DEFAULT_FONT_FLAGS_);
	font->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	font->RestoreDeviceObjects();
}



SpD3D9OConsole::~SpD3D9OConsole()
{
	if (commands_index != NULL)
	{
		delete commands_index;
		commands_index = NULL;
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
	if ((unsigned int)font->GetFontHeight() != font_height)
	{
		update_font();
	}

	// Get window dimensions
	RECT window_rect;
	if (!GetClientRect(*overlay->game_window, &window_rect))
	{
		// Handle error
	}

	// Create background/border rectangles
	SIZE char_size;
	font->GetTextExtent("|", &char_size);
	long console_height = (long)(char_size.cy  * (output_log_displayed_lines + 1));
	//long console_height = (long)((font_height  * (output_log_displayed_lines + 1))*1.5);
	
	D3DRECT border = { 0, 0, window_rect.right - window_rect.left, console_height+(2*border_width) };
	D3DRECT background = { border_width, border_width, (window_rect.right - window_rect.left)- border_width, console_height + border_width };


	// Calculate maximum number of characters & lines that can be displayed on-screen
	//unsigned int max_lines = (unsigned int)((/*(float)*/console_height / 1.5f) / /*(float)*/char_size.cx);
	unsigned int max_chars = (background.x2 - background.x1) / char_size.cx; // Maximum characters per line
	int max_input_chars = max_chars - prompt.length();
	if (caret_position == command.length())
	{
		max_input_chars--;
	}

	set_input_string_display_limits(max_input_chars);


	// Concatenate prompt if it's too long
	if (prompt.length() > (max_chars - 1))
	{
		prompt = prompt.substr(0, max_chars - 1);
	}

	// Add elements to prompt
	std::string full_prompt;
	add_prompt_elements(&full_prompt);

	// Concatenate extended prompt if it's too long
	if (full_prompt.length() > (max_chars - 1))
	{
		full_prompt = full_prompt.substr(0, max_chars - 1);
	}

	// Build console output log string
	std::string output_string = "";
	for (int i = output_log_displayed_lines - 1; i >= 0; i--)
	{
		std::string *current_line = &output_log.at(output_log.size() - (1 + i));
		if (current_line->length() > max_chars)
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

	// Add current command
	std::string cur_cmd = command;
	if (show_caret && (caret_position < command.length()))
	{
		cur_cmd.erase(caret_position, 1);
		cur_cmd.insert(caret_position, 1, caret);
	}
	if (cur_cmd.length() > max_input_chars)
	{
		cur_cmd = cur_cmd.substr(input_display_start, max_input_chars);
	}
	if (show_caret && (caret_position >= command.length()))
	{
		cur_cmd += caret;
	}
	cur_cmd.insert(0, full_prompt.c_str());
	output_string.append(cur_cmd);


	// Get autocomplete options
	std::vector<std::string> autocomplete_matches;
	int longest_autocomplete = 0;
	if (command.length() > 0)
	{
		cur_cmd = command;
		get_autocomplete_options(cur_cmd.c_str(), autocomplete_limit, &autocomplete_matches);
		std::string prompt_alignment_spaces;
		for (int i = 0; i < full_prompt.length(); i++)
		{
			prompt_alignment_spaces += ' ';
		}
		for (auto match : autocomplete_matches)
		{
			if (match.length() > longest_autocomplete)
			{
				longest_autocomplete = match.length();
			}
			output_string.append("\n").append(prompt_alignment_spaces).append(match);
		}
	}


	// Draw console background & border
	overlay->device->Clear(1, &border, D3DCLEAR_TARGET, border_color, 0, 0);
	overlay->device->Clear(1, &background, D3DCLEAR_TARGET, background_color, 0, 0);

	// Draw background for autocomplete dropdown
	if (autocomplete_matches.size() > 0)
	{
		long prompt_width = (long)(char_size.cx * full_prompt.length());
		background = { prompt_width + (int)border_width, console_height + (int)border_width, prompt_width + (int)border_width + (char_size.cx * longest_autocomplete), (long)(char_size.cy  * (output_log_displayed_lines + 1 + autocomplete_matches.size())) + (int)border_width };
		border = { background.x1 - (int)autocomplete_border_width, background.y1, background.x2 + (int)autocomplete_border_width, background.y2 + (int)autocomplete_border_width };
		overlay->device->Clear(1, &border, D3DCLEAR_TARGET, autocomplete_border_color, 0, 0);
		overlay->device->Clear(1, &background, D3DCLEAR_TARGET, autocomplete_background_color, 0, 0);
	}


	// Render the console
	font->BeginDrawing();
	font->DrawText((float)border_width, (float)border_width, font_color, output_string.c_str(), 0, 0);
	font->EndDrawing();
}


void SpD3D9OConsole::add_prompt_elements(std::string *full_prompt)
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
}



// Clears console by pushing blank messages to output
void SpD3D9OConsole::clear()
{
	for (int i = 0; i < output_log_displayed_lines; i++)
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
	if (overlay->enabled_elements & SP_D3D9O_CONSOLE_ENABLED)
	{
		// Console is currently open
		overlay->enabled_elements &= SP_D3D9O_CONSOLE_DISABLED; // Close console
	}
	else
	{
		// Console is not currently open
		overlay->enabled_elements |= SP_D3D9O_CONSOLE_ENABLED; // Open console

		#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
			// Flush the keyboard input buffer before the user starts typing
			SpD3D9OInputHandler::get()->flush_keyboard_input_buffer();
		#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
	}

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
	std::string exec_cmd; // Only used when executing a command
	std::vector<std::string> match; // Only used if tab key is pressed

	if (is_open() && !SpD3D9OInputHandler::get()->handled)        // If the console is visible, take input
	{
		switch (wParam)
		{
			case _CLOSE_CONSOLE_KEY_:
				toggle();
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_LEFT:
				if (caret_position > 0)
				{
					caret_position--;    // Move the caret back 1 if it's not already at 0
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_RIGHT:
				if (caret_position < (int)command.length())
				{
					caret_position++;    // Move the caret forward one if it's not already at the end of the string
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_UP:
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
			case VK_TAB:
				get_autocomplete_options(command.c_str(), 1, &match);
				if (match.size() > 0)
				{
					command = match.at(0);
					caret_position = command.length();
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			/*case VK_CLEAR:
				command.clear(); // Empty the string
				caret_position = 0; // Reset caret
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				SpD3D9OInputHandler::get()->handled = true;
				break;*/
			case VK_END:
			case VK_NEXT: // Page down
				caret_position = (int)command.length();
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_HOME:
			case VK_PRIOR: // Page up
				caret_position = 0;
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_RETURN:
				//if (command.length() > 0)
				//{
					command_log.push_back(command);
					command_log_position = (unsigned int)command_log.size();
					exec_cmd = command;
					if (echo)
					{
						// Add elements to prompt
						std::string full_prompt;
						add_prompt_elements(&full_prompt);
						print(command.insert(0, full_prompt).c_str());
					}
					command.clear();
					caret_position = 0;

					execute_command(exec_cmd.c_str());
				//}
				//else
				//{
					////toggle();
					//print(command.insert(0, prompt).c_str());
				//}
				SpD3D9OInputHandler::get()->handled = true;
				break;
			case VK_BACK:
				if (caret_position > 0)
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
						case 0x43: // Ctrl+C
							last_err = copy();
							if (last_err != 0)
							{
								print("ERROR: Unable to copy current input");
							}
							break;
						case 0x56: // Ctrl+V
							last_err = paste();
							if (last_err != 0)
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
							last_err = copy();
							if (last_err != 0)
							{
								print("ERROR: Unable to copy current input");
							}
							else
							{
								command.clear(); // Empty the string
								caret_position = 0; // Reset caret
								show_caret = true;
								next_caret_blink = GetTickCount() + caret_blink_delay;
							}
							break;
						default:
							break;
					}
					// Paste clipboard instead of typing

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


void SpD3D9OConsole::handle_text_input(WPARAM wParam)
{
	if (SpD3D9OInputHandler::get()->handled)
	{
		return;
	}

	std::string exec_cmd = command;
	switch (wParam)
	{
		case '\b':    // backspace
			if (caret_position > 0)
			{
				command.erase(caret_position - 1, 1);
				caret_position--;
			}
			show_caret = true;
			next_caret_blink = GetTickCount() + caret_blink_delay;
			break;

		//case '\n':
		case '\r':    // return/enter
			command_log.push_back(command);
			command_log_position = (unsigned int)command_log.size();
			command.clear();
			caret_position = 0;

			execute_command(exec_cmd.c_str());
			break;
		case '\t':    // tab
			/*if (command_log_position > 0)
			{
				command_log_position--;
				command = command_log.at(command_log_position);
				caret_position = command.length();
			}
			else
			{
				command_log_position = command_log.size() - 1;
				command = command_log.at(command_log_position);
				caret_position = command.length();
			}*/
			break;

		default:
			command.insert(caret_position, 1, (char)wParam);
			caret_position++;
			break;
	} // switch(wParam)

	SpD3D9OInputHandler::get()->handled = true;
}
#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


void SpD3D9OConsole::execute_command(const char *new_command, std::string *output)
{
	std::string command = new_command;
	trim(&command);

	if (command.length() == 0)
	{
		return;
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
		std::string command_output = "";
		commands.at(command_index).function(args, &command_output);
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
	}
	else
	{
		std::string out_str = std::string("ERROR: Unrecognized command \"").append(command_name).append("\"");
		if (output == NULL)
		{
			print(out_str.c_str());
		}
		else
		{
			output->append(out_str);
		}

	}
}


int SpD3D9OConsole::register_command(const char *new_command, void(*function)(std::vector<std::string>, std::string *), const char *help_message, const char *alias_for, std::vector<std::string> macro_args) // Static function
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
	for(int c = 0; c < invalid_command_chars.length(); c++)
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
	seqan::setHaystack(commands_finder, *commands_index);
	seqan::clear(commands_finder);

	return 0;
}


int SpD3D9OConsole::register_alias(const char *new_alias, const char *existing_command)
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

	return SpD3D9OConsole::register_command(new_alias, SpD3D9OConsole::commands.at(index).function, SpD3D9OConsole::commands.at(index).help_message.c_str(), existing_cmd.c_str());
}


void SpD3D9OConsole::update_font()
{

	if (font != NULL)
	{
		delete font;
		font = NULL;
	}

	font = new CD3DFont(_SP_D3D9O_C_DEFAULT_FONT_FAMILY_, font_height, _SP_D3D9O_C_DEFAULT_FONT_FLAGS_);

	font->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	font->RestoreDeviceObjects();
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

	// Remove newline and return-feed characters
	for (int c = 0; c < clipboard_str.length(); c++)
	{
		if (clipboard_str.c_str()[c] == '\n' || clipboard_str.c_str()[c] == '\r')
		{
			((char *)clipboard_str.c_str())[c] = ' ';
		}
	}

	command.insert(caret_position, clipboard_str);
	caret_position += clipboard_str.length();

	return 0;
}


// Copies current un-submitted console input to the clipboard
DWORD SpD3D9OConsole::copy()
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

	HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE, command.length()+1);
	if (!hglob) {
		err = GetLastError();
		CloseClipboard();
		return err;
	}

	memcpy(GlobalLock(hglob), command.c_str(), command.length()+1);
	GlobalUnlock(hglob);
	SetClipboardData(CF_TEXT, hglob);
	CloseClipboard();
	GlobalFree(hglob);

	return 0;
}


// Obtains the position (index) of a command, (Note: not the ID)
int SpD3D9OConsole::get_console_command_index(const char *command)
{
	int command_index = -1;
	seqan::String<char> cmd(command);
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



void SpD3D9OConsole::get_autocomplete_options(const char *str, unsigned int max_matches, std::vector<std::string> *matches)
{
	matches->clear();

	if (max_matches == 0)
	{
		return;
	}

	int found = 0; // Number of matches found thus far
	std::string lower_str = str;
	to_lower((char *)lower_str.c_str());
	seqan::String<char> search_string(lower_str.c_str());
	while (seqan::find(commands_finder, search_string) && found < max_matches)
	{
		if (seqan::position(commands_finder).i2 == 0 && (seqan::length(seqan::value(commands_set, seqan::position(commands_finder).i1)) != std::string(str).length()))
		{
			matches->push_back(commands.at(seqan::position(commands_finder).i1).command);
			found++;
		}
	}
	seqan::clear(commands_finder);
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