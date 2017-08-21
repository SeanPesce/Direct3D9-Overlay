// Author: Sean Pesce
// Default console command functions

#include "stdafx.h"
#include "SpD3D9OConsole.h"
#include "Shellapi.h"
#include <cstdlib>
#include <climits>
#include <sstream>
#include <iomanip> // std::hex
#include <algorithm> // std::find


extern SpD3D9Device *gl_pSpD3D9Device;


// Constants
const char *ERROR_TXT_FEED_DISABLED = "ERROR: Text feed is not enabled (use the command \"text_feed 1\" to enable)";
const char *ERROR_TOO_FEW_ARGUMENTS = "ERROR: Not enough arguments";
const char *ERROR_INVALID_ARGUMENT = "ERROR: Invalid argument(s)";


void error_code_to_string(DWORD last_error, std::string *message)
{
	message->clear();

	// Get the error message
	if (last_error == 0)
	{
		message->append("No error");
		return;
	}

	LPSTR msg_buff = NULL;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg_buff, 0, NULL);

	message->append(msg_buff, size);
	if (message->c_str()[message->length() - 1] == '\n')
	{
		message->erase(message->length() - 1, 1); // Remove newline from error message
	}

	// Free buffer memory
	LocalFree(msg_buff);
}


void get_text_feed_pos_str(std::string *str)
{
	str->clear();
	switch (gl_pSpD3D9Device->overlay->text_feed->position)
	{
		case (DT_NOCLIP | DT_TOP | DT_LEFT):
			str->append("Text feed position = top center");
			break;
		case (DT_NOCLIP | DT_TOP | DT_CENTER):
			str->append("Text feed position = top right");
			break;
		case (DT_NOCLIP | DT_TOP | DT_RIGHT):
			str->append("Text feed position = center left");
			break;
		case (DT_NOCLIP | DT_VCENTER | DT_LEFT):
			str->append("Text feed position = center left");
			break;
		case (DT_NOCLIP | DT_VCENTER | DT_CENTER):
			str->append("Text feed position = center");
			break;
		case (DT_NOCLIP | DT_VCENTER | DT_RIGHT):
			str->append("Text feed position = center right");
			break;
		case (DT_NOCLIP | DT_BOTTOM | DT_LEFT):
			str->append("Text feed position = bottom left");
			break;
		case (DT_NOCLIP | DT_BOTTOM | DT_CENTER):
			str->append("Text feed position = bottom center");
			break;
		case (DT_NOCLIP | DT_BOTTOM | DT_RIGHT):
		default:
			str->append("Text feed position = bottom right");
			break;
	}
}


void get_text_feed_style_str(std::string *str)
{
	switch (gl_pSpD3D9Device->overlay->text_feed->style)
	{
		case SP_D3D9O_OUTLINED_TEXT:
			str->append("Text feed font style = outlined");
			break;
		case SP_D3D9O_SHADOWED_TEXT:
			str->append("Text feed font style = shadowed");
			break;
		case SP_D3D9O_PLAIN_TEXT:
			str->append("Text feed font style = plain");
			break;
	}
}


// Returns true if the given string represents a number value of zero
bool string_is_zero(const char *c_str)
{
	if (c_str == NULL)
	{
		return false;
	}

	std::string str(c_str);
	trim(&str);

	if (str.length() == 0)
	{
		return false;
	}
	
	for (int i = 0; i < str.length(); i++)
	{
		if (i != 1 && str.c_str()[i] != '0')
		{
			return false;
		}
		else if (i == 1 && str.c_str()[i] != '0')
		{
			if (str.length() == 2 || (str.c_str()[i] != 'x' && str.c_str()[i] != 'X'))
			{
				return false;
			}
		}
	}
	return true;
}


// Return value: 1 = true, 0 = false, -1 = invalid argument
int parse_toggle_arg(const char *c_arg)
{
	if (c_arg == NULL)
	{
		return -1;
	}

	std::string arg(c_arg);
	trim(&arg);

	if (arg.length() < 1)
	{
		return -1;
	}
	else if (arg.length() == 1)
	{
		switch (arg.c_str()[0])
		{
			case '1':
				return 1;
				break;
			case '0':
				return 0;
				break;
			default:
				return -1;
				break;
		}
	}
	
	to_lower((char *)arg.c_str());

	if ((strcmp(arg.c_str(), "true") == 0) || (strcmp(arg.c_str(), "on") == 0) || (strcmp(arg.c_str(), "enable") == 0) || (strcmp(arg.c_str(), "enabled") == 0))
	{
		return 1;
	}
	else if((strcmp(arg.c_str(), "false") == 0) || (strcmp(arg.c_str(), "off") == 0) || (strcmp(arg.c_str(), "disable") == 0) || (strcmp(arg.c_str(), "disabled") == 0) || string_is_zero(arg.c_str()))
	{
		return 0;
	}

	long parsed_val = strtol(arg.c_str(), NULL, 0);
	if (parsed_val == 1)
	{
		return true;
	}

	return -1;
}


// Terminates the process
void cc_exit(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long code = strtol(args.at(0).c_str(), NULL, 0);
		if (code != LONG_MAX && code != LONG_MIN)
		{
			exit((int)code);
		}
	}

	exit(EXIT_SUCCESS);
}


// Closes the console
void cc_close(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->console->is_open())
	{
		gl_pSpD3D9Device->overlay->console->toggle();
	}
}


// Lists all available console commands/aliases
void cc_all_commands(std::vector<std::string> args, std::string *output)
{
	int lines = 0;
	for (auto cmd : SpD3D9OConsole::commands)
	{
		output->append("    ").append(cmd.command);
		lines++;
		if (lines < SpD3D9OConsole::commands.size())
		{
			output->append("\n");
		}
	}
}


