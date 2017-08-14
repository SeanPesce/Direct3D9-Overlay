// Author: Sean Pesce

#include "stdafx.h"
#include "SpD3D9OConsole.h"


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

	// Inititalize font interface
	font = new CD3DFont(font_family.c_str(), font_height, 0);
	font->InitializeDeviceObjects(overlay->device->m_pIDirect3DDevice9);
	font->RestoreDeviceObjects();
}



SpD3D9OConsole::~SpD3D9OConsole()
{

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

	if (gl_input_handler == NULL
		#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
			|| (gl_input_handler->oGetRawInputData == NULL)
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
		gl_input_handler->get_dinput_data();
	#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


	#if (defined _SP_USE_DETOUR_DISPATCH_MSG_INPUT_ || defined _SP_USE_WIN_HOOK_EX_INPUT_)
		MSG msg;
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_ || _SP_USE_WIN_HOOK_EX_INPUT_
}


void SpD3D9OConsole::draw()
{
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


	font->BeginDrawing();

	font->DrawText(0.0, 0.0, font_color, cur_cmd.c_str(), 0, 0);

	font->EndDrawing();
}



void SpD3D9OConsole::print(const char *new_message)
{
	std::string message = new_message;
	
	//size_t newline_pos = message.find('\n');

	// Assuming no newlines appear in the string
	output_log.push_back(message.c_str());
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
			gl_input_handler->flush_keyboard_input_buffer();
		#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
	}

	Sleep(200);

	return ((overlay->enabled_elements & SP_D3D9O_CONSOLE_ENABLED) != 0); // Weirdness to avoid compiler warnings
}



int open_console()
{
	extern SpD3D9Device *gl_pSpD3D9Device;

	if (!gl_pSpD3D9Device->overlay->console->is_open() && gl_input_handler != NULL)
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
	gl_input_handler->handled = false;

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
				gl_input_handler->handled = true;
				break;
			case DIK_RIGHT:
				if (caret_position < (int)command.length())
				{
					caret_position++;    // Move the caret forward one if it's not already at the end of the string
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				gl_input_handler->handled = true;
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
				gl_input_handler->handled = true;
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
				gl_input_handler->handled = true;
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
				if (gl_input_handler->shift) // Shift+DEL are pressed
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
				gl_input_handler->handled = true;
				break;
			case DIK_LSHIFT:
			case DIK_RSHIFT:
				gl_input_handler->shift = true;
				break;
			case DIK_LCONTROL:
			case DIK_RCONTROL:
				gl_input_handler->ctrl = true;
				break;
			case DIK_LALT:
			case DIK_RALT:
				gl_input_handler->alt = true;
				break;
			case DIK_LWIN:
			case DIK_RWIN:
				gl_input_handler->win = true;
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
				if (gl_input_handler->shift)
				{
					c = gl_input_handler->convert_shift_char[key_event->dwOfs];
				}
				else
				{
					c = gl_input_handler->convert_char[key_event->dwOfs];
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
				gl_input_handler->shift = false;
				break;
			case DIK_LCONTROL:
			case DIK_RCONTROL:
				gl_input_handler->ctrl = false;
				break;
			case DIK_LALT:
			case DIK_RALT:
				gl_input_handler->alt = false;
				break;
			case DIK_LWIN:
			case DIK_RWIN:
				gl_input_handler->win = false;
				break;
			default:
				break;
		}
	}
}
#else // !_SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
void SpD3D9OConsole::handle_key_press(WPARAM wParam)
{

	if (is_open() && !gl_input_handler->handled)        // If the console is visible, take input
	{
		switch (wParam)
		{
			case _CLOSE_CONSOLE_KEY_:
				toggle();
				gl_input_handler->handled = true;
				break;
			case VK_LEFT:
				if (caret_position > 0)
				{
					caret_position--;    // Move the caret back 1 if it's not already at 0
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				gl_input_handler->handled = true;
				break;
			case VK_RIGHT:
				if (caret_position < (int)command.length())
				{
					caret_position++;    // Move the caret forward one if it's not already at the end of the string
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				gl_input_handler->handled = true;
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
				gl_input_handler->handled = true;
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
				gl_input_handler->handled = true;
				break;
			case VK_DELETE:
				if (gl_input_handler->shift) // Shift+DEL are pressed
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
				gl_input_handler->handled = true;
				break;
			/*case VK_CLEAR:
				command.clear(); // Empty the string
				caret_position = 0; // Reset caret
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				gl_input_handler->handled = true;
				break;*/
			case VK_END:
			case VK_NEXT: // Page down
				caret_position = (int)command.length();
				gl_input_handler->handled = true;
				break;
			case VK_HOME:
			case VK_PRIOR: // Page up
				caret_position = 0;
				gl_input_handler->handled = true;
				break;
			case VK_RETURN:
				command_log.push_back(command);
				command_log_position = (unsigned int)command_log.size();

				//Send(command); // @TODO

				command.clear();
				caret_position = 0;
				gl_input_handler->handled = true;
				break;
			case VK_BACK:
				if (caret_position > 0)
				{
					command.erase(caret_position - 1, 1);
					caret_position--;
				}
				show_caret = true;
				next_caret_blink = GetTickCount() + caret_blink_delay;
				gl_input_handler->handled = true;
				break;
			#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
			default:
				char c;
				if (gl_input_handler->shift)
				{
					c = gl_input_handler->convert_shift_char[wParam];
				}
				else
				{
					c = gl_input_handler->convert_char[wParam];
				}

				if (c != '\0')
				{
					command.insert(caret_position, 1, c);
					caret_position++;
					show_caret = true;
					next_caret_blink = GetTickCount() + caret_blink_delay;
				}
				gl_input_handler->handled = true;
				break;
			#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
		} // switch(wParam)
	} // if(is_open())
}


void SpD3D9OConsole::handle_text_input(WPARAM wParam)
{
	if (gl_input_handler->handled)
	{
		return;
	}

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

			//Send(command); // @TODO

			command.clear();
			caret_position = 0;
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

	gl_input_handler->handled = true;
}
#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
