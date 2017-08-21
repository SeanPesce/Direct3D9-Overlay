// Author: Sean Pesce

#pragma once


#ifndef _SP_D3D9O_CONSOLE_H_
	#define _SP_D3D9O_CONSOLE_H_

#include "stdafx.h"
#include "SpD3D9.h"

#include "D3DFont.h"
#include "seqan/index.h" // String, StringSet, Index, and Finder classes


#ifndef hotkey_is_down
	#define hotkey_is_down(hotkey) (hotkey != 0 && (key_state[hotkey] & _SP_KEY_DOWN_))
#endif

#define _CLOSE_CONSOLE_KEY_ VK_ESCAPE // Escape key

#define _SP_D3D9O_C_DEFAULT_OUTPUT_LOG_CAPACITY_ 100
#define _SP_D3D9O_C_DEFAULT_COMMAND_LOG_CAPACITY_ 20
#define _SP_D3D9O_C_DEFAULT_ECHO_VALUE_ true
#define _SP_D3D9O_C_DEFAULT_OUTPUT_STREAM_VALUE_ true
#define _SP_D3D9O_C_DEFAULT_PROMPT_ ">"
#define _SP_D3D9O_C_DEFAULT_CARET_ '_'
#define _SP_D3D9O_C_DEFAULT_BLINK_DELAY_ 500
#define _SP_D3D9O_C_DEFAULT_FONT_HEIGHT_ _SP_D3D9O_TF_DEFAULT_FONT_HEIGHT_
#define _SP_D3D9O_C_DEFAULT_FONT_FAMILY_ "Courier New"
#define _SP_D3D9O_C_DEFAULT_FONT_FLAGS_ 0	// D3DFONT_BOLD, etc
#define _SP_D3D9O_C_DEFAULT_FONT_COLOR_ D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);	// White
#define _SP_D3D9O_C_DEFAULT_BACKGROUND_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f); // Black
#define _SP_D3D9O_C_DEFAULT_BORDER_COLOR_ D3DXCOLOR(0.5f, 0.5f, 0.5f, 0.5f); // Gray
#define _SP_D3D9O_C_DEFAULT_BORDER_WIDTH_ 3
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f); // Black
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_COLOR_ D3DXCOLOR(0.5f, 0.5f, 0.5f, 0.5f); // Gray
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_WIDTH_ 1
#define _SP_D3D9O_C_DEFAULT_OUTPUT_LINES_ 15
#define _SP_D3D9O_C_DEFAULT_AUTOCOMPLETE_LIMIT_ 5
#define _SP_D3D9O_C_DEFAULT_PROMPT_ELEMENTS_ (SP_D3D9O_PROMPT_ELEMENTS_DISABLED)

#define _SP_D3D9O_C_INVALID_CONSOLE_COMMAND_CHARS_ " \t\n\r"

// Denotes whether to display each element of the input prompt
enum SP_D3D9O_CONSOLE_PROMPT_ENUM {
	SP_D3D9O_PROMPT_ELEMENTS_DISABLED = 0,
	SP_D3D9O_PROMPT_USER = 1, // User profile
	SP_D3D9O_PROMPT_HOSTNAME = 2, // Computer name
	SP_D3D9O_PROMPT_CWD = 4 // Current working directory	// @TODO
};

typedef struct SP_D3D9O_CONSOLE_COMMAND {
	std::string command = "";
	void(*function)(std::vector<std::string>, std::string *) = NULL;
	std::string help_message = "";
	int id = -1; // ID of command in commands_set
	std::string alias_for = ""; // If not an empty string, the command is an alias or macro
	std::vector<std::string> macro_args; // If alias_for is non-empty and macro_args contains args, command is a macro. If alias_for is empty, macro_args is ignored
} SP_D3D9O_CONSOLE_COMMAND;


class SpD3D9OConsole
{
public:
	SpD3D9Overlay *overlay = NULL; // D3D9 overlay that this console belongs to
	
	bool echo = _SP_D3D9O_C_DEFAULT_ECHO_VALUE_;
	bool output_stream = _SP_D3D9O_C_DEFAULT_OUTPUT_STREAM_VALUE_; // If disabled, printing to console does nothing
	std::string prompt = _SP_D3D9O_C_DEFAULT_PROMPT_;
	int prompt_elements = _SP_D3D9O_C_DEFAULT_PROMPT_ELEMENTS_;
	char caret = _SP_D3D9O_C_DEFAULT_CARET_;
	int caret_blink_delay = _SP_D3D9O_C_DEFAULT_BLINK_DELAY_;  // Speed at which the cursor blinks, in milliseconds
	std::string command = ""; // Current command being typed