// Clears the console output
void cc_clear(std::vector<std::string> args, std::string *output)
{
	for (int i = 0; i < gl_pSpD3D9Device->overlay->console->output_log_displayed_lines; i++)
	{
		output->append("\n");
	}
}


// Prints the help string for a given command
void cc_help(std::vector<std::string> args, std::string *output)
{
	int index;
	std::string query = "help";

	if (args.size() >= 1)
	{
		query = args.at(0);
	}

	index = SpD3D9OConsole::get_console_command_index(query.c_str());

	if (index != -1)
	{
		if (SpD3D9OConsole::commands.at(index).alias_for.length() > 0)
		{
			output->append("(\"").append(query).append("\" is an alias for \"").append(SpD3D9OConsole::commands.at(index).alias_for).append("\")\n");
		}

		if (SpD3D9OConsole::commands.at(index).help_message.length() > 0)
		{
			output->append(SpD3D9OConsole::commands.at(index).help_message);
		}
		else
		{
			output->append("Command \"").append(query).append("\" has no help message.");
		}
	}
	else
	{
		output->append("ERROR: \"").append(query).append("\" is not a recognized console command.");
	}
}


// Prints the current date
void cc_date(std::vector<std::string> args, std::string *output)
{
	if (append_current_date_string(output, false, SP_DATE_MMDDYYYY))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current date");
	}
}

// Prints the current time
void cc_time(std::vector<std::string> args, std::string *output)
{
	if(append_current_timestamp_string(output, false))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current time");
	}
}

// Prints the current date and time
void cc_date_time(std::vector<std::string> args, std::string *output)
{
	if (append_current_date_string(output, false, SP_DATE_MMDDYYYY))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current date");
		return;
	}
	output->append("  ");
	if (append_current_timestamp_string(output, false))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current time");
	}
}


// Pauses console thread execution for the specified number of milliseconds
void cc_sleep(std::vector<std::string> args, std::string *output)
{
	long duration;

	if (args.size() > 0)
	{
		duration = strtol(args.at(0).c_str(), NULL, 0);
		if (duration > 0 && duration != LONG_MAX && duration != LONG_MIN)
		{
			Sleep(duration);
			output->append(std::string("Execution paused for ").append(std::to_string(duration)).append(" milliseconds").c_str());
		}
		else
		{
			output->append(std::string(ERROR_INVALID_ARGUMENT).append(" (Duration must be a positive integer)").c_str());
		}
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
}


// Creates an alias for an existing command
void cc_alias(std::vector<std::string> args, std::string *output)
{
	if (args.size() < 2)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return;
	}

	std::string *alias;
	std::string *command;
	int index = SpD3D9OConsole::get_console_command_index(args.at(0).c_str());
	if (index == -1)
	{
		index = SpD3D9OConsole::get_console_command_index(args.at(1).c_str());
		if (index == -1)
		{
			output->append("ERROR: Unable to create alias; the specified command was not recognized");
			return;
		}
		alias = &args.at(0);
		command = &args.at(1);
	}
	else
	{
		alias = &args.at(1);
		command = &args.at(0);
	}

	int ret = SpD3D9OConsole::register_command(alias->c_str(), SpD3D9OConsole::commands.at(index).function, SpD3D9OConsole::commands.at(index).help_message.c_str(), command->c_str());
	if (ret == 0)
	{
		output->append("SUCCESS: Created alias \"").append(*alias).append("\" for command \"").append(*command).append("\"");
	}
	else
	{
		switch (ret)
		{
			case ERROR_INVALID_ADDRESS:
				output->append("ERROR: Unable to create alias; null pointer was encountered");
				break;
			case ERROR_INVALID_PARAMETER:
				output->append("ERROR: Alias cannot be an empty string");
				break;
			case ERROR_SXS_XML_E_BADCHARINSTRING:
				output->append("ERROR: Alias must not contain whitespace characters.");
				break;
			case ERROR_DUP_NAME:
				output->append("ERROR: There is already an existing command or alias with the specified name");
				break;
			default:
				output->append("ERROR: Unable to create alias");
				break;
		}
	}
}


// Returns a list of commands that contain the given string
void cc_search_command(std::vector<std::string> args, std::string *output)
{
	if (args.size() < 1 || args.at(0).length() < 1)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
	else
	{
		std::vector<unsigned int> found_ids; // Used to make sure no commands are returned twice
		seqan::String<char> search_str(args.at(0));
		while (seqan::find(SpD3D9OConsole::commands_finder, search_str))
		{
			if (found_ids.size() == 0)
			{
				output->append("Search results:\n");
			}
			unsigned int id = seqan::positionToId(SpD3D9OConsole::commands_set, seqan::position(SpD3D9OConsole::commands_finder).i1);
			if (std::find(found_ids.begin(), found_ids.end(), id) == found_ids.end())
			{
				output->append("    ").append(std::string(seqan::toCString(seqan::valueById(SpD3D9OConsole::commands_set, id)))).append("\n");
				found_ids.push_back(id);
			}
		}
		seqan::clear(SpD3D9OConsole::commands_finder);
		if (found_ids.size() > 0)
		{
			output->erase(output->length() - 1, 1); // Remove extra newline
		}
		else
		{
			output->append("0 results.");
		}
	}
}


