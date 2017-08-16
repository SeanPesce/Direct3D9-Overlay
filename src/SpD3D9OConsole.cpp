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

	for (int i = 0; i < output_log_displayed_lines; i++)
	{
		output_log.push_back("");
	}

	// Inititalize font interface
	font = new CD3DFont(font_family.c_str(), font_height, 0);
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
	// Draw console background
	RECT window_rect;
	if (!GetClientRect(*overlay->game_window, &window_rect))
	{
		// Handle error
	}
	D3DRECT background = { 0, 0, window_rect.right - window_rect.left, (long)((font_height  * (output_log_displayed_lines+1))*1.5) };
	overlay->device->Clear(1, &background, D3DCLEAR_TARGET, background_color, 0, 0);


	std::string output_string = "";
	for (int i = output_log_displayed_lines - 1; i >= 0; i--)
	{
		output_string.append(output_log.at(output_log.size()-(1+i))).append("\n");
	}

	// Draw console text
	DWORD current_time = GetTickCount();
	if (current_time >= next_caret_blink)
	{
		show_caret = !show_caret;
		next_caret_blink = current_time + caret_blink_delay;
	}

	std::string cur_cmd = command;
	if (show_caret)
	{
		if (caret_position != command.length())
		{
			cur_cmd.erase(caret_position, 1);
		}
		cur_cmd.insert(caret_position, 1, caret);
	}
	cur_cmd.insert(0, prompt.c_str());
	output_string.append(cur_cmd);

	font->BeginDrawing();
	font->DrawText(0.0, 0.0, font_color, output_string.c_str(), 0, 0);
	font->EndDrawing();
}



void SpD3D9OConsole::print(const char *new_message)
{
	std::string message = new_message;
	
	int newline_pos;

	if ((newline_pos = message.find('\n')) != std::string::npos)
	{
		do
		{
			std::string line = message.substr(0, newline_pos-1);
			output_log.push_back(line.c_str());
			message.erase(0, newline_pos);
		} while ((newline_pos = message.find('\n')) != std::string::npos);
		output_log.push_back(message.c_str());
	}
	else
	{
		// No newlines appear in the string
		output_log.push_back(message.c_str());
	}
}



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
				if (command.length() > 0)
				{
					command_log.push_back(command);
					command_log_position = (unsigned int)command_log.size();
					std::string exec_cmd = command;
					print(command.insert(0, prompt).c_str());
					command.clear();
					caret_position = 0;

					execute_command(exec_cmd.c_str());
				}
				else
				{
					toggle();
				}
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
					command.insert(caret_position, 1, c);
					caret_position++;
					show_caret = true;
					next_caret_blink = GetTickCount() + caret_blink_delay;
				}
				SpD3D9OInputHandler::get()->handled = true;
				break;
			#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
		} // switch(wParam)
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


void SpD3D9OConsole::execute_command(const char *new_command)
{
	std::string command = new_command;
	trim(&command);
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
	parse_args(command_args.c_str(), &args);

	//overlay->text_feed->print(std::string("Command name: \"").append(command_name).append("\"").c_str(), 10000, false);
	//overlay->text_feed->print(std::string("Command args: \"").append(command_args).append("\"").c_str(), 10000, false);

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
		if (command_output.size() > 0)
		{
			print(command_output.c_str());
			overlay->text_feed->print(command_output.c_str(), 10000, false);
		}
	}
	else
	{
		print(std::string("ERROR: Unrecognized command \"").append(command_name).append("\"").c_str());
		overlay->text_feed->print(std::string("ERROR: Unrecognized command \"").append(command_name).append("\"").c_str(), 10000, false);
	}
}


int SpD3D9OConsole::register_command(const char *new_command, void(*function)(std::vector<std::string>, std::string *), const char *help_message) // Static function
{
	if (new_command == NULL || help_message == NULL || function == NULL)
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


void parse_args(const char *args_c_str, std::vector<std::string> *args)
{
	args->clear();

	std::string args_str = args_c_str;
	trim(&args_str);

	while(args_str.c_str()[0] != '\0')
	{
		if (args_str.c_str()[0] == '\'' || args_str.c_str()[0] == '"')
		{
			char quote = args_str.c_str()[0];
			int pos = args_str.find_first_of(quote, 1);
			while (pos != std::string::npos && args_str.c_str()[pos - 1] == '\\')
			{
				args_str.erase(pos-1, 1);
				pos = args_str.find_first_of(quote, pos);
			}
			if (pos != std::string::npos)
			{
				args->push_back(args_str.substr(1, pos-1));
				args_str.erase(0, pos+1);
				trim(&args_str);
				continue;
			}
		}


		int pos = args_str.find_first_of(" \r\n\t");
		if (pos == std::string::npos)
		{
			args->push_back(args_str);
			break;
		}
		else
		{
			args->push_back(args_str.substr(0, pos));
			args_str.erase(0, pos);
			trim(&args_str);
			continue;
		}
		
	}
	
}