// Author: Sean Pesce

#pragma once

#ifndef SP_IO_HPP
	#define SP_IO_HPP


#include <chrono>	// localtime_s
#include <fstream>	// ofstream
#include <iostream> // cout, endl
#include <limits>	// numeric_limits
#include <sstream>  // stringstream

#ifdef _WIN32
	#include <Windows.h>
#endif // _WIN32


/////////////////////// Constants ///////////////////////
#ifdef _WIN32
	#define GetAsyncKeyboardState get_async_keyboard_state
	#define _SP_KEY_DOWN_ 2147483648
	#define _SP_KEY_TOGGLED_ 1
#endif // _WIN32



/////////////////////// Shell I/O ///////////////////////


/**
	sp_print_intro(const char*)
	
	Prints the program name and author information.

	@param program_name	The name of the program.
 */
void sp_print_intro(const char* program_name);


/**
	enter_to_continue(const char*)
	
	Prints a prompt message and waits for the user to press the Enter (Return) key.

	@param prompt	The prompt message to print before the user presses Enter.
 */
void enter_to_continue(const char* prompt);


/**
	enter_to_continue(const char*, const char*)
	
	Prints a prompt message, waits for the user to press Enter (Return) key, and
	prints a second message before continuing.

	@param prompt	The prompt message to print before the user presses Enter.
	@param continue_msg	The message to print after the user presses Enter.
 */
void enter_to_continue(const char* prompt, const char* continue_msg);



/////////////////////// Text File I/O ///////////////////////


/**
	file_write_text(const char*, const char*)

	Writes the specified output to a text file. If the file already exists, it
	is overwritten.
	
	@param file	Path and file name of the file to write to.
	@param msg	The string that will be written to file.
	
	@return 1 on success; return 0 if the file could not be written.
 */
int file_write_text(const char *file, const char *msg);



/////////////////////// Text Data I/O ///////////////////////


/**
	generate_current_timestamp(char*, bool)

	Constructs a timestamp string for the current 24-hour time,
	formatted as "HH:MM:SS" or "[HH:MM:SS]", and stores it in
	the specified char* buffer with a trailing null character.

	@param timestamp_string_buff	A buffer to hold the generated timestamp string.
									The buffer must be AT LEAST sizeof(char[11]) if
									surround_with_brackets is true, and must be AT
									LEAST sizeof(char[9]) if surround_with_brackets
									is false.
	@param surround_with_brackets	Specifies whether the generated timestamp should
									be contained within bracket characters ('[' and ']').

	@return 0 on success; return localtime_s error code on failure.
*/
int generate_current_timestamp(char *timestamp_string_buff, bool surround_with_brackets);



/////////////////////// Keyboard I/O ///////////////////////


/**
	get_vk_hotkey(const char*, const char*, const char*)
	
	Parses config file for a specified hotkey, denoted as a virtual key code.
	
	For more info on virtual key codes:
		https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
	
	@param settings_file	Name of the file that will be parsed for the hotkey (generally an .ini file)
	@param section			Title of the section of the settings file where the key will be found (Format in the file should be "[section]")
	@param key_name			Name of the key where the hotkey value is specified (Format in the file should be "key_name=XX", where "XX"
							denotes a hex-formatted virtual key code)
	
	@return the virtual key code for the specified hotkey as an unsigned int
 */
unsigned int get_vk_hotkey(const char *settings_file, const char *section, const char *key_name);


#ifdef _WIN32

/**
	get_async_keyboard_state(SHORT*)

	Gets the async key state for all 256 virtual keys and stores them in the given buffer.
	Note: The buffer must be at least the size of an array of 256 SHORTs.

	@param keyboard_state_buffer Buffer where async key state data will be stored
*/
void get_async_keyboard_state(SHORT *keyboard_state_buffer);

#endif // _WIN32


#endif // SP_IO_HPP