// Sets the maximum number of autocomplete suggestions (0 = off)
void cc_autocomplete_limit(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_lim = strtol(args.at(0).c_str(), NULL, 10);
		if (args.at(0).length() == 1 && args.at(0).c_str()[0] == '0')
		{
			gl_pSpD3D9Device->overlay->console->autocomplete_limit = 0;
		}
		else if (new_lim == 0L || new_lim == LONG_MAX || new_lim == LONG_MIN)
		{
			output->append("ERROR: Invalid autocomplete suggestion limit specified.\n");
		}
		else
		{
			gl_pSpD3D9Device->overlay->console->autocomplete_limit = (unsigned int)new_lim;
		}
	}
	output->append("Console autocomplete suggestion limit = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->autocomplete_limit));
}


// Pastes clipboard text data into the console input field
void cc_paste(std::vector<std::string> args, std::string *output)
{
	DWORD err = 0;
	std::string err_msg;

	if (!OpenClipboard(NULL))
	{
		err = GetLastError();
		if (err == ERROR_INVALID_WINDOW_HANDLE)
		{
			// Invalid window handle, try game window
			if (!OpenClipboard(*gl_pSpD3D9Device->overlay->game_window))
			{
				err = GetLastError();
				error_code_to_string(err, &err_msg);
				output->append("ERROR: Unable to open clipboard (clipboard might be holding non-text data)\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
				return;
			}
			
		}
		else
		{
			error_code_to_string(err, &err_msg);
			output->append("ERROR: Unable to open clipboard (clipboard might be holding non-text data)\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
			return;
		}
	}

	HANDLE clipboard_data = GetClipboardData(CF_TEXT); // ANSI text format
	if (clipboard_data == NULL)
	{
		err = GetLastError();
		error_code_to_string(err, &err_msg);
		output->append("ERROR: Unable to access clipboard data\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		CloseClipboard();
		return;
	}

	char *clipboard_text = (char*)GlobalLock(clipboard_data);
	if (clipboard_text == NULL)
	{
		err = GetLastError();
		error_code_to_string(err, &err_msg);
		output->append("ERROR: Unable to read clipboard data\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		CloseClipboard();
		return;
	}

	std::string clipboard_str = clipboard_text;
	GlobalUnlock(clipboard_data);
	CloseClipboard();
	
	// Remove newline and return feed characters
	for (int c = 0; c < clipboard_str.length(); c++)
	{
		if (clipboard_str.c_str()[c] == '\n' || clipboard_str.c_str()[c] == '\r')
		{
			((char *)clipboard_str.c_str())[c] = ' ';
		}
	}

	gl_pSpD3D9Device->overlay->console->command.insert(gl_pSpD3D9Device->overlay->console->caret_position, clipboard_str);
	gl_pSpD3D9Device->overlay->console->caret_position += clipboard_str.length();

	std::string plural = "";
	if (clipboard_str.length() != 1)
	{
		plural = "s";
	}
	output->append("SUCCESS: Copied ").append(std::to_string(clipboard_str.length())).append(" character").append(plural).append(" to console input");
}


// Enables/disables the console
void cc_console_enabled(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				if (gl_pSpD3D9Device->overlay->console->is_open())
				{
					gl_pSpD3D9Device->overlay->console->toggle();
				}
				break;
			case 1:
				if (!gl_pSpD3D9Device->overlay->console->is_open())
				{
					gl_pSpD3D9Device->overlay->console->toggle();
				}
				break;
			default:
				output->append("ERROR: Console value must be either 1 or 0 (1 = open, 0 = hidden)\n");
				break;
		}
	}
	
	if (gl_pSpD3D9Device->overlay->console->is_open())
	{
		output->append("Console = open");
	}
	else
	{
		output->append("Console = hidden");
	}
}


// Sets console input echo
void cc_console_echo(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->echo = false;
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->echo = true;
				break;
			default:
				output->append("ERROR: Console echo value must be either 1 or 0 (1 = on, 0 = off)\n");
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->echo)
	{
		output->append("echo = on");
	}
	else
	{
		output->append("echo = off");
	}
}


// Changes the console input prompt string
void cc_console_prompt(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->console->prompt = args.at(0);
	}
	output->append("Input prompt = \"").append(gl_pSpD3D9Device->overlay->console->prompt).append("\"");
}


