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
	gl_pSpD3D9Device->overlay->console->toggle();
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


// Creates an alias for an existing command
void cc_alias(std::vector<std::string> args, std::string *output)
{
	if (args.size() < 2)
	{
		output->append("ERROR: Too few arguments");
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

	int ret = SpD3D9OConsole::register_command(alias->c_str(), SpD3D9OConsole::commands.at(index).function, SpD3D9OConsole::commands.at(index).help_message.c_str());
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
		output->append("ERROR: No search term was specified.");
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


// Prints to text feed
void cc_print_text_feed(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->text_feed->print(args.at(0).c_str());
	}
	else
	{
		output->append("ERROR: Too few arguments");
	}
}


// Enables/disables the console
void cc_console_enabled(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		if (args.at(0).length() == 1 && (args.at(0).c_str()[0] == '1' || args.at(0).c_str()[0] == '0'))
		{
			switch (args.at(0).c_str()[0])
			{
				case '0':
					gl_pSpD3D9Device->overlay->console->toggle();
					break;
				case '1':
					break;
				default:
					break;
			}
		}
		else
		{
			output->append("ERROR: Console value must be either 1 or 0 (1 = open, 0 = hidden)\n");
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


// Changes the console input prompt string
void cc_console_prompt(std::vector<std::string> args, std::string *output)
{
	if (args.size() > 0)
	{
		gl_pSpD3D9Device->overlay->console->prompt = args.at(0);
	}
	output->append("Input prompt = \"").append(gl_pSpD3D9Device->overlay->console->prompt).append("\"");
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

	gl_pSpD3D9Device->overlay->console->prompt = _SP_D3D9O_C_DEFAULT_PROMPT_;
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
	output->append("    Border width = ").append(std::to_string(gl_pSpD3D9Device->overlay->console->border_width)).append(" pixels");
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
			output->append("SUCCESS: Library was loaded (Base addres = 0x").append(hex_stream.str()).append(")");
		}
	}
	else
	{
		output->append("ERROR: No library specified");
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
				// Successfully unloaded the library
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
		output->append("ERROR: No library specified");
	}
}


// Opens the specified URL in the system's default web browser
void cc_open_web_page(std::vector<std::string> args, std::string *output)
{
	DWORD err;
	std::string err_msg;
	if (args.size() < 1)
	{
		output->append("ERROR: No URL specified");
	}
	else
	{
		if ((err = (DWORD)ShellExecute(0, 0, args.at(0).c_str(), 0, 0, SW_SHOW)) < 32)
		{
			error_code_to_string(err, &err_msg);
			// Try adding "http://"
			if (((DWORD)ShellExecute(0, 0, std::string(args.at(0)).insert(0,"http://").c_str(), 0, 0, SW_SHOW)) < 32)
			{
				output->append("ERROR: Unable to open URL \"").append(args.at(0)).append("\"\nError code ").append(std::to_string(err)).append(" (").append(err_msg).append(")");
			}
		}
	}
}


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

void register_default_console_commands()
{
	SpD3D9OConsole::register_command("help", cc_help, "help [command]\n    Prints the help message for the given command.");
	SpD3D9OConsole::register_command("exit", cc_exit, "exit [exit code]\n    Exits the game.");
	SpD3D9OConsole::register_command("quit", cc_exit, "quit [exit code]\n    Exits the game.");
	SpD3D9OConsole::register_command("commands", cc_all_commands, "commands\n    Lists all available console commands");
	SpD3D9OConsole::register_command("search_command", cc_search_command, "search_command <query>\n    Returns a list of available commands that contain the given query string.");
	SpD3D9OConsole::register_command("close", cc_close, "close\n    Closes the console overlay.");
	SpD3D9OConsole::register_command("clear", cc_clear, "clear\n    Clears console output.");
	SpD3D9OConsole::register_command("alias", cc_alias, "alias <ALIAS|COMMAND> <ALIAS|COMMAND>\n    Creates an alias for an existing console command/alias.");
	SpD3D9OConsole::register_command("paste", cc_paste, "paste\n    Copies ANSI text data from the clipboard to the console input");
	SpD3D9OConsole::register_command("beep", cc_beep, "beep <frequency> <duration>\n    Generates a beeping sound at the given frequency (hz) for the given duration (milliseconds).\n    Execution is halted until the beep is completed.");
	SpD3D9OConsole::register_command("load_library", cc_load_library, "load_library <filename>\n    Loads the specified dynamic link library (DLL) file.");
	SpD3D9OConsole::register_command("unload_library", cc_free_library, "unload_library <filename|HMODULE>\n    Unloads the specified dynamic link library (DLL) module.\n    The module can be specified through the .dll file name or its starting address in memory (HMODULE).");
	SpD3D9OConsole::register_command("free_library", cc_free_library, "free_library <filename|HMODULE>\n    Unloads the specified dynamic link library (DLL) module.\n    The module can be specified through the .dll file name or its starting address in memory (HMODULE).");
	SpD3D9OConsole::register_command("web", cc_open_web_page, "web <URL>\n    Opens a web page in the system default web browser.");
	SpD3D9OConsole::register_command("print_text_feed", cc_print_text_feed, "print_text_feed <message>\n    Prints a message to the overlay text feed.");
	SpD3D9OConsole::register_command("console", cc_console_enabled, "console [is_open]\n    Opens/closes the console (1 = open, 0 = hidden).");
	SpD3D9OConsole::register_command("console_restore_developer_default_settings", cc_console_restore_dev_defaults, "console_restore_developer_default_settings\n    Restores all console settings to developer-preferred values.");
	SpD3D9OConsole::register_command("console_font_size", cc_console_font_size, "console_font_size [size]\n    Sets the console overlay font size.");
	SpD3D9OConsole::register_command("console_autocomplete_limit", cc_autocomplete_limit, "autocomplete_limit [limit]\n    Sets the maximum number of autocomplete suggestions to be shown (0 = off).");
	SpD3D9OConsole::register_command("console_prompt", cc_console_prompt, "console_prompt [prompt]\n    Sets the console input prompt string.");
	SpD3D9OConsole::register_command("console_caret", cc_console_caret, "console_caret [caret]\n    Sets the console input caret character.");
	SpD3D9OConsole::register_command("console_caret_blink", cc_console_caret_blink, "console_caret_blink [blink_delay]\n    Sets the console input caret blink delay time (in milliseconds).");
	SpD3D9OConsole::register_command("console_border_width", cc_console_border_width, "console_border_width [width]\n    Sets the console border width.");
	SpD3D9OConsole::register_command("echo", cc_echo, "echo [args]\n    Prints each argument on a separate line.");
}