// Author: Sean Pesce
// Default console command functions

#include "stdafx.h"
#include "SpD3D9OConsole.h"
#include <cstdlib>
#include <climits>
#include <sstream>
#include <iomanip> // std::hex


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


void cc_all_commands(std::vector<std::string> args, std::string *output)
{
	for (auto cmd : SpD3D9OConsole::commands)
	{
		output->append("\"").append(cmd.command).append("\"\n");
	}
	output->erase(output->length() - 1, 1);
}


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



void cc_print(std::vector<std::string> args, std::string *output)
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

void register_default_console_commands()
{
	SpD3D9OConsole::register_command("exit", cc_exit, "exit [exit code]");
	SpD3D9OConsole::register_command("quit", cc_exit, "exit [exit code]");
	SpD3D9OConsole::register_command("close", cc_close, "Closes the console window");
	SpD3D9OConsole::register_command("alias", cc_alias, "alias <ALIAS|COMMAND> <ALIAS|COMMAND>");
	SpD3D9OConsole::register_command("paste", cc_paste, "Copies ANSI text data from the clipboard to the console input");
	SpD3D9OConsole::register_command("beep", cc_beep, "beep <frequency> <duration>");
	SpD3D9OConsole::register_command("commands", cc_all_commands, "Lists all available console commands");
	SpD3D9OConsole::register_command("load_library", cc_load_library, "load_library <filename>");
	SpD3D9OConsole::register_command("unload_library", cc_free_library, "unload_library <filename|HMODULE>");
	SpD3D9OConsole::register_command("free_library", cc_free_library, "free_library <filename|HMODULE>");
}