// Enables/disables the username element of the console input prompt
void cc_console_prompt_user(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->prompt_elements &= (~SP_D3D9O_PROMPT_USER);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->prompt_elements |= (SP_D3D9O_PROMPT_USER);
				break;
			default:
				output->append("ERROR: Console prompt username value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_USER)
	{
		output->append("Show user profile name in console prompt = enabled");
	}
	else
	{
		output->append("Show user profile name in console prompt = disabled");
	}
}


// Enables/disables the hostname element of the console input prompt
void cc_console_prompt_host(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->prompt_elements &= (~SP_D3D9O_PROMPT_HOSTNAME);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->prompt_elements |= (SP_D3D9O_PROMPT_HOSTNAME);
				break;
			default:
				output->append("ERROR: Console prompt hostname value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_HOSTNAME)
	{
		output->append("Show hostname in console prompt = enabled");
	}
	else
	{
		output->append("Show hostname in console prompt = disabled");
	}
}


// Enables/disables the current working directory element of the console input prompt
void cc_console_prompt_cwd(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->prompt_elements &= (~SP_D3D9O_PROMPT_CWD);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->prompt_elements |= (SP_D3D9O_PROMPT_CWD);
				break;
			default:
				output->append("ERROR: Console prompt working directory value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_CWD)
	{
		output->append("Show working directory in console prompt = enabled");
	}
	else
	{
		output->append("Show working directory in console prompt = disabled");
	}
}


// Changes the console input caret character
void cc_console_caret(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		if (args.at(0).length() == 1)
		{
			gl_pSpD3D9Device->overlay->console->caret = args.at(0).c_str()[0];
		}
		else
		{
			output->append("ERROR: Invalid argument (Caret must be a single character)\n");
		}
	}
	output->append("Caret character = '");
	*output += gl_pSpD3D9Device->overlay->console->caret;
	output->append("'");
}


// Changes the console input caret blink delay
void cc_console_caret_blink(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_speed = strtol(args.at(0).c_str(), NULL, 10);
		if ((new_speed >= 0) && ((new_speed == 0 && args.at(0).c_str()[0] == '0') || (new_speed != 0 && new_speed != LONG_MAX && new_speed != LONG_MIN)))
		{
			gl_pSpD3D9Device->overlay->console->caret_blink_delay = new_speed;
		}
		else
		{
			output->append("ERROR: Invalid argument (Caret blink delay must be a non-negative integer value)\n");
		}
	}
	
	if (gl_pSpD3D9Device->overlay->console->caret_blink_delay > 0)
	{
		output->append("Caret blink delay = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->caret_blink_delay)).append(" milliseconds");
	}
	else
	{
		output->append("Caret blinking disabled.");
	}
}


// Changes the console border thickness
void cc_console_border_width(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_width = strtol(args.at(0).c_str(), NULL, 10);
		if ((new_width >= 0) && ((new_width == 0 && args.at(0).c_str()[0] == '0') || (new_width != 0 && new_width != LONG_MAX && new_width != LONG_MIN)))
		{
			gl_pSpD3D9Device->overlay->console->border_width = new_width;
			if (new_width == 0)
			{
				gl_pSpD3D9Device->overlay->console->autocomplete_border_width = 0;
			}
			else
			{
				gl_pSpD3D9Device->overlay->console->autocomplete_border_width = 1;
			}
		}
		else
		{
			output->append("ERROR: Invalid argument (Width must be a non-negative integer value)\n");
		}
	}
	output->append("Console border width = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->border_width)).append(" pixels");
}


// Changes the console font size
void cc_console_font_size(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_font_size = strtol(args.at(0).c_str(), NULL, 10);
		if (new_font_size > 0 && new_font_size != LONG_MAX && new_font_size != LONG_MIN)
		{
			gl_pSpD3D9Device->overlay->console->font_height = new_font_size;
		}
		else
		{
			output->append("ERROR: Invalid argument (Font size must be a positive integer value)\n");
		}
	}
	output->append("Console font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->font_height));
}


// Restores developer default settings for the console
void cc_console_restore_dev_defaults(std::vector<std::string> args, std::string *output)
{
	gl_pSpD3D9Device->overlay->console->toggle();

	gl_pSpD3D9Device->overlay->console->echo = _SP_D3D9O_C_DEFAULT_ECHO_VALUE_;
	gl_pSpD3D9Device->overlay->console->prompt = _SP_D3D9O_C_DEFAULT_PROMPT_;
	gl_pSpD3D9Device->overlay->console->prompt_elements = _SP_D3D9O_C_DEFAULT_PROMPT_ELEMENTS_;
	gl_pSpD3D9Device->overlay->console->caret = _SP_D3D9O_C_DEFAULT_CARET_;
	gl_pSpD3D9Device->overlay->console->caret_blink_delay = _SP_D3D9O_C_DEFAULT_BLINK_DELAY_;  // Speed at which the cursor blinks, in milliseconds
	gl_pSpD3D9Device->overlay->console->font_height = _SP_D3D9O_C_DEFAULT_FONT_HEIGHT_;
	gl_pSpD3D9Device->overlay->console->font_color = _SP_D3D9O_C_DEFAULT_FONT_COLOR_;
	gl_pSpD3D9Device->overlay->console->background_color = _SP_D3D9O_C_DEFAULT_BACKGROUND_COLOR_;
	gl_pSpD3D9Device->overlay->console->border_color = _SP_D3D9O_C_DEFAULT_BORDER_COLOR_;
	gl_pSpD3D9Device->overlay->console->border_width = _SP_D3D9O_C_DEFAULT_BORDER_WIDTH_;
	gl_pSpD3D9Device->overlay->console->autocomplete_background_color = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_COLOR_;
	gl_pSpD3D9Device->overlay->console->autocomplete_border_color = _SP_D3D9O_C_DEFAULT_BORDER_COLOR_;
	gl_pSpD3D9Device->overlay->console->autocomplete_border_width = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_WIDTH_;
	gl_pSpD3D9Device->overlay->console->output_log_displayed_lines = _SP_D3D9O_C_DEFAULT_OUTPUT_LINES_; // Number of lines of previous output to display
	gl_pSpD3D9Device->overlay->console->output_log_capacity = _SP_D3D9O_C_DEFAULT_OUTPUT_LOG_CAPACITY_; // Number of lines of output to keep in memory (oldest are deleted when max is hit)
	gl_pSpD3D9Device->overlay->console->command_log_capacity = _SP_D3D9O_C_DEFAULT_COMMAND_LOG_CAPACITY_; // Number of console commands to keep logged (oldest are deleted when max is hit)
	gl_pSpD3D9Device->overlay->console->autocomplete_limit = _SP_D3D9O_C_DEFAULT_AUTOCOMPLETE_LIMIT_; // Maximum number of autocomplete suggestions to show

	if (gl_pSpD3D9Device->overlay->console->output_log_capacity < gl_pSpD3D9Device->overlay->console->output_log_displayed_lines)
	{
		gl_pSpD3D9Device->overlay->console->output_log_capacity = gl_pSpD3D9Device->overlay->console->output_log_displayed_lines;
	}
	for (int i = 0; i < gl_pSpD3D9Device->overlay->console->output_log_displayed_lines; i++)
	{
		gl_pSpD3D9Device->overlay->console->output_log.push_back("");
	}

	gl_pSpD3D9Device->overlay->console->toggle();

	output->append("Restored console developer default settings:\n");
	if (gl_pSpD3D9Device->overlay->console->echo)
	{
		output->append("    echo = on\n");
	}
	else
	{
		output->append("    echo = off\n");
	}
	output->append("    Font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->font_height)).append("\n");
	output->append("    Input prompt = \"").append(gl_pSpD3D9Device->overlay->console->prompt).append("\"\n");
	output->append("    Caret character = '");
	*output += gl_pSpD3D9Device->overlay->console->caret;
	output->append("'\n");
	if (gl_pSpD3D9Device->overlay->console->caret_blink_delay > 0)
	{
		output->append("    Caret blink delay = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->caret_blink_delay)).append(" milliseconds\n");
	}
	else
	{
		output->append("    Caret blinking disabled.\n");
	}
	output->append("    Autocomplete suggestion limit = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->autocomplete_limit)).append("\n");
	output->append("    Border width = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->border_width)).append(" pixels\n");
	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_USER)
	{
		output->append("    Show user profile name in prompt = enabled\n");
	}
	else
	{
		output->append("    Show user profile name in prompt = disabled\n");
	}
	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_HOSTNAME)
	{
		output->append("    Show hostname in prompt = enabled\n");
	}
	else
	{
		output->append("    Show hostname in prompt = disabled\n");
	}
	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_CWD)
	{
		output->append("    Show working directory in prompt = enabled");
	}
	else
	{
		output->append("    Show working directory in prompt = disabled");
	}
}


// Enables/disables overlay text feed
void cc_text_feed_enabled(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->text_feed->set_enabled(false);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->text_feed->set_enabled(true);
				break;
			default:
				output->append("ERROR: Text feed value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		output->append("Text feed = enabled");
	}
	else
	{
		output->append("Text feed = disabled");
	}
}

// Changes the text feed font size
void cc_text_feed_font_size(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_font_size = strtol(args.at(0).c_str(), NULL, 10);
		if (new_font_size > 0 && new_font_size != LONG_MAX && new_font_size != LONG_MIN)
		{
			extern int current_text_feed_font_size;
			current_text_feed_font_size = new_font_size;
			gl_pSpD3D9Device->overlay->text_feed->set_font_height(new_font_size);
			output->append("Text feed font size = ").append(std::to_string(new_font_size));
		}
		else
		{
			output->append("ERROR: Invalid argument (Font size must be a positive integer value)\n");
			output->append("Text feed font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->text_feed->font_height));
		}
	}
	else
	{
		output->append("Text feed font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->text_feed->font_height));
	}
}


// Enables/disables overlay text feed info bar
void cc_text_feed_info_bar(std::vector<std::string> args, std::string *output)
{
	extern int user_pref_show_text_feed_info_bar;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar = SP_D3D9O_INFO_BAR_DISABLED;
				break;
			case 1:
				if (user_pref_show_text_feed_info_bar)
				{
					gl_pSpD3D9Device->overlay->text_feed->show_info_bar = user_pref_show_text_feed_info_bar;
				}
				else
				{
					gl_pSpD3D9Device->overlay->text_feed->show_info_bar = SP_D3D9O_INFO_BAR_TITLE;
				}
				break;
			default:
				output->append("ERROR: Text feed info bar value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->text_feed->show_info_bar)
	{
		output->append("Text feed info bar = enabled");
	}
	else
	{
		output->append("Text feed info bar = disabled");
	}
}


// Enables/disables overlay text feed info bar date element
void cc_text_feed_info_date(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		if (args.size() > 0)
		{
			switch (parse_toggle_arg(args.at(0).c_str()))
			{
			case 0:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar &= (~SP_D3D9O_INFO_BAR_DATE);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar |= SP_D3D9O_INFO_BAR_DATE;
				break;
			default:
				output->append("ERROR: Text feed date value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
			}
		}

		if (gl_pSpD3D9Device->overlay->text_feed->show_info_bar & SP_D3D9O_INFO_BAR_DATE)
		{
			output->append("Text feed info bar date = enabled");
		}
		else
		{
			output->append("Text feed info bar date = disabled");
		}
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


// Enables/disables overlay text feed info bar time element
void cc_text_feed_info_time(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		if (args.size() > 0)
		{
			switch (parse_toggle_arg(args.at(0).c_str()))
			{
			case 0:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar &= (~SP_D3D9O_INFO_BAR_TIME);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar |= SP_D3D9O_INFO_BAR_TIME;
				break;
			default:
				output->append("ERROR: Text feed time value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
			}
		}

		if (gl_pSpD3D9Device->overlay->text_feed->show_info_bar & SP_D3D9O_INFO_BAR_TIME)
		{
			output->append("Text feed info bar time = enabled");
		}
		else
		{
			output->append("Text feed info bar time = disabled");
		}
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


// Enables/disables overlay text feed info bar FPS counter element
void cc_text_feed_info_fps(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		if (args.size() > 0)
		{
			switch (parse_toggle_arg(args.at(0).c_str()))
			{
			case 0:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar &= (~SP_D3D9O_INFO_BAR_FPS);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->text_feed->show_info_bar |= SP_D3D9O_INFO_BAR_FPS;
				break;
			default:
				output->append("ERROR: Text feed FPS counter value must be either 1 or 0 (1 = enabled, 0 = disabled)\n");
				break;
			}
		}

		if (gl_pSpD3D9Device->overlay->text_feed->show_info_bar & SP_D3D9O_INFO_BAR_FPS)
		{
			output->append("Text feed info bar FPS counter = enabled");
		}
		else
		{
			output->append("Text feed info bar FPS counter = disabled");
		}
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


// Changes the text feed title
void cc_text_feed_title(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->text_feed->set_title(args.at(0).c_str());
		output->append("Text feed title = \"").append(args.at(0).c_str()).append("\"");
	}
	else
	{
		std::string title;
		gl_pSpD3D9Device->overlay->text_feed->get_title(&title);
		output->append("Text feed title = \"").append(title).append("\"");
	}
}


// Prints to text feed
void cc_text_feed_print(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->text_feed->print(args.at(0).c_str());
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
}


// Cycles between 9 preset text feed positions
void cc_text_feed_cycle_position(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		extern int next_overlay_text_position();
		next_overlay_text_position();
		get_text_feed_pos_str(output);
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


void cc_text_feed_position(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		get_text_feed_pos_str(output);
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


// Cycles through the 3 text feed text styles (plain, shadowed, outlined)
void cc_text_feed_cycle_style(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		extern int next_overlay_text_style();
		next_overlay_text_style();
		get_text_feed_style_str(output);
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


void cc_text_feed_style(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		get_text_feed_style_str(output);
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
	}
}


// Loads a DLL
void cc_load_library(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		HMODULE hmod = LoadLibrary(args.at(0).c_str());
		DWORD err = GetLastError();
		if (hmod == NULL)
		{
			// Failed to load library
			std::string err_msg;
			error_code_to_string(err, &err_msg);
			output->append("ERROR: Failed to load library (\"").append(args.at(0)).append("\")\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		}
		else
		{
			std::stringstream hex_stream;
			hex_stream << std::hex << hmod;

			// Load plugin functions (if any)
			SpD3D9Overlay::load_plugin_funcs(hmod, args.at(0).c_str());

			output->append("SUCCESS: Library was loaded (").append(args.at(0).c_str()).append(" base address = 0x").append(hex_stream.str()).append(")");

			std::list<SP_D3D9_PLUGIN>::const_iterator plugin;
			for (plugin = SpD3D9Overlay::loaded_libraries.begin(); plugin != SpD3D9Overlay::loaded_libraries.end(); plugin++)
			{
				if (plugin->module == hmod)
				{
					std::string func_list = "";
					if (plugin->init_func != NULL)
					{
						func_list.append("\n    initialize_plugin");
						plugin->init_func();
					}
					if (plugin->main_loop_func != NULL)
					{
						func_list.append("\n    main_loop");
					}
					if (plugin->draw_overlay_func != NULL)
					{
						func_list.append("\n    draw_overlay");
					}
					if (plugin->present_func != NULL)
					{
						func_list.append("\n    present");
					}
					if (plugin->end_scene_func != NULL)
					{
						func_list.append("\n    end_scene");
					}
					if (plugin->get_raw_input_data_func != NULL)
					{
						func_list.append("\n    get_raw_input_data");
					}
					if (plugin->disable_player_input_func != NULL)
					{
						func_list.append("\n    disable_player_input");
					}

					if (func_list.length() > 0)
					{
						output->append("\n").append(args.at(0).c_str()).append(" is an overlay plugin. Found function(s):").append(func_list);
					}
					break;
				}
			}
		}
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
}


// Unloads a DLL
void cc_free_library(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		// Try unloading library by filename first
		HMODULE hmod = GetModuleHandle(args.at(0).c_str());
		if (hmod != NULL)
		{
			// Successfully obtained HMODULE
			bool lib_freed = FreeLibrary(hmod);
			DWORD err = GetLastError();
			if (!lib_freed)
			{
				std::string err_msg;
				error_code_to_string(err, &err_msg);
				output->append("ERROR: Failed to unload library (\"").append(args.at(0)).append("\")\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
			}
			else
			{
				// Remove library from plugin list, if necessary
				SpD3D9Overlay::run_plugin_funcs = false;
				Sleep(20);

				std::list<SP_D3D9_PLUGIN>::const_iterator plugin;
				for (plugin = SpD3D9Overlay::loaded_libraries.begin(); plugin != SpD3D9Overlay::loaded_libraries.end(); plugin++)
				{
					if (plugin->module == hmod)
					{
						SpD3D9Overlay::loaded_libraries.erase(plugin);
					}
				}

				SpD3D9Overlay::run_plugin_funcs = true;
				output->append("SUCCESS: Library was unloaded");
			}
			return;
		}
		
		// Try parsing argument as library starting address in decimal
		long hmod_L = strtol(args.at(0).c_str(), NULL, 10);
		if (hmod_L != 0L && hmod_L != LONG_MAX && hmod_L != LONG_MIN)
		{
			hmod = (HMODULE)hmod_L;
			if (FreeLibrary(hmod))
			{
				// Remove library from plugin list, if necessary
				SpD3D9Overlay::run_plugin_funcs = false;
				Sleep(20);

				std::list<SP_D3D9_PLUGIN>::const_iterator plugin;
				for (plugin = SpD3D9Overlay::loaded_libraries.begin(); plugin != SpD3D9Overlay::loaded_libraries.end(); plugin++)
				{
					if (plugin->module == hmod)
					{
						SpD3D9Overlay::loaded_libraries.erase(plugin);
					}
				}

				SpD3D9Overlay::run_plugin_funcs = true;

				output->append("SUCCESS: Library was unloaded");
				return;
			}
		}

		hmod_L = strtol(args.at(0).c_str(), NULL, 16);
		if (hmod_L != 0L && hmod_L != LONG_MAX && hmod_L != LONG_MIN)
		{
			hmod = (HMODULE)hmod_L;
			if (FreeLibrary(hmod))
			{
				// Successfully unloaded the library
				output->append("SUCCESS: Library was unloaded");
				return;
			}
		}
		output->append("ERROR: Failed to unload library (\"").append(args.at(0)).append("\")");
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
}


// Opens the specified URL in the system's default web browser
void cc_open_web_page(std::vector<std::string> args, std::string *output)
{
	DWORD err;
	std::string err_msg;
	if (args.size() < 1)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
	else
	{
		std::string arg0 = args.at(0);
		to_lower((char *)arg0.c_str());
		std::string http_prefix = "http://";
		std::string https_prefix = "https://";
		std::string url = "";

		// Make sure URL is a valid https url
		if (arg0.length() < http_prefix.length())
		{
			url.append(https_prefix).append(arg0);
		}
		else if (strcmp(std::string(arg0).substr(0, http_prefix.length()).c_str(), http_prefix.c_str()) == 0)
		{
			// Change http to https prefix
			url.append(arg0);
			url.insert(4, "s");
		}
		else if (strcmp(std::string(arg0).substr(0, https_prefix.length()).c_str(), https_prefix.c_str()) != 0)
		{
			url.append(https_prefix).append(arg0);
		}
		else
		{
			url.append(arg0);
		}
		
		// Load the verified URL
		if ((err = (DWORD)ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW)) < 32)
		{
			error_code_to_string(err, &err_msg);
			output->append("ERROR: Unable to open URL \"").append(args.at(0)).append("\"\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		}
	}
}


// Executes a command using the system shell
/*void cc_shell(std::vector<std::string> args, std::string *output)
{
	DWORD err;
	std::string err_msg;
	if (args.size() > 0)
	{
		std::string shell_cmd(args.at(0));
		shell_cmd.append(" > d3d9_shell_cmd_output.tmp");

		//if ((err = (DWORD)ShellExecute(0, 0, NULL, 0, 0, SW_SHOW)) < 32)
		{
			error_code_to_string(err, &err_msg);
			output->append("ERROR: Unable to execute command \"").append(args.at(0)).append("\"\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		}

		
	}
	else
	{
		output->append("ERROR: No shell command specified");
	}
}*/


// Beeps for the specified frequency and duration
void cc_beep(std::vector<std::string> args, std::string *output)
{
	DWORD frequency = 500;
	DWORD duration = 500;

	if (args.size() == 1)
	{
		long freq = strtol(args.at(0).c_str(), NULL, 0);
		if (freq != 0 && freq != LONG_MAX && freq != LONG_MIN)
		{
			frequency = (DWORD)freq;
		}
	}
	else if (args.size() > 1)
	{
		long val = strtol(args.at(0).c_str(), NULL, 0);
		if (val != 0 && val != LONG_MAX && val != LONG_MIN)
		{
			frequency = (DWORD)val;
		}

		val = strtol(args.at(1).c_str(), NULL, 0);
		if (val != 0 && val != LONG_MAX && val != LONG_MIN)
		{
			duration = (DWORD)val;
		}
	}

	output->append("BEEP: Frequency=").append(std::to_string(frequency)).append("hz, Duration=").append(std::to_string(duration)).append("milliseconds");

	Beep(frequency, duration);
}


// Prints each argument on a separate line
void cc_echo(std::vector<std::string> args, std::string *output)
{
	for (auto arg : args)
	{
		output->append(arg).append("\n");
	}
	if (output->length() > 0)
	{
		output->erase(output->length() - 1, 1); // Remove extra newline
	}
}



DWORD WINAPI cc_run_thread(LPVOID lpParam)
{
	char *cmd = (char *)lpParam;
	int result = system(cmd);
	delete cmd;
	return result;
}
// Opens a file with the system resolver
void cc_run(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		char *cmd = new char[args.at(0).length()+1];
		strcpy_s(cmd, sizeof(char) * (args.at(0).length() + 1), args.at(0).c_str());
		cmd[args.at(0).length()] = '\0';
		DWORD thread_id;
		HANDLE thread_handle = CreateThread(
			NULL,				// Default security attributes
			0,					// Use default stack size
			cc_run_thread,		// Thread function name
			cmd,				// Argument to thread function
			0,					// Use default creation flags
			&thread_id);		// Returns the thread identifier

		std::stringstream hex_stream;
		hex_stream << std::hex << thread_handle;
		output->append("Running \"").append(args.at(0)).append("\"\nHandle = 0x").append(hex_stream.str()).append("; Thread ID = ").append(std::to_string(thread_id));
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
	}
}

void register_default_console_commands()
{
	SpD3D9OConsole::register_command("help", cc_help, "help [command]\n    Prints the help message for the given command.");
	SpD3D9OConsole::register_alias("h", "help");
	SpD3D9OConsole::register_command("exit", cc_exit, "exit [exit code]\n    Exits the game.");
	SpD3D9OConsole::register_alias("quit", "exit");
	SpD3D9OConsole::register_command("commands", cc_all_commands, "commands\n    Lists all available console commands");
	SpD3D9OConsole::register_command("search_command", cc_search_command, "search_command <query>\n    Returns a list of available commands that contain the given query string.");
	SpD3D9OConsole::register_command("close", cc_close, "close\n    Closes the console overlay.");
	SpD3D9OConsole::register_command("clear", cc_clear, "clear\n    Clears console output.");
	SpD3D9OConsole::register_command("sleep", cc_sleep, "sleep <duration>\n    Pauses execution for the specified duration (in milliseconds).");
	SpD3D9OConsole::register_alias("wait", "sleep");
	SpD3D9OConsole::register_command("date", cc_date, "date\n    Prints the current date (in MM/DD/YYYY format).");
	SpD3D9OConsole::register_command("date_time", cc_date_time, "date_time\n    Prints the current date (in MM/DD/YYYY format) and 24-hour time.");
	SpD3D9OConsole::register_command("time", cc_time, "time\n    Prints the current 24-hour time.");
	SpD3D9OConsole::register_command("alias", cc_alias, "alias <ALIAS|COMMAND> <ALIAS|COMMAND>\n    Creates an alias for an existing console command/alias.");
	SpD3D9OConsole::register_command("paste", cc_paste, "paste\n    Copies ANSI text data from the clipboard to the console input");
	SpD3D9OConsole::register_command("beep", cc_beep, "beep <frequency> <duration>\n    Generates a beeping sound at the given frequency (hz) for the given duration (milliseconds).\n    Execution is halted until the beep is completed.");
	SpD3D9OConsole::register_command("load_library", cc_load_library, "load_library <filename>\n    Loads the specified dynamic link library (DLL) file.");
	SpD3D9OConsole::register_command("free_library", cc_free_library, "free_library <filename|HMODULE>\n    Unloads the specified dynamic link library (DLL) module.\n    The module can be specified through the .dll file name or its starting address in memory (HMODULE).");
	SpD3D9OConsole::register_alias("unload_library", "free_library");
	//SpD3D9OConsole::register_command("shell", cc_shell, "shell <URL>\n    Executes a shell command in the system default shell.");
	SpD3D9OConsole::register_command("web", cc_open_web_page, "web <URL>\n    Opens a web page in the system default web browser.");
	SpD3D9OConsole::register_command("console", cc_console_enabled, "console [is_open]\n    Opens/closes the console (1 = open, 0 = hidden).");
	SpD3D9OConsole::register_command("console_restore_developer_default_settings", cc_console_restore_dev_defaults, "console_restore_developer_default_settings\n    Restores all console settings to developer-preferred values.");
	SpD3D9OConsole::register_command("console_echo", cc_console_echo, "console_echo [is_open]\n    Enables/disables console input echo (1 = on, 0 = off).");
	SpD3D9OConsole::register_command("console_font_size", cc_console_font_size, "console_font_size [size]\n    Sets the console overlay font size.");
	SpD3D9OConsole::register_command("console_autocomplete_limit", cc_autocomplete_limit, "autocomplete_limit [limit]\n    Sets the maximum number of autocomplete suggestions to be shown (0 = off).");
	SpD3D9OConsole::register_command("console_prompt", cc_console_prompt, "console_prompt [prompt]\n    Sets the console input prompt string.");
	SpD3D9OConsole::register_command("console_prompt_user", cc_console_prompt_user, "console_prompt_user [is_enabled]\n    Enables/disables the username element of the console input prompt.");
	SpD3D9OConsole::register_command("console_prompt_hostname", cc_console_prompt_host, "console_prompt_hostname [is_enabled]\n    Enables/disables the hostname element of the console input prompt.");
	SpD3D9OConsole::register_command("console_prompt_cwd", cc_console_prompt_cwd, "console_prompt_cwd [is_enabled]\n    Enables/disables the working directory element of the console input prompt.");
	SpD3D9OConsole::register_command("console_caret", cc_console_caret, "console_caret [caret]\n    Sets the console input caret character.");
	SpD3D9OConsole::register_command("console_caret_blink", cc_console_caret_blink, "console_caret_blink [blink_delay]\n    Sets the console input caret blink delay time (in milliseconds).");
	SpD3D9OConsole::register_command("console_border_width", cc_console_border_width, "console_border_width [width]\n    Sets the console border width.");
	SpD3D9OConsole::register_command("echo", cc_echo, "echo [args]\n    Prints each argument on a separate line.");
	SpD3D9OConsole::register_command("run", cc_run, "run <file>\n    Opens or runs a file using the system resolver.");
	SpD3D9OConsole::register_alias("open", "run");
	SpD3D9OConsole::register_command("text_feed", cc_text_feed_enabled, "text_feed [is_enabled]\n    Enables/disables the overlay text feed (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_info_bar", cc_text_feed_info_bar, "text_feed_info_bar [is_enabled]\n    Enables/disables the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_date", cc_text_feed_info_date, "text_feed_date [is_enabled]\n    Enables/disables the date element of the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_time", cc_text_feed_info_time, "text_feed_time [is_enabled]\n    Enables/disables the time element of the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_fps", cc_text_feed_info_fps, "text_feed_fps [is_enabled]\n    Enables/disables the FPS counter element of the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_print", cc_text_feed_print, "text_feed_print <message>\n    Prints a message to the overlay text feed.");
	SpD3D9OConsole::register_command("text_feed_font_size", cc_text_feed_font_size, "text_feed_font_size [size]\n    Sets the overlay text feed font size.");
	SpD3D9OConsole::register_command("text_feed_title", cc_text_feed_title, "text_feed_title [title]\n    Sets the overlay text feed title message (only shown if text feed info bar is enabled).");
	SpD3D9OConsole::register_command("text_feed_position", cc_text_feed_position, "text_feed_position\n    Returns the current overlay text feed position.");
	SpD3D9OConsole::register_command("text_feed_style", cc_text_feed_style, "text_feed_style\n    Returns the current overlay text feed style (plain, outlined, shadowed).");
	SpD3D9OConsole::register_command("text_feed_cycle_position", cc_text_feed_cycle_position, "text_feed_cycle_position\n    Cycles through the 9 text feed position presets.");
	SpD3D9OConsole::register_command("text_feed_cycle_style", cc_text_feed_cycle_style, "text_feed_cycle_style\n    Cycles through the 3 text feed font style presets (plain, outlined, shadowed).");
}