	CD3DFont *font = NULL;
	int font_height = _SP_D3D9O_C_DEFAULT_FONT_HEIGHT_;
	D3DXCOLOR font_color = _SP_D3D9O_C_DEFAULT_FONT_COLOR_;
	std::string font_family = _SP_D3D9O_C_DEFAULT_FONT_FAMILY_;
	D3DXCOLOR background_color = _SP_D3D9O_C_DEFAULT_BACKGROUND_COLOR_;
	D3DXCOLOR border_color = _SP_D3D9O_C_DEFAULT_BORDER_COLOR_;
	unsigned int border_width = _SP_D3D9O_C_DEFAULT_BORDER_WIDTH_;
	D3DXCOLOR autocomplete_background_color = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_COLOR_;
	D3DXCOLOR autocomplete_border_color = _SP_D3D9O_C_DEFAULT_BORDER_COLOR_;
	unsigned int autocomplete_border_width = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_WIDTH_;

	std::vector<std::string> command_log; // Log of console commands previously entered
	std::vector<std::string> output_log; // Log of console commands previously entered and their resulting outputs
	unsigned int output_log_displayed_lines = _SP_D3D9O_C_DEFAULT_OUTPUT_LINES_; // Number of lines of previous output to display
	unsigned int output_log_capacity = _SP_D3D9O_C_DEFAULT_OUTPUT_LOG_CAPACITY_; // Number of lines of output to keep in memory (oldest are deleted when max is hit)
	unsigned int command_log_capacity = _SP_D3D9O_C_DEFAULT_COMMAND_LOG_CAPACITY_; // Number of console commands to keep logged (oldest are deleted when max is hit)

	unsigned int caret_position = 0; // Position of cursor in current command
	unsigned int input_display_start = 0;
	unsigned int input_display_end = 0;
	unsigned int command_log_position = 0; // Used to obtain previous commands with the up/down keys
	unsigned int autocomplete_limit = _SP_D3D9O_C_DEFAULT_AUTOCOMPLETE_LIMIT_; // Maximum number of autocomplete suggestions to show

	// Constructor/destructor
	SpD3D9OConsole(SpD3D9Overlay *new_overlay);
	~SpD3D9OConsole();

	void SpD3D9OConsole::get_input(); // Called from main overlay input loop
	bool SpD3D9OConsole::toggle();
	bool SpD3D9OConsole::is_open();
	void SpD3D9OConsole::draw();
	void SpD3D9OConsole::add_prompt_elements(std::string *full_prompt); // Adds extra prompt elements, if enabled (username, hostname, working directory, etc)
	void SpD3D9OConsole::print(const char *new_message); // Prints text to output log
	void SpD3D9OConsole::execute_command(const char *new_command, std::string *output = NULL);
	void SpD3D9OConsole::clear(); // Clears console by pushing blank messages to output
	DWORD SpD3D9OConsole::copy(); // Copies current un-submitted console input to the clipboard
	DWORD SpD3D9OConsole::paste(); // Paste clipboard data into console input
	#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		void SpD3D9OConsole::handle_key_event(DIDEVICEOBJECTDATA *event);
	#else // !_SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		void SpD3D9OConsole::handle_key_press(WPARAM wParam);
		void SpD3D9OConsole::handle_text_input(WPARAM wParam);
	#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
	static int register_command(const char *command, void(*function)(std::vector<std::string>, std::string *), const char *help_message, const char *alias_for = "", std::vector<std::string> macro_args = {});
	static int register_alias(const char *new_alias, const char *existing_command);
	static void get_autocomplete_options(const char *str, unsigned int suggestion_count, std::vector<std::string> *matches);

	static std::vector<SP_D3D9O_CONSOLE_COMMAND> commands;		// Set of available console commands and corresponding functions
	static seqan::StringSet<seqan::String<char>> commands_set;	// Set of available console command strings
	static seqan::Index<seqan::StringSet<seqan::String<char>>> *commands_index;
	static seqan::Finder<seqan::Index<seqan::StringSet<seqan::String<char>>>> commands_finder;
	static int SpD3D9OConsole::get_console_command_index(const char *command); // Obtains the position (index) of a command, (Note: not the ID)

private:
	bool show_caret = false;
	DWORD next_caret_blink = 0; // Time of next caret toggle

	void SpD3D9OConsole::update_font(); // Update text to new font family/size/flags/etc
	void SpD3D9OConsole::set_input_string_display_limits(unsigned int max_chars);
};


int open_console(); // Opens console

void to_lower(char *string); // Converts a C string to lowercase
void trim(std::string *string, const char *new_mask = " \r\n\t"); // Trims whitespace from ends of string
char parse_args(const char *args_c_str, std::vector<std::string> *args, std::string *output_file); // Breaks an argument string into argument tokens
bool resolve_arg(const char *args_c_str, int *index, std::string *arg);

#endif // _SP_D3D9O_CONSOLE_H_