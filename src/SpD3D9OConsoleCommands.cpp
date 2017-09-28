// Author: Sean Pesce
// Default console command functions

#include "stdafx.h"
#include "SpD3D9OConsole.h"
#include "SP_AoB_Scan.hpp"
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
const char *ERROR_INVALID_TOGGLE_ARGUMENT = "ERROR: Argument must be either 1 or 0 (1 = enabled, 0 = disabled)\n";


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


void console_settings_to_string(std::string *output, const char* line_prefix = _SP_D3D9O_C_OUTPUT_INDENT_)
{
	if (gl_pSpD3D9Device == NULL || gl_pSpD3D9Device->overlay == NULL || gl_pSpD3D9Device->overlay->console == NULL)
	{
		SetLastError(PEERDIST_ERROR_NOT_INITIALIZED);
		return;
	}


	if (output == NULL)
	{
		SetLastError(ERROR_INVALID_ADDRESS);
		return;
	}

	const char *line_pref_backup = "";
	if (line_prefix == NULL)
	{
		line_prefix = line_pref_backup;
	}

	if (gl_pSpD3D9Device->overlay->console->echo)
	{
		output->append(line_prefix).append("echo = on\n");
	}
	else
	{
		output->append(line_prefix).append("echo = off\n");
	}
	if (gl_pSpD3D9Device->overlay->console->output_stream)
	{
		output->append(line_prefix).append("Output stream = enabled\n");
	}
	else
	{
		output->append(line_prefix).append("Output stream = disabled\n");
	}
	output->append(line_prefix).append("Output lines = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->output_log_displayed_lines)).append("\n");
	output->append(line_prefix).append("Font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->font_height)).append("\n");
	output->append(line_prefix).append("Input prompt = \"").append(gl_pSpD3D9Device->overlay->console->prompt).append("\"\n");
	output->append(line_prefix).append("Caret character = '");
	*output += gl_pSpD3D9Device->overlay->console->caret;
	output->append("'\n");
	if (gl_pSpD3D9Device->overlay->console->box_caret)
		output->append("Box caret mode = enabled");
	else
		output->append("Box caret mode = disabled");
	if (gl_pSpD3D9Device->overlay->console->caret_blink_delay > 0)
	{
		output->append(line_prefix).append("Caret blink delay = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->caret_blink_delay)).append(" milliseconds\n");
	}
	else
	{
		output->append(line_prefix).append("Caret blinking disabled.\n");
	}
	if (gl_pSpD3D9Device->overlay->console->show_cursor)
	{
		output->append(line_prefix).append("Console mouse cursor = enabled\n");
	}
	else
	{
		output->append(line_prefix).append("Console mouse cursor = disabled\n");
	}
	output->append(line_prefix).append("Console mouse cursor size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->cursor_size)).append("\n");
	if (gl_pSpD3D9Device->overlay->console->autocomplete_preview)
		output->append(line_prefix).append("Preview current autocomplete suggestion = enabled\n");
	else
		output->append(line_prefix).append("Preview current autocomplete suggestion = disabled\n");
	output->append(line_prefix).append("Autocomplete suggestion limit = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->autocomplete_limit)).append("\n");
	output->append(line_prefix).append("Border width = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->border_width)).append(" pixels\n");
	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_USER)
	{
		output->append(line_prefix).append("Show user profile name in prompt = enabled\n");
	}
	else
	{
		output->append(line_prefix).append("Show user profile name in prompt = disabled\n");
	}
	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_HOSTNAME)
	{
		output->append(line_prefix).append("Show hostname in prompt = enabled\n");
	}
	else
	{
		output->append(line_prefix).append("Show hostname in prompt = disabled\n");
	}
	if (gl_pSpD3D9Device->overlay->console->prompt_elements & SP_D3D9O_PROMPT_CWD)
	{
		output->append(line_prefix).append("Show working directory in prompt = enabled");
	}
	else
	{
		output->append(line_prefix).append("Show working directory in prompt = disabled");
	}
}


void args_to_string(std::vector<std::string> args, std::string *str)
{
	for (auto macro_arg : args)
	{
		std::string arg = macro_arg;

		if (arg.length() == 0 || (arg.find_first_of(" '\n\"\t\r") != std::string::npos))
		{
			// Argument contains whitespace/quotes, so must be a string argument
			int last_match = 0;
			while ((last_match < (int)arg.length()) && (last_match = (int)arg.find_first_of("'", last_match)) != std::string::npos)
			{
				arg.insert(last_match, "\\");
				last_match += 2; // +1 for new '\\' char, +1 to move past the '\'' char
			}

			arg.append("'");
			arg.insert(0, "'");
		}
		arg.insert(0, " ");
		str->append(arg);
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
	
	for (int i = 0; i < (int)str.length(); i++)
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


// Checks if a char appears in a string.
//		If char exists in string, returns first index where char appears; otherwise, returns -1
int in_string(char c, std::string str)
{
	for (int i = 0; i < (int)str.length(); i++)
	{
		if (c == str.c_str()[i]) return i;
	}
	return -1;
}


// Checks if a string has only wildcard characters.
//		If uniform is true, all characters must be identical.
bool is_wildcard(std::string *str, bool uniform = true, std::string wildcards = "*?")
{
	if (str == NULL || (int)str->length() < 1)
	{
		return false;
	}

	char start = str->c_str()[0];
	if (in_string(start, wildcards) == -1)
	{
		return false;
	}
	else
	{
		if (uniform)
		{
			wildcards.clear();
			wildcards += start;
		}

		for (int i = 0; i < (int)str->length(); i++)
		{
			if (in_string(str->c_str()[i], wildcards) == -1) return false;
		}
	}
	return true;
}


// Checks if a char is a valid character for a numberic hex value
bool is_hex_char(char c, bool allow_0x_prefix = false)
{
	if ((c >= 48 && c <= 57) // 0-9
		|| (c >= 65 && c <= 70) // A-F
		|| (c >= 97 && c <= 102) // a-f
		)
	{
		return true;
	}
	else if (allow_0x_prefix && (c == 88 || c == 120)) // 'x' or 'X'
	{
		return true;
	}
	return false;
}


// Checks if a string can be parsed as a numeric hex value
bool is_hex_value(std::string *arg, bool allow_0x_prefix = false)
{
	if (arg == NULL || arg->length() < 1)
	{
		// Empty string
		return false;
	}

	int i = 0;
	switch (arg->length())
	{
		case 2:
			if (!is_hex_char(arg->c_str()[1])) return false;
		case 1:
			if (!is_hex_char(arg->c_str()[0])) return false;
			break;

		default:
			if (allow_0x_prefix && arg->c_str()[0] == '0' && (arg->c_str()[1] == 'x' || arg->c_str()[1] == 'X')) i = 2; // Check for "0x" prefix
			
			for (; i < (int)arg->length(); i++)
			{
				if (!is_hex_char(arg->c_str()[i]))
				{
					return false;
				}
			}
			break;
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
int cc_exit(std::vector<std::string> args, std::string *output)
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
	return CONSOLE_COMMAND_SUCCESS;
}


// Closes the console
int cc_close(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->console->is_open())
	{
		gl_pSpD3D9Device->overlay->console->toggle();
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Lists all available console commands/aliases
int cc_all_commands(std::vector<std::string> args, std::string *output)
{
	int lines = 0;
	for (auto cmd : SpD3D9OConsole::commands)
	{
		output->append("    ").append(cmd.command);
		lines++;
		if (lines < (int)SpD3D9OConsole::commands.size())
		{
			output->append("\n");
		}
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Clears the console output
int cc_console_clear(std::vector<std::string> args, std::string *output)
{
	gl_pSpD3D9Device->overlay->console->clear();
	return CONSOLE_COMMAND_SUCCESS;
}


// Prints the help string for a given command
int cc_help(std::vector<std::string> args, std::string *output)
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
			output->append("(\"").append(query).append("\" is an alias for \"").append(SpD3D9OConsole::commands.at(index).alias_for);
			args_to_string(SpD3D9OConsole::commands.at(index).macro_args, output);
			output->append("\")\n");
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
		return ERROR_PROC_NOT_FOUND;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Sets whether the Windows cursor is visible
/*int cc_windows_cursor(std::vector<std::string> args, std::string *output)
{
	// NOTE: ShowCursor() only works if called from the thread that initialized the window
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	int cursor_display_count;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				cursor_display_count = ShowCursor(false);
				output->append("New cursor display count = ").append(std::to_string(cursor_display_count)).append("\n");
				break;
			case 1:
				cursor_display_count = ShowCursor(true);
				output->append("New cursor display count = ").append(std::to_string(cursor_display_count)).append("\n");
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	CURSORINFO cursor_info;
	cursor_info.cbSize = sizeof(CURSORINFO);
	if (GetCursorInfo(&cursor_info))
	{
		switch (cursor_info.flags)
		{
			case 0:
				output->append("Windows cursor = hidden");
				break;
			case CURSOR_SHOWING:
				output->append("Windows cursor = displayed");
				break;
			case CURSOR_SUPPRESSED:
				output->append("Windows cursor = suppressed");
				break;
			default:
				output->append("Windows cursor status is unknown (flag value = ").append(std::to_string(cursor_info.flags)).append(")");
				ret_val = ERROR_INVALID_DATA;
				break;
		}
	}
	else
	{
		// Handle error; unable to obtain cursor info
		ret_val = GetLastError();
		output->append("ERROR: Unable to obtain Windows cursor status (Error code ").append(std::to_string(ret_val)).append(")");
	}
	return ret_val;
}*/


// Sets whether the DirectX9 cursor is visible
/*int cc_d3d9_cursor(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	bool cursor_is_visible;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->ShowCursor(false);
				break;
			case 1:
				gl_pSpD3D9Device->ShowCursor(true);
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	
	return ret_val;
}*/


// Prints the current date
int cc_date(std::vector<std::string> args, std::string *output)
{
	if (append_current_date_string(output, false, SP_DATE_MMDDYYYY))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current date");
		return ERROR_INVALID_TIME;
	}
	return CONSOLE_COMMAND_SUCCESS;
}

// Prints the current time
int cc_time(std::vector<std::string> args, std::string *output)
{
	if(append_current_timestamp_string(output, false))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current time");
		return ERROR_INVALID_TIME;
	}
	return CONSOLE_COMMAND_SUCCESS;
}

// Prints the current date and time
int cc_date_time(std::vector<std::string> args, std::string *output)
{
	if (append_current_date_string(output, false, SP_DATE_MMDDYYYY))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current date");
		return ERROR_INVALID_TIME;
	}
	output->append("  ");
	if (append_current_timestamp_string(output, false))
	{
		output->clear();
		output->append("ERROR: Failed to obtain current time");
		return ERROR_INVALID_TIME;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Pauses console thread execution for the specified number of milliseconds
int cc_sleep(std::vector<std::string> args, std::string *output)
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
			return ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Creates an alias for an existing command
int cc_alias(std::vector<std::string> args, std::string *output)
{
	if (args.size() < 2)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}

	int ret;
	std::vector<std::string> macro_args; // Only used if alias contains arguments
	if (args.size() > 2)
	{
		std::vector<std::string>::const_iterator arg = args.begin();
		arg++;
		arg++;
		for (; arg != args.end(); arg++)
		{
			macro_args.push_back(*arg);
		}
		ret = SpD3D9OConsole::register_alias(args.at(0).c_str(), args.at(1).c_str(), macro_args);
	}
	else
	{
		ret = SpD3D9OConsole::register_alias(args.at(0).c_str(), args.at(1).c_str());
	}

	switch (ret)
	{
		case 0:
			// Alias created successfully
			output->append("SUCCESS: Created alias \"").append(args.at(0)).append("\" for command \"").append(args.at(1));
			if (args.size() > 2)
			{
				args_to_string(macro_args, output);
			}
			output->append("\"");
			break;
		case ERROR_INVALID_ADDRESS:
			output->append("ERROR: Unable to create alias; null pointer was encountered");
			return ERROR_INVALID_ADDRESS;
			break;
		case ERROR_INVALID_PARAMETER:
			output->append("ERROR: Alias cannot be an empty string");
			return ERROR_INVALID_PARAMETER;
			break;
		case ERROR_SXS_XML_E_BADCHARINSTRING:
			output->append("ERROR: Alias must not contain whitespace characters.");
			return ERROR_SXS_XML_E_BADCHARINSTRING;
			break;
		case ERROR_DUP_NAME:
			output->append("ERROR: There is already an existing command or alias with the specified name (\"").append(args.at(0)).append("\")");
			return ERROR_DUP_NAME;
			break;
		case ERROR_PROC_NOT_FOUND:
			output->append("ERROR: Unable to create alias; the specified command was not recognized (\"").append(args.at(1)).append("\")");
			return ERROR_PROC_NOT_FOUND;
			break;
		default:
			output->append("ERROR: Unable to create alias");
			return ERROR_DS_UNKNOWN_ERROR;
			break;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Returns a list of commands that contain the given string
int cc_search_command(std::vector<std::string> args, std::string *output)
{
	if (args.size() < 1 || args.at(0).length() < 1)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}
	else
	{
		std::vector<unsigned int> found_ids; // Used to make sure no commands are returned twice
		seqan::String<char> search_str(args.at(0));
		seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> commands_finder;
		seqan::setHaystack(commands_finder, *SpD3D9OConsole::commands_index);
		while (seqan::find(commands_finder, search_str))
		{
			if (found_ids.size() == 0)
			{
				output->append("Search results:\n");
			}
			unsigned int id = seqan::positionToId(SpD3D9OConsole::commands_set, seqan::position(commands_finder).i1);
			if (std::find(found_ids.begin(), found_ids.end(), id) == found_ids.end())
			{
				output->append("    ").append(std::string(seqan::toCString(seqan::valueById(SpD3D9OConsole::commands_set, id)))).append("\n");
				found_ids.push_back(id);
			}
		}
		seqan::clear(commands_finder);
		if (found_ids.size() > 0)
		{
			output->erase(output->length() - 1, 1); // Remove extra newline
		}
		else
		{
			output->append("0 results.");
		}
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Shows/hides remaining characters for the currently-highlighted autocomplete suggestion in the console input field
int cc_autocomplete_preview(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->autocomplete_preview = false;
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->autocomplete_preview = true;
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->autocomplete_preview)
		output->append("Preview current autocomplete suggestion = enabled");
	else
		output->append("Preview current autocomplete suggestion = disabled");

	return ret_val;
}


// Sets the maximum number of autocomplete suggestions (0 = off)
int cc_autocomplete_limit(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
			ret_val = ERROR_INVALID_PARAMETER;
		}
		else
		{
			gl_pSpD3D9Device->overlay->console->autocomplete_limit = (unsigned int)new_lim;
		}
	}
	output->append("Console autocomplete suggestion limit = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->autocomplete_limit));
	return ret_val;
}


// Pastes clipboard text data into the console input field
int cc_paste(std::vector<std::string> args, std::string *output)
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
				return ERROR_INVALID_WINDOW_HANDLE;
			}
			
		}
		else
		{
			error_code_to_string(err, &err_msg);
			output->append("ERROR: Unable to open clipboard (clipboard might be holding non-text data)\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
			return ERROR_INVALID_DATA;
		}
	}

	HANDLE clipboard_data = GetClipboardData(CF_TEXT); // ANSI text format
	if (clipboard_data == NULL)
	{
		err = GetLastError();
		error_code_to_string(err, &err_msg);
		output->append("ERROR: Unable to access clipboard data\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		CloseClipboard();
		return ERROR_INVALID_ACCESS;
	}

	char *clipboard_text = (char*)GlobalLock(clipboard_data);
	if (clipboard_text == NULL)
	{
		err = GetLastError();
		error_code_to_string(err, &err_msg);
		output->append("ERROR: Unable to read clipboard data\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
		CloseClipboard();
		return ERROR_INVALID_ACCESS;
	}

	std::string clipboard_str = clipboard_text;
	GlobalUnlock(clipboard_data);
	CloseClipboard();
	
	// Remove newline and return feed characters
	for (int c = 0; c < (int)clipboard_str.length(); c++)
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
	return CONSOLE_COMMAND_SUCCESS;
}


// Enables/disables player input reaching the game
int cc_game_input(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				SpD3D9OInputHandler::get()->disable_game_input = true;
				break;
			case 1:
				SpD3D9OInputHandler::get()->disable_game_input = false;
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	if (SpD3D9OInputHandler::get()->disable_game_input)
	{
		output->append("Game input = disabled");
	}
	else
	{
		output->append("Game input = enabled");
	}
	return ret_val;
}


// Enables/disables the console
int cc_console_enabled(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Sets console input echo
int cc_console_echo(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Enables/disables console output
int cc_console_output(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->output_stream = false;
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->output_stream = true;
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->output_stream)
	{
		output->append("    Output stream = enabled");
	}
	else
	{
		output->append("    Output stream = disabled");
	}
	return ret_val;
}


// Changes the number of displayed console output lines
int cc_console_output_lines(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_output_lines = strtol(args.at(0).c_str(), NULL, 10);
		if (new_output_lines > 0 && new_output_lines != LONG_MAX && new_output_lines != LONG_MIN)
		{
			bool console_enabled = gl_pSpD3D9Device->overlay->console->is_open();
			if (console_enabled)
			{
				gl_pSpD3D9Device->overlay->console->toggle();
			}
			gl_pSpD3D9Device->overlay->console->clear_selection();

			if (new_output_lines > _SP_D3D9O_C_MAX_OUTPUT_LINES_)
			{
				new_output_lines = _SP_D3D9O_C_MAX_OUTPUT_LINES_;
			}
			else if (new_output_lines < 1)
			{
				new_output_lines = 1;
			}
			for (int i = (gl_pSpD3D9Device->overlay->console->output_log.size() - 1); i <= new_output_lines; i++)
			{
				// Print extra blank lines if current output log is too small for new output display size
				gl_pSpD3D9Device->overlay->console->print("");
			}
			gl_pSpD3D9Device->overlay->console->output_log_displayed_lines = new_output_lines;

			if (console_enabled)
			{
				gl_pSpD3D9Device->overlay->console->toggle();
			}
			output->append("Console output lines = ").append(std::to_string(new_output_lines));
		}
		else
		{
			output->append("ERROR: Invalid argument (Number of output lines must be a positive integer value)\n");
			output->append("Console output lines = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->output_log_displayed_lines));
			return ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		output->append("Console output lines = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->output_log_displayed_lines));
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Enables/disables console mouse cursor
int cc_console_cursor(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->show_cursor = false;
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->show_cursor = true;
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->show_cursor)
	{
		output->append("    Console mouse cursor = enabled");
	}
	else
	{
		output->append("    Console mouse cursor = disabled");
	}
	return ret_val;
}


// Changes the console mouse cursor size
int cc_console_cursor_size(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_cursor_size = strtol(args.at(0).c_str(), NULL, 10);
		if (new_cursor_size > 0 && new_cursor_size != LONG_MAX && new_cursor_size != LONG_MIN)
		{
			if (new_cursor_size > _SP_D3D9O_C_MAX_FONT_SIZE_) // Max cursor size
			{
				new_cursor_size = _SP_D3D9O_C_MAX_FONT_SIZE_;
			}
			else if (new_cursor_size < _SP_D3D9O_C_MIN_FONT_SIZE_) // Min cursor size
			{
				new_cursor_size = _SP_D3D9O_C_MIN_FONT_SIZE_;
			}
			gl_pSpD3D9Device->overlay->console->cursor_size = new_cursor_size;
			output->append("Console mouse cursor size = ").append(std::to_string(new_cursor_size));
		}
		else
		{
			output->append("ERROR: Invalid argument (Cursor size must be a positive integer value)\n");
			output->append("Console mouse cursor size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->cursor_size));
			return ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		output->append("Console mouse cursor size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->cursor_size));
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Executes each argument as a separate console command
int cc_console_execute(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		for (auto arg : args)
		{
			gl_pSpD3D9Device->overlay->console->execute_command(arg.c_str(), NULL, output);
			if (output->length() > 0)
			{
				gl_pSpD3D9Device->overlay->console->print(output->c_str());
			}
			output->clear();
		}

		output->append("\nExecuted ").append(std::to_string(args.size())).append(" console command");
		if (args.size() > 1)
		{
			output->append("s");
		}
		output->append(".");
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Opens a plaintext script file and executes each line as a separate console command
int cc_console_script(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		// Attempt to open script file (passed as first argument)
		std::ifstream script_file(args.at(0).c_str());

		// Check if script file could be opened
		if (script_file.fail())
		{
			// Failed to open file
			output->append("ERROR: Unable to open the specified script file (\"").append(args.at(0).c_str()).append("\")");
			return ERROR_FILE_NOT_FOUND;
		}


		// Process each line of the script file as a separate console command
		std::string command;
		int line = 0;
		int cmd_exec_val;
		while (std::getline(script_file, command))
		{
			line++;

			// Attempt to run the current command
			if (command.length() > 0)
			{
				cmd_exec_val = gl_pSpD3D9Device->overlay->console->execute_command(command.c_str(), NULL, output);
				gl_pSpD3D9Device->overlay->console->print(output->c_str());
			}
			output->clear();

			if (cmd_exec_val == CONSOLE_COMMAND_NOT_FOUND_ERROR)
			{
				// Specified command doesn't exist
				break;
			}
		}

		if (cmd_exec_val == CONSOLE_COMMAND_NOT_FOUND_ERROR)
		{
			output->append("\nERROR: Invalid command in script at line ").append(std::to_string(line));
			return ERROR_PROC_NOT_FOUND;
		}
		else
		{
			output->append("\nFinished executing script (").append(std::to_string(line)).append(" line");
			if (line != 1)
			{
				output->append("s");
			}
			output->append(")");
		}
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}

	return CONSOLE_COMMAND_SUCCESS;
}


// Changes the console input prompt string
int cc_console_prompt(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->console->clear_selection();
		gl_pSpD3D9Device->overlay->console->prompt = args.at(0);
	}
	output->append("Input prompt = \"").append(gl_pSpD3D9Device->overlay->console->prompt).append("\"");
	return CONSOLE_COMMAND_SUCCESS;
}


// Enables/disables the username element of the console input prompt
int cc_console_prompt_user(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->prompt_elements &= (~SP_D3D9O_PROMPT_USER);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->prompt_elements |= (SP_D3D9O_PROMPT_USER);
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Enables/disables the hostname element of the console input prompt
int cc_console_prompt_host(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->prompt_elements &= (~SP_D3D9O_PROMPT_HOSTNAME);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->prompt_elements |= (SP_D3D9O_PROMPT_HOSTNAME);
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Enables/disables the current working directory element of the console input prompt
int cc_console_prompt_cwd(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->prompt_elements &= (~SP_D3D9O_PROMPT_CWD);
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->prompt_elements |= (SP_D3D9O_PROMPT_CWD);
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Changes the console input caret character
int cc_console_caret(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		if (args.at(0).length() == 1)
		{
			gl_pSpD3D9Device->overlay->console->caret = args.at(0).c_str()[0];
		}
		else
		{
			output->append("ERROR: Invalid argument (Caret must be a single character)\n");
			ret_val = ERROR_INVALID_PARAMETER;
		}
	}
	output->append("Caret character = '");
	*output += gl_pSpD3D9Device->overlay->console->caret;
	output->append("'");
	return ret_val;
}


// Enables/disables box caret mode
int cc_console_box_caret(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		switch (parse_toggle_arg(args.at(0).c_str()))
		{
			case 0:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->box_caret = false;
				break;
			case 1:
				gl_pSpD3D9Device->overlay->console->clear_selection();
				gl_pSpD3D9Device->overlay->console->box_caret = true;
				break;
			default:
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
				break;
		}
	}

	if (gl_pSpD3D9Device->overlay->console->box_caret)
		output->append("Box caret mode = enabled");
	else
		output->append("Box caret mode = disabled");

	return ret_val;
}


// Changes the console input caret blink delay
int cc_console_caret_blink(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
			ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Changes the console border thickness
int cc_console_border_width(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		long new_width = strtol(args.at(0).c_str(), NULL, 10);
		if ((new_width >= 0) && ((new_width == 0 && args.at(0).c_str()[0] == '0') || (new_width != 0 && new_width != LONG_MAX && new_width != LONG_MIN)))
		{
			gl_pSpD3D9Device->overlay->console->clear_selection();
			gl_pSpD3D9Device->overlay->console->border_width = new_width;
			if (new_width == 0)
			{
				gl_pSpD3D9Device->overlay->console->autocomplete_border_width = 0;
			}
			else
			{
				gl_pSpD3D9Device->overlay->console->autocomplete_border_width = 1;
			}
			gl_pSpD3D9Device->overlay->needs_update = true;
		}
		else
		{
			output->append("ERROR: Invalid argument (Width must be a non-negative integer value)\n");
			ret_val = ERROR_INVALID_PARAMETER;
		}
	}
	output->append("Console border width = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->border_width)).append(" pixels");
	return ret_val;
}


// Changes the console font size
int cc_console_font_size(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
	if (args.size() > 0)
	{
		long new_font_size = strtol(args.at(0).c_str(), NULL, 10);
		if (new_font_size > 0 && new_font_size != LONG_MAX && new_font_size != LONG_MIN)
		{
			if (new_font_size > _SP_D3D9O_C_MAX_FONT_SIZE_) // Max font size
			{
				new_font_size = _SP_D3D9O_C_MAX_FONT_SIZE_;
			}
			else if (new_font_size < _SP_D3D9O_C_MIN_FONT_SIZE_) // Min font size
			{
				new_font_size = _SP_D3D9O_C_MIN_FONT_SIZE_;
			}
			gl_pSpD3D9Device->overlay->console->font_height = new_font_size;
		}
		else
		{
			output->append("ERROR: Invalid argument (Font size must be a positive integer value)\n");
			ret_val = ERROR_INVALID_PARAMETER;
		}
	}
	output->append("Console font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->font_height));
	return ret_val;
}


// Restores developer default settings for the console
int cc_console_restore_dev_defaults(std::vector<std::string> args, std::string *output)
{
	gl_pSpD3D9Device->overlay->console->restore_default_settings();

	output->append("Restored developer default console settings:\n");
	
	console_settings_to_string(output);

	return CONSOLE_COMMAND_SUCCESS;
}


// Re-loads user's console settings preferences from the config file
int cc_console_restore_user_prefs(std::vector<std::string> args, std::string *output)
{
	gl_pSpD3D9Device->overlay->console->get_user_prefs();

	output->append("Restored user-preferred console settings:\n");
	
	console_settings_to_string(output);

	return CONSOLE_COMMAND_SUCCESS;
}


// Enables/disables overlay text feed
int cc_text_feed_enabled(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}

// Changes the text feed font size
int cc_text_feed_font_size(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		long new_font_size = strtol(args.at(0).c_str(), NULL, 10);
		if (new_font_size > 0 && new_font_size != LONG_MAX && new_font_size != LONG_MIN)
		{
			extern int current_text_feed_font_size;
			if (new_font_size > _SP_D3D9O_TF_MAX_FONT_SIZE_) // Max font size
			{
				new_font_size = _SP_D3D9O_TF_MAX_FONT_SIZE_;
			}
			else if (new_font_size < _SP_D3D9O_TF_MIN_FONT_SIZE_) // Min font size
			{
				new_font_size = _SP_D3D9O_TF_MIN_FONT_SIZE_;
			}
			current_text_feed_font_size = new_font_size;
			gl_pSpD3D9Device->overlay->text_feed->set_font_height(new_font_size);
			output->append("Text feed font size = ").append(std::to_string(new_font_size));
		}
		else
		{
			output->append("ERROR: Invalid argument (Font size must be a positive integer value)\n");
			output->append("Text feed font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->text_feed->font_height));
			return ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		output->append("Text feed font size = ").append(std::to_string(gl_pSpD3D9Device->overlay->text_feed->font_height));
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Enables/disables overlay text feed info bar
int cc_text_feed_info_bar(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Enables/disables overlay text feed info bar date element
int cc_text_feed_info_date(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Enables/disables overlay text feed info bar time element
int cc_text_feed_info_time(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Enables/disables overlay text feed info bar FPS counter element
int cc_text_feed_info_fps(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;
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
				output->append(ERROR_INVALID_TOGGLE_ARGUMENT);
				ret_val = ERROR_INVALID_PARAMETER;
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
	return ret_val;
}


// Changes the text feed title
int cc_text_feed_title(std::vector<std::string> args, std::string *output)
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
	return CONSOLE_COMMAND_SUCCESS;
}


// Prints to text feed
int cc_text_feed_print(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->text_feed->print(args.at(0).c_str());
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Cycles between 9 preset text feed positions
int cc_text_feed_cycle_position(std::vector<std::string> args, std::string *output)
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
		return ERROR_BAD_ENVIRONMENT;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


int cc_text_feed_position(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		get_text_feed_pos_str(output);
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
		return ERROR_BAD_ENVIRONMENT;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Cycles through the 3 text feed text styles (plain, shadowed, outlined)
int cc_text_feed_cycle_style(std::vector<std::string> args, std::string *output)
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
		return ERROR_BAD_ENVIRONMENT;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


int cc_text_feed_style(std::vector<std::string> args, std::string *output)
{
	if (gl_pSpD3D9Device->overlay->text_feed->is_enabled())
	{
		get_text_feed_style_str(output);
	}
	else
	{
		output->append(ERROR_TXT_FEED_DISABLED);
		return ERROR_BAD_ENVIRONMENT;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Loads a DLL
int cc_load_library(std::vector<std::string> args, std::string *output)
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
			return ERROR_OPEN_FAILED;
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
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Unloads a DLL
int cc_free_library(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		// Try unloading library by filename first
		HMODULE hmod = GetModuleHandle(args.at(0).c_str());
		if (hmod != NULL)
		{
			// Successfully obtained HMODULE
			BOOL lib_freed = FreeLibrary(hmod);
			DWORD err = GetLastError();
			if (!lib_freed)
			{
				std::string err_msg;
				error_code_to_string(err, &err_msg);
				output->append("ERROR: Failed to unload library (\"").append(args.at(0)).append("\")\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
				return ERROR_UNABLE_TO_UNLOAD_MEDIA;
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
			return CONSOLE_COMMAND_SUCCESS;
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
				return CONSOLE_COMMAND_SUCCESS;
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
				return CONSOLE_COMMAND_SUCCESS;
			}
		}
		output->append("ERROR: Failed to unload library (\"").append(args.at(0)).append("\")");
		return ERROR_UNABLE_TO_UNLOAD_MEDIA;
	}
	else
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Searches process memory for the given array of bytes
int cc_aob_scan(std::vector<std::string> args, std::string *output)
{
	int ret_val = CONSOLE_COMMAND_SUCCESS;

	// Parse args to build AoB string
	std::string aob;
	bool invalid = false; // Set to true if an invalid byte value was passed to this function
	int i = 0;
	if (args.size() < 2)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		if (args.size() > 0) output->append(" (Byte array should contain 2 or more bytes)");
		return ERROR_BAD_ARGUMENTS;
	}
	else
	{
		for (; i < (int)args.size(); i++)
		{
			switch (args.at(i).length())
			{
				case 1:
					// ?, *, or H
					if (!is_hex_char(args.at(i).c_str()[0]))
					{
						if (in_string(args.at(i).c_str()[0], "*?") != -1)
						{
							args.at(i).clear();
							args.at(i).append("??");
						}
						else
						{
							invalid = true;
						}
					}
					else
					{
						args.at(i).insert(0, "0");
					}
					break;

				case 2:
					// ??, **, or HH
					if (is_wildcard(&args.at(i)))
					{
						args.at(i).clear();
						args.at(i).append("??");
					}
					else if (!is_hex_value(&args.at(i)))
					{
						invalid = true;
					}
					break;

				case 3:
					// 0xH
					if (args.at(i).c_str()[0] == '0' && (args.at(i).c_str()[1] == 'x' || args.at(i).c_str()[1] == 'X') && is_hex_char(args.at(i).c_str()[2]))
					{
						args.at(i).erase(0, 2);
						args.at(i).insert(0, "0");
					}
					else
					{
						invalid = true;
					}
					break;

				case 4:
					// 0xHH
					if (args.at(i).c_str()[0] == '0' && (args.at(i).c_str()[1] == 'x' || args.at(i).c_str()[1] == 'X') && is_hex_char(args.at(i).c_str()[2]) && is_hex_char(args.at(i).c_str()[3]))
						args.at(i).erase(0, 2);
					else
						invalid = true;
					break;

				default:
					invalid = true;
					break;
			} // switch (args.at(i).length())

			if (invalid)
			{
				output->append(std::string("ERROR: Invalid byte value in byte array at index ").append(std::to_string(i)).append(" (\"").append(args.at(i)).append("\")"));
				return ERROR_INVALID_PARAMETER;
			}
			else
			{
				if (aob.length() != 0) aob.append(" ");
				aob.append(args.at(i));
			}
		} // for loop
	}

	// Search for AoB
	output->append("Scanning for byte pattern: ").append(aob).append("\nResults:");
	std::vector<uint8_t*> results;
	void *result = aob_scan(&aob, NULL, &results);
	if (result != NULL)
	{
		for(int i = 0; i < (int)results.size(); i++)
		{
			std::stringstream hex_stream;
			hex_stream << std::hex << (void*)results.at(i);
			output->append("\n    0x").append(hex_stream.str());
		}
		output->append("\nSUCCESS: Found matching byte pattern");
		if ((int)results.size() != 1) output->append("s");
		output->append(" at ").append(std::to_string(results.size()));
		if ((int)results.size() > 1)
			output->append(" different memory locations");
		else
			output->append(" memory location");
		if ((int)results.size() >= MAX_AOBSCAN_RESULTS)
			output->append(" (Increase max result limit to view more search results).");
		else
			output->append(".");
		if ((int)results.size() > (gl_pSpD3D9Device->overlay->console->output_log_displayed_lines - 3)) output->append("\nWARNING: Number of results exceeds output display lines. Only the first ").append(std::to_string(gl_pSpD3D9Device->overlay->console->output_log_displayed_lines - 3)).append(" results are shown.");
	}
	else
	{
		ret_val = (int)GetLastError();
		switch (ret_val)
		{
			case SP_NO_ERROR:
				// Byte array was not found
				output->append("\nSearch returned 0 results.");
				ret_val = CONSOLE_COMMAND_SUCCESS;
				break;
			case SP_ERROR_INVALID_PARAMETER:
				output->append("\nERROR: Invalid search array (too short or bad address)");
				break;
			case SP_ERROR_INSUFFICIENT_BUFFER:
				output->append("\nERROR: Search array is too large (Max AoB length = ").append(std::to_string(MAX_AOB_LENGTH)).append(" bytes); try increasing limit.");
				break;
			case SP_ERROR_INVALID_ADDRESS:
				// Invalid starting address
				output->append("\nERROR: Starting address was invalid");
				break;
			default:
				// Byte array was not found due to an unknown error
				output->append("\nERROR: Code ").append(std::to_string(ret_val));
				break;
		}
	}
	return ret_val;
}


// Opens the specified URL in the system's default web browser
int cc_open_web_page(std::vector<std::string> args, std::string *output)
{
	DWORD err;
	std::string err_msg;
	if (args.size() < 1)
	{
		output->append(ERROR_TOO_FEW_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
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
			return ERROR_FILE_NOT_FOUND;
		}
	}
	return CONSOLE_COMMAND_SUCCESS;
}


// Executes a command using the system shell
/*int cc_shell(std::vector<std::string> args, std::string *output)
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
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
}*/


// Beeps for the specified frequency and duration
int cc_beep(std::vector<std::string> args, std::string *output)
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
	return CONSOLE_COMMAND_SUCCESS;
}


// Prints each argument on a separate line
int cc_echo(std::vector<std::string> args, std::string *output)
{
	for (auto arg : args)
	{
		output->append(arg).append("\n");
	}
	if (output->length() > 0)
	{
		output->erase(output->length() - 1, 1); // Remove extra newline
	}
	return CONSOLE_COMMAND_SUCCESS;
}



DWORD WINAPI cc_run_thread(LPVOID lpParam)
{
	char *cmd = (char *)lpParam;
	int result = system(cmd);
	delete cmd;
	return result;
}
// Opens a file with the system resolver
int cc_run(std::vector<std::string> args, std::string *output)
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
		return ERROR_BAD_ARGUMENTS;
	}
	return CONSOLE_COMMAND_SUCCESS;
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
	SpD3D9OConsole::register_command("sleep", cc_sleep, "sleep <duration>\n    Pauses execution for the specified duration (in milliseconds).");
	SpD3D9OConsole::register_alias("wait", "sleep");
	//SpD3D9OConsole::register_command("windows_cursor", cc_windows_cursor, "windows_cursor [is_enabled]\n    Sets whether the Windows cursor is visible (1 = showing, 0 = hidden).");
	//SpD3D9OConsole::register_command("directx_cursor", cc_d3d9_cursor, "directx_cursor [is_enabled]\n    Sets whether the DirectX cursor is visible (1 = showing, 0 = hidden).");
	SpD3D9OConsole::register_command("date", cc_date, "date\n    Prints the current date (in MM/DD/YYYY format).");
	SpD3D9OConsole::register_command("date_time", cc_date_time, "date_time\n    Prints the current date (in MM/DD/YYYY format) and 24-hour time.");
	SpD3D9OConsole::register_command("time", cc_time, "time\n    Prints the current 24-hour time.");
	SpD3D9OConsole::register_command("alias", cc_alias, "alias <alias> <existing command> [arguments...]\n    Creates an alias for an existing console command/alias.\n    When the alias is called, the existing command is executed with the given arguments (if any).\n    If the alias is called with additional arguments, the extra arguments are added to the end of the argument list provided when the alias was created.");
	SpD3D9OConsole::register_command("paste", cc_paste, "paste\n    Copies ANSI text data from the clipboard to the console input");
	SpD3D9OConsole::register_command("beep", cc_beep, "beep <frequency> <duration>\n    Generates a beeping sound at the given frequency (hz) for the given duration (milliseconds).\n    Execution is halted until the beep is completed.");
	SpD3D9OConsole::register_command("load_library", cc_load_library, "load_library <filename>\n    Loads the specified dynamic link library (DLL) file.");
	SpD3D9OConsole::register_command("free_library", cc_free_library, "free_library <filename|HMODULE>\n    Unloads the specified dynamic link library (DLL) module.\n    The module can be specified through the .dll file name or its starting address in memory (HMODULE).");
	SpD3D9OConsole::register_alias("unload_library", "free_library");
	SpD3D9OConsole::register_command("aob_scan", cc_aob_scan, "aob_scan <byte> [byte...]\n    Scans process memory for the given byte array (AoB = \"Array of Bytes\"). Each byte in the byte pattern should be passed as a separate argument.\n    Wildcard bytes (bytes that can be any value) can be passed as \"??\" or \"**\".");
	//SpD3D9OConsole::register_command("shell", cc_shell, "shell <URL>\n    Executes a shell command in the system default shell.");
	SpD3D9OConsole::register_command("web", cc_open_web_page, "web <URL>\n    Opens a web page in the system default web browser.");
	SpD3D9OConsole::register_command("game_input", cc_game_input, "game_input [is_enabled]\n    Enables/disables game input. If input is disabled, mouse, keyboard, and other input will not affect the game state.");
	SpD3D9OConsole::register_command("console", cc_console_enabled, "console [is_open]\n    Opens/closes the console (1 = open, 0 = hidden).");
	SpD3D9OConsole::register_command("console_restore_developer_default_settings", cc_console_restore_dev_defaults, "console_restore_developer_default_settings\n    Restores all console settings to developer-preferred values.");
	SpD3D9OConsole::register_command("console_restore_user_preferred_settings", cc_console_restore_user_prefs, std::string("console_restore_user_preferred_settings\n    Re-loads console settings from user preference configuration file (").append(_SP_D3D9O_C_PREF_FILE_).append(")").c_str());
	SpD3D9OConsole::register_command("console_clear", cc_console_clear, "console_clear\n    Clears console output.");
	SpD3D9OConsole::register_alias("clear", "console_clear");
	SpD3D9OConsole::register_command("console_echo", cc_console_echo, "console_echo [is_enabled]\n    Enables/disables console input echo (1 = on, 0 = off).");
	SpD3D9OConsole::register_command("console_output", cc_console_output, "console_output [is_enabled]\n    Enables/disables console output stream (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("console_output_lines", cc_console_output_lines, "console_output_lines [line_count]\n    Sets the size of the console output window to the specified number of output lines.");
	SpD3D9OConsole::register_command("console_execute", cc_console_execute, "console_execute <command> [command...]\n    Executes each argument as a separate console command.");
	SpD3D9OConsole::register_command("console_script", cc_console_script, "console_script <file>\n    Opens a plain-text script file and executes each line as a separate console command.");
	SpD3D9OConsole::register_alias("script", "console_script");
	SpD3D9OConsole::register_command("console_font_size", cc_console_font_size, std::string("console_font_size [size]\n    Sets the console overlay font size. Font size ranges from ").append(std::to_string(_SP_D3D9O_C_MIN_FONT_SIZE_)).append(" to ").append(std::to_string(_SP_D3D9O_C_MAX_FONT_SIZE_)).append(".").c_str());
	SpD3D9OConsole::register_command("console_autocomplete_preview", cc_autocomplete_preview, "console_autocomplete_preview [is_enabled]\n    Shows/hides preview of remaining characters for the currently-highlighted autocomplete suggestion in the console input field (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("console_autocomplete_limit", cc_autocomplete_limit, "console_autocomplete_limit [limit]\n    Sets the maximum number of autocomplete suggestions to be shown (0 = off).");
	SpD3D9OConsole::register_command("console_prompt", cc_console_prompt, "console_prompt [prompt]\n    Sets the console input prompt string.");
	SpD3D9OConsole::register_command("console_prompt_user", cc_console_prompt_user, "console_prompt_user [is_enabled]\n    Enables/disables the username element of the console input prompt.");
	SpD3D9OConsole::register_command("console_prompt_hostname", cc_console_prompt_host, "console_prompt_hostname [is_enabled]\n    Enables/disables the hostname element of the console input prompt.");
	SpD3D9OConsole::register_command("console_prompt_cwd", cc_console_prompt_cwd, "console_prompt_cwd [is_enabled]\n    Enables/disables the working directory element of the console input prompt.");
	SpD3D9OConsole::register_command("console_caret", cc_console_caret, "console_caret [caret]\n    Sets the console input caret character.");
	SpD3D9OConsole::register_command("console_box_caret", cc_console_box_caret, "console_box_caret [is_enabled]\n    Enables/disables box caret mode. If box caret mode is enabled, caret character setting is ignored.");
	SpD3D9OConsole::register_command("console_caret_blink", cc_console_caret_blink, "console_caret_blink [blink_delay]\n    Sets the console input caret blink delay time (in milliseconds).");
	SpD3D9OConsole::register_command("console_border_width", cc_console_border_width, "console_border_width [width]\n    Sets the console border width.");
	SpD3D9OConsole::register_command("console_cursor", cc_console_cursor, "console_cursor [is_enabled]\n    Enables/disables the console mouse cursor.");
	SpD3D9OConsole::register_command("console_cursor_size", cc_console_cursor_size, std::string("console_cursor_size [size]\n    Sets the size of the console mouse cursor. Mouse cursor size ranges from ").append(std::to_string(_SP_D3D9O_C_MIN_FONT_SIZE_)).append(" to ").append(std::to_string(_SP_D3D9O_C_MAX_FONT_SIZE_)).append(".").c_str());
	SpD3D9OConsole::register_command("echo", cc_echo, "echo [args]\n    Prints each argument on a separate line.");
	SpD3D9OConsole::register_command("open", cc_run, "open <file>\n    Opens or runs a file using the system resolver.");
	//SpD3D9OConsole::register_alias("run", "open");
	SpD3D9OConsole::register_command("text_feed", cc_text_feed_enabled, "text_feed [is_enabled]\n    Enables/disables the overlay text feed (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_info_bar", cc_text_feed_info_bar, "text_feed_info_bar [is_enabled]\n    Enables/disables the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_date", cc_text_feed_info_date, "text_feed_date [is_enabled]\n    Enables/disables the date element of the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_time", cc_text_feed_info_time, "text_feed_time [is_enabled]\n    Enables/disables the time element of the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_fps", cc_text_feed_info_fps, "text_feed_fps [is_enabled]\n    Enables/disables the FPS counter element of the overlay text feed info bar (1 = enabled, 0 = disabled).");
	SpD3D9OConsole::register_command("text_feed_print", cc_text_feed_print, "text_feed_print <message>\n    Prints a message to the overlay text feed.");
	SpD3D9OConsole::register_command("text_feed_font_size", cc_text_feed_font_size, std::string("text_feed_font_size [size]\n    Sets the overlay text feed font size. Font size ranges from 1 to ").append(std::to_string(_SP_D3D9O_TF_MAX_FONT_SIZE_)).append(".").c_str());
	SpD3D9OConsole::register_command("text_feed_title", cc_text_feed_title, "text_feed_title [title]\n    Sets the overlay text feed title message (only shown if text feed info bar is enabled).");
	SpD3D9OConsole::register_command("text_feed_position", cc_text_feed_position, "text_feed_position\n    Returns the current overlay text feed position.");
	SpD3D9OConsole::register_command("text_feed_style", cc_text_feed_style, "text_feed_style\n    Returns the current overlay text feed style (plain, outlined, shadowed).");
	SpD3D9OConsole::register_command("text_feed_cycle_position", cc_text_feed_cycle_position, "text_feed_cycle_position\n    Cycles through the 9 text feed position presets.");
	SpD3D9OConsole::register_command("text_feed_cycle_style", cc_text_feed_cycle_style, "text_feed_cycle_style\n    Cycles through the 3 text feed font style presets (plain, outlined, shadowed).");
}